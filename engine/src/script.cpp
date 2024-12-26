/*
  Q Light Controller Plus
  script.cpp

  Copyright (C) Heikki Junnila
                Massimo Callegari

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

#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#if !defined(Q_OS_IOS)
#include <QProcess>
#endif
#include <QDebug>
#include <QUrl>
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
#include <QRandomGenerator>
#endif
 
#include "genericfader.h"
#include "fadechannel.h"
#include "mastertimer.h"
#include "universe.h"
#include "script.h"
#include "doc.h"

#define KXMLQLCScriptCommand "Command"

const QString Script::stopOnExitCmd = QString("stoponexit");
const QString Script::startFunctionCmd = QString("startfunction");
const QString Script::stopFunctionCmd = QString("stopfunction");
const QString Script::blackoutCmd = QString("blackout");

const QString Script::waitCmd = QString("wait");
const QString Script::waitKeyCmd = QString("waitkey");
const QString Script::waitFunctionStartCmd = QString("waitfunctionstart");
const QString Script::waitFunctionStopCmd = QString("waitfunctionstop");

const QString Script::setFixtureCmd = QString("setfixture");
const QString Script::systemCmd = QString("systemcommand");

const QString Script::labelCmd = QString("label");
const QString Script::jumpCmd = QString("jump");

const QString Script::blackoutOn = QString("on");
const QString Script::blackoutOff = QString("off");

const QStringList knownKeywords(QStringList() << "ch" << "val" << "arg");

/****************************************************************************
 * Initialization
 ****************************************************************************/

Script::Script(Doc* doc) : Function(doc, Function::ScriptType)
    , m_currentCommand(0)
    , m_waitCount(0)
    , m_waitFunction(NULL)
{
    setName(tr("New Script"));
}

Script::~Script()
{
}

QIcon Script::getIcon() const
{
    return QIcon(":/script.png");
}

quint32 Script::totalDuration()
{
    quint32 totalDuration = 0;

    for (int i = 0; i < m_lines.count(); i++)
    {
        QList <QStringList> tokens = m_lines[i];
        if (tokens.isEmpty() || tokens[0].size() < 2)
            continue;

        if (tokens[0][0] == Script::waitCmd)
        {
            bool ok = false;
            quint32 waitTime = getValueFromString(tokens[0][1], &ok);
            if (ok == true)
                totalDuration += waitTime;
        }
    }

    return totalDuration;
}

Function* Script::createCopy(Doc* doc, bool addToDoc)
{
    Q_ASSERT(doc != NULL);

    Function* copy = new Script(doc);
    if (copy->copyFrom(this) == false)
    {
        delete copy;
        copy = NULL;
    }
    if (addToDoc == true && doc->addFunction(copy) == false)
    {
        delete copy;
        copy = NULL;
    }

    return copy;
}

bool Script::copyFrom(const Function* function)
{
    const Script* script = qobject_cast<const Script*> (function);
    if (script == NULL)
        return false;

    setData(script->data());

    return Function::copyFrom(function);
}

/****************************************************************************
 * Script data
 ****************************************************************************/

bool Script::setData(const QString& str)
{
    m_data = str;
    m_syntaxErrorLines.clear();

    // Construct individual code lines from the data
    m_lines.clear();
    if (m_data.isEmpty() == false)
    {
        int i = 1;
        QStringList lines = m_data.split(QRegExp("(\r\n|\n\r|\r|\n)"));
        foreach (QString line, lines)
        {
            bool ok = false;
            if (line.isEmpty() == false)
            {
                m_lines << tokenizeLine(line + QString("\n"), &ok);
                if (ok == false)
                    m_syntaxErrorLines.append(i);
            }
            i++;
        }
    }

    scanForLabels();

    return true;
}

void Script::scanForLabels()
{
    // Map all labels to their individual line numbers for fast jumps
    m_labels.clear();
    for (int i = 0; i < m_lines.size(); i++)
    {
        QList <QStringList> line = m_lines[i];
        if (line.isEmpty() == false &&
            line.first().size() == 2 && line.first()[0] == Script::labelCmd)
        {
            m_labels[line.first()[1]] = i;
            qDebug() << QString("Map label '%1' to line '%2'").arg(line.first()[1]).arg(i);
        }
    }
}

bool Script::appendData(const QString &str)
{
    m_data.append(str + QString("\n"));
    m_lines << tokenizeLine(str + QString("\n"));

    return true;
}

QString Script::data() const
{
    return m_data;
}

QStringList Script::dataLines() const
{
    QStringList result = m_data.split(QRegExp("(\r\n|\n\r|\r|\n)"));
    while (result.count() && result.last().isEmpty())
        result.takeLast();

    return result;
}

QList<quint32> Script::functionList() const
{
    QList<quint32> list;

    for (int i = 0; i < m_lines.count(); i++)
    {
        QList <QStringList> tokens = m_lines[i];
        if (tokens.isEmpty() == true)
            continue;

        if (tokens[0].size() >= 2 && tokens[0][0] == Script::startFunctionCmd)
        {
            list.append(tokens[0][1].toUInt());
            list.append(i);
        }
    }

    return list;
}

QList<quint32> Script::fixtureList() const
{
    QList<quint32> list;

    for (int i = 0; i < m_lines.count(); i++)
    {
        QList <QStringList> tokens = m_lines[i];
        if (tokens.isEmpty() == true)
            continue;

        if (tokens[0].size() >= 2 && tokens[0][0] == Script::setFixtureCmd)
        {
            list.append(tokens[0][1].toUInt());
            list.append(i);
        }
    }

    return list;
}

QList<int> Script::syntaxErrorsLines()
{
    return m_syntaxErrorLines;
}

/****************************************************************************
 * Load & Save
 ****************************************************************************/

bool Script::loadXML(QXmlStreamReader &root)
{
    if (root.name() != KXMLQLCFunction)
    {
        qWarning() << Q_FUNC_INFO << "Function node not found";
        return false;
    }

    if (root.attributes().value(KXMLQLCFunctionType).toString() != typeToString(Function::ScriptType))
    {
        qWarning() << Q_FUNC_INFO << root.attributes().value(KXMLQLCFunctionType).toString()
                   << "is not a script";
        return false;
    }

    /* Load script contents */
    while (root.readNextStartElement())
    {
        if (root.name() == KXMLQLCFunctionSpeed)
        {
            loadXMLSpeed(root);
        }
        else if (root.name() == KXMLQLCFunctionDirection)
        {
            loadXMLDirection(root);
        }
        else if (root.name() == KXMLQLCFunctionRunOrder)
        {
            loadXMLRunOrder(root);
        }
        else if (root.name() == KXMLQLCScriptCommand)
        {
            appendData(QUrl::fromPercentEncoding(root.readElementText().toUtf8()));
            //appendData(tag.text().toUtf8());
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown script tag:" << root.name();
            root.skipCurrentElement();
        }
    }

    scanForLabels();

    return true;
}

bool Script::saveXML(QXmlStreamWriter *doc)
{
    Q_ASSERT(doc != NULL);

    /* Function tag */
    doc->writeStartElement(KXMLQLCFunction);

    /* Common attributes */
    saveXMLCommon(doc);

    /* Speed */
    saveXMLSpeed(doc);

    /* Direction */
    saveXMLDirection(doc);

    /* Run order */
    saveXMLRunOrder(doc);

    /* Contents */
    foreach (QString cmd, dataLines())
    {
        doc->writeTextElement(KXMLQLCScriptCommand, QUrl::toPercentEncoding(cmd));
    }

    /* End the <Function> tag */
    doc->writeEndElement();

    return true;
}

/****************************************************************************
 * Running
 ****************************************************************************/

void Script::preRun(MasterTimer *timer)
{
    // Reset
    m_waitCount = 0;
    m_waitFunction = NULL;
    m_currentCommand = 0;
    m_startedFunctions.clear();
    m_stopOnExit = true;

    Function::preRun(timer);
}

void Script::write(MasterTimer *timer, QList<Universe *> universes)
{
    if (stopped() || isPaused())
        return;

    incrementElapsed();

    if (waiting() == false)
    {
        // Not currently waiting for anything. Free to proceed to next command.
        while (m_currentCommand < m_lines.size() && stopped() == false)
        {
            bool continueLoop = executeCommand(m_currentCommand, timer, universes);
            m_currentCommand++;
            if (continueLoop == false)
                break; // Executed command told to skip to the next cycle
        }

        // In case a wait command is the last command, don't stop the script prematurely
        if (m_currentCommand >= m_lines.size() && m_waitCount == 0 && m_waitFunction == NULL)
            stop(FunctionParent::master());
    }

    // Handle GenericFader tasks (setltp/sethtp/setfixture)
    //if (m_fader != NULL)
    //    m_fader->write(universes);
}

void Script::postRun(MasterTimer *timer, QList<Universe *> universes)
{
    // Stop all functions started by this script
    foreach (Function *function, m_startedFunctions)
        function->stop(FunctionParent::master());

    m_startedFunctions.clear();

    dismissAllFaders();

    Function::postRun(timer, universes);
}

bool Script::waiting()
{
    if (m_waitCount > 0)
    {
        // Still waiting for at least one cycle.
        m_waitCount--;
        return true;
    }
    else if (m_waitFunction != NULL)
    {
        // Still waiting for the function to start/stop.
        return true;
    }
    // Not waiting.
    return false;
}

quint32 Script::getValueFromString(QString str, bool *ok)
{
    if (str.startsWith("random") == false)
    {
        *ok = true;
        return Function::stringToSpeed(str);
    }

    QString strippedStr = str.remove("random(");
    strippedStr.remove(")");
    if (strippedStr.contains(",") == false)
        return -1;

    QStringList valList = strippedStr.split(",");
    int min = Function::stringToSpeed(valList.at(0));
    int max = Function::stringToSpeed(valList.at(1));

    *ok = true;
#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
    return qrand() % ((max + 1) - min) + min;
#else
      return QRandomGenerator::global()->generate() % ((max + 1) - min) + min;
#endif
}

bool Script::executeCommand(int index, MasterTimer* timer, QList<Universe *> universes)
{
    if (index < 0 || index >= m_lines.size())
    {
        qWarning() << "Invalid command index:" << index;
        return false;
    }

    QList <QStringList> tokens = m_lines[index];
    if (tokens.isEmpty() == true)
        return true; // Empty line

    bool continueLoop = true;
    QString error;
    if (tokens[0].size() < 2)
    {
        error = QString("Syntax error");
    }
    else if (tokens[0][0] == Script::stopOnExitCmd)
    {
        error = handleStopOnExit(tokens);
    }
    else if (tokens[0][0] == Script::startFunctionCmd)
    {
        error = handleStartFunction(tokens, timer);
    }
    else if (tokens[0][0] == Script::stopFunctionCmd)
    {
        error = handleStopFunction(tokens);
    }
    else if (tokens[0][0] == Script::blackoutCmd)
    {
        error = handleBlackout(tokens);
        continueLoop = false;
    }
    else if (tokens[0][0] == Script::waitCmd)
    {
        // Waiting should break out of the execution loop to prevent skipping
        // straight to the next command. If there is no error in wait parsing,
        // we must wait at least one cycle.
        error = handleWait(tokens);
        if (error.isEmpty() == true)
            continueLoop = false;
    }
    else if (tokens[0][0] == Script::waitKeyCmd)
    {
        // Waiting for a key should break out of the execution loop to prevent
        // skipping straight to the next command. If there is no error in waitkey
        // parsing,we must wait at least one cycle.
        error = handleWaitKey(tokens);
        if (error.isEmpty() == true)
            continueLoop = false;
    }
    else if (tokens[0][0] == Script::waitFunctionStartCmd)
    {
        // Waiting for a funcion should break out of the execution loop to
        // prevent skipping straight to the next command. If there is no error
        // in waitfunctionstart parsing, we must wait at least one cycle.
        error = handleWaitFunction(tokens, true);
        if (error.isEmpty() == true)
            continueLoop = false;
    }
    else if (tokens[0][0] == Script::waitFunctionStopCmd)
    {
        // Waiting for a funcion should break out of the execution loop to
        // prevent skipping straight to the next command. If there is no error
        // in waitfunctionstop parsing, we must wait at least one cycle.
        error = handleWaitFunction(tokens, false);
        if (error.isEmpty() == true)
            continueLoop = false;
    }
    else if (tokens[0][0] == Script::setFixtureCmd)
    {
        error = handleSetFixture(tokens, universes);
    }
    else if (tokens[0][0] == Script::systemCmd)
    {
        error = handleSystemCommand(tokens);
    }
    else if (tokens[0][0] == Script::labelCmd)
    {
        error = handleLabel(tokens);
    }
    else if (tokens[0][0] == Script::jumpCmd)
    {
        // Jumping can cause an infinite non-waiting loop, causing starvation
        // among other functions. Therefore, the script must relinquish its
        // time slot after each jump. If there is no error in jumping, the jump
        // must have happened.
        error = handleJump(tokens);
        if (error.isEmpty() == true)
            continueLoop = false;
    }
    else
    {
        error = QString("Unknown command: %1").arg(tokens[0][0]);
    }

    if (error.isEmpty() == false)
        qWarning() << QString("Script:%1, line:%2, error:%3").arg(name()).arg(index).arg(error);

    return continueLoop;
}

QString Script::handleStopOnExit(const QList<QStringList>& tokens)
{
    qDebug() << Q_FUNC_INFO;

    if (tokens.size() > 1)
        return QString("Too many arguments");

    bool flag = QVariant(tokens[0][1]).toBool();
    
    m_stopOnExit = flag;

    return QString();
}

QString Script::handleStartFunction(const QList<QStringList>& tokens, MasterTimer* timer)
{
    qDebug() << Q_FUNC_INFO;

    if (tokens.size() > 1)
        return QString("Too many arguments");

    bool ok = false;
    quint32 id = tokens[0][1].toUInt(&ok);
    if (ok == false)
        return QString("Invalid function ID: %1").arg(tokens[0][1]);

    Doc* doc = qobject_cast<Doc*> (parent());
    Q_ASSERT(doc != NULL);

    Function* function = doc->function(id);
    if (function != NULL)
    {
        function->start(timer, FunctionParent::master());

        if (m_stopOnExit)
        {
            m_startedFunctions << function;
        }
        return QString();
    }
    else
    {
        return QString("No such function (ID %1)").arg(id);
    }
}

QString Script::handleStopFunction(const QList <QStringList>& tokens)
{
    qDebug() << Q_FUNC_INFO;

    if (tokens.size() > 1)
        return QString("Too many arguments");

    bool ok = false;
    quint32 id = tokens[0][1].toUInt(&ok);
    if (ok == false)
        return QString("Invalid function ID: %1").arg(tokens[0][1]);

    Doc *doc = qobject_cast<Doc*> (parent());
    Q_ASSERT(doc != NULL);

    Function *function = doc->function(id);
    if (function != NULL)
    {
        function->stop(FunctionParent::master());

        m_startedFunctions.removeAll(function);
        return QString();
    }
    else
    {
        return QString("No such function (ID %1)").arg(id);
    }
}

QString Script::handleBlackout(const QList <QStringList>& tokens)
{
    qDebug() << Q_FUNC_INFO;

    if (tokens.size() > 1)
        return QString("Too many arguments");

    InputOutputMap::BlackoutRequest request = InputOutputMap::BlackoutRequestNone;

    if (tokens[0][1] == blackoutOn)
    {
        request = InputOutputMap::BlackoutRequestOn;
    }
    else if (tokens[0][1] == blackoutOff)
    {
        request = InputOutputMap::BlackoutRequestOff;
    }
    else
    {
        return QString("Invalid argument: %1").arg(tokens[0][1]);
    }

    Doc* doc = qobject_cast<Doc*> (parent());
    Q_ASSERT(doc != NULL);

    doc->inputOutputMap()->requestBlackout(request);

    return QString();
}

QString Script::handleWait(const QList<QStringList>& tokens)
{
    qDebug() << Q_FUNC_INFO;

    if (tokens.size() > 2)
        return QString("Too many arguments");

    bool ok = false;
    uint time = getValueFromString(tokens[0][1], &ok);

    qDebug() << "Wait time:" << time;

    m_waitCount = time / MasterTimer::tick();

    return QString();
}

QString Script::handleWaitKey(const QList<QStringList>& tokens)
{
    qDebug() << Q_FUNC_INFO << tokens;

    if (tokens.size() > 1)
        return QString("Too many arguments");

    QString key = QString(tokens[0][1]).remove("\"");
    qDebug() << "Ought to wait for" << key;

    return QString();
}

QString Script::handleWaitFunction(const QList<QStringList> &tokens, bool start)
{
    qDebug() << Q_FUNC_INFO << tokens;

    if (tokens.size() > 1)
        return QString("Too many arguments");

    bool ok = false;
    quint32 id = tokens[0][1].toUInt(&ok);
    if (ok == false)
        return QString("Invalid function ID: %1").arg(tokens[0][1]);

    Doc *doc = qobject_cast<Doc *>(parent());
    Q_ASSERT(doc != NULL);

    Function *function = doc->function(id);
    if (function == NULL)
    {
        return QString("No such function (ID %1)").arg(id);
    }

    if (start)
    {
        if (!function->isRunning())
        {
            m_waitFunction = function;
            connect(m_waitFunction, SIGNAL(running(quint32)), this, SLOT(slotWaitFunctionStarted(quint32)));
        }
    }
    else
    {
        if (!function->stopped())
        {
            m_waitFunction = function;
            connect(m_waitFunction, SIGNAL(stopped(quint32)), this, SLOT(slotWaitFunctionStopped(quint32)));
        }
    }

    return QString();
}

void Script::slotWaitFunctionStarted(quint32 fid)
{
    if (m_waitFunction != NULL && m_waitFunction->id() == fid) {
        disconnect(m_waitFunction, SIGNAL(running(quint32)), this, SLOT(slotWaitFunctionStarted(quint32)));
        m_waitFunction = NULL;
    }
}

void Script::slotWaitFunctionStopped(quint32 fid)
{
    if (m_waitFunction != NULL && m_waitFunction->id() == fid) {
        disconnect(m_waitFunction, SIGNAL(stopped(quint32)), this, SLOT(slotWaitFunctionStopped(quint32)));
        m_startedFunctions.removeAll(m_waitFunction);
        m_waitFunction = NULL;
    }
}

QString Script::handleSetFixture(const QList<QStringList>& tokens, QList<Universe *> universes)
{
    qDebug() << Q_FUNC_INFO;

    if (tokens.size() > 4)
        return QString("Too many arguments");

    bool ok = false;
    quint32 id = 0;
    quint32 ch = 0;
    uchar value = 0;
    double time = 0;

    id = getValueFromString(tokens[0][1], &ok);
    if (ok == false)
        return QString("Invalid fixture (ID: %1)").arg(tokens[0][1]);

    for (int i = 1; i < tokens.size(); i++)
    {
        QStringList list = tokens[i];
        list[0] = list[0].toLower().trimmed();
        if (list.size() == 2)
        {
            ok = false;
            if (list[0] == "val" || list[0] == "value")
                value = uchar(getValueFromString(list[1], &ok));
            else if (list[0] == "ch" || list[0] == "channel")
                ch = getValueFromString(list[1], &ok);
            else if (list[0] == "time")
                time = getValueFromString(list[1], &ok);
            else
                return QString("Unrecognized keyword: %1").arg(list[0]);

            if (ok == false)
                return QString("Invalid value (%1) for keyword: %2").arg(list[1]).arg(list[0]);
        }
    }

    Doc *doc = qobject_cast<Doc*> (parent());
    Q_ASSERT(doc != NULL);

    Fixture *fxi = doc->fixture(id);
    if (fxi != NULL)
    {
        if (ch < fxi->channels())
        {
            int address = fxi->address() + ch;
            if (address < 512)
            {
                quint32 universe = fxi->universe();
                QSharedPointer<GenericFader> fader = m_fadersMap.value(universe, QSharedPointer<GenericFader>());
                if (fader.isNull())
                {
                    fader = universes[universe]->requestFader();
                    fader->adjustIntensity(getAttributeValue(Intensity));
                    fader->setBlendMode(blendMode());
                    fader->setParentFunctionID(this->id());
                    fader->setName(name());
                    m_fadersMap[universe] = fader;
                }

                FadeChannel *fc = fader->getChannelFader(doc, universes[universe], fxi->id(), ch);
                fc->setTarget(value);
                fc->setFadeTime(time);

                return QString();
            }
            else
            {
                return QString("Invalid address: %1").arg(address);
            }
        }
        else
        {
            return QString("Fixture (%1) has no channel number %2").arg(fxi->name()).arg(ch);
        }
    }
    else
    {
        return QString("No such fixture (ID: %1)").arg(id);
    }
}

QString Script::handleSystemCommand(const QList<QStringList> &tokens)
{
    qDebug() << Q_FUNC_INFO;

    QString programName = tokens[0][1];
    QStringList programArgs;
    for (int i = 1; i < tokens.size(); i++)
        programArgs << tokens[i][1];
#if !defined(Q_OS_IOS)
    QProcess *newProcess = new QProcess();

    // startDetached() enables to delete QProcess object without killing actual process
    qint64 pid;
    newProcess->setProgram(programName);
    newProcess->setArguments(programArgs);
    newProcess->startDetached(&pid);
    delete newProcess;
#endif
    return QString();
}

QString Script::handleLabel(const QList<QStringList>& tokens)
{
    // A label just exists. Not much to do here.
    qDebug() << Q_FUNC_INFO;

    if (tokens.size() > 1)
        return QString("Too many arguments");

    qDebug() << QString("Found label '%1'").arg(tokens[0][1]);

    return QString();
}

QString Script::handleJump(const QList<QStringList>& tokens)
{
    qDebug() << Q_FUNC_INFO;

    if (tokens.size() > 1)
        return QString("Too many arguments");

    if (m_labels.contains(tokens[0][1]) == true)
    {
        int lineNumber = m_labels[tokens[0][1]];
        Q_ASSERT(lineNumber >= 0 && lineNumber < m_lines.size());
        m_currentCommand = lineNumber;

        // cleanup m_startedFunctions to avoid infinite growth
        QList<Function*>::iterator it = m_startedFunctions.begin();
        while (it != m_startedFunctions.end()) {
            if ((*it)->stopped())
                it = m_startedFunctions.erase(it);
            else
                ++it;
        }

        return QString();
    }
    else
    {
        return QString("No such label: %1").arg(tokens[0][1]);
    }
}

QList <QStringList> Script::tokenizeLine(const QString& str, bool* ok)
{
    QList<QStringList> tokens;
    QString keyword;
    QString value;

    if (ok != NULL)
        *ok = true; // in case, this is set to false afterwards

    if (str.simplified().startsWith("//") == true || str.simplified().isEmpty() == true)
    {
        tokens << QStringList(); // Return an empty string list for commented lines
    }
    else
    {
        // Truncate everything after the first comment sign
        QString line = str;
        int left = 0;

        while (left != -1)
        {
            left = line.indexOf("//", left);
            if (left != -1)
            {
                // if we stumbled into a URL like http:// or ftp://
                // then it's not a comment !
                if (line.at(left - 1) != ':')
                    line.truncate(left);
                left += 2;
            }
        }

        left = 0;
        while (left < line.length())
        {
            // Find the next colon to get the keyword
            int right = line.indexOf(":", left);
            if (right == -1)
            {
                qDebug() << "Syntax error:" << line.mid(left);
                if (ok != NULL)
                    *ok = false;
                break;
            }
            else
            {
                // Keyword found
                keyword = line.mid(left, right - left);
                left = right + 1;
            }

            // Try to see if there is a value between quotes
            int quoteleft = -1;
            if (line.mid(left, 1) == "\"")
                quoteleft = left + 1;
            if (quoteleft != -1)
            {
                int quoteright = line.indexOf("\"", quoteleft + 1);
                if (quoteright != -1)
                {
                    // Don't include the "" in the string
                    value = line.mid(quoteleft, quoteright - quoteleft);
                    left = quoteright + 2;
                }
                else
                {
                    qDebug() << "Syntax error:" << line.mid(quoteleft);
                    if (ok != NULL)
                        *ok = false;
                    break;
                }
            }
            else
            {
                // No quotes. Find the next whitespace.
                right = line.indexOf(QRegExp("\\s"), left);
                if (right == -1)
                {
                    qDebug() << "Syntax error:" << line.mid(left);
                    if (ok != NULL)
                        *ok = false;
                    break;
                }
                else
                {
                    // Value found
                    value = line.mid(left, right - left);
                    left = right + 1;
                }
            }

            if (tokens.count() > 0 && knownKeywords.contains(keyword.trimmed()) == false)
            {
                qDebug() << "Syntax error. Unknown keyword detected:" << keyword.trimmed();
                if (ok != NULL)
                    *ok = false;
                break;
            }
            else
                tokens << (QStringList() << keyword.trimmed() << value.trimmed());
        }
    }

    qDebug() << "Tokens:" << tokens;

    return tokens;
}

