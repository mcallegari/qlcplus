/*
  Q Light Controller Plus
  script.h

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

#ifndef SCRIPT_H
#define SCRIPT_H

#include <QStringList>
#include <QObject>
#include <QMap>
#include "function.h"

class GenericFader;
class MasterTimer;
class Universe;
class Doc;

/** @addtogroup engine_functions Functions
 * @{
 */

class Script : public Function
{
    Q_OBJECT

    /************************************************************************
     * Script keywords
     ************************************************************************/
public:
    static const QString stopOnExitCmd;
    static const QString startFunctionCmd;
    static const QString stopFunctionCmd;
    static const QString blackoutCmd;

    static const QString waitCmd;
    static const QString waitKeyCmd;
    static const QString waitFunctionStartCmd;
    static const QString waitFunctionStopCmd;

    static const QString setFixtureCmd;
    static const QString systemCmd;

    static const QString labelCmd;
    static const QString jumpCmd;

    static const QString blackoutOn;
    static const QString blackoutOff;

    /************************************************************************
     * Initialization
     ************************************************************************/
public:
    Script(Doc* doc);
    virtual ~Script();

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

    /** Parse existing, tokenized lines for jump label definitions.
     * Used to be part of data() but was not called after loadXML,
     * so moved to a separate function */
    void scanForLabels();

    /** Get the script data lines as a list of  strings */
    QStringList dataLines() const;

    /** Convenience method to retrieve Functions used by this Script.
     *  The returned list is formatted as: Function ID / line number */
    QList<quint32> functionList() const;

    /** Convenience method to retrieve Fixtures used by this Script.
     *  The returned list is formatted as: Fixture ID / line number */
    QList<quint32> fixtureList() const;

    QList<int> syntaxErrorsLines();

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

private:
    /**
     * Execute one command from the given line number.
     *
     * @param index Line number to execute
     * @param timer The MasterTimer that runs the house
     * @param universes The universe array that governs DMX data
     * @return true to continue loop immediately, false to return control back
     *         to MasterTimer.
     */
    bool executeCommand(int index, MasterTimer *timer, QList<Universe*> universes);

    /**
     * Check, if the script should still wait or if it should proceed to executing
     * the next command.
     *
     * @return true if no longer waiting, false if script is waiting
     */
    bool waiting();

    /**
     * Parse a string in the form "random(min,max)" and returns
     * a randomized value between min and max
     *
     * @return the randomized value requested
     */
    static quint32 getValueFromString(QString str, bool *ok);

    /**
     * Handle "stoponexit" command.
     *
     * @param tokens A list of keyword:value pairs
     * @return An empty string if successful. Otherwise an error string.
     */
    QString handleStopOnExit(const QList<QStringList>& tokens);

    /**
     * Handle "startfunction" command.
     *
     * @param tokens A list of keyword:value pairs
     * @param timer The MasterTimer that should run the function
     * @return An empty string if successful. Otherwise an error string.
     */
    QString handleStartFunction(const QList<QStringList>& tokens, MasterTimer* timer);

    /**
     * Handle "stopfunction" command.
     *
     * @param tokens A list of keyword:value pairs
     * @return An empty string if successful. Otherwise an error string.
     */
    QString handleStopFunction(const QList<QStringList>& tokens);

    /**
     * Handle "blackout" command.
     *
     * @param tokens A list of keyword:value pairs
     * @return An empty string if successful. Otherwise an error string.
     */
    QString handleBlackout(const QList<QStringList>& tokens);

    /**
     * Handle "wait" command.
     *
     * @param tokens A list of keyword:value pairs
     * @return An empty string if successful. Otherwise an error string.
     */
    QString handleWait(const QList<QStringList>& tokens);

    /**
     * Handle "waitkey" command.
     *
     * @param tokens A list of keyword:value pairs
     * @return An empty string if successful. Otherwise an error string.
     */
    QString handleWaitKey(const QList<QStringList>& tokens);

    /**
     * Handle 'waitfunctionstart' and "waitfunctionstop" commands.
     *
     * @param tokens A list of keyword:value pairs
     * @param start Whether to wait for start or stop
     * @return An empty string if successful. Otherwise an error string.
     */
    QString handleWaitFunction(const QList<QStringList>& tokens, bool start);

    /**
     * Handle "setfixture" command.
     *
     * @param tokens A list of keyword:value pairs
     * @param universes The universe array to write DMX data
     * @return An empty string if successful. Otherwise an error string.
     */
    QString handleSetFixture(const QList<QStringList>& tokens, QList<Universe*> universes);

    /**
     * Handle "systemcommand" command.
     *
     * @param tokens A list of keyword:value pairs
     * @return An empty string if successful. Otherwise an error string.
     */
    QString handleSystemCommand(const QList<QStringList>& tokens);

    /**
     * Handle "label" command.
     *
     * @param tokens A list of keyword:value pairs
     * @return An empty string if successful. Otherwise an error string.
     */
    QString handleLabel(const QList<QStringList>& tokens);

    /**
     * Handle "jump" command.
     *
     * @param tokens A list of keyword:value pairs
     * @return An empty string if successful. Otherwise an error string.
     */
    QString handleJump(const QList<QStringList>& tokens);

    /**
     * Parse one line of script data into a list of token string lists
     * QList(QStringList(keyword,value),QStringList(keyword,value),...)
     *
     * @param line The script line to parse
     * @param ok Tells if the line was parsed OK or not
     * @return A list of tokens parsed from the line
     */
    static QList <QStringList> tokenizeLine(const QString& line, bool* ok = NULL);

protected slots:
    /** Triggered when the script's execution pauses to await the starting of a function */
    void slotWaitFunctionStarted(quint32 fid);

    /** Triggered when the script's execution pauses to await the completion of a function */
    void slotWaitFunctionStopped(quint32 fid);

private:
    bool m_stopOnExit;           //! add (true) or do not add (false) functions to m_startedFunctions
    int m_currentCommand;        //! Current command line being handled
    quint32 m_waitCount;         //! Timer ticks to wait before executing the next line
    Function* m_waitFunction;      //! Function to wait for before executing the next line
    QList < QList<QStringList> > m_lines; //! Raw data parsed into lines of tokens
    QMap <QString,int> m_labels; //! Labels and their line numbers
    QList <Function*> m_startedFunctions; //! Functions started by this script
    QList <int> m_syntaxErrorLines;

    GenericFader* m_fader;
};

/** @} */

#endif
