/*
  Q Light Controller
  cuelistrunner.h

  Copyright (c) Heikki Junnila

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

#ifndef CUELISTRUNNER_H
#define CUELISTUNNER_H

#include <QList>
#include <QMap>

#include "function.h"

class FadeChannel;
class ChaserStep;
class Function;
class Universe;
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
     * Stop a specific runnign step
     * @param stepIndex Index of the running step to stop
     */
    void stopStep(int stepIndex);

    /**
     * Set the NEW current step number. The value of m_currentStep is changed
     * on the next call to write().
     *
     * @param step Step number to set
     * @param intensity Optional startup intensity
     */
    void setCurrentStep(int step, qreal intensity = 1.0);

    /**
     * Get the current step number.
     *
     * @return Current step number
     */
    int currentStep() const;

    /**
     * Get the running step number.
     *
     * @return Running step number
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
    bool write(MasterTimer* timer, QList<Universe*> universes);

    /**
     * Perform postRun operations. Call this from the parent function's postRun().
     *
     * @param timer The MasterTimer that runs the show
     * @param universes DMX address space
     */
    void postRun(MasterTimer* timer, QList<Universe *> universes);
};

#endif
