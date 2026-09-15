/*
  Q Light Controller Plus
  scriptrunner.cpp

  Copyright (C) Massimo Callegari

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0.txt

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
*/

#include <QQmlEngine>
#include <QJSEngine>
#include <QJSValue>
#include <QRandomGenerator>
#include <utility>
#if !defined(Q_OS_IOS)
#include <QProcess>
#endif
#include <QMutexLocker>
#include <QElapsedTimer>
#include <QVariant>
#include <QDebug>

#include "scriptrunner.h"
#include "genericfader.h"
#include "fadechannel.h"
#include "mastertimer.h"
#include "inputoutputmap.h"
#include "universe.h"
#include "qlcpalette.h"
#include "qlcfixturehead.h"

// How often (in milliseconds) the various wait*() polling loops below are
// allowed to ask the JS engine to run a garbage collection pass.
static const int GC_INTERVAL_MS = 5000;

QVariantMap ScriptRunner::s_storedValues;
QMutex ScriptRunner::s_storedValuesMutex;

ScriptRunner::ScriptRunner(Doc *doc, const QString &content, QObject *parent)
    : QThread(parent)
    , m_gcRunning(false)
    , m_doc(doc)
    , m_content(content)
    , m_running(false)
    , m_engine(NULL)
    , m_stopOnExit(true)
    , m_waitCount(0)
    , m_waitFunctionId(Function::invalidId())
    , m_waitingForBeat(false)
{
}

ScriptRunner::~ScriptRunner()
{
    stop();
}

void ScriptRunner::execute()
{
    if (m_running)
        return;

    m_running = true;

    start();
}

void ScriptRunner::stop()
{
    if (m_running == false)
        return;

    // Flip the flag first, before anything else.
    // Every blocking loop in this class polls m_running to know when to give up.
    m_running = false;

    // waitTime() can be sleeping on m_waitCondition rather than polling, so
    // it needs an explicit wake-up here - otherwise a script sitting in a
    // long waitTime() call wouldn't notice m_running has gone false until
    // its next periodic wake-up at the latest.
    m_waitCondition.wakeAll();

    if (m_engine)
        m_engine->setInterrupted(true);

    // Block until the worker thread (run()) has actually returned before
    // touching anything it owns. This was missing entirely before - stop()
    // fired setInterrupted() and immediately deleted the engine via
    // deleteLater() while the script might still legitimately be executing
    // native code for a little while longer.
    wait();

    // Whatever run() didn't already clean up on its own, clean up now.
    // No-op if run() got there first (the common case).
    finishAndCleanUp();
}

void ScriptRunner::finishAndCleanUp()
{
    QMutexLocker locker(&m_mutex);

    // Already cleaned up - without this guard, calling this twice would
    // double-delete m_engine.
    if (m_engine == NULL && m_startedFunctions.isEmpty() && m_fadersMap.isEmpty())
        return;

    m_running = false;

    // Delete directly to avoid leaking memory; deleteLater() fails here
    // because the worker thread does not run a Qt event loop.
    delete m_engine;
    m_engine = NULL;

    // Stop all functions started by this script that are not flagged to not stop on exit.
    foreach (quint32 fID, m_startedFunctions)
    {
        Function *function = m_doc->function(fID);
        if (function != NULL)
            function->stop(FunctionParent::master());
    }
    m_startedFunctions.clear();

    // request to delete all the active faders
    QMapIterator<quint32, QSharedPointer<GenericFader>> it(m_fadersMap);
    while (it.hasNext())
    {
        it.next();
        it.value()->requestDelete();
    }
    m_fadersMap.clear();

    m_functionQueue.clear();
    m_fixtureValueQueue.clear();
    disconnect(m_doc->masterTimer(), SIGNAL(functionStarted(quint32)), this, SLOT(slotWaitFunctionStarted(quint32)));
    disconnect(m_doc->masterTimer(), SIGNAL(functionStopped(quint32)), this, SLOT(slotWaitFunctionStopped(quint32)));
    disconnect(m_doc->inputOutputMap(), SIGNAL(beat()), this, SLOT(slotBeatOccurred()));
    m_waitFunctionId = Function::invalidId();
    m_waitingForBeat = false;
}

QStringList ScriptRunner::collectScriptData()
{
    QStringList syntaxErrorList;
    QJSEngine *engine = new QJSEngine();
    QJSValue objectValue = engine->newQObject(this);
    engine->globalObject().setProperty("Engine", objectValue);
    QQmlEngine::setObjectOwnership(this, QQmlEngine::CppOwnership);

    QJSValue script = engine->evaluate("(function run() { " + m_content + " })");
    if (script.isError())
    {
        QString msg = QString("Uncaught exception at line %2. %3")
                        .arg(script.property("lineNumber").toInt())
                        .arg(script.toString());
        qWarning() << msg;
        //qDebug() << "Stack: " << script.property("stack").toString();
        syntaxErrorList << msg;
    }
    else
    {
        qDebug() << "All good.";
    }

    if (script.isCallable() == false)
    {
        qDebug() << "ERROR. No function method found.";
    }
    else
    {
        QJSValue ret = script.call(QJSValueList());
        if (ret.isError())
        {
            QString msg = QString("Uncaught exception at line %2. %3")
                            .arg(ret.property("lineNumber").toInt())
                            .arg(ret.toString());
            qWarning() << msg;
            syntaxErrorList << msg;
        }
    }

    delete engine;

    return syntaxErrorList;
}

int ScriptRunner::currentWaitTime() const
{
    QMutexLocker locker(&m_mutex);
    return m_waitCount * MasterTimer::tick();
}

quint64 ScriptRunner::fixtureValueKey(quint32 universe, quint32 fixtureID, quint32 channel)
{
    // 16 bits for the universe, 24 for the fixture ID, 24 for the channel:
    // comfortably more range than any of these can legitimately take today,
    // and it fits exactly into 64 bits.
    return (quint64(universe & 0xFFFFu) << 48)
         | (quint64(fixtureID & 0xFFFFFFu) << 24)
         |  quint64(channel & 0xFFFFFFu);
}

bool ScriptRunner::write(MasterTimer *timer, QList<Universe *> universes)
{
    {
        QMutexLocker locker(&m_mutex);
        if (m_waitCount > 0)
        {
            m_waitCount--;
            if (m_waitCount == 0)
                // Wake up a script that's sleeping in waitTime() as soon as
                // its wait is actually over, rather than waiting for that
                // loop's own (throttled) periodic wake-up.
                m_waitCondition.wakeAll();
        }
    }

    // Pull the whole fixture-value queue out under the lock, then work
    // on our own local copy - this keeps the critical section short
    // and avoids holding the mutex while we call into GenericFader.
    QHash<quint64, FixtureValue> fixtureValues;
    {
        QMutexLocker locker(&m_mutex);
        fixtureValues.swap(m_fixtureValueQueue);
    }

    for (const FixtureValue &val : std::as_const(fixtureValues))
    {
        // val.m_universe is cached at the time setFixture() was called
        // and then sits in the queue until the next write(). If the
        // number of active universes has since shrunk (or was smaller
        // than expected to begin with), universes[val.m_universe] below
        // is an out-of-bounds QList access - undefined behaviour, and a
        // very likely crash. Validate it before indexing.
        if (val.m_universe >= quint32(universes.count()))
        {
            qWarning() << QString("Skipping fixture value: invalid universe %1").arg(val.m_universe);
            continue;
        }

        QSharedPointer<GenericFader> fader;
        {
            QMutexLocker locker(&m_mutex);
            fader = m_fadersMap.value(val.m_universe, QSharedPointer<GenericFader>());
            if (fader.isNull())
            {
                fader = universes[val.m_universe]->requestFader();
                if (fader.isNull())
                    continue;
                //fader->adjustIntensity(getAttributeValue(Intensity));
                //fader->setBlendMode(blendMode());
                m_fadersMap[val.m_universe] = fader;
            }
        }

        const uchar targetValue = val.m_value;
        const uint fadeTime = val.m_fadeTime;
        fader->updateChannel(m_doc, universes[val.m_universe], val.m_fixtureID, val.m_channel,
                             [targetValue, fadeTime](FadeChannel &fc)
        {
            fc.setStart(fc.current());
            fc.setTarget(targetValue);
            fc.setFadeTime(fadeTime);
            fc.setElapsed(0);
            fc.setReady(false);
        });
    }

    // Process as many queued function start/stop/wait requests as we
    // can without blocking on one that isn't ready yet.
    while (true)
    {
        QPair<quint32, FunctionOperation> pair;
        {
            QMutexLocker locker(&m_mutex);
            // if we have to wait, or there's nothing queued, stop here
            if (m_waitFunctionId != Function::invalidId() || m_functionQueue.isEmpty())
                break;
            pair = m_functionQueue.head();
        }

        quint32 fID = pair.first;
        FunctionOperation operation = pair.second;

        Function *function = m_doc->function(fID);
        if (function == NULL)
        {
            qWarning() << QString("No such function (ID %1)").arg(fID);
            // Must remove the bad entry - the old code's plain "continue;"
            // left it at the head of the queue forever, spinning write()
            // (called once per MasterTimer tick) in an infinite loop and
            // freezing DMX output for every universe.
            QMutexLocker locker(&m_mutex);
            m_functionQueue.removeFirst();
            continue;
        }

        bool waiting = false;

        if (operation == FunctionOperation::START || operation == FunctionOperation::START_DONT_STOP)
        {
            function->start(timer, FunctionParent::master());
            if (operation == FunctionOperation::START)
            {
                QMutexLocker locker(&m_mutex);
                m_startedFunctions.insert(fID);
            }
        }
        else if (operation == FunctionOperation::STOP)
        {
            function->stop(FunctionParent::master());
            QMutexLocker locker(&m_mutex);
            m_startedFunctions.remove(fID);
        }
        else if (operation == FunctionOperation::WAIT_START)
        {
            if (!function->isRunning())
            {
                // the function is not running, so we wait and we stop dequeuing
                QMutexLocker locker(&m_mutex);
                m_waitFunctionId = fID;
                connect(m_doc->masterTimer(), SIGNAL(functionStarted(quint32)), SLOT(slotWaitFunctionStarted(quint32)), Qt::UniqueConnection);
                waiting = true;
            }
        }
        else if (operation == FunctionOperation::WAIT_STOP)
        {
            if (!function->stopped())
            {
                // the function has to start or is still running, so we wait and we stop dequeuing
                QMutexLocker locker(&m_mutex);
                m_waitFunctionId = fID;
                connect(m_doc->masterTimer(), SIGNAL(functionStopped(quint32)), SLOT(slotWaitFunctionStopped(quint32)), Qt::UniqueConnection);
                waiting = true;
            }
        }

        if (waiting)
            break;

        // we can continue with the next function in the queue
        QMutexLocker locker(&m_mutex);
        if (!m_functionQueue.isEmpty())
            m_functionQueue.removeFirst();
    }

    // If the JS call method has ended on its own, the thread has finished,
    // therefore there's nothing else to run here
    if (m_running == false)
        return false;

    return true;
}

void ScriptRunner::slotWaitFunctionStarted(quint32 fid)
{
    QMutexLocker locker(&m_mutex);
    if (m_waitFunctionId == fid)
    {
        disconnect(m_doc->masterTimer(), SIGNAL(functionStarted(quint32)), this, SLOT(slotWaitFunctionStarted(quint32)));
        m_waitFunctionId = Function::invalidId();
    }
}

void ScriptRunner::slotWaitFunctionStopped(quint32 fid)
{
    QMutexLocker locker(&m_mutex);
    if (m_waitFunctionId == fid)
    {
        disconnect(m_doc->masterTimer(), SIGNAL(functionStopped(quint32)), this, SLOT(slotWaitFunctionStopped(quint32)));
        m_startedFunctions.remove(fid);
        m_waitFunctionId = Function::invalidId();
    }
}

void ScriptRunner::slotBeatOccurred()
{
    QMutexLocker locker(&m_mutex);
    if (m_waitingForBeat)
    {
        disconnect(m_doc->inputOutputMap(), SIGNAL(beat()), this, SLOT(slotBeatOccurred()));
        m_waitingForBeat = false;
    }
}

void ScriptRunner::run()
{
    {
        QMutexLocker locker(&m_mutex);
        m_waitCount = 0;
    }

    m_engine = new QJSEngine();
    QJSValue objectValue = m_engine->newQObject(this);
    m_engine->globalObject().setProperty("Engine", objectValue);
    QQmlEngine::setObjectOwnership(this, QQmlEngine::CppOwnership);

    m_gcTimer.start(); // Start global garbage collection timer

    QJSValue script = m_engine->evaluate("(function run() { " + m_content + " })");

    if (script.isCallable() == false)
    {
        qDebug() << "ERROR. No function method found.";
    }
    else
    {
        QJSValue ret = script.call(QJSValueList());
        if (ret.isError())
        {
            QString msg("Uncaught exception at line %2. Error: %3");
            qWarning() << msg.arg(ret.property("lineNumber").toInt())
                             .arg(ret.toString());
        }

        qDebug() << "[ScriptRunner] Code executed";
    }

    // Routing both "stopped by user" and "finished by itself" through
    // finishAndCleanUp ensures single cleanup regardless of how the script ends.
    finishAndCleanUp();
}

void ScriptRunner::maybeCollectGarbage()
{
    if (m_engine == NULL || m_gcRunning)
        return;

    if (m_gcTimer.elapsed() >= GC_INTERVAL_MS)
    {
        m_gcRunning = true;
        m_engine->collectGarbage();
        m_gcTimer.restart();
        m_gcRunning = false;
    }
}

Fixture* ScriptRunner::validateFixtureChannel(quint32 fxID, quint32 channel)
{
    Fixture *fxi = m_doc->fixture(fxID);
    if (fxi == NULL)
    {
        qWarning() << QString("No such fixture (ID: %1)").arg(fxID);
        return NULL;
    }

    if (channel >= fxi->channels())
    {
        qWarning() << QString("Fixture (%1) has no channel number %2").arg(fxi->name()).arg(channel);
        return NULL;
    }

    return fxi;
}

/************************************************************************
 * JS exported methods
 ************************************************************************/

int ScriptRunner::getChannelValue(int universe, int channel)
{
    if (m_running == false)
        return false;

    QList<Universe*> uniList = m_doc->inputOutputMap()->claimUniverses();
    uchar dmxValue = 0;

    if (universe >= 0 && universe < uniList.count())
    {
        Universe *uni = uniList.at(universe);
        dmxValue = uni->preGMValue(channel);
    }
    m_doc->inputOutputMap()->releaseUniverses(false);

    return dmxValue;
}

bool ScriptRunner::setFixture(quint32 fxID, quint32 channel, uchar value, uint time)
{
    if (m_running == false)
        return false;

    qDebug() << Q_FUNC_INFO;

    Fixture *fxi = validateFixtureChannel(fxID, channel);
    if (fxi == NULL)
        return false;

    int address = fxi->address() + channel;
    if (address >= 512)
    {
        qWarning() << QString("Invalid address: %1").arg(address);
        return false;
    }

    // enqueue this fixture value to be processed at the next write call
    FixtureValue val;
    val.m_universe = fxi->universe();
    val.m_fixtureID = fxID;
    val.m_channel = channel;
    val.m_value = value;
    val.m_fadeTime = time;

    QMutexLocker locker(&m_mutex);
    // Overwrite the value in place to prevent infinite queue growth.
    m_fixtureValueQueue.insert(fixtureValueKey(val.m_universe, fxID, channel), val);

    return true;
}

int ScriptRunner::getFixtureChannelValue(quint32 fxID, quint32 channel)
{
    if (m_running == false)
        return 0;

    Fixture *fxi = validateFixtureChannel(fxID, channel);
    if (fxi == NULL)
        return 0;

    int universe = int(fxi->universe());
    int address = fxi->address() + int(channel);

    return getChannelValue(universe, address);
}

bool ScriptRunner::stopOnExit(bool value)
{
    m_stopOnExit = value;

    return true;
}

Function* ScriptRunner::getFunctionIfRunning(quint32 fID) const
{
    if (m_running == false)
        return NULL;

    Function *function = m_doc->function(fID);
    if (function == NULL)
    {
        qWarning() << QString("No such function (ID %1)").arg(fID);
        return NULL;
    }

    return function;
}

bool ScriptRunner::enqueueFunction(quint32 fID, FunctionOperation operation)
{
    Function *function = getFunctionIfRunning(fID);
    if (function == NULL)
        return false;

    QPair<quint32, FunctionOperation> pair;
    pair.first = fID;
    pair.second = operation;

    QMutexLocker locker(&m_mutex);
    m_functionQueue.enqueue(pair);

    return true;
}

bool ScriptRunner::waitForCondition(std::function<bool()> isPending)
{
    while (m_running)
    {
        bool pending;
        {
            QMutexLocker locker(&m_mutex);
            pending = isPending();
        }

        if (!pending)
            break;

        maybeCollectGarbage(); // Use global GC checker

        usleep(10000);
    }

    return m_running;
}

bool ScriptRunner::waitForFunctionOperation(quint32 fID, FunctionOperation operation)
{
    QPair<quint32, FunctionOperation> pair(fID, operation);

    // Block thread in polling loop until write() fully resolves this request.
    return waitForCondition([this, pair, fID]() {
        return m_functionQueue.contains(pair) || m_waitFunctionId == fID;
    });
}

bool ScriptRunner::startFunction(quint32 fID)
{
    return enqueueFunction(fID, m_stopOnExit ? FunctionOperation::START : FunctionOperation::START_DONT_STOP);
}

bool ScriptRunner::stopFunction(quint32 fID)
{
    return enqueueFunction(fID, FunctionOperation::STOP);
}

bool ScriptRunner::isFunctionRunning(quint32 fID)
{
    Function *function = getFunctionIfRunning(fID);
    return function == NULL ? false : function->isRunning();
}

float ScriptRunner::getFunctionAttribute(quint32 fID, int attributeIndex) const
{
    Function *function = getFunctionIfRunning(fID);
    return function == NULL ? 0 : function->getAttributeValue(attributeIndex);
}

bool ScriptRunner::setFunctionAttribute(quint32 fID, int attributeIndex, float value)
{
    Function *function = getFunctionIfRunning(fID);
    if (function == NULL)
        return false;

    function->adjustAttribute(value, attributeIndex);

    return true;
}

bool ScriptRunner::setFunctionAttribute(quint32 fID, QString attributeName, float value)
{
    Function *function = getFunctionIfRunning(fID);
    if (function == NULL)
        return false;

    int attrIndex = function->getAttributeIndex(attributeName);
    function->adjustAttribute(value, attrIndex);

    return true;
}

bool ScriptRunner::systemCommand(QString command)
{
    if (m_running == false)
        return false;

    qDebug() << Q_FUNC_INFO;

    // tokenize the command by splitting the base
    // program name and the arguments
    QStringList tokens = command.split(" ");
    if (tokens.count() == 0)
        return false;

    QString programName = tokens.first();
    QString multiPartArg;
    QStringList programArgs;
    for (int i = 1; i < tokens.size(); i++)
    {
        QString token = tokens.at(i);
        if (token.startsWith("'") && token.endsWith("'"))
        {
            programArgs << token.mid(1, token.length() - 2);
        }
        else if (token.startsWith("'"))
        {
            multiPartArg.clear();
            multiPartArg.append(token.mid(1));
        }
        else
        {
            if (multiPartArg.isEmpty())
                programArgs << token;
            else
            {
                multiPartArg.append(" ");
                if (token.endsWith("'"))
                {
                    multiPartArg.append(token.mid(0, token.length() - 1));
                    programArgs << multiPartArg;
                    multiPartArg.clear();
                }
                else
                {
                    multiPartArg.append(token);
                }
            }
        }
    }

#if !defined(Q_OS_IOS)
    qint64 pid;
    // Uses static overload to prevent leaking QProcess instances
    QProcess::startDetached(programName, programArgs, QString(), &pid);
#endif

    return true;
}

bool ScriptRunner::waitTime(uint ms)
{
    if (m_running == false)
        return false;

    qDebug() << Q_FUNC_INFO;

    QMutexLocker locker(&m_mutex);
    m_waitCount += ms / MasterTimer::tick();

    while (m_running && m_waitCount > 0)
    {
        m_waitCondition.wait(&m_mutex, 10);
        maybeCollectGarbage();
    }

    return m_running;
}

bool ScriptRunner::waitTime(QString time)
{
    return waitTime(Function::stringToSpeed(time));
}

bool ScriptRunner::waitTick(uint ticks)
{
    if (m_running == false || ticks == 0)
        return false;

    QMutexLocker locker(&m_mutex);
    m_waitCount += ticks;

    while (m_running && m_waitCount > 0)
    {
        m_waitCondition.wait(&m_mutex, 10);
        maybeCollectGarbage();
    }

    return m_running;
}

bool ScriptRunner::waitBeat(uint beats)
{
    if (m_running == false || beats == 0)
        return false;

    for (uint i = 0; i < beats; ++i)
    {
        {
            QMutexLocker locker(&m_mutex);
            m_waitingForBeat = true;
            connect(m_doc->inputOutputMap(), SIGNAL(beat()), this, SLOT(slotBeatOccurred()), Qt::UniqueConnection);
        }

        if (!waitForCondition([this]() { return m_waitingForBeat; }))
            return false;
    }

    return m_running;
}

bool ScriptRunner::waitForFunctionOperation(quint32 fID, FunctionOperation operation)
{
    QPair<quint32, FunctionOperation> pair(fID, operation);

    return waitForCondition([this, pair, fID]() {
        return m_functionQueue.contains(pair) || m_waitFunctionId == fID;
    });
}

bool ScriptRunner::waitFunctionStart(quint32 fID)
{
    if (!enqueueFunction(fID, FunctionOperation::WAIT_START))
        return false;

    return waitForFunctionOperation(fID, FunctionOperation::WAIT_START);
}

bool ScriptRunner::waitFunctionStop(quint32 fID)
{
    if (!enqueueFunction(fID, FunctionOperation::WAIT_STOP))
        return false;

    return waitForFunctionOperation(fID, FunctionOperation::WAIT_STOP);
}

bool ScriptRunner::setBlackout(bool enable)
{
    if (m_running == false)
        return false;

    qDebug() << Q_FUNC_INFO;

    m_doc->inputOutputMap()->requestBlackout(enable ? InputOutputMap::BlackoutRequestOn :
                                                      InputOutputMap::BlackoutRequestOff);

    return true;
}

bool ScriptRunner::setBPM(int bpm)
{
    if (m_running == false)
        return false;

    qDebug() << Q_FUNC_INFO;

    m_doc->inputOutputMap()->setBpmNumber(bpm);

    return true;
}

int ScriptRunner::getBPM()
{
    if (m_running == false)
        return 0;

    return m_doc->inputOutputMap()->bpmNumber();
}

void ScriptRunner::debugLog(QString message)
{
    qDebug() << "[Script]" << message;
}

bool ScriptRunner::storeValue(QString key, QJSValue value)
{
    if (m_running == false)
        return false;

    // Convert to a QVariant so the value can outlive both this script's
    // QJSEngine (which is destroyed when the script exits) and be picked
    // up cleanly by a completely different QJSEngine belonging to a later
    // run of a "one-shot" script. A QJSValue is only valid for the
    // lifetime of the specific QJSEngine that created it, so keeping the
    // QJSValue itself around here wouldn't be safe. toVariant() handles
    // numbers, strings, booleans, arrays and plain objects.
    QMutexLocker locker(&s_storedValuesMutex);
    s_storedValues[key] = value.toVariant();

    return true;
}

QStringList ScriptRunner::listValues()
{
    if (m_running == false)
        return QStringList();

    QMutexLocker locker(&s_storedValuesMutex);
    return s_storedValues.keys();
}

QJSValue ScriptRunner::getValue(QString key)
{
    if (m_running == false || m_engine == NULL)
        return QJSValue(QJSValue::UndefinedValue);

    QVariant stored;
    {
        QMutexLocker locker(&s_storedValuesMutex);
        if (!s_storedValues.contains(key))
            return QJSValue(QJSValue::UndefinedValue);
        stored = s_storedValues.value(key);
    }

    // Re-wrap as a QJSValue belonging to THIS script's engine - a QJSValue
    // can't be shared across QJSEngine instances.
    return m_engine->toScriptValue(stored);
}

bool ScriptRunner::clearValue(QString key)
{
    if (m_running == false)
        return false;

    QMutexLocker locker(&s_storedValuesMutex);
    return s_storedValues.remove(key) > 0;
}

bool ScriptRunner::clearAllValues()
{
    if (m_running == false)
        return false;

    QMutexLocker locker(&s_storedValuesMutex);
    s_storedValues.clear();

    return true;
}

int ScriptRunner::random(QString minTime, QString maxTime)
{
    if (m_running == false)
        return 0;

    return random(Function::stringToSpeed(minTime), Function::stringToSpeed(maxTime));
}

int ScriptRunner::random(uint minTime, uint maxTime)
{
    if (m_running == false)
        return 0;

    // Previously: QRandomGenerator::global()->generate() % ((maxTime + 1) - minTime) + minTime;
    // in 32-bit int/uint arithmetic. If maxTime is INT_MAX, "maxTime + 1"
    // overflows a signed int (UB); generate() itself can already exceed
    // INT_MAX, so mixing signed/unsigned produces a garbage modulus for
    // large or inverted (max < min) ranges. Do the arithmetic in 64-bit.
    if (maxTime < minTime)
        std::swap(minTime, maxTime);

    qint64 range = qint64(maxTime) - qint64(minTime) + 1;
    quint64 offset = QRandomGenerator::global()->generate64() % quint64(range);

    return int(qint64(minTime) + qint64(offset));
}

QJSValue ScriptRunner::getPalette(quint32 pID)
{
    if (m_running == false || m_engine == nullptr)
        return QJSValue(QJSValue::UndefinedValue);

    QLCPalette *palette = m_doc->palette(pID);
    if (palette == nullptr)
    {
        qWarning() << QString("No such palette (ID %1)").arg(pID);
        return QJSValue(QJSValue::UndefinedValue);
    }

    QJSValue obj = m_engine->newObject();
    obj.setProperty("id", palette->id());
    obj.setProperty("name", palette->name());
    obj.setProperty("type", QLCPalette::typeToString(palette->type()));

    QJSValue valArray = m_engine->newArray();
    QVariantList vals = palette->values();
    for (int i = 0; i < vals.count(); ++i)
        valArray.setProperty(i, m_engine->toScriptValue(vals.at(i)));

    obj.setProperty("values", valArray);
    return obj;
}

bool ScriptRunner::applyPaletteHead(quint32 pID, quint32 fxID, int head, uint fadeTime)
{
    // Delegate directly to applyPalette by wrapping the single head request into an object
    QJSValue targetObj = m_engine->newObject();
    targetObj.setProperty("fxID", fxID);
    targetObj.setProperty("head", head);

    QJSValue array = m_engine->newArray();
    array.setProperty(0, targetObj);

    return applyPalette(pID, array, fadeTime);
}

bool ScriptRunner::applyPalette(quint32 pID, QJSValue fixtureIDs, uint fadeTime)
{
    if (m_running == false)
        return false;

    QLCPalette *palette = m_doc->palette(pID);
    if (palette == nullptr)
    {
        qWarning() << QString("No such palette (ID %1)").arg(pID);
        return false;
    }

    struct TargetSpec { quint32 fxID; int head; }; // head: -1 = all heads

    QList<TargetSpec> targets;
    QList<quint32> fxList;

    auto parseItem = [&](const QJSValue &item) {
        if (item.isNumber())
        {
            quint32 fid = item.toUInt();
            targets.append({ fid, -1 });
            if (!fxList.contains(fid))
                fxList.append(fid);
        }
        else if (item.isObject() && item.hasProperty("fxID"))
        {
            quint32 fid = item.property("fxID").toUInt();
            int headIdx = item.hasProperty("head") ? item.property("head").toInt() : -1;
            targets.append({ fid, headIdx });
            if (!fxList.contains(fid))
                fxList.append(fid);
        }
    };

    if (fixtureIDs.isArray())
    {
        quint32 length = fixtureIDs.property("length").toUInt();
        for (quint32 i = 0; i < length; ++i)
            parseItem(fixtureIDs.property(i));
    }
    else
    {
        parseItem(fixtureIDs);
    }

    if (fxList.isEmpty())
        return false;

    QList<SceneValue> sceneValues = palette->valuesFromFixtures(m_doc, fxList);
    if (sceneValues.isEmpty())
        return false;

    QMutexLocker locker(&m_mutex);
    for (const SceneValue &sv : sceneValues)
    {
        Fixture *fxi = validateFixtureChannel(sv.fxi, sv.channel);
        if (fxi == nullptr)
            continue;

        int chHead = -1;
        for (int h = 0; h < fxi->heads(); ++h)
        {
            if (fxi->head(h).channels().contains(sv.channel))
            {
                chHead = h;
                break;
            }
        }

        bool matched = false;
        for (const TargetSpec &target : targets)
        {
            if (target.fxID == sv.fxi)
            {
                if (target.head == -1 || chHead == target.head || chHead == -1)
                {
                    matched = true;
                    break;
                }
            }
        }

        if (!matched)
            continue;

        FixtureValue val;
        val.m_universe = fxi->universe();
        val.m_fixtureID = sv.fxi;
        val.m_channel = sv.channel;
        val.m_value = sv.value;
        val.m_fadeTime = fadeTime;

        m_fixtureValueQueue.insert(fixtureValueKey(val.m_universe, val.m_fixtureID, val.m_channel), val);
    }

    return true;
}

quint32 ScriptRunner::createPalette(QString name, QString typeStr, QJSValue values)
{
    if (m_running == false)
        return QLCPalette::invalidId(); //0xFFFFFFFFu

    QLCPalette::PaletteType pType = QLCPalette::stringToType(typeStr);
    if (pType == QLCPalette::Undefined)
    {
        qWarning() << QString("Invalid palette type: %1").arg(typeStr);
        return QLCPalette::invalidId();
    }

    QVariantList valList;
    if (values.isArray())
    {
        quint32 length = values.property("length").toUInt();
        for (quint32 i = 0; i < length; ++i)
            valList.append(values.property(i).toVariant());
    }
    else if (!values.isUndefined() && !values.isNull())
    {
        valList.append(values.toVariant());
    }

    if (valList.isEmpty())
    {
        qWarning() << "Cannot create a palette without values";
        return QLCPalette::invalidId();
    }

    // Construct parentless on the ScriptRunner worker thread, then hand
    // ownership to Doc's thread before registering it, since Doc expects
    // to own/signal on its own (main UI) thread.
    QLCPalette *palette = new QLCPalette(pType, nullptr);
    palette->setName(name);
    palette->setValues(valList);
    palette->moveToThread(m_doc->thread());

    bool success = false;
    QMetaObject::invokeMethod(m_doc, [this, palette, &success]() {
        success = m_doc->addPalette(palette, QLCPalette::invalidId());
    }, Qt::BlockingQueuedConnection);

    if (!success)
    {
        QMetaObject::invokeMethod(palette, &QObject::deleteLater, Qt::QueuedConnection);
        return QLCPalette::invalidId();
    }

    return palette->id();
}

bool ScriptRunner::updatePalette(quint32 pID, QString name, QString typeStr, QJSValue values)
{
    Q_UNUSED(typeStr);

    if (m_running == false)
        return false;

    QLCPalette *palette = m_doc->palette(pID);
    if (palette == nullptr)
    {
        qWarning() << QString("Cannot update: No such palette (ID %1)").arg(pID);
        return false;
    }

    QVariantList valList;
    bool hasValues = false;
    if (!values.isUndefined() && !values.isNull())
    {
        if (values.isArray())
        {
            quint32 length = values.property("length").toUInt();
            for (quint32 i = 0; i < length; ++i)
                valList.append(values.property(i).toVariant());
        }
        else
        {
            valList.append(values.toVariant());
        }
        if (!valList.isEmpty())
            hasValues = true;
    }

    // Run on Doc's thread so QObject signals fired by setName/setValues emit safely.
    QMetaObject::invokeMethod(m_doc, [palette, name, valList, hasValues]() {
        if (!name.isEmpty())
            palette->setName(name);
        if (hasValues)
            palette->setValues(valList);
    }, Qt::BlockingQueuedConnection);

    return true;
}
