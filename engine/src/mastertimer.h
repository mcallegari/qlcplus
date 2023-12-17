/*
  Q Light Controller Plus
  mastertimer.h

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

#ifndef MASTERTIMER_H
#define MASTERTIMER_H

#include <QHash>
#include <QObject>
#include <QMutex>
#include <QList>

class MasterTimerPrivate;
class QElapsedTimer;
class GenericFader;
class FadeChannel;
class DMXSource;
class Function;
class Universe;
class Doc;

/** @addtogroup engine Engine
 * @{
 */

class MasterTimer : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(MasterTimer)

    friend class MasterTimerPrivate;

    /*************************************************************************
     * Initialization
     *************************************************************************/
public:
    /**
     * Create a new MasterTimer instance. MasterTimer takes care of running
     * functions and driving internal DMX universe dumping to output plugins.
     *
     * @param parent The parent Doc that owns the instance
     */
    MasterTimer(Doc* doc);

    /** Destroy a MasterTimer instance */
    virtual ~MasterTimer();

    /** Start the MasterTimer */
    void start();

    /** Stop the MasterTimer */
    void stop();

    /** Get the timer tick frequency in Hertz */
    static uint frequency();

    /** Get the length of one timer tick in milliseconds */
    static uint tick();

signals:
    void tickReady();

private:
    /** Execute one timer tick (called by MasterTimerPrivate) */
    void timerTick();

private:
    /** The timer tick frequency in Hertz */
    static uint s_frequency;

    /** Duration in milliseconds of a single tick */
    static uint s_tick;

    /** The private reference to a MasterTimer platform dependent implementation */
    MasterTimerPrivate* d_ptr;

    /*********************************************************************
     * Functions
     *********************************************************************/
public:
    /** Start the given function */
    /** This should be called by the function itself */
    virtual void startFunction(Function* function);

    /** Stop all functions. Doesn't affect registered DMX sources. */
    void stopAllFunctions();

    /** Fade all functions for a given time and then stop them all */
    void fadeAndStopAll(int timeout);

    /** Get the number of currently running functions */
    int runningFunctions() const;

signals:
    /** Tells that the list of running functions has changed */
    void functionListChanged();

    /** Emitted when a Function is started */
    void functionStarted(quint32 id);

    /** Emitted when a Function has just been stopped */
    void functionStopped(quint32 id);

private:
    /** Execute one timer tick for each registered Function */
    void timerTickFunctions(QList<Universe *> universes);

private:
    /** List of currently running functions */
    QList <Function*> m_functionList;
    QList <Function*> m_startQueue;

    /** Mutex that guards access to m_startQueue */
    QMutex m_functionListMutex;

    /** Flag for stopping all functions */
    bool m_stopAllFunctions;

    /*************************************************************************
     * DMX Sources
     *************************************************************************/
public:
    /**
     * Register a DMXSource for additional DMX data output (sliders and
     * other directly user-controlled gadgets). Each DMXSource instance
     * can be registered exactly once.
     *
     * @param source The DMXSource to register
     */
    virtual void registerDMXSource(DMXSource *source);

    /**
     * Unregister a previously registered DMXSource. This should be called
     * in the DMXSource's destructor (at the latest).
     *
     * @param source The DMXSource to unregister
     */
    virtual void unregisterDMXSource(DMXSource *source);

private:
    /** Execute one timer tick for each registered DMXSource */
    void timerTickDMXSources(QList<Universe *> universes);

private:
    /** List of currently registered DMX sources */
    QList <DMXSource*> m_dmxSourceList;

    /** Mutex that guards access to m_dmxSourceList
     *
     * In case both m_functionListMutex and m_dmxSourceListMutex are needed,
     * always lock m_functionListMutex first!
     */
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    QMutex m_dmxSourceListMutex;
#else
    QRecursiveMutex m_dmxSourceListMutex;
#endif

    /*************************************************************************
     * Beats generation
     *************************************************************************/
public:
    enum BeatsSourceType
    {
        None,       //! MasterTimer won't bother to perform any beat generation
        Internal,   //! MasterTimer will be the beat generator
        External    //! Beats requests will come from an external source,
                    //! like MIDI beat clock or AudioInput BPM detection
    };

    /** Set the type of source for beats generation. */
    void setBeatSourceType(BeatsSourceType type);

    /** Get the current type of beat generation source */
    BeatsSourceType beatSourceType() const;

    /** Requests a $bpm number to be generated by MasterTimer if m_beatSourceType is "Internal",
     *  otherwise informs MasterTimer about how many $bpm have been detected by an external generator */
    void requestBpmNumber(int bpm);

    /** Get the current number of BPM under MasterTimer control */
    int bpmNumber() const;

    /** Get the duration of a beat in milliseconds */
    int beatTimeDuration() const;

    /** Get the time in milliseconds to the next beat */
    int timeToNextBeat() const;

    /** Get a positive or negative time offset in milliseconds that
     *  indicates how close the current timer tick is from a beat.
     *
     *  Example: a Scene with 1 beat fade in, at 120BPM (= 500ms)
     *  If the Scene is started 60ms after a beat, then this method returns 60,
     *  so the fade in should be 440ms long.
     *  If the Scene is started 380ms after a beat, then this method returns -120,
     *  so the fade in should be 620ms long.
     */
    int nextBeatTimeOffset() const;

    /** Return true if the current tick is also a beat, otherwise false */
    bool isBeat() const;

    /** Request MasterTimer to generate a beat. This is typically used by
     *  external beat detectors/generators. Note that this is a request, and
     *  not an immediate beat generation, since MasterTimer still works with ticks
     *  so the requested beat will happen in the worst case after a s_tick time, so typically 20ms
     *  unless otherwise specified by MASTERTIMER_FREQUENCY.
     *  This is quite safe cause even 300bpm should happen every 200ms. */
    void requestBeat();

signals:
    void bpmNumberChanged(int bpm);
    void beat();

private:
    /** The current type of beat source */
    BeatsSourceType m_beatSourceType;
    /** The current number of beats per minute emitted by a generator */
    int m_currentBPM;
    /** The duration of a beat in milliseconds according to m_currentBPM */
    int m_beatTimeDuration;
    /** Flag to request a beat generation at the next MasterTimer tick */
    bool m_beatRequested;
    /** The reference of a platform dependent timer to measure precise elapsed time */
    QElapsedTimer *m_beatTimer;
    /** Time offset in milliseconds when the last beat occured */
    int m_lastBeatOffset;
};

/** @} */

#endif
