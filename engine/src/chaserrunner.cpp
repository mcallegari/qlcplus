/*
  Q Light Controller Plus
  chaserrunner.cpp

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

#include <QDebug>
#include <QTime>

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
    , m_startOffset(0)
    , m_next(false)
    , m_previous(false)
    , m_newStartStepIdx(-1)
    , m_lastRunStepIdx(-1)
    , m_roundTime(new QTime)
    , m_order()
    , m_intensity(1.0)
{
    Q_ASSERT(chaser != NULL);

    if (m_chaser->isSequence() == true)
    {
        qDebug() << "[ChaserRunner] startTime:" << startTime;
        int idx = 0;
        quint32 stepsTime = 0;
        foreach(ChaserStep step, chaser->steps())
        {
            if (startTime < stepsTime + step.duration)
            {
                m_newStartStepIdx = idx;
                m_startOffset = startTime - stepsTime;
                break;
            }
            idx++;
            stepsTime += step.duration;
        }
    }

    m_direction = m_chaser->direction();
    connect(chaser, SIGNAL(changed(quint32)), this, SLOT(slotChaserChanged()));
    m_roundTime->start();

    fillOrder();
}

ChaserRunner::~ChaserRunner()
{
    clearRunningList();
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
    // Recalculate the speed of each running step
    foreach(ChaserRunnerStep *step, m_runnerSteps)
    {
        step->m_fadeIn = stepFadeIn(step->m_index);
        step->m_fadeOut = stepFadeOut(step->m_index);
        step->m_duration = stepDuration(step->m_index);
    }
}

uint ChaserRunner::stepFadeIn(int stepIdx) const
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

uint ChaserRunner::stepFadeOut(int stepIdx) const
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

uint ChaserRunner::stepDuration(int stepIdx) const
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
    if (uint(m_roundTime->elapsed()) >= (stepDuration(m_lastRunStepIdx) / 4))
        next();
}

void ChaserRunner::stopStep(int stepIndex)
{
    foreach(ChaserRunnerStep *step, m_runnerSteps)
    {
        if (stepIndex == step->m_index && step->m_function != NULL)
        {
            qDebug() << "Stopping step idx:" << stepIndex << "(running:" << m_runnerSteps.count() << ")";
            step->m_function->stop();
            step->m_function = NULL;
            m_runnerSteps.removeOne(step);
            delete step;
        }
    }
}

void ChaserRunner::setCurrentStep(int step, qreal intensity)
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

int ChaserRunner::currentStepIndex() const
{
    return m_lastRunStepIdx;
}

int ChaserRunner::runningStepsNumber() const
{
    return m_runnerSteps.count();
}

ChaserRunnerStep* ChaserRunner::currentRunningStep() const
{
    if (m_runnerSteps.count() > 0)
        return m_runnerSteps.at(0);
    return NULL;
}

int ChaserRunner::computeNextStep(int currentStep) const
{
    int nextStep = currentStep;

    if (m_chaser->runOrder() == Function::Random)
    {
        nextStep = m_order.indexOf(nextStep);
        if (nextStep == -1)
        {
            qDebug() << Q_FUNC_INFO << "order not found";
            nextStep = currentStep;
        }
    }

    // Next step
    if (m_direction == Function::Forward)
    {
        nextStep++;
    }
    else
    {
        nextStep--;
    }

    if (nextStep < m_chaser->steps().size() && nextStep >= 0)
    {
        if (m_chaser->runOrder() == Function::Random)
        {
            nextStep = randomStepIndex(nextStep);
        }
        return nextStep; // In the middle of steps. No need to go any further.
    }

    if (m_chaser->runOrder() == Function::SingleShot)
    {
        return -1; // Forward or Backward SingleShot has been completed.
    }
    else if (m_chaser->runOrder() == Function::Loop)
    {
        if (m_direction == Function::Forward)
        {
            if (nextStep >= m_chaser->steps().size())
                nextStep = 0;
            else
                nextStep = m_chaser->steps().size() - 1; // Used by CueList with manual prev
        }
        else // Backward
        {
            if (nextStep < 0)
                nextStep = m_chaser->steps().size() - 1;
            else
                nextStep = 0;
        }
    }
    else if (m_chaser->runOrder() == Function::Random)
    {
        nextStep = randomStepIndex(nextStep);
    }
    else // Ping Pong
    {
        // Change direction, but don't run the first/last step twice.
        if (m_direction == Function::Forward)
        {
            nextStep = m_chaser->steps().size() - 2;
        }
        else // Backwards
        {
            nextStep = 1;
        }

        // Make sure we don't go beyond limits.
        nextStep = CLAMP(nextStep, 0, m_chaser->steps().size() - 1);
    }

    return nextStep;
}

void ChaserRunner::shuffle(QVector<int> & data)
{
   int n = data.size();
   for (int i = n - 1; i > 0; --i)
   {
       qSwap(data[i], data[qrand() % (i + 1)]);
   }
}

int ChaserRunner::randomStepIndex(int step) const
{
   if (m_chaser->runOrder() == Function::Random && step >= 0 && step < m_order.size())
       return m_order[step];

   return step;
}

void ChaserRunner::fillOrder()
{
    fillOrder(m_chaser->steps().size());
}

void ChaserRunner::fillOrder(int size)
{
   m_order.resize(size);
   for (int i = 0; i < size; ++i)
       m_order[i] = i;

   shuffle(m_order);
}

/****************************************************************************
 * Intensity
 ****************************************************************************/

void ChaserRunner::adjustIntensity(qreal fraction, int stepIndex)
{
    if (stepIndex == -1)
        stepIndex = m_lastRunStepIdx;

    m_intensity = CLAMP(fraction, qreal(0.0), qreal(1.0));
    foreach(ChaserRunnerStep *step, m_runnerSteps)
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

void ChaserRunner::clearRunningList()
{
    // empty the running queue
    foreach(ChaserRunnerStep *step, m_runnerSteps)
    {
        if (step->m_function != NULL && step->m_function->isRunning())
        {
            step->m_function->stop();
            step->m_function = NULL;
        }
        delete step;
    }
    m_runnerSteps.clear();
}

/****************************************************************************
 * Running
 ****************************************************************************/

void ChaserRunner::startNewStep(int index, MasterTimer* timer, bool manualFade)
{
    if (m_chaser == NULL || m_chaser->steps().count() == 0)
        return;

    if (index < 0 || index >= m_chaser->steps().count())
        index = 0; // fallback to the first step

    ChaserStep step(m_chaser->steps().at(index));
    Function *func = m_doc->function(step.fid);
    if (func != NULL && func->stopped() == true)
    {
        ChaserRunnerStep *newStep = new ChaserRunnerStep();
        newStep->m_index = index;
        if (manualFade == true)
            newStep->m_fadeIn = 0;
        else
            newStep->m_fadeIn = stepFadeIn(index);
        newStep->m_fadeOut = stepFadeOut(index);
        newStep->m_duration = stepDuration(index);

        if (m_startOffset != 0)
            newStep->m_elapsed = m_startOffset + MasterTimer::tick();
        else
            newStep->m_elapsed = MasterTimer::tick();
        m_startOffset = 0;

        newStep->m_function = func;

        if (m_chaser->isSequence())
        {
            Scene *s = qobject_cast<Scene*>(func);
            // blind == true is a workaround to reuse the same scene
            // without messing up the previous values
            for (int i = 0; i < step.values.count(); i++)
                s->setValue(step.values.at(i), true);
        }

        // Set intensity before starting the function. Otherwise the intensity
        // might momentarily jump too high.
        newStep->m_function->adjustAttribute(m_intensity, Function::Intensity);
        // Start the fire up !
        newStep->m_function->start(timer, true, 0, newStep->m_fadeIn, newStep->m_fadeOut);
        m_runnerSteps.append(newStep);
        m_roundTime->restart();
    }
}

int ChaserRunner::getNextStepIndex()
{
    int currentStepIndex = m_lastRunStepIdx;

    if (m_chaser->runOrder() == Function::Random)
    {
        currentStepIndex = m_order.indexOf(currentStepIndex);
        if (currentStepIndex == -1)
        {
            qDebug() << Q_FUNC_INFO << "order not found";
            currentStepIndex = m_lastRunStepIdx;
        }
    }

    if (currentStepIndex == -1 &&
        m_chaser->direction() == Function::Backward)
            currentStepIndex = m_chaser->steps().size();

    // Next step
    if (m_direction == Function::Forward)
    {
        // "Previous" for a forward chaser is -1
        if (m_previous == true)
            currentStepIndex--;
        else
            currentStepIndex++;
    }
    else
    {
        // "Previous" for a backward scene is +1
        if (m_previous == true)
            currentStepIndex++;
        else
            currentStepIndex--;
    }

    m_next = false;
    m_previous = false;

    if (currentStepIndex < m_chaser->steps().size() && currentStepIndex >= 0)
    {
        if (m_chaser->runOrder() == Function::Random)
        {
            currentStepIndex = randomStepIndex(currentStepIndex);
        }
        return currentStepIndex; // In the middle of steps. No need to go any further.
    }

    if (m_chaser->runOrder() == Function::SingleShot)
    {
        return -1; // Forward or Backward SingleShot has been completed.
    }
    else if (m_chaser->runOrder() == Function::Loop)
    {
        if (m_direction == Function::Forward)
        {
            if (currentStepIndex >= m_chaser->steps().size())
                currentStepIndex = 0;
            else
                currentStepIndex = m_chaser->steps().size() - 1; // Used by CueList with manual prev
        }
        else // Backward
        {
            if (currentStepIndex < 0)
                currentStepIndex = m_chaser->steps().size() - 1;
            else
                currentStepIndex = 0;
        }
    }
    else if (m_chaser->runOrder() == Function::Random)
    {
        fillOrder();
        if (m_direction == Function::Forward)
        {
            if (currentStepIndex >= m_chaser->steps().size())
                currentStepIndex = 0;
            else
                currentStepIndex = m_chaser->steps().size() - 1; // Used by CueList with manual prev
        }
        else // Backward
        {
            if (currentStepIndex < 0)
                currentStepIndex = m_chaser->steps().size() - 1;
            else
                currentStepIndex = 0;
        }
        currentStepIndex = randomStepIndex(currentStepIndex);
    }
    else // Ping Pong
    {
        // Change direction, but don't run the first/last step twice.
        if (m_direction == Function::Forward)
        {
            currentStepIndex = m_chaser->steps().size() - 2;
            m_direction = Function::Backward;
        }
        else // Backwards
        {
            currentStepIndex = 1;
            m_direction = Function::Forward;
        }

        // Make sure we don't go beyond limits.
        currentStepIndex = CLAMP(currentStepIndex, 0, m_chaser->steps().size() - 1);
    }

    return currentStepIndex;
}

bool ChaserRunner::write(MasterTimer* timer, QList<Universe *> universes)
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
        qDebug() << "Starting from step" << m_lastRunStepIdx << "@ offset" << m_startOffset;
        startNewStep(m_lastRunStepIdx, timer, false);
        emit currentStepChanged(m_lastRunStepIdx);
    }

    foreach(ChaserRunnerStep *step, m_runnerSteps)
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
            delete step;
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
        m_lastRunStepIdx = getNextStepIndex();
        if (m_lastRunStepIdx != -1)
        {
            startNewStep(m_lastRunStepIdx, timer, false);
            emit currentStepChanged(m_lastRunStepIdx);
        }
        else
        {
            return false;
        }
    }

    return true;
}

void ChaserRunner::postRun(MasterTimer* timer, QList<Universe*> universes)
{
    Q_UNUSED(universes);
    Q_UNUSED(timer);

    qDebug() << Q_FUNC_INFO;
    clearRunningList();
}

