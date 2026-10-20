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
#include <QDebug>

#include "scriptrunner.h"
#include "genericfader.h"
#include "fadechannel.h"
#include "mastertimer.h"
#include "universe.h"

// How often (in milliseconds) waitTime() may run a JS garbage collection pass
static const int GC_INTERVAL_MS = 5000;

ScriptRunner::ScriptRunner(Doc *doc, const QString &content, QObject *parent)
    : QThread(parent)
    , m_doc(doc)
    , m_content(content)
    , m_running(false)
    , m_engine(NULL)
    , m_stopOnExit(true)
    , m_waitCount(0)
    , m_waitFunctionId(Function::invalidId())
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
    // Flip the flag first: every blocking call in this class checks it to know
    // when to give up. The exchange also makes sure only one caller tears
    // things down.
    if (m_running.exchange(false) == false)
        return;

    // wake up a script that is sleeping in waitTime()
    m_waitCondition.wakeAll();

    {
        QMutexLocker locker(&m_mutex);
        if (m_engine)
            m_engine->setInterrupted(true);
    }

    // Wait for run() to return before tearing anything down. The script may
    // still be executing native code for a little while after the interrupt.
    wait();

    // Take ownership of what has to be released under the lock, then release
    // it without holding the lock
    QList<quint32> startedFunctions;
    QMap<quint32, QSharedPointer<GenericFader> > faders;
    {
        QMutexLocker locker(&m_mutex);

        // Delete the engine directly: the worker thread does not run an event
        // loop, so deleteLater() would never be processed and leak it
        delete m_engine;
        m_engine = NULL;

        // Nothing queued by the stopped script must be dispatched anymore
        m_functionQueue.clear();
        m_fixtureValueQueue.clear();

        startedFunctions.swap(m_startedFunctions);
        faders.swap(m_fadersMap);
    }

    // Stop all functions started by this script
    foreach (quint32 fID, startedFunctions)
    {
        Function *function = m_doc->function(fID);
        if (function == NULL)
            continue;

        function->stop(FunctionParent::master());
    }

    // request to delete all the active faders
    foreach (QSharedPointer<GenericFader> fader, faders)
    {
        if (!fader.isNull())
            fader->requestDelete();
    }
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
    // Take the queued fixture values out under the lock and work on a private
    // copy, so that the lock is not held while calling into the faders
    QHash<quint64, FixtureValue> fixtureValues;
    {
        QMutexLocker locker(&m_mutex);
        if (m_waitCount > 0 && --m_waitCount == 0)
            // wake up a script sleeping in waitTime() as soon as its wait is over
            m_waitCondition.wakeAll();

        fixtureValues.swap(m_fixtureValueQueue);
    }

    for (const FixtureValue &val : std::as_const(fixtureValues))
    {

        // The universe index was computed when setFixture() was called and
        // has been sitting in the queue since then. Make sure it is still
        // valid before indexing into the list.
        if (val.m_universe >= quint32(universes.count()))
        {
            qWarning() << QString("Skipping fixture value: invalid universe %1").arg(val.m_universe);
            continue;
        }

        QSharedPointer<GenericFader> fader;
        {
            QMutexLocker locker(&m_mutex);
            fader = m_fadersMap.value(val.m_universe, QSharedPointer<GenericFader>());
        }

        if (fader.isNull())
        {
            fader = universes[val.m_universe]->requestFader();
            if (fader.isNull())
                continue;
            //fader->adjustIntensity(getAttributeValue(Intensity));
            //fader->setBlendMode(blendMode());

            QMutexLocker locker(&m_mutex);
            m_fadersMap[val.m_universe] = fader;
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

    // Dispatch the queued start/stop/wait requests, unless the script is
    // waiting for a Function
    while (true)
    {
        QPair<quint32, FunctionOperation> pair;
        {
            QMutexLocker locker(&m_mutex);
            if (m_waitFunctionId != Function::invalidId() || m_functionQueue.isEmpty())
                break;
            pair = m_functionQueue.head();
        }

        const quint32 fID = pair.first;
        const FunctionOperation operation = pair.second;
        bool waiting = false;

        Function *function = m_doc->function(fID);
        if (function == NULL)
        {
            qWarning() << QString("No such function (ID %1)").arg(fID);
            // Drop the bad entry. A plain "continue" leaves it at the head of
            // the queue, so this loop would spin forever on the MasterTimer
            // thread and freeze DMX output for every universe.
        }
        else if (operation == FunctionOperation::START || operation == FunctionOperation::START_DONT_STOP)
        {
            function->start(timer, FunctionParent::master());
            if (operation == FunctionOperation::START)
            {
                QMutexLocker locker(&m_mutex);
                m_startedFunctions << fID;
            }
        }
        else if (operation == FunctionOperation::STOP)
        {
            function->stop(FunctionParent::master());
            QMutexLocker locker(&m_mutex);
            m_startedFunctions.removeAll(fID);
        }
        else if (operation == FunctionOperation::WAIT_START)
        {
            if (!function->isRunning())
            {
                // the function is not running, so we we wait and we stop dequeuing
                QMutexLocker locker(&m_mutex);
                m_waitFunctionId = fID;
                connect(m_doc->masterTimer(), SIGNAL(functionStarted(quint32)), SLOT(slotWaitFunctionStarted(quint32)));
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
                connect(m_doc->masterTimer(), SIGNAL(functionStopped(quint32)), SLOT(slotWaitFunctionStopped(quint32)));
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

    // If the JS call method has ended on its own, the thread has finished.
    // Keep running until every queued operation has been dispatched, otherwise
    // commands issued right before the script fell off the end would be lost
    QMutexLocker locker(&m_mutex);
    if (m_running == false && m_functionQueue.isEmpty() && m_fixtureValueQueue.isEmpty() &&
        m_waitFunctionId == Function::invalidId())
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
        m_startedFunctions.removeAll(fid);
        m_waitFunctionId = Function::invalidId();
    }
}

void ScriptRunner::run()
{
    {
        QMutexLocker locker(&m_mutex);
        m_waitCount = 0;
    }

    QJSEngine *engine = new QJSEngine();
    {
        QMutexLocker locker(&m_mutex);
        m_engine = engine;

        // stop() may have been called before the engine existed, in which
        // case it had nothing to interrupt
        if (m_running == false)
            engine->setInterrupted(true);
    }

    m_gcTimer.start();

    QJSValue objectValue = engine->newQObject(this);
    engine->globalObject().setProperty("Engine", objectValue);
    QQmlEngine::setObjectOwnership(this, QQmlEngine::CppOwnership);

    QJSValue script = engine->evaluate("(function run() { " + m_content + " })");

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

    // Only this thread uses the engine, so free it as soon as the script is
    // done. stop() does nothing once the script has ended on its own, so it
    // would otherwise never be deleted.
    {
        QMutexLocker locker(&m_mutex);
        delete m_engine;
        m_engine = NULL;
    }

    // Signal write() that the JS code is done. Note that this must not stop
    // the Script right away: pending queued operations are dispatched by
    // write() on the MasterTimer thread and have to be flushed first
    m_running = false;
}

void ScriptRunner::maybeCollectGarbage()
{
    // m_engine is only modified by this thread while the script runs (and by
    // stop() once the thread has finished), so reading it here is safe
    if (m_engine == NULL || m_gcTimer.elapsed() < GC_INTERVAL_MS)
        return;

    m_engine->collectGarbage();
    m_gcTimer.restart();
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

    Fixture *fxi = m_doc->fixture(fxID);
    if (fxi == NULL)
    {
        qWarning() << QString("No such fixture (ID: %1)").arg(fxID);
        return false;
    }

    if (channel >= fxi->channels())
    {
        qWarning() << QString("Fixture (%1) has no channel number %2").arg(fxi->name()).arg(channel);
        return false;
    }

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
    // Overwrite a pending value for the same channel instead of queueing another one
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
    // Use the static overload: the QProcess allocated here was never deleted
    QProcess::startDetached(programName, programArgs, QString(), &pid);
#endif

    return true;
}

bool ScriptRunner::waitTime(uint ms)
{
    QMutexLocker locker(&m_mutex);

    // This has to happen before the m_running check: Script::totalDuration()
    // runs the script through collectScriptData() with m_running == false
    // and sums up the waits via currentWaitTime().
    m_waitCount += ms / MasterTimer::tick();

    if (m_running == false)
        return false;

    qDebug() << Q_FUNC_INFO;

    // Sleep until write() has counted m_waitCount down to zero. The timeout
    // makes sure a stop request is noticed even if no wake-up arrives.
    while (m_running && m_waitCount > 0)
    {
        m_waitCondition.wait(&m_mutex, 10);

        // A GC pass can take a while. Never run it with the mutex held: write()
        // needs it on the MasterTimer thread every tick and would stall DMX output.
        locker.unlock();
        maybeCollectGarbage();
        locker.relock();
    }

    return true;
}

bool ScriptRunner::waitTime(QString time)
{
    return waitTime(Function::stringToSpeed(time));
}

bool ScriptRunner::waitFunctionStart(quint32 fID)
{
    return enqueueFunction(fID, FunctionOperation::WAIT_START);
}

bool ScriptRunner::waitFunctionStop(quint32 fID)
{
    return enqueueFunction(fID, FunctionOperation::WAIT_STOP);
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

    int min = Function::stringToSpeed(minTime);
    int max = Function::stringToSpeed(maxTime);

    return random(min, max);
}

int ScriptRunner::random(int minTime, int maxTime)
{
    if (m_running == false)
        return 0;

    // "(maxTime + 1) - minTime" overflows a 32 bit int when maxTime is INT_MAX,
    // and mixing it with the unsigned result of generate() gives a garbage
    // modulus for inverted (max < min) ranges. Do the math in 64 bit instead.
    if (maxTime < minTime)
        std::swap(minTime, maxTime);

    const qint64 range = qint64(maxTime) - qint64(minTime) + 1;
    const quint64 offset = QRandomGenerator::global()->generate64() % quint64(range);

    return int(qint64(minTime) + qint64(offset));
}
