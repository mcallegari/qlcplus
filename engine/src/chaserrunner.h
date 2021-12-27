/*
  Q Light Controller Plus
  chaserrunner.h

  Copyright (c) Heikki Junnila
                Massimo Callegari
                Jano Svitok

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
#include "chaseraction.h"

class QElapsedTimer;
class FadeChannel;
class ChaserStep;
class Function;
class Universe;
class Chaser;
class Doc;

/** @addtogroup engine_functions Functions
 * @{
 */

typedef struct
{
    int m_index;                        //! Index of the step from the original Chaser
    Function *m_function;               //! Currently active function
    quint32 m_elapsed;                  //! Elapsed milliseconds
    quint32 m_elapsedBeats;             //! Elapsed beats
    uint m_fadeIn;                      //! Step fade in in ms
    uint m_fadeOut;                     //! Step fade out in ms
    uint m_duration;                    //! Step hold in ms
    Universe::BlendMode m_blendMode;    //! The original Function blend mode
    int m_intensityOverrideId;          //! An ID to control the step intensity
    int m_pIntensityOverrideId;         //! An ID to control the step parent intensity
} ChaserRunnerStep;

class ChaserRunner : public QObject
{
    Q_OBJECT

public:
    ChaserRunner(const Doc* doc, const Chaser* chaser, quint32 startTime = 0);
    ~ChaserRunner();

private slots:
    void slotChaserChanged();

private:
    const Doc *m_doc;
    const Chaser *m_chaser;

    /************************************************************************
     * Speed
     ************************************************************************/
private:
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
    void setAction(ChaserAction &action);

    /**
     * Produce a tap event to the runner, possibly producing a next() call.
     */
    void tap();

    /**
     * Get the current step number.
     *
     * @return Current step number
     */
    int currentStepIndex() const;

    /**
     * Compute next step for manual fading
     */
    int computeNextStep(int currentStepIndex) const;

    /**
     * Get the running step number.
     *
     * @return Running step number
     */
    int runningStepsNumber() const;

    /**
     * Get the first step of the running list.
     * If none is running this returns NULL
     */
    ChaserRunnerStep *currentRunningStep() const;

private:
    /**
     * Shuffle the current steps order
     */
    static void shuffle(QVector<int> & data);

    /**
     * Retrieve the randomized index of a
     * step at the given index
     */
    int randomStepIndex(int step) const;

    /**
     * Convenience function to fill the m_oder
     * array with all the steps indices
     */
    void fillOrder();

    /**
     * Fill the m_order array with the step indices
     * and the given size
     */
    void fillOrder(int size);

signals:
    /** Tells that the current step number has changed. */
    void currentStepChanged(int stepNumber);

private:
    Function::Direction m_direction;        //! Run-time direction (reversed by ping-pong)
    QList <ChaserRunnerStep *> m_runnerSteps;  //! Queue of the currently running steps
    quint32 m_startOffset;                  //! Start step offset time in milliseconds
    ChaserAction m_pendingAction;           //! Action to be performed on steps at the next write call
    int m_lastRunStepIdx;                   //! Index of the last step ran
    quint32 m_lastFunctionID;               //! ID of the last Function ran (Scene only)
    QElapsedTimer *m_roundTime;             //! Counts the time between steps
    QVector<int> m_order;                   //! Array of step indices in a randomized order

    /************************************************************************
     * Intensity
     ************************************************************************/
public:
    /** Adjust the intensities of chaser steps */
    void adjustStepIntensity(qreal fraction, int stepIndex = -1, int fadeControl = 0);

    /************************************************************************
     * Running
     ************************************************************************/
private:
    /**
     * Stop every running Function and clear the running list
     */
    void clearRunningList();

    /**
     * Start a Chaser step Function with the given $index, at the given
     * master and step intensity and from the given $elapsed time.
     * $fadeControl specifies how the step Function should fade, according to
     * the Chaser::FadeControlMode enumeration:
     * - Chaser::FromFunction will use the original Function fadeIn time
     * - Chaser::Crossfade means that the user is manually overriding the Function intensity with a slider
     *                     so the Function fadeIn time will be set to 0
     * - Chaser::LinkedCrossfade is like Crossfade, and the Function will also be requested
     *                           to use the Universe::AdditiveBlend mode
     */
    void startNewStep(int index, MasterTimer *timer, qreal mIntensity, qreal sIntensity,
                      int fadeControl, quint32 elapsed = 0);

    /**
     * Get the index of the next step that should be started,
     * according to the Chaser running order
     */
    int getNextStepIndex();

private:
    FunctionParent functionParent() const;

public:
    /**
     * Call this from the parent function's write() method to run the steps.
     *
     * @param timer The MasterTimer that runs the show
     * @param universes DMX address space
     * @return true if the chaser should continue, otherwise false
     */
    bool write(MasterTimer* timer, QList<Universe*> universes);

    /** If running, pauses the runner and all the current running steps. */
    void setPause(bool enable, QList<Universe *> universes);

    /**
     * Perform postRun operations. Call this from the parent function's postRun().
     *
     * @param timer The MasterTimer that runs the show
     * @param universes DMX address space
     */
    void postRun(MasterTimer* timer, QList<Universe *> universes);
};

/** @} */

#endif
