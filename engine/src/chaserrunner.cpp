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
#include "chaserrunner.h"
#include "genericfader.h"
#include "mastertimer.h"
#include "fadechannel.h"
#include "chaserstep.h"
#include "qlcmacros.h"
#include "fixture.h"
#include "chaser.h"
#include "scene.h"
#include "doc.h"

ChaserRunner::ChaserRunner(const Doc* doc, const Chaser* chaser, quint32 startTime)
    : QObject(NULL)
    , m_doc(doc)
    , m_chaser(chaser)

    , m_updateOverrideSpeeds(false)
    , m_direction(Function::Forward)
    , m_currentFunction(NULL)
    , m_elapsed(0)
    , m_startOffset(0)
    , m_next(false)
    , m_previous(false)
    , m_currentStep(0)
    , m_newCurrent(-1)
    , m_roundTime(new QTime)
    , m_intensity(1.0)
{
    Q_ASSERT(chaser != NULL);

    if (chaser->isSequence() == true)
    {
        qDebug() << "[ChaserRunner] startTime:" << startTime;
        int idx = 0;
        quint32 stepsTime = 0;
        foreach(ChaserStep step, chaser->steps())
        {
            if (startTime < stepsTime + step.duration)
            {
                m_newCurrent = idx;
                m_startOffset = startTime - stepsTime;
                break;
            }
            idx++;
            stepsTime += step.duration;
        }
    }

    if (m_chaser->direction() == Function::Backward)
        m_currentStep = m_chaser->steps().size() - 1;

    connect(chaser, SIGNAL(changed(quint32)), this, SLOT(slotChaserChanged()));
    reset();
}

ChaserRunner::~ChaserRunner()
{
    delete m_roundTime;
    m_roundTime = NULL;
}

/****************************************************************************
 * Speed
 ****************************************************************************/

void ChaserRunner::slotChaserChanged()
{
    // Handle (possible) speed change on the next write() pass
    m_updateOverrideSpeeds = true;
}

uint ChaserRunner::currentFadeIn() const
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
            if (m_currentStep >= 0 && m_currentStep < m_chaser->steps().size())
                speed = m_chaser->steps().at(m_currentStep).fadeIn;
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

uint ChaserRunner::currentFadeOut() const
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
            if (m_currentStep >= 0 && m_currentStep < m_chaser->steps().size())
                speed = m_chaser->steps().at(m_currentStep).fadeOut;
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

uint ChaserRunner::currentDuration() const
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
            if (m_currentStep >= 0 && m_currentStep < m_chaser->steps().size())
                speed = m_chaser->steps().at(m_currentStep).duration;
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

void ChaserRunner::next()
{
    m_next = true;
    m_previous = false;
}

void ChaserRunner::previous()
{
    m_next = false;
    m_previous = true;
}

void ChaserRunner::tap()
{
    if (uint(m_roundTime->elapsed()) >= (currentDuration() / 4))
        next();
}

void ChaserRunner::setCurrentStep(int step)
{
    if (step >= 0 && step < m_chaser->steps().size())
    {
        m_newCurrent = step;
    }
    else
        m_newCurrent = 0;
    m_next = false;
    m_previous = false;
}

int ChaserRunner::currentStep() const
{
    return m_currentStep;
}

void ChaserRunner::reset()
{
    // Restore original direction since Ping-Pong switches m_direction
    m_direction = m_chaser->direction();

    if (m_direction == Function::Backward)
        m_currentStep = m_chaser->steps().size() - 1;
    else
        m_currentStep = 0;

    m_elapsed = 0;
    m_next = false;
    m_previous = false;
    m_currentFunction = NULL;

    m_roundTime->start();
}

/****************************************************************************
 * Intensity
 ****************************************************************************/

void ChaserRunner::adjustIntensity(qreal fraction)
{
    m_intensity = CLAMP(fraction, qreal(0.0), qreal(1.0));
    if (m_currentFunction != NULL)
        m_currentFunction->adjustAttribute(m_intensity, Function::Intensity);
}

/****************************************************************************
 * Running
 ****************************************************************************/

bool ChaserRunner::write(MasterTimer* timer, UniverseArray* universes)
{
    Q_UNUSED(universes);

    // Nothing to do
    if (m_chaser->steps().size() == 0)
        return false;

    if (m_newCurrent != -1)
    {
        qDebug() << "Starting from step" << m_currentStep << "@ offset" << m_startOffset;

        // Manually-set current step
        m_currentStep = m_newCurrent;
        m_newCurrent = -1;

        // No need to do roundcheck here, since manually-set steps are
        // always within m_chaser->steps() limits.
        if (m_startOffset != 0)
            m_elapsed = m_startOffset + MasterTimer::tick();
        else
            m_elapsed = MasterTimer::tick();
        m_startOffset = 0;

        switchFunctions(timer);
        emit currentStepChanged(m_currentStep);
    }
    else if (m_elapsed == 0)
    {
        // First step
        m_elapsed = MasterTimer::tick();
        switchFunctions(timer);
        emit currentStepChanged(m_currentStep);
    }
    else if (m_next == true || m_previous == true ||
             (currentDuration() != Function::infiniteSpeed() && m_elapsed >= currentDuration()))
    {
        // Next step
        if (m_direction == Function::Forward)
        {
            // "Previous" for a forwards chaser is -1
            if (m_previous == true)
                m_currentStep--;
            else
                m_currentStep++;
        }
        else
        {
            // "Previous" for a backwards scene is +1
            if (m_previous == true)
                m_currentStep++;
            else
                m_currentStep--;
        }

        if (roundCheck() == false)
            return false;

        m_elapsed = MasterTimer::tick();
        m_next = false;
        m_previous = false;

        switchFunctions(timer);
        emit currentStepChanged(m_currentStep);
    }
    else
    {
        // Current step. UINT_MAX is the maximum hold time.
        if (m_elapsed < UINT_MAX)
            m_elapsed += MasterTimer::tick();
    }

    // When the speeds of the chaser change, they need to be updated to the lower
    // level (only current function) as well. Otherwise the new speeds would take
    // effect only on the next step change.
    if (m_updateOverrideSpeeds == true)
    {
        m_updateOverrideSpeeds = false;
        if (m_currentFunction != NULL)
        {
            m_currentFunction->setOverrideFadeInSpeed(currentFadeIn());
            m_currentFunction->setOverrideFadeOutSpeed(currentFadeOut());
        }
    }

    return true;
}

void ChaserRunner::postRun(MasterTimer* timer, UniverseArray* universes)
{
    Q_UNUSED(universes);
    Q_UNUSED(timer);

    if (m_currentFunction != NULL && m_currentFunction->stopped() == false)
        m_currentFunction->stop();
    m_currentFunction = NULL;
}

bool ChaserRunner::roundCheck()
{
    if (m_currentStep < m_chaser->steps().size() && m_currentStep >= 0)
        return true; // In the middle of steps. No need to go any further.

    if (m_chaser->runOrder() == Function::SingleShot)
    {
        return false; // Forwards or Backwards SingleShot has been completed.
    }
    else if (m_chaser->runOrder() == Function::Loop)
    {
        if (m_direction == Function::Forward)
        {
            if (m_currentStep >= m_chaser->steps().size())
                m_currentStep = 0;
            else
                m_currentStep = m_chaser->steps().size() - 1; // Used by CueList with manual prev
        }
        else // Backwards
        {
            if (m_currentStep < 0)
                m_currentStep = m_chaser->steps().size() - 1;
            else
                m_currentStep = 0;
        }
    }
    else // Ping Pong
    {
        // Change direction, but don't run the first/last step twice.
        if (m_direction == Function::Forward)
        {
            m_currentStep = m_chaser->steps().size() - 2;
            m_direction = Function::Backward;
        }
        else // Backwards
        {
            m_currentStep = 1;
            m_direction = Function::Forward;
        }

        // Make sure we don't go beyond limits.
        m_currentStep = CLAMP(m_currentStep, 0, m_chaser->steps().size() - 1);
    }

    // Let's continue
    return true;
}

void ChaserRunner::switchFunctions(MasterTimer* timer)
{
    if (m_currentFunction != NULL)
        m_currentFunction->stop();

    ChaserStep step(m_chaser->steps().at(m_currentStep));
    m_currentFunction = m_doc->function(step.fid);
    qDebug() << Q_FUNC_INFO << "Step #" << m_currentStep << ", function ID:" << step.fid;
    if (m_currentFunction != NULL && m_currentFunction->stopped() == true)
    {
        if (m_chaser->isSequence())
        {
            Scene *s = qobject_cast<Scene*>(m_currentFunction);
            // blind == true is a workaround to reuse the same scene
            // without messing up the previous values
            for (int i = 0; i < step.values.count(); i++)
                s->setValue(step.values.at(i), true);
        }

        // Set intensity before starting the function. Otherwise the intensity
        // might momentarily jump too high.
        m_currentFunction->adjustAttribute(m_intensity, Function::Intensity);

        // Start function using step-specific or global speed settings
        // Don't override duration because that would mess up everything when
        // a chaser/cuelist starts another chaser. Overriding the duration with
        // the current chaser's duration would mean that only the first step is
        // run from the sub-chaser. If the subfunction is an RGBMatrix or EFX,
        // the step duration probably isb not the wanted subfunction speed, either
        m_currentFunction->start(timer, true, 0, currentFadeIn(), currentFadeOut());
    }

    m_roundTime->restart();
}
