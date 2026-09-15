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

#include <QJSEngine>
#include <QJSValue>
#include <QRandomGenerator>
#include <utility>
#if !defined(Q_OS_IOS)
#include <QProcess>
#endif
#include <QMutexLocker>
#include <QElapsedTimer>
#include <QDebug>

#include "scriptrunner.h"
#include "genericfader.h"
#include "fadechannel.h"
#include "mastertimer.h"
#include "inputoutputmap.h"
#include "universe.h"

// How often (in milliseconds) the various wait*() polling loops below are
// allowed to ask the JS engine to run a garbage collection pass.
static const int GC_INTERVAL_MS = 5000;

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

    foreach (quint32 fID, m_startedFunctions)
    {
        Function *function = m_doc->function(fID);
        if (function != NULL)
            function->stop(FunctionParent::master());
    }
    m_startedFunctions.clear();

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
                m_waitCondition.wakeAll();
        }
    }

    QHash<quint64, FixtureValue> fixtureValues;
    {
        QMutexLocker locker(&m_mutex);
        fixtureValues.swap(m_fixtureValueQueue);
    }

    for (const FixtureValue &val : std::as_const(fixtureValues))
    {
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
    // if we don't have to wait and there are some functions in the queue
        while (true)
    {
        QPair<quint32, FunctionOperation> pair;
        {
            QMutexLocker locker(&m_mutex);
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
                QMutexLocker locker(&m_mutex);
                m_waitFunctionId = fID;
                connect(m_doc->masterTimer(), SIGNAL(functionStopped(quint32)), SLOT(slotWaitFunctionStopped(quint32)), Qt::UniqueConnection);
                waiting = true;
            }
        }

        if (waiting)
            break;

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

        maybeCollectGarbage();

        usleep(10000);
    }

    return m_running;
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
}bool ScriptRunner::waitTick(uint ticks)
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

void ScriptRunner::slotBeatOccurred()
{
    QMutexLocker locker(&m_mutex);
    if (m_waitingForBeat)
    {
        disconnect(m_doc->inputOutputMap(), SIGNAL(beat()), this, SLOT(slotBeatOccurred()));
        m_waitingForBeat = false;
    }
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
