/*
  Q Light Controller
  mastertimer.h

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

#ifndef MASTERTIMER_H
#define MASTERTIMER_H

#include <QHash>
#include <QObject>
#include <QMutex>
#include <QList>
#include <QTime>

class MasterTimerPrivate;
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

private:
    /** Execute one timer tick (called by MasterTimerPrivate) */
    void timerTick();

private:
    /** The timer tick frequency in Hertz */
    static uint s_frequency;

    /** Duration in milliseconds of a single tick */
    static uint s_tick;

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
    virtual void registerDMXSource(DMXSource* source, QString name);

    /**
     * Unregister a previously registered DMXSource. This should be called
     * in the DMXSource's destructor (at the latest).
     *
     * @param source The DMXSource to unregister
     */
    virtual void unregisterDMXSource(DMXSource* source);

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
    QMutex m_dmxSourceListMutex;
    bool m_simpleDeskRegistered;

    /*************************************************************************
     * Generic Fader
     *************************************************************************/
private:
    /**
     * Get a pointer to the MasterTimer's GenericFader. The pointer must not be
     * deleted. The fader can be used e.g. by Scene functions to gracefully fade
     * down such intensity (HTP) channels that are no longer in use.
     */
    GenericFader* fader() const;
public:
    void faderAdd(const FadeChannel& ch);
    void faderForceAdd(const FadeChannel& ch);
    QHash<FadeChannel,FadeChannel> faderChannels() const;
    QHash<FadeChannel,FadeChannel> const& faderChannelsRef() const;
    QMutex* faderMutex() const;

private:
    /** Execute one timer tick for the GenericFader */
    void timerTickFader(QList<Universe *> universes);

private:
    QMutex m_faderMutex;
    GenericFader* m_fader;

private:
    MasterTimerPrivate* d_ptr;
};

/** @} */

#endif
