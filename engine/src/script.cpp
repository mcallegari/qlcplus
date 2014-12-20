/*
  Q Light Controller
  script.cpp

  Copyright (C) Heikki Junnila

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

#include <QDomDocument>
#include <QDomElement>
#include <QDomText>
#include <QProcess>
#include <QDebug>
#include <QUrl>

#include "genericfader.h"
#include "fadechannel.h"
#include "mastertimer.h"
#include "qlcmacros.h"
#include "universe.h"
#include "script.h"
#include "doc.h"

#define KXMLQLCScriptCommand "Command"

const QString Script::startFunctionCmd = QString("startfunction");
const QString Script::stopFunctionCmd = QString("stopfunction");

const QString Script::waitCmd = QString("wait");
const QString Script::waitKeyCmd = QString("waitkey");

const QString Script::setFixtureCmd = QString("setfixture");
const QString Script::systemCmd = QString("systemcommand");

const QString Script::labelCmd = QString("label");
const QString Script::jumpCmd = QString("jump");

/****************************************************************************
 * Initialization
 ****************************************************************************/

Script::Script(Doc* doc) : Function(doc, Function::Script)
    , m_currentCommand(0)
    , m_waitCount(0)
    , m_fader(NULL)
{
    setName(tr("New Script"));
}

Script::~Script()
{
    if (m_fader != NULL)
        delete m_fader;
    m_fader = NULL;
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
        QStringList lines = m_data.split(QRegExp("(\r\n|\n\r|\r|\n)"), QString::KeepEmptyParts);
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

    // Map all labels to their individual line numbers for fast jumps
    m_labels.clear();
    for (int i = 0; i < m_lines.size(); i++)
    {
        QList <QStringList> line = m_lines[i];
        if (line.isEmpty() == false &&
            line.first().size() == 2 && line.first()[0] == Script::labelCmd)
        {
            m_labels[line.first()[1]] = i;
        }
    }

    return true;
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
    return m_data.split(QRegExp("(\r\n|\n\r|\r|\n)"), QString::KeepEmptyParts);
}

QList<int> Script::syntaxErrorsLines()
{
    return m_syntaxErrorLines;
}

/****************************************************************************
 * Load & Save
 ****************************************************************************/

bool Script::loadXML(const QDomElement& root)
{
    if (root.tagName() != KXMLQLCFunction)
    {
        qWarning() << Q_FUNC_INFO << "Function node not found";
        return false;
    }

    if (root.attribute(KXMLQLCFunctionType) != typeToString(Function::Script))
    {
        qWarning() << Q_FUNC_INFO << root.attribute(KXMLQLCFunctionType)
                   << "is not a script";
        return false;
    }

    /* Load script contents */
    QDomNode node = root.firstChild();
    while (node.isNull() == false)
    {
        QDomElement tag = node.toElement();

        if (tag.tagName() == KXMLQLCFunctionSpeed)
        {
            loadXMLSpeed(tag);
        }
        else if (tag.tagName() == KXMLQLCFunctionDirection)
        {
            loadXMLDirection(tag);
        }
        else if (tag.tagName() == KXMLQLCFunctionRunOrder)
        {
            loadXMLRunOrder(tag);
        }
        else if (tag.tagName() == KXMLQLCScriptCommand)
        {
            appendData(QUrl::fromPercentEncoding(tag.text().toUtf8()));
            //appendData(tag.text().toUtf8());
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown script tag:" << tag.tagName();
        }

        node = node.nextSibling();
    }

    return true;
}

bool Script::saveXML(QDomDocument* doc, QDomElement* wksp_root)
{
    QDomElement root;
    QDomElement tag;
    QDomText text;

    Q_ASSERT(doc != NULL);
    Q_ASSERT(wksp_root != NULL);

    /* Function tag */
    root = doc->createElement(KXMLQLCFunction);
    wksp_root->appendChild(root);

    /* Common attributes */
    saveXMLCommon(&root);

    /* Speed */
    saveXMLSpeed(doc, &root);

    /* Direction */
    saveXMLDirection(doc, &root);

    /* Run order */
    saveXMLRunOrder(doc, &root);

    /* Contents */
    foreach(QString cmd, dataLines())
    {
        tag = doc->createElement(KXMLQLCScriptCommand);
        root.appendChild(tag);
        text = doc->createTextNode(QUrl::toPercentEncoding(cmd));
        //text = doc->createTextNode(cmd.toUtf8());
        tag.appendChild(text);
    }

    return true;
}

/****************************************************************************
 * Running
 ****************************************************************************/

void Script::preRun(MasterTimer* timer)
{
    // Reset
    m_waitCount = 0;
    m_currentCommand = 0;
    m_startedFunctions.clear();

    Function::preRun(timer);
}

void Script::write(MasterTimer* timer, QList<Universe *> universes)
{
    incrementElapsed();

    if (stopped() == false)
    {
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

            // In case wait() is the last command, don't stop the script prematurely
            if (m_currentCommand >= m_lines.size() && m_waitCount == 0)
                stop();
        }

        // Handle GenericFader tasks (setltp/sethtp/setfixture)
        if (m_fader != NULL)
            m_fader->write(universes);
    }
}

void Script::postRun(MasterTimer* timer, QList<Universe *> universes)
{
    // Stop all functions started by this script
    foreach (Function* function, m_startedFunctions)
        function->stop();
    m_startedFunctions.clear();

    // Stops keeping HTP channels up
    if (m_fader != NULL)
        delete m_fader;
    m_fader = NULL;

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
    else
    {
        // Not waiting.
        return false;
    }
}

quint32 Script::getValueFromString(QString str, bool *ok)
{
    if (str.startsWith("random") == false)
    {
        if (str.contains("."))
            return Function::stringToSpeed(str);
        else
            return str.toUInt(ok);
    }

    QString strippedStr = str.remove("random(");
    strippedStr.remove(")");
    if (strippedStr.contains(",") == false)
        return -1;

    QStringList valList = strippedStr.split(",");
    int min = 0;
    if (valList.at(0).contains("."))
        min = Function::stringToSpeed(valList.at(0));
    else
        min = valList.at(0).toInt();
    int max = 0;
    if (valList.at(1).contains("."))
        max = Function::stringToSpeed(valList.at(1));
    else
        max = valList.at(1).toInt();

    *ok = true;
    return qrand() % ((max + 1) - min) + min;
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
    else if (tokens[0][0] == Script::startFunctionCmd)
    {
        error = handleStartFunction(tokens, timer);
    }
    else if (tokens[0][0] == Script::stopFunctionCmd)
    {
        error = handleStopFunction(tokens);
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
        if (function->stopped() == true)
            function->start(timer, true);
        else
            qWarning() << "Function (" << function->name() << ") is already running.";

        m_startedFunctions << function;
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

    Doc* doc = qobject_cast<Doc*> (parent());
    Q_ASSERT(doc != NULL);

    Function* function = doc->function(id);
    if (function != NULL)
    {
        if (function->stopped() == false)
            function->stop();
        else
            qWarning() << "Function (" << function->name() << ") is not running.";

        m_startedFunctions.removeAll(function);
        return QString();
    }
    else
    {
        return QString("No such function (ID %1)").arg(id);
    }
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

    Doc* doc = qobject_cast<Doc*> (parent());
    Q_ASSERT(doc != NULL);

    Fixture* fxi = doc->fixture(id);
    if (fxi != NULL)
    {
        if (ch < fxi->channels())
        {
            int address = fxi->address() + ch;
            if (address < 512)
            {
                GenericFader* gf = fader();
                Q_ASSERT(gf != NULL);

                FadeChannel fc;
                fc.setFixture(doc, fxi->id());
                fc.setChannel(ch);
                fc.setTarget(value);
                fc.setFadeTime(time);

                // If the script has used the channel previously, it might still be in
                // the bowels of GenericFader so get the starting value from there.
                // Otherwise get it from universes (HTP channels are always 0 then).
                quint32 uni = fc.universe();
                if (gf->channels().contains(fc) == true)
                    fc.setStart(gf->channels()[fc].current());
                else
                    fc.setStart(universes[uni]->preGMValue(address));
                fc.setCurrent(fc.start());

                gf->add(fc);

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

    QProcess *newProcess = new QProcess();
    newProcess->start(programName, programArgs);

    return QString();
}

QString Script::handleLabel(const QList<QStringList>& tokens)
{
    // A label just exists. Not much to do here.
    qDebug() << Q_FUNC_INFO;

    if (tokens.size() > 1)
        return QString("Too many arguments");

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
    int left = 0;
    int right = 0;
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
        left = line.indexOf("//");
        if (left != -1)
            line.truncate(left);

        left = 0;
        while (left < line.length())
        {
            // Find the next colon to get the keyword
            right = line.indexOf(":", left);
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

            tokens << (QStringList() << keyword.trimmed() << value.trimmed());
            qDebug() << "Tokens:" << tokens;
        }
    }

    return tokens;
}

GenericFader* Script::fader()
{
    // Create a fader if it doesn't exist yet
    if (m_fader == NULL)
    {
        Doc* doc = qobject_cast<Doc*> (parent());
        Q_ASSERT(doc != NULL);
        m_fader = new GenericFader(doc);
    }
    return m_fader;
}
