/*
  Q Light Controller Plus
  scriptv4.h

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

#ifndef SCRIPTV4_H
#define SCRIPTV4_H

#include <QStringList>
#include <QObject>
#include <QMap>

#include "script.h"

class ScriptRunner;
class MasterTimer;
class Universe;
class Doc;

/** @addtogroup engine_functions Functions
 * @{
 */

class ScriptV4 : public ScriptApi
{
    Q_OBJECT


    /************************************************************************
     * Initialization
     ************************************************************************/
public:
    ScriptV4(Doc* doc);
    virtual ~ScriptV4();

    /** @reimp */
    QIcon getIcon() const;

    /** @reimp */
    quint32 totalDuration();

    /************************************************************************
     * Copying
     ************************************************************************/
public:
    /** @reimp */
    Function* createCopy(Doc* doc, bool addToDoc = true);

    /** @reimp */
    bool copyFrom(const Function* function);

    /************************************************************************
     * Script data
     ************************************************************************/
public:
    /** Set the raw script data */
    bool setData(const QString& str);

    /** Append a line of script to the raw data */
    bool appendData(const QString& str);

    /** Get the raw script data */
    QString data() const;

    /** Get the script data lines as a list of  strings */
    QStringList dataLines() const;

    /** Convenience method to retrieve Functions used by this Script.
     *  The returned list is formatted as: Function ID / line number */
    QList<quint32> functionList() const;

    /** Convenience method to retrieve Fixtures used by this Script.
     *  The returned list is formatted as: Fixture ID / line number */
    QList<quint32> fixtureList() const;

    QList<int> syntaxErrorsLines();
    QStringList syntaxErrorsLinesString();

private:
    QString m_data;

    /************************************************************************
     * Load & Save
     ************************************************************************/
public:
    /** @reimp */
    bool loadXML(QXmlStreamReader &root);

    /** @reimp */
    bool saveXML(QXmlStreamWriter *doc);

    /************************************************************************
     * Running
     ************************************************************************/
public:
    /** @reimp */
    void preRun(MasterTimer *timer);

    /** @reimp */
    void write(MasterTimer *timer, QList<Universe*> universes);

    /** @reimp */
    void postRun(MasterTimer *timer, QList<Universe*> universes);

protected slots:
    void slotRunnerFinished();

private:
    /**
     * Parse a string in the form "random(min,max)" and returns
     * a randomized value between min and max
     *
     * @return the randomized value requested
     */
    static quint32 getValueFromString(QString str, bool *ok);

    /**
     * Parse one line of script data into a list of token string lists
     * QList(QStringList(keyword,value),QStringList(keyword,value),...)
     *
     * @param line The script line to parse
     * @param ok Tells if the line was parsed OK or not
     * @return A list of tokens parsed from the line
     */
    static QString convertLine(const QString& line, bool* ok = NULL);

    static QString convertLegacyMethod(QString method);

private:
    ScriptRunner *m_runner;
    QList <int> m_syntaxErrorLines;
};

/** @} */

#endif
