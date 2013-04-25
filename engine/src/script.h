/*
  Q Light Controller
  script.h

  Copyright (C) Heikki Junnila

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  Version 2 as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details. The license is
  in the file "COPYING".

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef SCRIPT_H
#define SCRIPT_H

#include <QStringList>
#include <QObject>
#include <QMap>
#include "function.h"

class UniverseArray;
class GenericFader;
class MasterTimer;
class Doc;

class Script : public Function
{
    Q_OBJECT

    /************************************************************************
     * Script keywords
     ************************************************************************/
public:
    static const QString startFunctionCmd;
    static const QString stopFunctionCmd;

    static const QString waitCmd;
    static const QString waitKeyCmd;

    static const QString setFixtureCmd;

    static const QString labelCmd;
    static const QString jumpCmd;

    /************************************************************************
     * Initialization
     ************************************************************************/
public:
    Script(Doc* doc);
    virtual ~Script();

    /************************************************************************
     * Copying
     ************************************************************************/
public:
    /** @reimpl */
    Function* createCopy(Doc* doc, bool addToDoc = true);

    /** @reimpl */
    bool copyFrom(const Function* function);

    /************************************************************************
     * Script data
     ************************************************************************/
public:
    /** Set the raw script data */
    bool setData(const QString& str);

    /** Get the raw script data */
    QString data() const;

private:
    QString m_data;

    /************************************************************************
     * Load & Save
     ************************************************************************/
public:
    /** @reimpl */
    bool loadXML(const QDomElement& root);

    /** @reimpl */
    bool saveXML(QDomDocument* doc, QDomElement* root);

    /************************************************************************
     * Running
     ************************************************************************/
public:
    /** @reimpl */
    void preRun(MasterTimer* timer);

    /** @reimpl */
    void write(MasterTimer* timer, UniverseArray* universes);

    /** @reimpl */
    void postRun(MasterTimer* timer, UniverseArray* universes);

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
    bool executeCommand(int index, MasterTimer* timer, UniverseArray* universes);

    /**
     * Check, if the script should still wait or if it should proceed to executing
     * the next command.
     *
     * @return true if no longer waiting, false if script is waiting
     */
    bool waiting();

    /**
     * Handle "startfunction" command.
     *
     * @param command The first keyword:value pair
     * @param timer The MasterTimer that should run the function
     * @return An empty string if successful. Otherwise an error string.
     */
    QString handleStartFunction(const QList<QStringList>& tokens, MasterTimer* timer);

    /**
     * Handle "stopfunction" command.
     *
     * @param command The first keyword:value pair
     * @return An empty string if successful. Otherwise an error string.
     */
    QString handleStopFunction(const QList<QStringList>& tokens);

    /**
     * Handle "wait" command.
     *
     * @param command The first keyword:value pair
     * @return An empty string if successful. Otherwise an error string.
     */
    QString handleWait(const QList<QStringList>& tokens);

    /**
     * Handle "waitkey" command.
     *
     * @param command The first keyword:value pair
     * @return An empty string if successful. Otherwise an error string.
     */
    QString handleWaitKey(const QList<QStringList>& tokens);

    /**
     * Handle "setfixture" command.
     *
     * @param command The first keyword:value pair
     * @param tokens All keyword:value pairs (including the first one)
     * @param universes The universe array to write DMX data
     * @return An empty string if successful. Otherwise an error string.
     */
    QString handleSetFixture(const QList<QStringList>& tokens, UniverseArray* universes);

    /**
     * Handle "label" command.
     *
     * @param command The first keyword:value pair
     * @return An empty string if successful. Otherwise an error string.
     */
    QString handleLabel(const QList<QStringList>& tokens);

    /**
     * Handle "jump" command.
     *
     * @param command The first keyword:value pair
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

    /** Get the script's GenericFader (and create it if necessary) */
    GenericFader* fader();

private:
    int m_currentCommand;        //! Current command line being handled
    quint32 m_waitCount;         //! Timer ticks to wait before executing the next line
    QList <QList<QStringList> > m_lines; //! Raw data parsed into lines of tokens
    QMap <QString,int> m_labels; //! Labels and their line numbers
    QList <Function*> m_startedFunctions; //! Functions started by this script

    GenericFader* m_fader;
};

#endif
