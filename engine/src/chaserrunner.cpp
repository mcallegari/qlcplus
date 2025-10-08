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
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
#include <QRandomGenerator>
#endif
#include <QDebug>

#include "chaserrunner.h"
#include "mastertimer.h"
#include "chaserstep.h"
#include "qlcmacros.h"
#include "chaser.h"
#include "scene.h"
#include "doc.h"

ChaserRunner::ChaserRunner(const Doc *doc, const Chaser *chaser, quint32 startTime)
    : QObject(NULL)
    , m_doc(doc)
    , m_chaser(chaser)
    , m_updateOverrideSpeeds(false)
    , m_startOffset(0)
    , m_lastRunStepIdx(-1)
    , m_lastFunctionID(Function::invalidId())
    , m_roundTime(new QElapsedTimer())
    , m_order()
{
    Q_ASSERT(chaser != NULL);

    m_pendingAction.m_action = ChaserNoAction;
    m_pendingAction.m_masterIntensity = 1.0;
    m_pendingAction.m_stepIntensity = 1.0;
    m_pendingAction.m_fadeMode = Chaser::FromFunction;
    m_pendingAction.m_stepIndex = -1;

    if (startTime > 0)
    {
        qDebug() << "[ChaserRunner] startTime:" << startTime;
        int idx = 0;
        quint32 stepsTime = 0;
        foreach (ChaserStep step, chaser->steps())
        {
            uint duration = m_chaser->durationMode() == Chaser::Common ? m_chaser->duration() : step.duration;

            if (startTime < stepsTime + duration)
            {
                m_pendingAction.m_action = ChaserSetStepIndex;
                m_pendingAction.m_stepIndex = idx;
                m_startOffset = startTime - stepsTime;
                qDebug() << "[ChaserRunner] Starting from step:" << idx;
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
 * Speed
 ****************************************************************************/

void ChaserRunner::slotChaserChanged()
{
    // Handle (possible) speed change on the next write() pass
    m_updateOverrideSpeeds = true;
    QList<ChaserRunnerStep*> delList;
    foreach (ChaserRunnerStep *step, m_runnerSteps)
    {
        if (!m_chaser->steps().contains(ChaserStep(step->m_function->id())))
        {
            // Disappearing function: remove step
            delList.append(step);
        }
        else
        {
            // Recalculate the speed of each running step
            step->m_fadeIn = stepFadeIn(step->m_index);
            step->m_fadeOut = stepFadeOut(step->m_index);
            step->m_duration = stepDuration(step->m_index);
        }
    }
    foreach (ChaserRunnerStep *step, delList)
    {
        step->m_function->stop(functionParent());
        m_runnerSteps.removeAll(step);
        delete step;
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
                if (stepIdx >= 0 && stepIdx < m_chaser->stepsCount())
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
                if (stepIdx >= 0 && stepIdx < m_chaser->stepsCount())
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
                if (stepIdx >= 0 && stepIdx < m_chaser->stepsCount())
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

void ChaserRunner::setAction(ChaserAction &action)
{
    // apply the actions that can be applied immediately
    switch (action.m_action)
    {
        case ChaserNoAction:
            m_pendingAction.m_masterIntensity = action.m_masterIntensity;
            m_pendingAction.m_stepIntensity = action.m_stepIntensity;
        break;

        case ChaserStopStep:
        {
            bool stopped = false;

            foreach (ChaserRunnerStep *step, m_runnerSteps)
            {
                if (action.m_stepIndex == step->m_index)
                {
                    qDebug() << "[ChaserRunner] Stopping step idx:" << action.m_stepIndex << "(running:" << m_runnerSteps.count() << ")";
                    m_lastFunctionID = step->m_function->type() == Function::SceneType ? step->m_function->id() : Function::invalidId();
                    step->m_function->stop(functionParent());
                    m_runnerSteps.removeOne(step);
                    delete step;
                    stopped = true;
                }
            }

            if (stopped && m_runnerSteps.size() == 1)
            {
                ChaserRunnerStep *lastStep = m_runnerSteps.at(0);
                m_lastRunStepIdx = lastStep->m_index;
                emit currentStepChanged(m_lastRunStepIdx);
            }
        }
        break;

        // copy to pending action. Will be processed at the next write call
        default:
            m_pendingAction.m_stepIndex = action.m_stepIndex;
            m_pendingAction.m_masterIntensity = action.m_masterIntensity;
            m_pendingAction.m_stepIntensity = action.m_stepIntensity;
            m_pendingAction.m_fadeMode = action.m_fadeMode;
            m_pendingAction.m_action = action.m_action;
        break;
    }
}

void ChaserRunner::tap()
{
    if (uint(m_roundTime->elapsed()) >= (stepDuration(m_lastRunStepIdx) / 4))
        m_pendingAction.m_action = ChaserNextStep;
}

int ChaserRunner::currentStepIndex() const
{
    return m_lastRunStepIdx;
}

int ChaserRunner::runningStepsNumber() const
{
    return m_runnerSteps.count();
}

ChaserRunnerStep *ChaserRunner::currentRunningStep() const
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
            qDebug() << "[ChaserRunner] steps order not found";
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

    if (nextStep < m_chaser->stepsCount() && nextStep >= 0)
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
            if (nextStep >= m_chaser->stepsCount())
                nextStep = 0;
            else
                nextStep = m_chaser->stepsCount() - 1; // Used by CueList with manual prev
        }
        else // Backward
        {
            if (nextStep < 0)
                nextStep = m_chaser->stepsCount() - 1;
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
            nextStep = m_chaser->stepsCount() - 2;
        }
        else // Backwards
        {
            nextStep = 1;
        }

        // Make sure we don't go beyond limits.
        nextStep = CLAMP(nextStep, 0, m_chaser->stepsCount() - 1);
    }

    return nextStep;
}

void ChaserRunner::shuffle(QVector<int> & data)
{
   int n = data.size();
   for (int i = n - 1; i > 0; --i)
   {
#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
      qSwap(data[i], data[qrand() % (i + 1)]);
#else
      qSwap(data[i], data[QRandomGenerator::global()->generate() % (i + 1)]);
#endif
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
    fillOrder(m_chaser->stepsCount());
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

void ChaserRunner::adjustStepIntensity(qreal fraction, int requestedStepIndex, int fadeControl)
{
    fraction = CLAMP(fraction, qreal(0.0), qreal(1.0));

    //qDebug() << "Adjust intensity" << fraction << "step:" << requestedStepIndex << "fade:" << fadeControl;

    int stepIndex = requestedStepIndex;
    if (stepIndex == -1)
    {
        stepIndex = m_lastRunStepIdx;
        // store the intensity to be applied at the next step startup
        m_pendingAction.m_masterIntensity = fraction;
    }

    foreach (ChaserRunnerStep *step, m_runnerSteps)
    {
        if (stepIndex == step->m_index && step->m_function != NULL)
        {
            if (requestedStepIndex == -1 && step->m_function->type() == Function::SceneType)
            {
                Scene *scene = qobject_cast<Scene *>(step->m_function);
                scene->adjustAttribute(fraction, step->m_pIntensityOverrideId);
            }
            else
            {
                step->m_function->adjustAttribute(fraction, step->m_intensityOverrideId);
            }
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
    startNewStep(stepIndex, m_doc->masterTimer(), m_pendingAction.m_masterIntensity, fraction, fadeControl);
}

/****************************************************************************
 * Running
 ****************************************************************************/

void ChaserRunner::clearRunningList()
{
    // empty the running queue
    foreach (ChaserRunnerStep *step, m_runnerSteps)
    {
        if (step->m_function)
        {
            // restore the original Function fade out time
            step->m_function->setOverrideFadeOutSpeed(stepFadeOut(step->m_index));
            step->m_function->stop(functionParent(), m_chaser->type() == Function::SequenceType);
            m_lastFunctionID = step->m_function->type() == Function::SceneType ? step->m_function->id() : Function::invalidId();
        }
        delete step;
    }
    m_runnerSteps.clear();
}

void ChaserRunner::startNewStep(int index, MasterTimer *timer, qreal mIntensity, qreal sIntensity,
                                int fadeControl, quint32 elapsed)
{
    if (m_chaser == NULL || m_chaser->stepsCount() == 0)
        return;

    if (index < 0 || index >= m_chaser->stepsCount())
        index = 0; // fallback to the first step

    ChaserStep step(m_chaser->steps().at(index));
    Function *func = m_doc->function(step.fid);
    if (func == NULL)
        return;

    ChaserRunnerStep *newStep = new ChaserRunnerStep();
    newStep->m_index = index;

    // check if blending between Scenes is needed
    if (m_lastFunctionID != Function::invalidId() &&
        func->type() == Function::SceneType)
    {
        Scene *scene = qobject_cast<Scene *>(func);
        scene->setBlendFunctionID(m_lastFunctionID);
    }

    // this happens only during crossfades
    if (m_runnerSteps.count())
    {
        ChaserRunnerStep *lastStep = m_runnerSteps.last();
        if (lastStep->m_function &&
            lastStep->m_function->type() == Function::SceneType &&
            func->type() == Function::SceneType)
        {
            Scene *lastScene = qobject_cast<Scene *>(lastStep->m_function);
            lastScene->setBlendFunctionID(Function::invalidId());
            Scene *scene = qobject_cast<Scene *>(func);
            scene->setBlendFunctionID(lastStep->m_function->id());
        }
    }

    switch (fadeControl)
    {
        case Chaser::FromFunction:
            newStep->m_fadeIn = stepFadeIn(index);
            newStep->m_fadeOut = stepFadeOut(index);
        break;
        case Chaser::Blended:
            newStep->m_fadeIn = stepFadeIn(index);
            newStep->m_fadeOut = stepFadeOut(index);
        break;
        case Chaser::Crossfade:
            newStep->m_fadeIn = 0;
            newStep->m_fadeOut = 0;
        break;
        case Chaser::BlendedCrossfade:
            newStep->m_fadeIn = 0;
            newStep->m_fadeOut = 0;
        break;
    }

    newStep->m_duration = stepDuration(index);

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

    qDebug() << "[ChaserRunner] Starting step" << index << "fade in" << newStep->m_fadeIn
             << "fade out" << newStep->m_fadeOut << "intensity" << mIntensity
             << "fadeMode" << fadeControl;

    // Set intensity before starting the function. Otherwise the intensity
    // might momentarily jump too high.
    if (func->type() == Function::SceneType)
    {
        Scene *scene = qobject_cast<Scene *>(func);
        newStep->m_intensityOverrideId = func->requestAttributeOverride(Function::Intensity, sIntensity);
        newStep->m_pIntensityOverrideId = scene->requestAttributeOverride(Scene::ParentIntensity, mIntensity);
        qDebug() << "[ChaserRunner] Set step intensity:" << sIntensity << ", master:" << mIntensity;
    }
    else
    {
        newStep->m_intensityOverrideId = func->requestAttributeOverride(Function::Intensity, mIntensity * sIntensity);
    }

    // Start the fire up!
    func->start(timer, functionParent(), 0, newStep->m_fadeIn, newStep->m_fadeOut,
                func->defaultSpeed(), m_chaser->tempoType());
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
            qDebug() << "[ChaserRunner] steps order not found";
            currentStepIndex = m_lastRunStepIdx;
        }
    }

    if (currentStepIndex == -1 &&
        m_chaser->direction() == Function::Backward)
            currentStepIndex = m_chaser->stepsCount();

    // Handle reverse Ping Pong at boundaries
    if (m_chaser->runOrder() == Function::PingPong &&
        m_pendingAction.m_action == ChaserPreviousStep)
    {
        if (currentStepIndex == 0)
            m_direction = Function::Backward;
        else if (currentStepIndex == m_chaser->stepsCount() - 1)
            m_direction = Function::Forward;
    }

    // Next step
    if (m_direction == Function::Forward)
    {
        // "Previous" for a forward chaser is -1
        if (m_pendingAction.m_action == ChaserPreviousStep)
            currentStepIndex--;
        else
            currentStepIndex++;
    }
    else
    {
        // "Previous" for a backward scene is +1
        if (m_pendingAction.m_action == ChaserPreviousStep)
            currentStepIndex++;
        else
            currentStepIndex--;
    }

    if (currentStepIndex < m_chaser->stepsCount() && currentStepIndex >= 0)
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
            if (currentStepIndex >= m_chaser->stepsCount())
                currentStepIndex = 0;
            else
                currentStepIndex = m_chaser->stepsCount() - 1; // Used by CueList with manual prev
        }
        else // Backward
        {
            if (currentStepIndex < 0)
                currentStepIndex = m_chaser->stepsCount() - 1;
            else
                currentStepIndex = 0;
        }
    }
    else if (m_chaser->runOrder() == Function::Random)
    {
        fillOrder();
        if (m_direction == Function::Forward)
        {
            if (currentStepIndex >= m_chaser->stepsCount())
                currentStepIndex = 0;
            else
                currentStepIndex = m_chaser->stepsCount() - 1; // Used by CueList with manual prev
        }
        else // Backward
        {
            if (currentStepIndex < 0)
                currentStepIndex = m_chaser->stepsCount() - 1;
            else
                currentStepIndex = 0;
        }
        // Don't run the same function 2 times in a row
        while (currentStepIndex < m_chaser->stepsCount()
                && randomStepIndex(currentStepIndex) == m_lastRunStepIdx)
            ++currentStepIndex;
        currentStepIndex = randomStepIndex(currentStepIndex);
    }
    else // Ping Pong
    {
        // Change direction, but don't run the first/last step twice.
        if (m_direction == Function::Forward)
        {
            currentStepIndex = m_chaser->stepsCount() - 2;
            m_direction = Function::Backward;
        }
        else // Backwards
        {
            currentStepIndex = 1;
            m_direction = Function::Forward;
        }

        // Make sure we don't go beyond limits.
        currentStepIndex = CLAMP(currentStepIndex, 0, m_chaser->stepsCount() - 1);
    }

    return currentStepIndex;
}

void ChaserRunner::setPause(bool enable, QList<Universe *> universes)
{
    // Nothing to do
    if (m_chaser->stepsCount() == 0)
        return;

    qDebug() << "[ChaserRunner] processing pause request:" << enable;

    foreach (ChaserRunnerStep *step, m_runnerSteps)
        step->m_function->setPause(enable);

    // there might be a Scene fading out, so request pause
    // to faders bound to the Scene ID running on universes
    Function *f = m_doc->function(m_lastFunctionID);
    if (f != NULL && f->type() == Function::SceneType)
    {
        foreach (Universe *universe, universes)
            universe->setFaderPause(m_lastFunctionID, enable);
    }
}

FunctionParent ChaserRunner::functionParent() const
{
    return FunctionParent(FunctionParent::Function, m_chaser->id());
}

bool ChaserRunner::write(MasterTimer *timer, QList<Universe *> universes)
{
    // Nothing to do
    if (m_chaser->stepsCount() == 0)
        return false;

    switch (m_pendingAction.m_action)
    {
        case ChaserNextStep:
        case ChaserPreviousStep:
            clearRunningList();
            // the actual action will be performed below, on startNewStep
        break;
        case ChaserSetStepIndex:
            if (m_pendingAction.m_stepIndex != -1)
            {
                clearRunningList();
                if (m_chaser->runOrder() == Function::Random)
                    m_lastRunStepIdx = randomStepIndex(m_pendingAction.m_stepIndex);
                else
                    m_lastRunStepIdx = m_pendingAction.m_stepIndex;

                qDebug() << "[ChaserRunner] Starting from step" << m_lastRunStepIdx << "@ offset" << m_startOffset;
                startNewStep(m_lastRunStepIdx, timer, m_pendingAction.m_masterIntensity,
                             m_pendingAction.m_stepIntensity, m_pendingAction.m_fadeMode);
                emit currentStepChanged(m_lastRunStepIdx);
            }
        break;
        case ChaserPauseRequest:
            setPause(m_pendingAction.m_fadeMode ? true : false, universes);
        break;
        default:
        break;
    }

    quint32 prevStepRoundElapsed = 0;

    foreach (ChaserRunnerStep *step, m_runnerSteps)
    {
        if (m_chaser->tempoType() == Function::Beats && timer->isBeat())
        {
            step->m_elapsedBeats += 1000;
            qDebug() << "[ChaserRunner] Function" << step->m_function->name() << "duration:" << step->m_duration << "beats:" << step->m_elapsedBeats;
        }

        if (step->m_duration != Function::infiniteSpeed() &&
            ((m_chaser->tempoType() == Function::Time && step->m_elapsed >= step->m_duration) ||
             (m_chaser->tempoType() == Function::Beats && step->m_elapsedBeats >= step->m_duration)))
        {
            if (step->m_duration != 0)
                prevStepRoundElapsed = step->m_elapsed % step->m_duration;

            m_lastFunctionID = step->m_function->type() == Function::SceneType ? step->m_function->id() : Function::invalidId();
            step->m_function->stop(functionParent(), m_chaser->type() == Function::SequenceType);
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
            int blend = m_pendingAction.m_action == ChaserNoAction ? Chaser::FromFunction : m_pendingAction.m_fadeMode;

            startNewStep(m_lastRunStepIdx, timer, m_pendingAction.m_masterIntensity,
                         m_pendingAction.m_stepIntensity, blend, prevStepRoundElapsed);
            emit currentStepChanged(m_lastRunStepIdx);
        }
        else
        {
            m_pendingAction.m_action = ChaserNoAction;
            return false;
        }
    }

    m_pendingAction.m_action = ChaserNoAction;
    return true;
}

void ChaserRunner::postRun(MasterTimer *timer, QList<Universe*> universes)
{
    Q_UNUSED(universes);
    Q_UNUSED(timer);

    qDebug() << Q_FUNC_INFO;
    clearRunningList();
}
