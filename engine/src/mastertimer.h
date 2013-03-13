/*
  Q Light Controller
  mastertimer.h

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

#ifndef MASTERTIMER_H
#define MASTERTIMER_H

#include <QObject>
#include <QMutex>
#include <QList>
#include <QTime>

class MasterTimerPrivate;
class UniverseArray;
class GenericFader;
class OutputMap;
class DMXSource;
class Function;
class Doc;

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
    /** An OutputMap instance that routes all values to correct plugins. */
    static const uint s_frequency;

    /*********************************************************************
     * Functions
     *********************************************************************/
public:
    /** Start running the given function */
    virtual void startFunction(Function* function);

    /** Stop all functions. Doesn't affect registered DMX sources. */
    void stopAllFunctions();

    /** Get the number of currently running functions */
    int runningFunctions() const;

signals:
    /** Tells that the list of running functions has changed */
    void functionListChanged();

private:
    /** Execute one timer tick for each registered Function */
    void timerTickFunctions(UniverseArray* universes);

private:
    /** List of currently running functions */
    QList <Function*> m_functionList;
    QList <Function*> m_startQueue;

    /** Mutex that guards access to m_functionList */
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
    virtual void registerDMXSource(DMXSource* source);

    /**
     * Unregister a previously registered DMXSource. This should be called
     * in the DMXSource's destructor (at the latest).
     *
     * @param source The DMXSource to unregister
     */
    virtual void unregisterDMXSource(DMXSource* source);

private:
    /** Execute one timer tick for each registered DMXSource */
    void timerTickDMXSources(UniverseArray* universes);

private:
    /** List of currently running functions */
    QList <DMXSource*> m_dmxSourceList;

    /** Mutex that guards access to m_functionList */
    QMutex m_dmxSourceListMutex;

    /*************************************************************************
     * Generic Fader
     *************************************************************************/
public:
    /**
     * Get a pointer to the MasterTimer's GenericFader. The pointer must not be
     * deleted. The fader can be used e.g. by Scene functions to gracefully fade
     * down such intensity (HTP) channels that are no longer in use.
     */
    GenericFader* fader() const;

private:
    /** Execute one timer tick for the GenericFader */
    void timerTickFader(UniverseArray* universes);

private:
    GenericFader* m_fader;

private:
    MasterTimerPrivate* d_ptr;
};

#endif
