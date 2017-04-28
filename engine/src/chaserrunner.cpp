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

#include <QElapsedTimer>
#include <QDebug>

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
    , m_roundTime(new QElapsedTimer())
    , m_order()
    , m_intensity(1.0)
{
    Q_ASSERT(chaser != NULL);

    if (m_chaser->type() == Function::SequenceType)
    {
        qDebug() << "[ChaserRunner] startTime:" << startTime;
        int idx = 0;
        quint32 stepsTime = 0;
        foreach(ChaserStep step, chaser->steps())
        {
            uint duration = m_chaser->durationMode() == Chaser::Common ? m_chaser->speeds().duration() : step.speeds.duration();

            if (startTime < stepsTime + duration)
            {
                m_newStartStepIdx = idx;
                m_startOffset = startTime - stepsTime;
                qDebug() << "New start index:" << m_newStartStepIdx;
                break;
            }
            idx++;
            stepsTime += duration;
        }
    }

    m_direction = m_chaser->direction();
    connect(chaser, SIGNAL(changed(quint32)), this, SLOT(slotChaserChanged()));
    m_roundTime->restart();

    fillOrder();
}

ChaserRunner::~ChaserRunner()
{
    clearRunningList();
    delete m_roundTime;
}

/****************************************************************************
 * Speeds
 ****************************************************************************/

void ChaserRunner::slotChaserChanged()
{
    // Handle (possible) speeds change on the next write() pass
    m_updateOverrideSpeeds = true;
    QList<ChaserRunnerStep*> delList;
    foreach(ChaserRunnerStep *step, m_runnerSteps)
    {
        if (!m_chaser->steps().contains(ChaserStep(step->m_function->id())))
        {
            // Disappearing function: remove step
            delList.append(step);
        }
        else
        {
            // Recalculate the speeds of each running step
            step->m_speeds.setFadeIn(stepFadeIn(step->m_index));
            step->m_speeds.setFadeOut(stepFadeOut(step->m_index));
            step->m_speeds.setHold(stepHold(step->m_index));
        }
    }
    foreach(ChaserRunnerStep* step, delList)
    {
        step->m_function->stop(functionParent());
        delete step;
        m_runnerSteps.removeAll(step);
    }
}

quint32 ChaserRunner::stepFadeIn(int stepIdx) const
{
    quint32 fadeIn = 0;
    switch (m_chaser->fadeInMode())
    {
    case Chaser::Common:
        // All steps' fade in speed is dictated by the chaser
        fadeIn = m_chaser->speeds().fadeIn();
        break;
    case Chaser::PerStep:
        // Each step specifies its own fade in speed
        if (stepIdx >= 0 && stepIdx < m_chaser->steps().size())
            fadeIn = m_chaser->steps().at(stepIdx).speeds.fadeIn();
        else
            fadeIn = Speed::originalValue();
        break;
    default:
    case Chaser::Default:
        // Don't touch members' fade in speed at all
        fadeIn = Speed::originalValue();
        break;
    }

    return fadeIn;
}

quint32 ChaserRunner::stepFadeOut(int stepIdx) const
{
    quint32 fadeOut = 0;
    switch (m_chaser->fadeOutMode())
    {
    case Chaser::Common:
        // All steps' fade out speed is dictated by the chaser
        fadeOut = m_chaser->speeds().fadeOut();
        break;
    case Chaser::PerStep:
        // Each step specifies its own fade out speed
        if (stepIdx >= 0 && stepIdx < m_chaser->steps().size())
            fadeOut = m_chaser->steps().at(stepIdx).speeds.fadeOut();
        else
            fadeOut = Speed::originalValue();
        break;
    default:
    case Chaser::Default:
        // Don't touch members' fade out speed at all
        fadeOut = Speed::originalValue();
        break;
    }

    return fadeOut;
}

uint ChaserRunner::stepHold(int stepIdx) const
{
    uint hold = 0;
    switch (m_chaser->durationMode())
    {
    default:
    case Chaser::Default:
    case Chaser::Common:
        // All steps' hold is dictated by the chaser
        hold = m_chaser->speeds().hold();
        break;
    case Chaser::PerStep:
        // Each step specifies its own hold
        if (stepIdx >= 0 && stepIdx < m_chaser->steps().size())
            hold = m_chaser->steps().at(stepIdx).speeds.hold();
        else
            hold = m_chaser->speeds().hold();
        break;
    }

    return hold;
}

uint ChaserRunner::stepDuration(int stepIdx) const
{
    uint duration = 0;
    switch (m_chaser->durationMode())
    {
    default:
    case Chaser::Default:
    case Chaser::Common:
        // All steps' duration is dictated by the chaser
        duration = m_chaser->speeds().duration();
        break;
    case Chaser::PerStep:
        // Each step specifies its own duration
        if (stepIdx >= 0 && stepIdx < m_chaser->steps().size())
            duration = m_chaser->steps().at(stepIdx).speeds.duration();
        else
            duration = m_chaser->speeds().duration();
        break;
    }

    return duration;
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
    bool stopped = false;

    foreach(ChaserRunnerStep *step, m_runnerSteps)
    {
        if (stepIndex == step->m_index)
        {
            qDebug() << "Stopping step idx:" << stepIndex << "(running:" << m_runnerSteps.count() << ")";
            step->m_function->stop(functionParent());
            // restore the original Function blend mode
            step->m_function->setBlendMode(step->m_blendMode);
            m_runnerSteps.removeOne(step);
            delete step;
            stopped = true;
        }
    }

    if (stopped && m_runnerSteps.size() == 1)
    {
        ChaserRunnerStep *lastStep = m_runnerSteps.at(0);
        m_lastRunStepIdx = lastStep->m_index;
        // when only one step remains in the running list,
        // it has to run with its original blend mode
        if (lastStep->m_function)
            lastStep->m_function->setBlendMode(lastStep->m_blendMode);
        emit currentStepChanged(m_lastRunStepIdx);
    }
}

void ChaserRunner::setCurrentStep(int step, qreal intensity)
{
    if (step >= 0 && step < m_chaser->steps().size())
        m_newStartStepIdx = step;
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

void ChaserRunner::adjustIntensity(qreal fraction, int requestedStepIndex, int fadeControl)
{
    fraction = CLAMP(fraction, qreal(0.0), qreal(1.0));

    //qDebug() << "Adjust intensity" << fraction << "step:" << requestedStepIndex << "fade:" << fadeControl;

    int stepIndex = requestedStepIndex;
    if (stepIndex == -1)
    {
        stepIndex = m_lastRunStepIdx;
        // stepIndex == -1 means that the "global" intensity
        // of the chaser has to be changed
        m_intensity = fraction;
    }

    foreach(ChaserRunnerStep *step, m_runnerSteps)
    {
        if (stepIndex == step->m_index && step->m_function != NULL)
        {
            if (fadeControl == Chaser::BlendedCrossfade && fraction != 1.0)
                step->m_function->setBlendMode(Universe::AdditiveBlend);
            else
                step->m_function->setBlendMode(step->m_blendMode);
            step->m_function->adjustAttribute(fraction, Function::Intensity);
            return;
        }
    }

    // No need to start a new step if it is not wanted
    if (requestedStepIndex == -1)
        return;

    // Don't start a step with an intensity of zero
    if (fraction == qreal(0.0))
        return;

    // not found ? It means we need to start a new step and crossfade kicks in !
    startNewStep(stepIndex, m_doc->masterTimer(), fraction, fadeControl);
}

void ChaserRunner::clearRunningList()
{
    // empty the running queue
    foreach(ChaserRunnerStep *step, m_runnerSteps)
    {
        if (step->m_function)
        {
            // restore the original Function blend mode
            step->m_function->setBlendMode(step->m_blendMode);
            step->m_function->stop(functionParent());
        }
        delete step;
    }
    m_runnerSteps.clear();
}

/****************************************************************************
 * Running
 ****************************************************************************/

void ChaserRunner::startNewStep(int index, MasterTimer* timer, qreal intensity,
                                int fadeControl, quint32 elapsed)
{
    if (m_chaser == NULL || m_chaser->steps().count() == 0)
        return;

    if (index < 0 || index >= m_chaser->steps().count())
        index = 0; // fallback to the first step

    ChaserStep step(m_chaser->steps().at(index));
    Function *func = m_doc->function(step.fid);
    if (func == NULL)
        return;

    ChaserRunnerStep *newStep = new ChaserRunnerStep();
    newStep->m_index = index;
    newStep->m_blendMode = func->blendMode();

    if (fadeControl == Chaser::FromFunction)
    {
        newStep->m_speeds.setFadeIn(stepFadeIn(index));
    }
    else
    {
        newStep->m_speeds.setFadeIn(0);
        if (fadeControl == Chaser::BlendedCrossfade)
            func->setBlendMode(Universe::AdditiveBlend);
    }

    newStep->m_speeds.setFadeOut(stepFadeOut(index));
    newStep->m_speeds.setDuration(stepDuration(index));

    if (m_startOffset != 0)
        newStep->m_elapsed = m_startOffset + MasterTimer::tick();
    else
        newStep->m_elapsed = MasterTimer::tick() + elapsed;
    newStep->m_elapsedBeats = 0; //(newStep->m_elapsed / timer->beatTimeDuration()) * 1000;

    m_startOffset = 0;

    newStep->m_function = func;

    if (m_chaser->type() == Function::SequenceType)
    {
        Scene *s = qobject_cast<Scene*>(func);
        // blind == true is a workaround to reuse the same scene
        // without messing up the previous values
        for (int i = 0; i < step.values.count(); i++)
            s->setValue(step.values.at(i), true);
    }

    // Set intensity before starting the function. Otherwise the intensity
    // might momentarily jump too high.
    newStep->m_function->adjustAttribute(intensity, Function::Intensity);
    // Start the fire up !
    newStep->m_function->start(timer, functionParent(), 0, newStep->m_speeds);
    m_runnerSteps.append(newStep);
    m_roundTime->restart();
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
        if (m_previous)
            currentStepIndex--;
        else
            currentStepIndex++;
    }
    else
    {
        // "Previous" for a backward scene is +1
        if (m_previous)
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
        // Don't run the same function 2 times in a row
        while (currentStepIndex < m_chaser->steps().size()
                && randomStepIndex(currentStepIndex) == m_lastRunStepIdx)
            ++currentStepIndex;
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

void ChaserRunner::setPause(bool enable)
{
    // Nothing to do
    if (m_chaser->steps().size() == 0)
        return;

    foreach(ChaserRunnerStep *step, m_runnerSteps)
        step->m_function->setPause(enable);
}

FunctionParent ChaserRunner::functionParent() const
{
    return FunctionParent(FunctionParent::Function, m_chaser->id());
}

bool ChaserRunner::write(MasterTimer* timer, QList<Universe *> universes)
{
    Q_UNUSED(universes);

    // Nothing to do
    if (m_chaser->steps().size() == 0)
        return false;

    if (m_next || m_previous || m_newStartStepIdx != -1)
    {
        clearRunningList();
    }

    if (m_newStartStepIdx != -1)
    {
        m_lastRunStepIdx = m_newStartStepIdx;
        m_newStartStepIdx = -1;
        qDebug() << "Starting from step" << m_lastRunStepIdx << "@ offset" << m_startOffset;
        startNewStep(m_lastRunStepIdx, timer, m_intensity, false);
        emit currentStepChanged(m_lastRunStepIdx);
    }

    quint32 prevStepRoundElapsed = 0;


    foreach(ChaserRunnerStep *step, m_runnerSteps)
    {
        if (m_chaser->speeds().tempoType() == Speed::Beats && timer->isBeat())
        {
            step->m_elapsedBeats += 1000;
            qDebug() << "Function" << step->m_function->name() << "duration:" << step->m_speeds.duration() << "beats:" << step->m_elapsedBeats;
        }

        if (step->m_speeds.duration() != Speed::infiniteValue() &&
            ((m_chaser->speeds().tempoType() == Speed::Ms && step->m_elapsed >= step->m_speeds.duration()) ||
             (m_chaser->speeds().tempoType() == Speed::Beats && step->m_elapsedBeats >= step->m_speeds.duration())))
        {
            if (step->m_speeds.duration() != 0)
                prevStepRoundElapsed = step->m_elapsed % step->m_speeds.duration();

            step->m_function->stop(functionParent());
            delete step;
            m_runnerSteps.removeOne(step);
        }
        else
        {
            if (step->m_elapsed < UINT_MAX)
                step->m_elapsed += MasterTimer::tick();

            // When the speeds of the chaser change, they need to be updated to the lower
            // level (only current function) as well. Otherwise the new speeds would take
            // effect only on the next step change.
            if (m_updateOverrideSpeeds)
            {
                m_updateOverrideSpeeds = false;
                if (step->m_function != NULL)
                {
                    step->m_function->alternateSpeedsEdit(0).setFadeIn(step->m_speeds.fadeIn());
                    step->m_function->alternateSpeedsEdit(0).setFadeOut(step->m_speeds.fadeOut());
                }
            }
        }
    }

    if (m_runnerSteps.isEmpty())
    {
        m_lastRunStepIdx = getNextStepIndex();
        if (m_lastRunStepIdx != -1)
        {
            startNewStep(m_lastRunStepIdx, timer, m_intensity, false, prevStepRoundElapsed);
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
