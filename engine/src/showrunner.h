/*
  Q Light Controller
  showrunner.h

  Copyright (c) Massimo Callegari

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

#ifndef SHOWRUNNER_H
#define SHOWRUNNER_H

#include <QObject>
#include <QMutex>
#include <QMap>
#include <QSet>

#include <function.h>
#include "tempomap.h"

class ShowFunction;
class Function;
class Track;
class Show;
class Doc;

/** @addtogroup engine_functions Functions
 * @{
 */

class ShowRunner final : public QObject
{
    Q_OBJECT

public:
    ShowRunner(const Doc *doc, quint32 showID, quint32 startTime = 0);
    ~ShowRunner();

    /** Start the runner */
    void start();

    /** If running, pauses the runner and all the current running functions. */
    void setPause(bool enable);

    /** Stop the runner */
    void stop();

    void write(MasterTimer *timer);

private:
    const Doc *m_doc;

    /** The reference of the show to play */
    Show* m_show;

    /** The list of time-based Functions the Show needs to play */
    QList <ShowFunction *> m_timeFunctions;

    /** Index of the item in m_timeFunctions to be considered for playback */
    int m_currentTimeFunctionIndex;

    /** Elapsed time since runner start. Used also to move the cursor in the track view */
    quint32 m_elapsedTime;

    /** The list of beat-based Functions the Show needs to play */
    QList <ShowFunction *> m_beatFunctions;

    /** Index of the item in m_beatFunctions to be considered for playback */
    int m_currentBeatFunctionIndex;

    /** Elapsed beats since runner start */
    quint32 m_elapsedBeats;

    /** Flag used to sinchronize playback to beats */
    bool beatSynced;

    /** m_elapsedTime at the moment beatSynced became true, and the ms
     *  equivalent of m_elapsedBeats at that same moment (i.e. the resumed
     *  beat position, if any). Used to derive a smoothly advancing, but
     *  beat-zeroed, cursor position for a BPM based Show (see write()) */
    quint32 m_syncElapsedTime;
    quint32 m_syncBeatsTime;

    /** Total time (in ms) the runner has to run, computed from m_timeFunctions */
    quint32 m_totalRunTime;

    /** Total time (in beats, expressed as ms, i.e. 1000 per beat) the
     *  runner has to run, computed from m_beatFunctions */
    quint32 m_totalRunBeats;

    /** A copy of the Show tempo map, when it drives the Show (see
     *  Show::itemsInMs()). All the items are then positioned in ms and
     *  Beats tempo Functions run on the tempo map beats, which follow the
     *  global BPM where there are no sections */
    bool m_tempoMapActive;
    TempoMap m_tempoMap;

    /** List of the currently running Functions and their stop time */
    QList < QPair<Function *, quint32> > m_runningQueue;

private:
    FunctionParent functionParent() const;

    /************************************************************************
     * Output hold
     ************************************************************************/
private:
    /** Start the Audio items due at m_elapsedTime ahead of everything else
     *  and hold the Show until they are actually heard (an audio device can
     *  take hundreds of ms to wake up). Returns true if a hold began */
    bool startOutputHold();

    /** Resume the Show after a hold */
    void releaseOutputHold();

    /** Returns true if any running Function is still waiting for its output */
    bool isWaitingForOutput() const;

    /** Apply the track intensity of $sf to its Function $f */
    void requestTrackIntensity(ShowFunction *sf, Function *f);

private:
    /** True while the Show is held, waiting for an output to start */
    bool m_outputHold = false;

    /** How long (in ms) the current hold has lasted */
    quint32 m_outputHoldTime = 0;

    /** Items started by startOutputHold(), to be skipped by write() */
    QSet<ShowFunction *> m_preStartedFunctions;

    /** Functions paused by startOutputHold(), resumed on release */
    QList<Function *> m_holdPausedFunctions;

signals:
    void timeChanged(quint32 time);
    void showFinished();

    /************************************************************************
     * Intensity
     ************************************************************************/
public:
    /**
     * Adjust the intensity of show track
     */
    void adjustIntensity(qreal fraction, const Track *track);

private:
    QMap<quint32, qreal> m_intensityMap;

};

/** @} */

#endif
