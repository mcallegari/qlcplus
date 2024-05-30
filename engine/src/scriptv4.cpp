/*
  Q Light Controller Plus
  scriptv4.cpp

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

#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QRandomGenerator>
#include <QRegularExpression>
#include <QDebug>
#include <QUrl>

#include "scriptrunner.h"
#include "mastertimer.h"
#include "qlcmacros.h"
#include "universe.h"
#include "scriptv4.h"
#include "doc.h"

#define KXMLQLCScriptCommand QString("Command")
#define KXMLQLCScriptVersion QString("Version")

const QString Script::stopOnExitLegacy = QString("stoponexit");
const QString Script::stopOnExitCmd = QString("Engine.stopOnExit");
const QString Script::startFunctionLegacy = QString("startfunction");
const QString Script::startFunctionCmd = QString("Engine.startFunction");
const QString Script::stopFunctionLegacy = QString("stopfunction");
const QString Script::stopFunctionCmd = QString("Engine.stopFunction");
const QString Script::blackoutLegacy = QString("blackout");
const QString Script::blackoutCmd = QString("Engine.setBlackout");
const QString Script::waitLegacy = QString("wait");
const QString Script::waitCmd = QString("Engine.waitTime");
const QString Script::waitFunctionStartLegacy = QString("waitfunctionstart");
const QString Script::waitFunctionStartCmd = QString("Engine.waitFunctionStart");
const QString Script::waitFunctionStopLegacy = QString("waitfunctionstop");
const QString Script::waitFunctionStopCmd = QString("Engine.waitFunctionStop");
const QString Script::setFixtureLegacy = QString("setfixture");
const QString Script::setFixtureCmd = QString("Engine.setFixture");
const QString Script::systemLegacy = QString("systemcommand");
const QString Script::systemCmd = QString("Engine.systemCommand");
const QStringList knownKeywords(QStringList() << "ch" << "val" << "arg");

const QString Script::blackoutOn = QString("on"); // LEGACY - NOT USED
const QString Script::blackoutOff = QString("off"); // LEGACY - NOT USED
const QString Script::waitKeyCmd = QString("waitkey"); // LEGACY - NOT USED

/****************************************************************************
 * Initialization
 ****************************************************************************/

Script::Script(Doc* doc) : Function(doc, Function::ScriptType)
    , m_runner(NULL)
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

    ScriptRunner *runner = new ScriptRunner(doc(), m_data);
    runner->collectScriptData();
    totalDuration = runner->currentWaitTime();
    //runner->deleteLater();

    qDebug() << "Script total duration:" << totalDuration;

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
    if (str == m_data)
        return false;

    m_data = str;

    Doc* doc = qobject_cast<Doc*> (parent());
    Q_ASSERT(doc != NULL);
    doc->setModified();

    return true;
}

bool Script::appendData(const QString &str)
{
    //m_data.append(str + QString("\n"));
    m_data.append(convertLine(str + QString("\n")));

    return true;
}

QString Script::data() const
{
    return m_data;
}

QStringList Script::dataLines() const
{
    QStringList result = m_data.split(QRegularExpression("(\\r\\n|\\n\\r|\\r|\\n)"));

    while (result.count() && result.last().isEmpty())
        result.takeLast();

    return result;
}

QList<quint32> Script::functionList() const
{
    QList<quint32> list;
    int count = 0;

    foreach (QString line, dataLines())
    {
        count ++;
        if (line.startsWith(startFunctionCmd + "(") ||
                line.startsWith(stopFunctionCmd + "("))
        {
            QStringList tokens = line.split("(");
            if (tokens.isEmpty() || tokens.count() < 2)
                continue;

            tokens = tokens[1].split(")");
            if (tokens.isEmpty() || tokens.count() < 2)
                continue;

            QStringList params = tokens[0].split(",");
            if (tokens.isEmpty())
                continue;

            quint32 funcID = params[0].toUInt();
            if (list.contains(funcID) == false)
            {
                list.append(funcID);
                list.append(count - 1);
            }
        }
    }

    return list;
}

QList<quint32> Script::fixtureList() const
{
    QList<quint32> list;

    foreach (QString line, dataLines())
    {
        if (line.contains("setFixture"))
        {
            QStringList tokens = line.split("(");
            if (tokens.isEmpty() || tokens.count() < 2)
                continue;

            QStringList params = tokens[1].split(",");
            if (tokens.isEmpty())
                continue;

            quint32 fxID = params[0].toUInt();
            if (list.contains(fxID) == false)
                list.append(fxID);
        }
    }

    return list;
}

QStringList Script::syntaxErrorsLines()
{
    ScriptRunner *runner = new ScriptRunner(doc(), m_data);
    QStringList errorList = runner->collectScriptData();
    //runner->deleteLater();

    return errorList;
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

    QXmlStreamAttributes attrs = root.attributes();

    if (attrs.value(KXMLQLCFunctionType).toString() != typeToString(Function::ScriptType))
    {
        qWarning() << Q_FUNC_INFO << root.attributes().value(KXMLQLCFunctionType).toString()
                   << "is not a script";
        return false;
    }

    int version = 1;

    if (attrs.hasAttribute(KXMLQLCScriptVersion))
        version = attrs.value(KXMLQLCScriptVersion).toInt();

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
            if (version == 1)
                m_data.append(convertLine(QUrl::fromPercentEncoding(root.readElementText().toUtf8()) + QString("\n")));
            else
                m_data.append(QUrl::fromPercentEncoding(root.readElementText().toUtf8()) + QString("\n"));
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown script tag:" << root.name();
            root.skipCurrentElement();
        }
    }

    return true;
}

bool Script::saveXML(QXmlStreamWriter *doc)
{
    Q_ASSERT(doc != NULL);

    /* Function tag */
    doc->writeStartElement(KXMLQLCFunction);

    /* Common attributes */
    saveXMLCommon(doc);

    /* Version */
    doc->writeAttribute(KXMLQLCScriptVersion, QString::number(2));

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

void Script::preRun(MasterTimer* timer)
{
    m_runner = new ScriptRunner(doc(), m_data);
    connect(m_runner, SIGNAL(finished()), this, SLOT(slotRunnerFinished()));
    m_runner->execute();

    Function::preRun(timer);
}

void Script::write(MasterTimer *timer, QList<Universe *> universes)
{
    if (isPaused())
        return;

    incrementElapsed();

    if (m_runner)
    {
        if (m_runner->write(timer, universes) == false)
            stop(FunctionParent::master());
    }
}

void Script::postRun(MasterTimer* timer, QList<Universe *> universes)
{
    if (m_runner)
    {
        m_runner->stop();
        m_runner->exit();
        m_runner->wait();
    }

    Function::postRun(timer, universes);
}

void Script::slotRunnerFinished()
{
    delete m_runner;
    m_runner = NULL;
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
    return QRandomGenerator::global()->generate() % ((max + 1) - min) + min;
}

QString Script::convertLine(const QString& str, bool *ok)
{
    QStringList values;
    QString comment;
    QString keyword;
    QString command;
    QString value;

    if (ok != NULL)
        *ok = true; // in case, this is set to false afterwards

    if (str.simplified().startsWith("//") == true || str.simplified().isEmpty() == true)
        return str;

    qDebug() << "---> " << str;

    // Save everything after the first comment sign
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
            {
                comment = line.mid(left);
                line.truncate(left);
            }
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
            qDebug() << "Syntax error (colon missing after keyword):" << line.mid(left);
            if (ok != NULL)
                *ok = false;
            break;
        }
        else
        {
            // Keyword found
            keyword = line.mid(left, right - left);
            left = right + 1;
            if (command.isEmpty())
                command = keyword;
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
                // Include the "" in the string
                value = line.mid(quoteleft - 1, quoteright - quoteleft + 2);
                value.replace("\"", "'");
                left = quoteright + 2;
            }
            else
            {
                qDebug() << "Syntax error (unbalanced quotes):" << line.mid(quoteleft);
                if (ok != NULL)
                    *ok = false;
                break;
            }
        }
        else
        {
            // No quotes. Find the next whitespace.
            right = line.indexOf(QRegularExpression("\\s"), left);
            if (right == -1)
            {
                qDebug() << "Syntax error (whitespace before value missing):" << line.mid(left);
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

        if (values.count() > 0 && knownKeywords.contains(keyword.trimmed()) == false)
        {
            qDebug() << "Syntax error. Unknown keyword detected:" << keyword.trimmed();
            if (ok != NULL)
                *ok = false;
            break;
        }
        else
        {
            if (value.startsWith("random"))
            {
                QStringList rToks = value.split(",");
                QString min = rToks[0].mid(rToks[0].indexOf("(") + 1);
                QString max = rToks[1].mid(0, rToks[1].indexOf(")"));
                if (min.contains("s") || min.contains("m") || min.contains("h"))
                {
                    min.prepend("\"");
                    min.append("\"");
                }
                if (max.contains("s") || max.contains("m") || max.contains("h"))
                {
                    max.prepend("\"");
                    max.append("\"");
                }

                value = QString("Engine.random(%1,%2)").arg(min).arg(max);
            }
            else if (command == waitLegacy)
            {
                if (value.contains("s") || value.contains("m") || value.contains("h"))
                {
                    value.prepend("\"");
                    value.append("\"");
                }
            }

            values << value.trimmed();
        }
    }

    if (command == systemLegacy)
    {
        QString cmd = values.join(" ");
        cmd.prepend("\"");
        cmd.append("\"");
        values.clear();
        values << cmd;
    }

    qDebug() << "COMMAND:" << command << "Values:" << values;

    line = convertLegacyMethod(command);
    line.append("(");
    for (int i = 0; i < values.count(); i++)
    {
        line.append(values.at(i));
        if (i < values.count() - 1)
            line.append(",");
    }

    if (comment.isEmpty())
        line.append(");\n");
    else
        line.append("); " + comment);

    return line;
}

QString Script::convertLegacyMethod(QString method)
{
    if (method == stopOnExitLegacy) return stopOnExitCmd;
    else if (method == startFunctionLegacy) return startFunctionCmd;
    else if (method == stopFunctionLegacy) return stopFunctionCmd;
    else if (method == blackoutLegacy) return blackoutCmd;
    else if (method == waitLegacy) return waitCmd;
    else if (method == waitFunctionStartLegacy) return waitFunctionStartCmd;
    else if (method == waitFunctionStopLegacy) return waitFunctionStopCmd;
    else if (method == setFixtureLegacy) return setFixtureCmd;
    else if (method == systemLegacy) return systemCmd;
    else return "";
}

