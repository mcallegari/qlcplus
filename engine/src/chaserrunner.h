/*
  Q Light Controller
  chaserrunner.h

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

#ifndef CHASERRUNNER_H
#define CHASERRUNNER_H

#include <QList>
#include <QMap>

#include "function.h"

class Universe;
class FadeChannel;
class ChaserStep;
class Function;
class Chaser;
class QTime;
class Doc;

class ChaserRunner : public QObject
{
    Q_OBJECT

public:
    ChaserRunner(const Doc* doc, const Chaser* chaser, quint32 startTime = 0);
    ~ChaserRunner();

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
    uint currentFadeIn() const;

    /** Get the currently active fade out value (See Chaser::SpeedMode) */
    uint currentFadeOut() const;

    /** Get the currently active duration value (See Chaser::SpeedMode) */
    uint currentDuration() const;

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
     * Reset the runner to a state where nothing has been run yet.
     */
    void reset();

signals:
    /** Tells that the current step number has changed. */
    void currentStepChanged(int stepNumber);

private:
    Function::Direction m_direction; //! Run-time direction (reversed by ping-pong)
    Function* m_currentFunction;     //! Currently active function
    quint32 m_elapsed;               //! Elapsed milliseconds
    quint32 m_startOffset;           //! Steps offset start time in milliseconds
    bool m_next;                     //! If true, skips to the next step when write is called
    bool m_previous;                 //! If true, skips to the previous step when write is called
    int m_currentStep;               //! Current step in m_steps
    int m_newCurrent;                //! Manually set the current step
    QTime* m_roundTime;              //! Counts the time between steps

    /************************************************************************
     * Intensity
     ************************************************************************/
public:
    /**
     * Adjust the intensities of chaser steps.
     */
    void adjustIntensity(qreal fraction);

private:
    qreal m_intensity;

    /************************************************************************
     * Running
     ************************************************************************/
public:
    /**
     * Call this from the parent function's write() method to run the steps.
     *
     * @param timer The MasterTimer that runs the show
     * @param universes DMX address space
     * @return true if the chaser should continue, otherwise false
     */
    bool write(MasterTimer* timer, QList<Universe *> universes);

    /**
     * Perform postRun operations. Call this from the parent function's postRun().
     *
     * @param timer The MasterTimer that runs the show
     * @param universes DMX address space
     */
    void postRun(MasterTimer* timer, QList<Universe *> universes);

private:
    /** Ran at each end of m_steps. Returns false only when SingleShot has been
        completed. */
    bool roundCheck();

    /**
     * Stop the previous function (if applicable) and start a new one (current).
     *
     * @param timer The MasterTimer that runs the functions
     */
    void switchFunctions(MasterTimer* timer);
};

#endif
