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
#include <QDebug>
#include <QUrl>

#include "scriptrunner.h"
#include "mastertimer.h"
#include "qlcmacros.h"
#include "universe.h"
#include "scriptv4.h"
#include "doc.h"

#define KXMLQLCScriptCommand "Command"
#define KXMLQLCScriptVersion "Version"

const QString Script::startFunctionCmd = QString("startfunction");
const QString Script::stopFunctionCmd = QString("stopfunction");
const QString Script::blackoutCmd = QString("blackout");
const QString Script::waitCmd = QString("wait");
const QString Script::setFixtureCmd = QString("setfixture");
const QString Script::systemCmd = QString("systemcommand");

const QStringList knownKeywords(QStringList() << "ch" << "val" << "arg");

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
    quint32 totalDuration = 10000;

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
    syntaxErrorsLines(); // TODO remove from here

    return true;
}

bool Script::appendData(const QString &str)
{
    //m_data.append(str + QString("\n"));
    m_data.append(tokenizeLine(str + QString("\n")));

    return true;
}

QString Script::data() const
{
    return m_data;
}

QStringList Script::dataLines() const
{
    QStringList result = m_data.split(QRegExp("(\r\n|\n\r|\r|\n)"), QString::KeepEmptyParts);

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
    ScriptRunner *runner = new ScriptRunner(doc(), m_data);
    return runner->syntaxErrorsLines();
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
                m_data.append(tokenizeLine(QUrl::fromPercentEncoding(root.readElementText().toUtf8()) + QString("\n")));
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
    foreach(QString cmd, dataLines())
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
    }

    m_runner = NULL;

    Function::postRun(timer, universes);
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
    return qrand() % ((max + 1) - min) + min;
}

QString Script::tokenizeLine(const QString& str, bool *ok)
{
    QStringList tokens;
    QString comment;
    QString keyword;
    int keywordsFound = 0;
    QString value;

    if (ok != NULL)
        *ok = true; // in case, this is set to false afterwards

    if (str.simplified().startsWith("//") == true || str.simplified().isEmpty() == true)
        return str;

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
            keywordsFound++;
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
                value = line.mid(quoteleft - 1, quoteright - quoteleft + 1);
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
        {
            if (value.startsWith("random"))
                value.replace("random", "Engine.random");

            if (keywordsFound == 1)
                tokens << keyword.trimmed() << value.trimmed();
            else
                tokens << value.trimmed();
        }
    }

    line = convertLegacyMethod(tokens.first());
    line.append("(");
    for (int i = 1; i < tokens.count(); i++)
    {
        line.append(tokens.at(i));
        if (i < tokens.count() - 1)
            line.append(",");
    }

    line.append("); ");
    line.append(comment);

    qDebug() << "Tokens:" << tokens;

    return line;
}

QString Script::convertLegacyMethod(QString method)
{
    if (method == startFunctionCmd) return "Engine.startFunction";
    else if (method == stopFunctionCmd) return "Engine.stopFunction";
    else if (method == blackoutCmd) return "Engine.setBlackout";
    else if (method == waitCmd) return "Engine.waitTime";
    else if (method == setFixtureCmd) return "Engine.setFixture";
    else if (method == systemCmd) return "Engine.systemCommand";
    else return "";
}

