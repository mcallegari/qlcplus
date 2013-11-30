/*
  Q Light Controller
  chaserrunner.cpp

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

#include <QDebug>
#include <QTime>

#include "universearray.h"
#include "cuelistrunner.h"
#include "genericfader.h"
#include "mastertimer.h"
#include "fadechannel.h"
#include "chaserstep.h"
#include "qlcmacros.h"
#include "fixture.h"
#include "chaser.h"
#include "scene.h"
#include "doc.h"

CueListRunner::CueListRunner(const Doc* doc, const Chaser* chaser)
    : QObject(NULL)
    , m_doc(doc)
    , m_chaser(chaser)
    , m_updateOverrideSpeeds(false)
    , m_next(false)
    , m_previous(false)
    , m_newStartStepIdx(-1)
    , m_lastRunStepIdx(-1)
    , m_roundTime(new QTime)
    , m_intensity(1.0)
{
    Q_ASSERT(chaser != NULL);

    m_direction = m_chaser->direction();
    connect(chaser, SIGNAL(changed(quint32)), this, SLOT(slotChaserChanged()));
    m_roundTime->start();
}

CueListRunner::~CueListRunner()
{
    clearRunningList();
    delete m_roundTime;
    m_roundTime = NULL;
}

/****************************************************************************
 * Speed
 ****************************************************************************/

void CueListRunner::slotChaserChanged()
{
    // Handle (possible) speed change on the next write() pass
    m_updateOverrideSpeeds = true;
    // Recalculate the speed of each running step
    foreach(CueRunnerStep *step, m_runnerSteps)
    {
        step->m_fadeIn = stepFadeIn(step->m_index);
        step->m_fadeOut = stepFadeOut(step->m_index);
        step->m_duration = stepDuration(step->m_index);
    }
}

uint CueListRunner::stepFadeIn(int stepIdx) const
{
    uint speed = 0;
    if (m_chaser->overrideFadeInSpeed() != Function::defaultSpeed())
    {
        // Override speed is used when another function has started the chaser,
        // i.e. chaser inside a chaser that wants to impose its own fade in speed
        // to its members.
        speed = m_chaser->overrideFadeInSpeed();
    }
    else
    {
        switch (m_chaser->fadeInMode())
        {
        case Chaser::Common:
            // All steps' fade in speed is dictated by the chaser
            speed = m_chaser->fadeInSpeed();
            break;
        case Chaser::PerStep:
            // Each step specifies its own fade in speed
            if (stepIdx >= 0 && stepIdx < m_chaser->steps().size())
                speed = m_chaser->steps().at(stepIdx).fadeIn;
            else
                speed = Function::defaultSpeed();
            break;
        default:
        case Chaser::Default:
            // Don't touch members' fade in speed at all
            speed = Function::defaultSpeed();
            break;
        }
    }

    return speed;
}

uint CueListRunner::stepFadeOut(int stepIdx) const
{
    uint speed = 0;
    if (m_chaser->overrideFadeOutSpeed() != Function::defaultSpeed())
    {
        // Override speed is used when another function has started the chaser,
        // i.e. chaser inside a chaser that wants to impose its own fade out speed
        // to its members.
        speed = m_chaser->overrideFadeOutSpeed();
    }
    else
    {
        switch (m_chaser->fadeOutMode())
        {
        case Chaser::Common:
            // All steps' fade out speed is dictated by the chaser
            speed = m_chaser->fadeOutSpeed();
            break;
        case Chaser::PerStep:
            // Each step specifies its own fade out speed
            if (stepIdx >= 0 && stepIdx < m_chaser->steps().size())
                speed = m_chaser->steps().at(stepIdx).fadeOut;
            else
                speed = Function::defaultSpeed();
            break;
        default:
        case Chaser::Default:
            // Don't touch members' fade out speed at all
            speed = Function::defaultSpeed();
            break;
        }
    }

    return speed;
}

uint CueListRunner::stepDuration(int stepIdx) const
{
    uint speed = 0;
    if (m_chaser->overrideDuration() != Function::defaultSpeed())
    {
        // Override speed is used when another function has started the chaser,
        // i.e. chaser inside a chaser that wants to impose its own duration
        // to its members.
        speed = m_chaser->overrideDuration();
    }
    else
    {
        switch (m_chaser->durationMode())
        {
        default:
        case Chaser::Default:
        case Chaser::Common:
            // All steps' duration is dictated by the chaser
            speed = m_chaser->duration();
            break;
        case Chaser::PerStep:
            // Each step specifies its own duration
            if (stepIdx >= 0 && stepIdx < m_chaser->steps().size())
                speed = m_chaser->steps().at(stepIdx).duration;
            else
                speed = m_chaser->duration();
            break;
        }
    }

    return speed;
}

/****************************************************************************
 * Step control
 ****************************************************************************/

void CueListRunner::next()
{
    m_next = true;
    m_previous = false;
}

void CueListRunner::previous()
{
    m_next = false;
    m_previous = true;
}

void CueListRunner::tap()
{
    //if (uint(m_roundTime->elapsed()) >= (stepDuration(m_currentStep) / 4))
    //    next();
}

void CueListRunner::stopStep(int stepIndex)
{
    foreach(CueRunnerStep *step, m_runnerSteps)
    {
        if (stepIndex == step->m_index && step->m_function != NULL)
        {
            qDebug() << "Stopping step idx:" << stepIndex << "(running:" << m_runnerSteps.count() << ")";
            step->m_function->stop();
            step->m_function = NULL;
            m_runnerSteps.removeOne(step);
        }
    }
}

void CueListRunner::setCurrentStep(int step, qreal intensity)
{
    if (step >= 0 && step < m_chaser->steps().size())
    {
        m_newStartStepIdx = step;
    }
    else
        m_newStartStepIdx = 0;
    m_intensity = intensity;
    m_next = false;
    m_previous = false;
}

int CueListRunner::currentStep() const
{
    return m_lastRunStepIdx;
}

int CueListRunner::runningStepsNumber() const
{
    return m_runnerSteps.count();
}

/****************************************************************************
 * Intensity
 ****************************************************************************/

void CueListRunner::adjustIntensity(qreal fraction, int stepIndex)
{
    m_intensity = CLAMP(fraction, qreal(0.0), qreal(1.0));
    foreach(CueRunnerStep *step, m_runnerSteps)
    {
        if (stepIndex == step->m_index && step->m_function != NULL)
        {
            step->m_function->adjustAttribute(m_intensity, Function::Intensity);
            return;
        }
    }
    // not found ?? It means we need to start a new step and crossfade kicks in !
    startNewStep(stepIndex, m_doc->masterTimer(), true);
}

void CueListRunner::clearRunningList()
{
    // empty the running queue
    foreach(CueRunnerStep *step, m_runnerSteps)
    {
        if (step->m_function != NULL && step->m_function->isRunning())
        {
            step->m_function->stop();
            step->m_function = NULL;
        }
    }
    m_runnerSteps.clear();
}

/****************************************************************************
 * Running
 ****************************************************************************/

void CueListRunner::startNewStep(int index, MasterTimer* timer, bool manualFade)
{
    ChaserStep step(m_chaser->steps().at(index));
    Function *func = m_doc->function(step.fid);
    if (func != NULL && func->stopped() == true)
    {
        CueRunnerStep *newStep = new CueRunnerStep();
        newStep->m_index = index;
        if (manualFade == true)
            newStep->m_fadeIn = 0;
        else
            newStep->m_fadeIn = stepFadeIn(index);
        newStep->m_fadeOut = stepFadeOut(index);
        newStep->m_duration = stepDuration(index);
        newStep->m_elapsed = MasterTimer::tick();
        newStep->m_function = func;

        // Set intensity before starting the function. Otherwise the intensity
        // might momentarily jump too high.
        newStep->m_function->adjustAttribute(m_intensity, Function::Intensity);
        // Start the fire up !
        newStep->m_function->start(timer, true, 0, newStep->m_fadeIn, newStep->m_fadeOut);
        m_runnerSteps.append(newStep);
        m_roundTime->restart();
    }
}

int CueListRunner::getNextStepIndex()
{

    // Next step
    if (m_direction == Function::Forward)
    {
        // "Previous" for a forwards chaser is -1
        if (m_previous == true)
            m_lastRunStepIdx--;
        else
            m_lastRunStepIdx++;
    }
    else
    {
        // "Previous" for a backwards scene is +1
        if (m_previous == true)
            m_lastRunStepIdx++;
        else
            m_lastRunStepIdx--;
    }

    m_next = false;
    m_previous = false;

    if (m_lastRunStepIdx < m_chaser->steps().size() && m_lastRunStepIdx >= 0)
        return m_lastRunStepIdx; // In the middle of steps. No need to go any further.

    if (m_chaser->runOrder() == Function::SingleShot)
    {
        return -1; // Forwards or Backwards SingleShot has been completed.
    }
    else if (m_chaser->runOrder() == Function::Loop)
    {
        if (m_direction == Function::Forward)
        {
            if (m_lastRunStepIdx >= m_chaser->steps().size())
                m_lastRunStepIdx = 0;
            else
                m_lastRunStepIdx = m_chaser->steps().size() - 1; // Used by CueList with manual prev
        }
        else // Backwards
        {
            if (m_lastRunStepIdx < 0)
                m_lastRunStepIdx = m_chaser->steps().size() - 1;
            else
                m_lastRunStepIdx = 0;
        }
    }
    else // Ping Pong
    {
        // Change direction, but don't run the first/last step twice.
        if (m_direction == Function::Forward)
        {
            m_lastRunStepIdx = m_chaser->steps().size() - 2;
            m_direction = Function::Backward;
        }
        else // Backwards
        {
            m_lastRunStepIdx = 1;
            m_direction = Function::Forward;
        }

        // Make sure we don't go beyond limits.
        m_lastRunStepIdx = CLAMP(m_lastRunStepIdx, 0, m_chaser->steps().size() - 1);
    }

    return m_lastRunStepIdx;
}

bool CueListRunner::write(MasterTimer* timer, UniverseArray* universes)
{
    Q_UNUSED(universes);

    // Nothing to do
    if (m_chaser->steps().size() == 0)
        return false;

    if (m_next == true || m_previous == true || m_newStartStepIdx != -1)
    {
        clearRunningList();
    }

    if (m_newStartStepIdx != -1)
    {
        m_lastRunStepIdx = m_newStartStepIdx;
        m_newStartStepIdx = -1;
        qDebug() << "Starting from step" << m_lastRunStepIdx;
        startNewStep(m_lastRunStepIdx, timer, false);
        emit currentStepChanged(m_lastRunStepIdx);
    }

    foreach(CueRunnerStep *step, m_runnerSteps)
    {
        if (step->m_duration != Function::infiniteSpeed() &&
             step->m_elapsed >= step->m_duration)
        {
            if (step->m_function != NULL && step->m_function->isRunning())
            {
                step->m_function->stop();
                step->m_function = NULL;
            }

            m_runnerSteps.removeOne(step);
        }
        else
        {
            if (step->m_elapsed < UINT_MAX)
                step->m_elapsed += MasterTimer::tick();

            // When the speeds of the chaser change, they need to be updated to the lower
            // level (only current function) as well. Otherwise the new speeds would take
            // effect only on the next step change.
            if (m_updateOverrideSpeeds == true)
            {
                m_updateOverrideSpeeds = false;
                if (step->m_function != NULL)
                {
                    step->m_function->setOverrideFadeInSpeed(step->m_fadeIn);
                    step->m_function->setOverrideFadeOutSpeed(step->m_fadeOut);
                }
            }
        }
    }

    if (m_runnerSteps.isEmpty())
    {
        int nextIdx = getNextStepIndex();
        if (nextIdx != -1)
        {
            startNewStep(nextIdx, timer, false);
            emit currentStepChanged(nextIdx);
        }
        else
        {
            return false;
        }
    }

    return true;
}

void CueListRunner::postRun(MasterTimer* timer, UniverseArray* universes)
{
    Q_UNUSED(universes);
    Q_UNUSED(timer);

    qDebug() << Q_FUNC_INFO;
    clearRunningList();
}

