/*
  Q Light Controller
  chaser.h

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

#ifndef CHASER_H
#define CHASER_H

#include <QMutex>
#include <QColor>
#include <QList>

#include "function.h"
#include "scene.h"
#include "chaserrunner.h"

class QFile;
class QString;
class ChaserStep;
class MasterTimer;
class QDomDocument;

/** @addtogroup engine_functions Functions
 * @{
 */

/**
 * Chaser is a meta-function; it consists of other functions that are run in a
 * sequential order. Chaser contains information only on the running order,
 * direction and interval applied for its member functions. Therefore, a chaser
 * itself is nothing without other functions (more than usually, scenes) as its
 * members. If a member function's behaviour is changed, those changes are also
 * applied automatically to those chasers that hold the changed function(s) as
 * their members.
 */
class Chaser : public Function
{
    Q_OBJECT
    Q_DISABLE_COPY(Chaser)

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    Chaser(Doc* doc);
    virtual ~Chaser();

private:
    quint32 m_legacyHoldBus;

    /*********************************************************************
     * Copying
     *********************************************************************/
public:
    /** @reimpl */
    Function* createCopy(Doc* doc, bool addToDoc = true);

    /** Copy the contents for this function from another function */
    bool copyFrom(const Function* function);

    /*****************************************************************************
     * Sorting
     *****************************************************************************/
    /** Comparator function for qSort() */
    bool operator< (const Chaser& chs) const;

    /*********************************************************************
     * Chaser contents
     *********************************************************************/
public:
    /**
     * Add the given step to the chaser, either at the specified index, given
     * that 0 <= index < size or at the end if size < index < 0. The same
     * function can exist any number of times in a chaser. No checks for the
     * function's validity are made at this point, except that the chaser's own
     * ID cannot be added (i.e. a chaser cannot be its own direct member).
     *
     * @param step The step to add
     * @param index Insertion point. -1 to append.
     */
    bool addStep(const ChaserStep& step, int index = -1);

    /**
     * Remove a function from the given step index. If the given index is
     * out of bounds, this returns false.
     *
     * @param index The index number to remove
     * @return true if successful, otherwise false
     */
    bool removeStep(int index);

    /**
     * Replace the step at the given $index with the new $step.
     *
     * @param step The new step to replace with
     * @param index The index of the step to replace
     * @return true if successful, otherwise false (index out of bounds)
     */
    bool replaceStep(const ChaserStep& step, int index);

    /**
     * Move a step from $sourceIdx to $destIdx
     *
     * @param sourceIdx Source index of the step
     * @param destIdx Destination index of the step
     * @return true if successful, otherwise false (index out of bounds or dest == source)
     */
    bool moveStep(int sourceIdx, int destIdx);

    /**
     * Clear the chaser's list of steps
     */
    void clear();

    /** Get the Chaser steps number */
    int stepsCount();

    /**
     * Get a chaser step from a given index
     *
     * @return The requested Chaser Step
     */
    ChaserStep stepAt(int idx);

    /**
     * Get the chaser's list of steps
     *
     * @return List of function Chaser Steps
     */
    QList <ChaserStep> steps() const;

    /** Set the Chaser total duration in milliseconds */
    void setTotalDuration(quint32 msec);

    /** Get the Chaser total duration in milliseconds */
    quint32 totalDuration();

public slots:
    /**
     * Catches Doc::functionRemoved() so that destroyed members can be
     * removed immediately. This method removes all occurrences of the
     * given function ID, while removeStep() only removes one function ID
     * at the given index.
     *
     * @param fid The ID of the function that was removed
     */
    void slotFunctionRemoved(quint32 fid);

private:
    QList <ChaserStep> m_steps;
    QMutex m_stepListMutex;

    /*********************************************************************
     * Sequence mode
     *********************************************************************/
public:
    /**
     * Set this chaser to behave like a sequence and be a child of a scene
     * @param sceneID The ID of the scene to bound
     *
     */
    void enableSequenceMode(quint32 sceneID);

    /**
     * Returns if a chaser is a sequence or not
     *
     * @return The sequence flag
     */
    bool isSequence() const;

    /**
     * Returns the current bound scene ID
     *
     * @return The associated Scene for this Chaser in sequence mode
     */
    quint32 getBoundSceneID() const;

    /**
     * Set the time where the Chaser is placed over a timeline
     *
     * @param time The start time in milliseconds of the Chaser
     */
    void setStartTime(quint32 time);

    /**
     * Returns the time where the Chaser is placed over a timeline
     *
     * @return Start time in milliseconds of the Chaser
     */
    quint32 getStartTime() const;

    /**
     * Set the color to be used by a SequenceItem
     */
    void setColor(QColor color);

    /**
     * Get the color of this sequence
     */
    QColor getColor();

    /** Set the lock state of the item */
    void setLocked(bool locked);

    /** Get the lock state of the item */
    bool isLocked();

private:
    /** This Chaser is a Sequence that uses always the same Scene for each step */
    bool m_isSequence;
    /** The associated Scene of this Chaser when acting like a Sequence */
    quint32 m_boundSceneID;
    /** Absolute start time of this Chaser over a timeline (in milliseconds) */
    quint32 m_startTime;
    /** Color to use when displaying the sequence in the Show manager */
    QColor m_color;
    /** Flag to indicate if a Sequence item is locked in the Show Manager timeline */
    bool m_locked;

    /*********************************************************************
     * Speed modes
     *********************************************************************/
public:
    enum SpeedMode {
        Default, //! Use step function's own speed setting
        Common,  //! Impose a common chaser-specific speed to all steps
        PerStep  //! Impose a step-specific speed to each step
    };

    void setFadeInMode(SpeedMode mode);
    SpeedMode fadeInMode() const;

    void setFadeOutMode(SpeedMode mode);
    SpeedMode fadeOutMode() const;

    void setDurationMode(SpeedMode mode);
    SpeedMode durationMode() const;

    static QString speedModeToString(SpeedMode mode);
    static SpeedMode stringToSpeedMode(const QString& str);

private:
    SpeedMode m_fadeInMode;
    SpeedMode m_fadeOutMode;
    SpeedMode m_holdMode;

    /*********************************************************************
     * Save & Load
     *********************************************************************/
public:
    /** Save this function to an XML document */
    bool saveXML(QDomDocument* doc, QDomElement* wksp_root);

    /** Load this function contents from an XML document */
    bool loadXML(const QDomElement& root);

    /** @reimp */
    void postLoad();

    /*********************************************************************
     * Start/Next/Previous
     * ChaserRunner wrappers
     *********************************************************************/
public:
    /** Set the intensity at start */
    void setStartIntensity(qreal startIntensity);

    /** @reimpl */
    void tap();

    /** Set the current step index, or the startup step index if not started */
    void setStepIndex(int idx);

    /** Skip to the previous step */
    void previous();

    /** Skip to the next step */
    void next();

    /** Stop a specific running step */
    void stopStep(int stepIndex);

    /** Set the NEW current step number */
    void setCurrentStep(int step, qreal intensity = 1.0);

    /** Get the current step number */
    int currentStepIndex() const;

    /** Compute next step for manual fading */
    int computeNextStep(int currentStepIndex) const;

    /** Get the running step number. */
    int runningStepsNumber() const;

    /** Get the first step of the running list. If none is running this returns NULL */
    ChaserRunnerStep currentRunningStep() const;

    /** Adjust the intensities of chaser steps. */
    void adjustIntensity(qreal fraction, int stepIndex = -1);

private:
    /** Step index at chaser start */
    int m_startStepIndex;

    /** Intensity at start */
    qreal m_startIntensity;
    bool m_hasStartIntensity;

    /*********************************************************************
     * Running
     *********************************************************************/
private:
    /**
     * Create a ChaserRunner object from the given Chaser. The chaser's
     * step mutex is locked & unlocked in this method.
     *
     * @param self The parent Chaser function to create a runner for
     * @param doc The engine object
     * @return NULL if unsuccessful, otherwise a new ChaserRunner*
     */
    void createRunner(quint32 startTime = 0, int startStepIdx = 0);
public:
    /** @reimpl */
    void preRun(MasterTimer* timer);

    /** @reimpl */
    void write(MasterTimer* timer, QList<Universe *> universes);

    /** @reimpl */
    void postRun(MasterTimer* timer, QList<Universe *> universes);

signals:
    /** Tells that the current step number has changed. */
    void currentStepChanged(int stepNumber);

private:
    QMutex m_runnerMutex;
    ChaserRunner* m_runner;

    /*************************************************************************
     * Intensity
     *************************************************************************/
public:
    /** @reimpl */
    void adjustAttribute(qreal fraction, int attributeIndex);
};

/** @} */

#endif
