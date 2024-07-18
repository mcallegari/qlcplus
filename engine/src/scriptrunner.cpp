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
#if !defined(Q_OS_IOS)
#include <QProcess>
#endif
#include <QDebug>

#include "scriptrunner.h"
#include "genericfader.h"
#include "fadechannel.h"
#include "mastertimer.h"
#include "universe.h"

ScriptRunner::ScriptRunner(Doc *doc, QString &content, QObject *parent)
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
    if (m_running == false)
        return;

    if (m_engine)
    {
        m_engine->setInterrupted(true);
        m_engine->deleteLater();
        m_engine = NULL;
    }

    // Stop all functions started by this script
    foreach (quint32 fID, m_startedFunctions)
    {
        Function *function = m_doc->function(fID);
        if (function == NULL)
            continue;

        function->stop(FunctionParent::master());
    }
    m_startedFunctions.clear();

    // request to delete all the active faders
    foreach (QSharedPointer<GenericFader> fader, m_fadersMap.values())
    {
        if (!fader.isNull())
            fader->requestDelete();
    }
    m_fadersMap.clear();

    m_running = false;
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

int ScriptRunner::currentWaitTime()
{
    return m_waitCount * MasterTimer::tick();
}

bool ScriptRunner::write(MasterTimer *timer, QList<Universe *> universes)
{
    if (m_waitCount > 0)
        m_waitCount--;

    if (m_fixtureValueQueue.count())
    {
        while (!m_fixtureValueQueue.isEmpty())
        {
            FixtureValue val = m_fixtureValueQueue.dequeue();

            QSharedPointer<GenericFader> fader = m_fadersMap.value(val.m_universe, QSharedPointer<GenericFader>());
            if (fader.isNull())
            {
                fader = universes[val.m_universe]->requestFader();
                //fader->adjustIntensity(getAttributeValue(Intensity));
                //fader->setBlendMode(blendMode());
                m_fadersMap[val.m_universe] = fader;
            }

            FadeChannel *fc = fader->getChannelFader(m_doc, universes[val.m_universe], val.m_fixtureID, val.m_channel);

            fc->setStart(fc->current());
            fc->setTarget(val.m_value);
            fc->setFadeTime(val.m_fadeTime);
            fc->setElapsed(0);
            fc->setReady(false);
        }
    }
    // if we don't have to wait and there are some funtions in the queue
    if (m_waitFunctionId == Function::invalidId() && m_functionQueue.count())
    {
        while (!m_functionQueue.isEmpty())
        {
            QPair<quint32, FunctionOperation> &pair = m_functionQueue.head();
            quint32 fID = pair.first;
            FunctionOperation operation = pair.second;

            Function *function = m_doc->function(fID);
            if (function == NULL)
            {
                qWarning() << QString("No such function (ID %1)").arg(fID);
                continue;
            }

            if (operation == FunctionOperation::START || operation == FunctionOperation::START_DONT_STOP)
            {
                function->start(timer, FunctionParent::master());
                if (operation == FunctionOperation::START)
                    m_startedFunctions << fID;
            }
            else if (operation == FunctionOperation::STOP)
            {
                function->stop(FunctionParent::master());
                m_startedFunctions.removeAll(fID);
            }
            else if (operation == FunctionOperation::WAIT_START)
            {
                if (!function->isRunning())
                {
                    // the function is not running, so we we wait and we stop dequeuing
                    m_waitFunctionId = fID;
                    connect(m_doc->masterTimer(), SIGNAL(functionStarted(quint32)), SLOT(slotWaitFunctionStarted(quint32)));
                    break;
                }
            }
            else if (operation == FunctionOperation::WAIT_STOP)
            {
                if (!function->stopped())
                {
                    // the function has to start or is still running, so we wait and we stop dequeuing
                    m_waitFunctionId = fID;
                    connect(m_doc->masterTimer(), SIGNAL(functionStopped(quint32)), SLOT(slotWaitFunctionStopped(quint32)));
                    break;
                }
            }
            // we can continue with the next function in the queue
            m_functionQueue.removeFirst();
        }
    }

    // If the JS call method has ended on its own, the thread
    // has finished, therefore there's nothing else to run here
    if (m_running == false)
        return false;

    return true;
}

void ScriptRunner::slotWaitFunctionStarted(quint32 fid)
{
    if (m_waitFunctionId == fid)
    {
        disconnect(m_doc->masterTimer(), SIGNAL(functionStarted(quint32)), this, SLOT(slotWaitFunctionStarted(quint32)));
        m_waitFunctionId = Function::invalidId();
    }
}

void ScriptRunner::slotWaitFunctionStopped(quint32 fid)
{
    if (m_waitFunctionId == fid)
    {
        disconnect(m_doc->masterTimer(), SIGNAL(functionStopped(quint32)), this, SLOT(slotWaitFunctionStopped(quint32)));
        m_startedFunctions.removeAll(fid);
        m_waitFunctionId = Function::invalidId();
    }
}

void ScriptRunner::run()
{
    m_waitCount = 0;

    m_engine = new QJSEngine();
    QJSValue objectValue = m_engine->newQObject(this);
    m_engine->globalObject().setProperty("Engine", objectValue);
    QQmlEngine::setObjectOwnership(this, QQmlEngine::CppOwnership);

    QJSValue script = m_engine->evaluate("(function run() { " + m_content + " })");

    if (script.isCallable() == false)
    {
        qDebug() << "ERROR. No function method found.";
        return;
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
    }

    qDebug() << "[ScriptRunner] Code executed";

    // this thread is done. Wait for the calling Script to stop
    while (m_running)
        msleep(50);
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
    m_fixtureValueQueue.enqueue(val);

    return true;
}

bool ScriptRunner::stopOnExit(bool value)
{
    m_stopOnExit = value;

    return true;
}

Function* ScriptRunner::getFunctionIfRunning(quint32 fID)
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

float ScriptRunner::getFunctionAttribute(quint32 fID, int attributeIndex)
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
        if (token.startsWith("'"))
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
    QProcess *newProcess = new QProcess();
    newProcess->setProgram(programName);
    newProcess->setArguments(programArgs);
    newProcess->startDetached(&pid);
#endif

    return true;
}

bool ScriptRunner::waitTime(uint ms)
{
    m_waitCount += ms / MasterTimer::tick();

    if (m_running == false)
        return false;

    qDebug() << Q_FUNC_INFO;

    while (m_waitCount > 0)
    {
        if (m_running == false)
            break;

        usleep(10000);
    }

    return true;
}

bool ScriptRunner::waitTime(QString time)
{
    m_waitCount += Function::stringToSpeed(time) / MasterTimer::tick();

    if (m_running == false)
        return false;

    qDebug() << Q_FUNC_INFO;

    while (m_waitCount > 0)
    {
        if (m_running == false)
            break;

        usleep(10000);
    }

    return true;
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

    return QRandomGenerator::global()->generate() % ((max + 1) - min) + min;
}

int ScriptRunner::random(int minTime, int maxTime)
{
    if (m_running == false)
        return 0;

    return QRandomGenerator::global()->generate() % ((maxTime + 1) - minTime) + minTime;
}

