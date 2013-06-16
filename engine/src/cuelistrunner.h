/*
  Q Light Controller
  cuelistrunner.h

  Copyright (c) Heikki Junnila

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

#ifndef CUELISTRUNNER_H
#define CUELISTUNNER_H

#include <QList>
#include <QMap>

#include "function.h"

class UniverseArray;
class FadeChannel;
class ChaserStep;
class Function;
class Chaser;
class QTime;
class Doc;

typedef struct
{
    int m_index;          //! Index of the step from the original Chaser
    Function* m_function; //! Currently active function
    quint32 m_elapsed;    //! Elapsed milliseconds
    uint m_fadeIn;        //! Step fade In speed
    uint m_fadeOut;       //! Step fade Out speed
    uint m_duration;      //! Step duration
} CueRunnerStep;

class CueListRunner : public QObject
{
    Q_OBJECT

public:
    CueListRunner(const Doc* doc, const Chaser* chaser);
    ~CueListRunner();

private slots:
    void slotChaserChanged();

private:
    const Doc* m_doc;
    const Chaser* m_chaser;

    /************************************************************************
     * Speed
     ************************************************************************/
public:
    /** Get the currently active fade in value (See Chaser::SpeedMode) */
    uint stepFadeIn(int stepIdx) const;

    /** Get the currently active fade out value (See Chaser::SpeedMode) */
    uint stepFadeOut(int stepIdx) const;

    /** Get the currently active duration value (See Chaser::SpeedMode) */
    uint stepDuration(int stepIdx) const;

private:
    bool m_updateOverrideSpeeds;

    /************************************************************************
     * Step control
     ************************************************************************/
public:
    /**
     * Skip to the next scene, obeying direction and run order settings.
     */
    void next();

    /**
     * Skip to the previous scene, obeying direction and run order settings.
     */
    void previous();

    /**
     * Produce a tap event to the runner, possibly producing a next() call.
     */
    void tap();

    /**
     * Set the NEW current step number. The value of m_currentStep is changed
     * on the next call to write().
     *
     * @param step Step number to set
     */
    void setCurrentStep(int step);

    /**
     * Get the current step number.
     *
     * @return Current step number
     */
    int currentStep() const;

    /**
     * Get the current step number.
     *
     * @return Current step number
     */
    int runningStepsNumber() const;

signals:
    /** Tells that the current step number has changed. */
    void currentStepChanged(int stepNumber);

private:
    Function::Direction m_direction;        //! Run-time direction (reversed by ping-pong)
    QList <CueRunnerStep *> m_runnerSteps;  //! Queue of the currently running steps
    bool m_next;                            //! If true, skips to the next step when write is called
    bool m_previous;                        //! If true, skips to the previous step when write is called
    int m_newStartStepIdx;                  //! Manually set the start step
    int m_lastRunStepIdx;                   //! Index of the last step ran
    QTime* m_roundTime;                     //! Counts the time between steps

    /************************************************************************
     * Intensity
     ************************************************************************/
public:
    /**
     * Adjust the intensities of chaser steps.
     */
    void adjustIntensity(qreal fraction, int stepIndex);

private:
    qreal m_intensity;

    /************************************************************************
     * Running
     ************************************************************************/
private:
    void clearRunningList();
    void startNewStep(int index, MasterTimer *timer, bool manualFade);

    int getNextStepIndex();

public:
    /**
     * Call this from the parent function's write() method to run the steps.
     *
     * @param timer The MasterTimer that runs the show
     * @param universes DMX address space
     * @return true if the chaser should continue, otherwise false
     */
    bool write(MasterTimer* timer, UniverseArray* universes);

    /**
     * Perform postRun operations. Call this from the parent function's postRun().
     *
     * @param timer The MasterTimer that runs the show
     * @param universes DMX address space
     */
    void postRun(MasterTimer* timer, UniverseArray* universes);
};

#endif
