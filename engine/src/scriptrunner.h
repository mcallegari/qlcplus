/*
  Q Light Controller Plus
  scriptrunner.h

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

#ifndef SCRIPTRUNNER_H
#define SCRIPTRUNNER_H

#include <QThread>
#include <QQueue>
#include <QPair>

class GenericFader;
class MasterTimer;
class QJSEngine;
class QJSValue;
class Universe;
class Doc;

class ScriptRunner : public QThread
{
    Q_OBJECT

    /************************************************************************
     * Initialization
     ************************************************************************/
public:
    ScriptRunner(Doc *doc, QString &content, QObject *parent = 0);
    ~ScriptRunner();

    /** Start the thread execution and therefore the JavaScript code */
    void execute();

    void stop();

    QList<int> syntaxErrorsLines();

    /** Get the script's GenericFader (and create it if necessary) */
    GenericFader *fader();

    bool write(MasterTimer *timer, QList<Universe*> universes);

    /************************************************************************
     * JS exported methods
     ************************************************************************/
public slots:
    /**
     * Handle "setFixture" command.
     *
     * @param fxID The Fixture ID
     * @param channel The fixture channel to control
     * @param value the DMX value to set
     * @param time (optional) fade time to reach the requested DMX value
     * @return true if successful. False on error.
     */
    bool setFixture(quint32 fxID, quint32 channel, uchar value, uint time = 0);

    /**
     * Handle "stopFunction" command.
     *
     * @param fID The Function ID
     * @return true if successful. False on error.
     */
    bool startFunction(quint32 fID);

    /**
     * Handle "stopFunction" command.
     *
     * @param fID The Function ID
     * @return true if successful. False on error.
     */
    bool stopFunction(quint32 fID);

    /**
     * Handle "systemCommand" command.
     *
     * @param command The command to execute
     * @return true if successful. False on error.
     */
    bool systemCommand(QString command);

    /**
     * Handle "waitTime" command.
     *
     * @param ms The time in milliseconds to wait
     * @return true if successful. False on error.
     */
    bool waitTime(uint ms);

    /**
     * Handle "waitTime" command.
     *
     * @param time The time as string (e.g. 2s.140) to wait
     * @return true if successful. False on error.
     */
    bool waitTime(QString time);

    /**
     * Handle "setBlackout" command.
     *
     * @param enable If true, requests blackout, otherwise release blackout
     * @return true if successful. False on error.
     */
    bool setBlackout(bool enable);

    /**
     * Handle "random" command.
     *
     * @param minTime Minimum time, expressed as QLC+ styled string
     * @param maxTime Maximum time, expressed as QLC+ styled string
     * @return true if successful. False on error.
     */
    int random(QString minTime, QString maxTime);

    int random(int minTime, int maxTime);

protected:
    /** QThread reimplemented method */
    void run();

private:
    Doc *m_doc;
    QString m_content;
    bool m_running;

    QJSEngine *m_engine;
    // Queue holding the Function IDs to start/stop
    QQueue<QPair<quint32,bool>> m_functionQueue;
    // IDs of the Functions started by this script
    QList <quint32> m_startedFunctions;
    // Timer ticks to wait before executing the next line
    quint32 m_waitCount;
    GenericFader *m_fader;
};

#endif
