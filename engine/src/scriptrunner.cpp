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
    , m_waitCount(0)
    , m_fader(NULL)
{
}

ScriptRunner::~ScriptRunner()
{
    stop();

    // Stops keeping HTP channels up
    if (m_fader != NULL)
        delete m_fader;
    m_fader = NULL;
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

    m_running = false;
/*
    if (m_engine)
    {
        delete m_engine;
        m_engine = NULL;
    }
*/
    // Stop all functions started by this script
    foreach (quint32 fID, m_startedFunctions)
    {
        Function *function = m_doc->function(fID);
        if (function == NULL)
            continue;

        function->stop(FunctionParent::master());
    }
    m_startedFunctions.clear();
}

QList<int> ScriptRunner::syntaxErrorsLines()
{
    QList<int> syntaxErrorList;
    QJSEngine *engine = new QJSEngine();
    QJSValue objectValue = engine->newQObject(this);
    engine->globalObject().setProperty("Engine", objectValue);

    QJSValue script = engine->evaluate("(function run() { " + m_content + " })");
    if (script.isError())
    {
        syntaxErrorList << script.property("lineNumber").toInt();

        QString msg("Uncaught exception at line %2. Error: %3");
        qWarning() << msg.arg(script.property("lineNumber").toInt())
                         .arg(script.toString());
        //qDebug() << "Stack: " << script.property("stack").toString();
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
            syntaxErrorList << ret.property("lineNumber").toInt();

            QString msg("Uncaught exception at line %2. Error: %3");
            qWarning() << msg.arg(ret.property("lineNumber").toInt())
                             .arg(ret.toString());
        }
    }

    delete engine;

    return syntaxErrorList;
}

GenericFader *ScriptRunner::fader()
{
    // Create a fader if it doesn't exist yet
    if (m_fader == NULL)
        m_fader = new GenericFader(m_doc);

    return m_fader;
}

bool ScriptRunner::write(MasterTimer *timer, QList<Universe *> universes)
{
    if (m_waitCount > 0)
        m_waitCount--;

    if (m_functionQueue.count())
    {
        while (!m_functionQueue.isEmpty())
        {
            QPair<quint32,bool> pair = m_functionQueue.dequeue();
            quint32 fID = pair.first;
            bool start = pair.second;

            Function *function = m_doc->function(fID);
            if (function == NULL)
            {
                qWarning() << QString("No such function (ID %1)").arg(fID);
                continue;
            }

            if (start)
            {
                function->start(timer, FunctionParent::master());
                m_startedFunctions << fID;
            }
            else
            {
                function->stop(FunctionParent::master());
                m_startedFunctions.removeAll(fID);
            }
        }
    }

    // Handle GenericFader tasks (setFixture)
    if (m_fader)
        m_fader->write(universes);

    // If the JS call method has ended on its own, the thread
    // has finished, therefore there's nothing else to run here
    if (m_running == false)
        return false;

    return true;
}

void ScriptRunner::run()
{
    m_engine = new QJSEngine();
    QJSValue objectValue = m_engine->newQObject(this);
    m_engine->globalObject().setProperty("Engine", objectValue);

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
/*
    if (m_engine)
    {
        delete m_engine;
        m_engine = NULL;
    }
*/
    qDebug() << "ScriptRunner thread over.";
    // this thread is done. The calling Script can stop
    m_running = false;
}

/************************************************************************
 * JS exported methods
 ************************************************************************/

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

    GenericFader *gf = fader();
    Q_ASSERT(gf != NULL);

    FadeChannel fc(m_doc, fxi->id(), channel);
    fc.setTarget(value);
    fc.setFadeTime(time);

    // If the script has used the channel previously, it might still be in
    // the bowels of GenericFader so get the starting value from there.
    // Otherwise get it from universes (HTP channels are always 0 then).
    //quint32 uni = fc.universe();
    if (gf->channels().contains(fc) == true)
        fc.setStart(gf->channels()[fc].current());
    //else
    //    fc.setStart(universes[uni]->preGMValue(address)); // TODO ?
    fc.setCurrent(fc.start());

    gf->add(fc);

    return true;
}

bool ScriptRunner::startFunction(quint32 fID)
{
    if (m_running == false)
        return false;

    Function *function = m_doc->function(fID);
    if (function == NULL)
    {
        qWarning() << QString("No such function (ID %1)").arg(fID);
        return false;
    }

    QPair<quint32,bool> pair;
    pair.first = fID;
    pair.second = true;

    m_functionQueue.enqueue(pair);

    return true;
}

bool ScriptRunner::stopFunction(quint32 fID)
{
    if (m_running == false)
        return false;

    Function *function = m_doc->function(fID);
    if (function == NULL)
    {
        qWarning() << QString("No such function (ID %1)").arg(fID);
        return false;
    }

    QPair<quint32,bool> pair;
    pair.first = fID;
    pair.second = false;

    m_functionQueue.enqueue(pair);

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
                    multiPartArg.append(token.mid(0, token.length() - 2));
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
    QProcess *newProcess = new QProcess();
    newProcess->start(programName, programArgs);
#endif

    return true;
}

bool ScriptRunner::waitTime(uint ms)
{
    if (m_running == false)
        return false;

    qDebug() << Q_FUNC_INFO;

    m_waitCount = ms / MasterTimer::tick();

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
    if (m_running == false)
        return false;

    qDebug() << Q_FUNC_INFO;

    m_waitCount = Function::stringToSpeed(time) / MasterTimer::tick();

    while (m_waitCount > 0)
    {
        if (m_running == false)
            break;

        usleep(10000);
    }

    return true;
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

int ScriptRunner::random(QString minTime, QString maxTime)
{
    if (m_running == false)
        return 0;

    int min = Function::stringToSpeed(minTime);
    int max = Function::stringToSpeed(maxTime);

    return qrand() % ((max + 1) - min) + min;
}

int ScriptRunner::random(int minTime, int maxTime)
{
    if (m_running == false)
        return 0;

    return qrand() % ((maxTime + 1) - minTime) + minTime;
}

