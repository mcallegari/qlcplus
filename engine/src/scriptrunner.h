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
#include <QMap>

class GenericFader;
class MasterTimer;
class QJSEngine;
class QJSValue;
class Universe;
class Doc;

typedef struct
{
    quint32 m_universe;
    quint32 m_fixtureID;
    quint32 m_channel;
    uchar m_value;
    uint m_fadeTime;
} FixtureValue;

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

    QStringList collectScriptData();

    int currentWaitTime();

    bool write(MasterTimer *timer, QList<Universe*> universes);

    /************************************************************************
     * JS exported methods
     ************************************************************************/
public slots:

    /**
     * Handle "getChannelValue" command
     *
     * @param universe The universe of the channel
     * @param channel The channel index starting from 0
     * @return the channel DMX value
     */
    int getChannelValue(int universe, int channel);

    /**
     * Handle "setFixture" command
     *
     * @param fxID The Fixture ID
     * @param channel The fixture channel to control
     * @param value the DMX value to set
     * @param time (optional) fade time to reach the requested DMX value
     * @return true if successful. False on error.
     */
    bool setFixture(quint32 fxID, quint32 channel, uchar value, uint time = 0);

    /**
     * Handle "startFunction" command
     *
     * @param fID The Function ID to start
     * @return true if successful. False on error.
     */
    bool startFunction(quint32 fID);

    /**
     * Handle "stopFunction" command
     *
     * @param fID The Function ID to stop
     * @return true if successful. False on error.
     */
    bool stopFunction(quint32 fID);

    /**
     * Handle "isFunctionRunning" command
     *
     * @param fID The Function ID to be checked
     * @return true if function is running, otherwise false
     */
    bool isFunctionRunning(quint32 fID);

    /**
     * Handle "getFunctionAttribute" command
     *
     * @param fID Function ID
     * @param attributeIndex Index of the requested attribute
     * @return the requested attribute value or 0
     */
    float getFunctionAttribute(quint32 fID, int attributeIndex);

    /**
     * Handle "setFunctionAttribute" command (int version)
     *
     * @param fID The Function ID to be set
     * @param attributeIndex Index of the attribute to be set
     * @param value Value of the attribute
     * @return true if successful. False on error.
     */
    bool setFunctionAttribute(quint32 fID, int attributeIndex, float value);

    /**
     * Handle "setFunctionAttribute" command (string version)
     *
     * @param fID The Function ID to be set
     * @param attributeName Name of the attribute to be set
     * @param value Value of the attribute
     * @return true if successful. False on error.
     */
    bool setFunctionAttribute(quint32 fID, QString attributeName, float value);

    /**
     * Handle "systemCommand" command
     *
     * @param command The command to execute
     * @return true if successful. False on error.
     */
    bool systemCommand(QString command);

    /**
     * Handle "waitTime" command (int version)
     *
     * @param ms The time in milliseconds to wait
     * @return true if successful. False on error.
     */
    bool waitTime(uint ms);

    /**
     * Handle "waitTime" command (string version)
     *
     * @param time The time as string (e.g. 2s.140) to wait
     * @return true if successful. False on error.
     */
    bool waitTime(QString time);

    /**
     * Handle "setBlackout" command
     *
     * @param enable If true, requests blackout, otherwise release blackout
     * @return true if successful. False on error.
     */
    bool setBlackout(bool enable);

    /**
     * Handle "random" command (string version)
     *
     * @param minTime Minimum time, expressed as QLC+ styled string
     * @param maxTime Maximum time, expressed as QLC+ styled string
     * @return true if successful. False on error.
     */
    int random(QString minTime, QString maxTime);

    /**
     * Handle "random" command (int version)
     *
     * @param minTime Minimum time in milliseconds
     * @param maxTime Maximum time in milliseconds
     * @return true if successful. False on error.
     */
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
    // Queue holding Fixture values to send to Universes
    QQueue<FixtureValue> m_fixtureValueQueue;
    // IDs of the Functions started by this script
    QList <quint32> m_startedFunctions;
    // Timer ticks to wait before executing the next line
    quint32 m_waitCount;
    // Map used to lookup a GenericFader instance for a Universe ID
    QMap<quint32, QSharedPointer<GenericFader> > m_fadersMap;
};

#endif
