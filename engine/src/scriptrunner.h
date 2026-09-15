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
#include <QHash>
#include <QSet>
#include <QMutex>
#include <QWaitCondition>
#include <QElapsedTimer>
#include <atomic>
#include <functional>
#include "function.h"

class GenericFader;
class MasterTimer;
class QJSEngine;
class QJSValue;
class Universe;
class Doc;
class Fixture;

typedef struct
{
    quint32 m_universe;
    quint32 m_fixtureID;
    quint32 m_channel;
    uchar m_value;
    uint m_fadeTime;
} FixtureValue;

class ScriptRunner final : public QThread
{
    Q_OBJECT

    /************************************************************************
     * Initialization
     ************************************************************************/
public:
    ScriptRunner(Doc *doc, const QString &content, QObject *parent = 0);
    ~ScriptRunner();

    /** Start the thread execution and therefore the JavaScript code */
    void execute();

    void stop();

    QStringList collectScriptData();

    int currentWaitTime() const;

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
     * Handle "stopOnExit" command
     *
     * @param value Indicate to add (true) or to not add (false) to the Functions started by this script
     * @return true if successful. False on error.
     */
    bool stopOnExit(bool value);

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
    float getFunctionAttribute(quint32 fID, int attributeIndex) const;

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
     * End user documentation: Engine.waitTick(ticks) - pauses the script
     * for a given number of MasterTimer engine ticks (realtime effects).
     * Defaults to 1 tick if unspecified.
     */
    bool waitTick(uint ticks = 1);

    /**
     * End user documentation: Engine.waitBeat(beats) - pauses the script
     * until the given number of beats occur according to QLC+'s BPM tracker.
     * Defaults to 1 beat if unspecified.
     */
    bool waitBeat(uint beats = 1);

    /**
     * Handle "waitFunctionStart" command (string version)
     *
     * @param fID The Function ID to wait for starting
     * @return true if successful. False on error.
     */
    bool waitFunctionStart(quint32 fID);

    /**
     * Handle "waitFunctionStop" command (string version)
     *
     * @param fID The Function ID to wait for completion
     * @return true if successful. False on error.
     */
    bool waitFunctionStop(quint32 fID);

    /**
     * Handle "setBlackout" command
     *
     * @param enable If true, requests blackout, otherwise release blackout
     * @return true if successful. False on error.
     */
    bool setBlackout(bool enable);

    /**
     * Set the BPM (beat per minute) number of the internal beat generator
     *
     * @param bpm the number of beats per minute requested
     * @return true if successful. False on error.
     */
    bool setBPM(int bpm);

    /**
     * End user documentation: Engine.getBPM() - returns the current number of
     * beats per minute tracked by QLC+. The counterpart to Engine.setBPM().
     */
    int getBPM();

    /**
     * End user documentation: Engine.debugLog(message) - prints message to
     * QLC+'s debug output (qDebug).
     */
    void debugLog(QString message);

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
    int random(uint minTime, uint maxTime);

protected slots:
    /** Triggered when the script's execution pauses to await the starting of a function */
    void slotWaitFunctionStarted(quint32 fid);

    /** Triggered when the script's execution pauses to await the completion of a function */
    void slotWaitFunctionStopped(quint32 fid);

    /** Triggered when the script's execution pauses to await the next beat (see waitBeat()) */
    void slotBeatOccurred();

protected:
    /** QThread reimplemented method */
    void run() override;

private:
    /** Common code to check if script is running and if function exists */
    Function* getFunctionIfRunning(quint32 fID) const;

    /** ScriptRunner function operations enum to handle start/stop/wait commands */
    enum FunctionOperation
    {
        START = 0,
        START_DONT_STOP,
        STOP,
        WAIT_START,
        WAIT_STOP
    };

    /** Common code to enqueue function */
    bool enqueueFunction(quint32 fID, FunctionOperation operation);

    /**
     * Checks if the global GC interval has elapsed and triggers a garbage
     * collection pass if one is not already running.
     */
    void maybeCollectGarbage();

    // Global timer to track time elapsed since the last GC pass across all wait calls.
    QElapsedTimer m_gcTimer;

    // Prevents recursive GC calls if collectGarbage() triggers unexpected event loop processing.
    bool m_gcRunning;

        /**
     * Blocks the calling (script-interpreter) thread until the provided
     * condition evaluates to false. Handles periodic GC throttling.
     */
    bool waitForCondition(std::function<bool()> isPending);

    /**
     * Blocks the calling (script-interpreter) thread until the queued
     * WAIT_START/WAIT_STOP request for fID has actually been resolved
     * by write() (which runs on the MasterTimer thread).
     */
    bool waitForFunctionOperation(quint32 fID, FunctionOperation operation);

    /**
     * Releases everything a run of the script may have allocated or claimed:
     * the QJSEngine, any Functions this script started, and any GenericFaders
     * it requested. Safe to call more than once.
     */
    void finishAndCleanUp();

    /**
     * Packs a (universe, fixture, channel) triple into a single 64-bit key
     * used to de-duplicate queued fixture values (see m_fixtureValueQueue
     * below). 16 bits for the universe, 24 for the fixture ID and 24 for
     * the channel is comfortably more range than any of these can
     * legitimately take in QLC+ today.
     */
    static quint64 fixtureValueKey(quint32 universe, quint32 fixtureID, quint32 channel);

    /**
     * Validates the fixture and channel, returning the Fixture pointer if valid,
     * or nullptr if invalid. Logs appropriate warnings.
     */
    Fixture* validateFixtureChannel(quint32 fxID, quint32 channel);

private:
    Doc *m_doc;
    QString m_content;

    // Whether the script is currently running. Read from JS-exported slots
    // and write(), and written from execute()/stop()/run(). std::atomic<bool>
    // prevents data races without the cost of taking a mutex lock in the
    // hot path of nearly every method.
    std::atomic<bool> m_running;

    QJSEngine *m_engine;
    // Queue holding the Function IDs to start/stop
    QQueue<QPair<quint32, FunctionOperation>> m_functionQueue;

    // Keying by (universe, fixture, channel) bounds the queue size by the
    // number of distinct channels touched between two ticks rather than by
    // how many times setFixture() was called.
    QHash<quint64, FixtureValue> m_fixtureValueQueue;

    // Indicate to add (true) or to not add (false) to the Functions started by this script
    bool m_stopOnExit;

    // QSet de-duplicates automatically, preventing leaks when scripts call
    // startFunction() repeatedly on the same Function ID.
    QSet<quint32> m_startedFunctions;

    // Timer ticks to wait before executing the next line
    quint32 m_waitCount;

    // Condition variable used by waitTime() to sleep until write() (on the
    // MasterTimer thread) decrements m_waitCount to zero, instead of
    // busy-polling in a tight sleep loop.
    QWaitCondition m_waitCondition;

    // ID of the function that the script is waiting for
    quint32 m_waitFunctionId;

    // True while the script is blocked inside waitBeat(), waiting for
    // InputOutputMap::beat() to fire.
    bool m_waitingForBeat;

    // Map used to lookup a GenericFader instance for a Universe ID
    QMap<quint32, QSharedPointer<GenericFader> > m_fadersMap;

    // Guards every member above except m_running (which is std::atomic<bool>
    // - see above). These are written from the script's own thread (run(),
    // and any of the JS-exported slots above, which execute on that
    // thread), read and written from write() (which executes on the
    // MasterTimer thread), and also written from
    // slotWaitFunctionStarted/Stopped (which execute on whichever thread
    // this ScriptRunner object itself lives in). None of this was
    // synchronized before, which is a genuine data race on top of the
    // functional bugs fixed in later commits.
    mutable QMutex m_mutex;
};

#endif
