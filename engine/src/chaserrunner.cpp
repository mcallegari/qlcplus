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
    , m_updateOverrideTimings(false)
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

    if (m_chaser->isSequence())
    {
        qDebug() << "[ChaserRunner] startTime:" << startTime;
        int idx = 0;
        quint32 stepsTime = 0;
        foreach(ChaserStep step, chaser->steps())
        {
            if (startTime < stepsTime + step.timings.duration())
            {
                m_newStartStepIdx = idx;
                m_startOffset = startTime - stepsTime;
                break;
            }
            idx++;
            stepsTime += step.timings.duration();
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
 * Timings
 ****************************************************************************/

void ChaserRunner::slotChaserChanged()
{
    // Handle (possible) timings change on the next write() pass
    m_updateOverrideTimings = true;
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
            // Recalculate the timings of each running step
            step->m_timings.fadeIn = stepFadeIn(step->m_index);
            step->m_timings.fadeOut = stepFadeOut(step->m_index);
            step->m_timings.hold = stepHold(step->m_index);
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
        fadeIn = m_chaser->fadeIn();
        break;
    case Chaser::PerStep:
        // Each step specifies its own fade in speed
        if (stepIdx >= 0 && stepIdx < m_chaser->steps().size())
            fadeIn = m_chaser->steps().at(stepIdx).timings.fadeIn;
        else
            fadeIn = FunctionTimings::defaultValue();
        break;
    default:
    case Chaser::Default:
        // Don't touch members' fade in speed at all
        fadeIn = FunctionTimings::defaultValue();
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
        fadeOut = m_chaser->fadeOut();
        break;
    case Chaser::PerStep:
        // Each step specifies its own fade out speed
        if (stepIdx >= 0 && stepIdx < m_chaser->steps().size())
            fadeOut = m_chaser->steps().at(stepIdx).timings.fadeOut;
        else
            fadeOut = FunctionTimings::defaultValue();
        break;
    default:
    case Chaser::Default:
        // Don't touch members' fade out speed at all
        fadeOut = FunctionTimings::defaultValue();
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
        hold = m_chaser->hold();
        break;
    case Chaser::PerStep:
        // Each step specifies its own hold
        if (stepIdx >= 0 && stepIdx < m_chaser->steps().size())
            hold = m_chaser->steps().at(stepIdx).timings.hold;
        else
            hold = m_chaser->hold();
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
        duration = m_chaser->duration();
        break;
    case Chaser::PerStep:
        // Each step specifies its own duration
        if (stepIdx >= 0 && stepIdx < m_chaser->steps().size())
            duration = m_chaser->steps().at(stepIdx).timings.duration();
        else
            duration = m_chaser->duration();
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
            m_runnerSteps.removeOne(step);
            delete step;
            stopped = true;
        }
    }

    if (stopped && m_runnerSteps.size() == 1) {
        m_lastRunStepIdx = m_runnerSteps.at(0)->m_index;
        emit currentStepChanged(m_lastRunStepIdx);
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

void ChaserRunner::adjustIntensity(qreal fraction, int requestedStepIndex)
{
    fraction = CLAMP(fraction, qreal(0.0), qreal(1.0));

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

    // Quick & dirty fix: in startNewStep, <m_intensity> is the
    // intensity of the started function.
    // This function has to start with intensity value of <fraction>.
    qreal intensityBackup = m_intensity;
    m_intensity = fraction;

    // not found ?? It means we need to start a new step and crossfade kicks in !
    startNewStep(stepIndex, m_doc->masterTimer(), true);

    // Q&D fix: restore m_intensity as it was before.
    // We don't want to change the intensity of future steps.
    m_intensity = intensityBackup;
}

void ChaserRunner::clearRunningList()
{
    // empty the running queue
    foreach(ChaserRunnerStep *step, m_runnerSteps)
    {
        step->m_function->stop(functionParent());
        delete step;
    }
    m_runnerSteps.clear();
}

/****************************************************************************
 * Running
 ****************************************************************************/

void ChaserRunner::startNewStep(int index, MasterTimer* timer, bool manualFade, quint32 elapsed)
{
    if (m_chaser == NULL || m_chaser->steps().count() == 0)
        return;

    if (index < 0 || index >= m_chaser->steps().count())
        index = 0; // fallback to the first step

    ChaserStep step(m_chaser->steps().at(index));
    Function *func = m_doc->function(step.fid);
    if (func != NULL)
    {
        ChaserRunnerStep *newStep = new ChaserRunnerStep();
        newStep->m_index = index;
        if (manualFade)
            newStep->m_timings.fadeIn = 0;
        else
            newStep->m_timings.fadeIn = stepFadeIn(index);
        newStep->m_timings.fadeOut = stepFadeOut(index);
        newStep->m_timings.setDuration(stepDuration(index));

        if (m_startOffset != 0)
            newStep->m_elapsed = m_startOffset + MasterTimer::tick();
        else
            newStep->m_elapsed = MasterTimer::tick() + elapsed;
        newStep->m_elapsedBeats = 0; //(newStep->m_elapsed / timer->beatTimeDuration()) * 1000;

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
        newStep->m_function->start(timer, functionParent(), 0, newStep->m_timings, m_chaser->tempoType());
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
        startNewStep(m_lastRunStepIdx, timer, false);
        emit currentStepChanged(m_lastRunStepIdx);
    }

    quint32 prevStepRoundElapsed = 0;


    foreach(ChaserRunnerStep *step, m_runnerSteps)
    {
        if (m_chaser->tempoType() == Function::Beats && timer->isBeat())
        {
            step->m_elapsedBeats += 1000;
            qDebug() << "Function" << step->m_function->name() << "duration:" << step->m_timings.duration() << "beats:" << step->m_elapsedBeats;
        }

        if (step->m_timings.duration() != FunctionTimings::infiniteValue() &&
            ((m_chaser->tempoType() == Function::Time && step->m_elapsed >= step->m_timings.duration()) ||
             (m_chaser->tempoType() == Function::Beats && step->m_elapsedBeats >= step->m_timings.duration())))
        {
            if (step->m_timings.duration() != 0)
                prevStepRoundElapsed = step->m_elapsed % step->m_timings.duration();

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
            if (m_updateOverrideTimings)
            {
                m_updateOverrideTimings = false;
                if (step->m_function != NULL)
                {
                    step->m_function->setOverrideFadeIn(step->m_timings.fadeIn);
                    step->m_function->setOverrideFadeOut(step->m_timings.fadeOut);
                }
            }
        }
    }

    if (m_runnerSteps.isEmpty())
    {
        m_lastRunStepIdx = getNextStepIndex();
        if (m_lastRunStepIdx != -1)
        {
            startNewStep(m_lastRunStepIdx, timer, false, prevStepRoundElapsed);
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
