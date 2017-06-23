/*
  Q Light Controller Plus
  chasereditor.cpp

  Copyright (c) Massimo Callegari

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

#include "chasereditor.h"
#include "chaserstep.h"
#include "listmodel.h"
#include "sequence.h"
#include "chaser.h"

ChaserEditor::ChaserEditor(QQuickView *view, Doc *doc, QObject *parent)
    : FunctionEditor(view, doc, parent)
    , m_chaser(NULL)
    , m_playbackIndex(-1)
{
    m_view->rootContext()->setContextProperty("chaserEditor", this);

    m_stepsList = new ListModel(this);
    QStringList listRoles;
    listRoles << "funcID" << "isSelected" << "fadeIn" << "hold" << "fadeOut" << "duration" << "note";
    m_stepsList->setRoleNames(listRoles);
}

void ChaserEditor::setFunctionID(quint32 ID)
{
    if (m_chaser)
        disconnect(m_chaser, &Chaser::currentStepChanged, this, &ChaserEditor::slotStepChanged);

    m_chaser = qobject_cast<Chaser *>(m_doc->function(ID));
    FunctionEditor::setFunctionID(ID);
    if (m_chaser != NULL)
        connect(m_chaser, &Chaser::currentStepChanged, this, &ChaserEditor::slotStepChanged);

    updateStepsList();
}

bool ChaserEditor::isSequence() const
{
    if (m_chaser != NULL && m_chaser->type() == Function::SequenceType)
        return true;

    return false;
}

QVariant ChaserEditor::stepsList() const
{
    return QVariant::fromValue(m_stepsList);
}

bool ChaserEditor::addFunctions(QVariantList idsList, int insertIndex)
{
    if (m_chaser == NULL || idsList.count() == 0)
        return false;

    if (insertIndex == -1)
        insertIndex = m_chaser->stepsCount();

    for (QVariant vID : idsList) // C++11
    {
        quint32 fid = vID.toUInt();
        ChaserStep step(fid);
        if (m_chaser->durationMode() == Chaser::PerStep)
        {
            Function *func = m_doc->function(fid);
            if (func == NULL)
                continue;

            step.speeds.setDuration(func->totalRoundDuration());
            if (step.speeds.duration() == 0)
                step.speeds.setDuration(1000);
        }
        m_chaser->addStep(step, insertIndex++);
    }
    updateStepsList();
    return true;
}

bool ChaserEditor::addStep(int insertIndex)
{
    if (m_chaser == NULL || m_chaser->type() != Function::SequenceType)
    {
        qDebug() << "This is not a Sequence";
        return false;
    }

    Sequence *sequence = qobject_cast<Sequence*>(m_chaser);
    ChaserStep step(sequence->boundSceneID());
    Scene *currScene = qobject_cast<Scene*> (m_doc->function(sequence->boundSceneID()));

    if (currScene == NULL)
    {
        qDebug() << "The Sequence bound Scene is NULL";
        return false;
    }

    if (m_chaser->stepsCount() == 0)
    {
        QListIterator <SceneValue> it(currScene->values());
        while (it.hasNext() == true)
        {
            SceneValue chan(it.next());
            step.values.append(chan);
            //qDebug() << "Value added: " << chan.value;
        }
    }
    else
    {
        if (insertIndex == -1)
            step.values = m_chaser->stepAt(m_chaser->stepsCount() - 1)->values;
        else
            step.values = m_chaser->stepAt(insertIndex)->values;
    }

    qDebug() << "Values added: " << step.values.count();

    m_chaser->addStep(step, insertIndex);
    updateStepsList();
    return true;
}

int ChaserEditor::playbackIndex() const
{
    return m_playbackIndex;
}

void ChaserEditor::setPlaybackIndex(int playbackIndex)
{
    if (m_playbackIndex == playbackIndex)
        return;

    if (m_chaser != NULL && m_chaser->type() == Function::SequenceType && playbackIndex >= 0)
    {
        Sequence *sequence = qobject_cast<Sequence*>(m_chaser);
        Scene *currScene = qobject_cast<Scene*> (m_doc->function(sequence->boundSceneID()));

        if (currScene != NULL)
        {
            for(SceneValue scv : m_chaser->stepAt(playbackIndex)->values)
                currScene->setValue(scv);
        }
    }

    m_playbackIndex = playbackIndex;
    emit playbackIndexChanged(playbackIndex);
}

void ChaserEditor::setPreviewEnabled(bool enable)
{
    if (m_chaser != NULL && m_playbackIndex >= 0)
        m_chaser->setStepIndex(m_playbackIndex);

    FunctionEditor::setPreviewEnabled(enable);
}

void ChaserEditor::slotStepChanged(int index)
{
    setPlaybackIndex(index);
}

void ChaserEditor::updateStepsList()
{
    m_stepsList->clear();

    if (m_chaser != NULL)
    {
        foreach(ChaserStep step, m_chaser->steps())
        {
            QVariantMap stepMap;
            Function *func = m_doc->function(step.fid);

            stepMap.insert("funcID", step.fid);
            stepMap.insert("isSelected", false);

            switch (m_chaser->fadeInMode())
            {
                case Chaser::Common:
                    stepMap.insert("fadeIn", m_chaser->commonSpeeds().fadeIn());
                break;
                case Chaser::PerStep:
                    stepMap.insert("fadeIn", step.speeds.fadeIn());
                break;
                default:
                    stepMap.insert("fadeIn", func->speeds().fadeIn());
                break;
            }

            switch (m_chaser->fadeOutMode())
            {
                case Chaser::Common:
                    stepMap.insert("fadeOut", m_chaser->commonSpeeds().fadeOut());
                break;
                case Chaser::PerStep:
                    stepMap.insert("fadeOut", step.speeds.fadeOut());
                    break;
                default:
                    stepMap.insert("fadeOut", func->speeds().fadeOut());
                break;
            }

            switch (m_chaser->durationMode())
            {
                case Chaser::Common:
                    step.speeds.setHold(m_chaser->commonSpeeds().hold());
                    stepMap.insert("hold", (int)step.speeds.hold());
                    stepMap.insert("duration", (int)step.speeds.duration());
                break;
                case Chaser::PerStep:
                    stepMap.insert("hold", (int)step.speeds.hold());
                    stepMap.insert("duration", (int)step.speeds.duration());
                break;
                default:
                    step.speeds.setHold(func->speeds().hold());
                    stepMap.insert("hold", (int)step.speeds.hold());
                    stepMap.insert("duration", (int)step.speeds.duration());
                break;
            }

            stepMap.insert("note", step.note);
            m_stepsList->addDataMap(stepMap);
        }
    }
    emit stepsListChanged();
}

void ChaserEditor::setSelectedValue(FunctionSpeeds::SpeedComponentType type,
                                    QString param,
                                    uint value,
                                    bool selectedOnly)
{
    if (m_chaser == NULL)
        return;

    for (int i = 0; i < m_chaser->stepsCount(); i++)
    {
        QModelIndex idx = m_stepsList->index(i, 0, QModelIndex());

        QVariant isSelected = true;
        if (selectedOnly == true)
            isSelected = m_stepsList->data(idx, "isSelected");

        if (isSelected.isValid() && isSelected.toBool() == true)
        {
            m_stepsList->setDataWithRole(idx, param, value);

            ChaserStep step = m_chaser->steps().at(i);
            uint duration = m_chaser->durationMode() == Chaser::Common
                                ? m_chaser->commonSpeeds().duration()
                                : step.speeds.duration();

            /* Now update also the Chaser step */
            switch(type)
            {
                case FunctionSpeeds::FadeIn:
                    step.speeds.setFadeIn(value);
                    if (m_chaser->durationMode() == Chaser::Common)
                    {
                        step.speeds.setHold(Speed::sub(duration, step.speeds.fadeIn()));
                        m_stepsList->setDataWithRole(idx, "hold", step.speeds.hold());
                    }
                    else
                    {
                        m_stepsList->setDataWithRole(idx, "duration", step.speeds.duration());
                    }
                    break;
                case FunctionSpeeds::Hold:
                    step.speeds.setHold(value);
                    if (m_chaser->durationMode() == Chaser::Common)
                    {
                        step.speeds.setFadeIn(Speed::sub(duration, step.speeds.hold()));
                        m_stepsList->setDataWithRole(idx, "fadeIn", step.speeds.fadeIn());
                    }
                    else
                    {
                        m_stepsList->setDataWithRole(idx, "duration", step.speeds.duration());
                    }
                    break;
                case FunctionSpeeds::FadeOut:
                    step.speeds.setFadeOut(value);
                    break;
                case FunctionSpeeds::Duration:
                    step.speeds.setDuration(duration);
                    m_stepsList->setDataWithRole(idx, "hold", step.speeds.hold());
                    m_stepsList->setDataWithRole(idx, "fadeIn", step.speeds.fadeIn());
                    break;
            }

            m_chaser->replaceStep(step, i);
        }
    }
}

/*********************************************************************
 * Chaser playback modes
 *********************************************************************/

int ChaserEditor::runOrder() const
{
    if (m_chaser == NULL)
        return Function::Loop;

    return m_chaser->runOrder();
}

void ChaserEditor::setRunOrder(int runOrder)
{
    if (m_chaser == NULL || m_chaser->runOrder() == Function::RunOrder(runOrder))
        return;

    m_chaser->setRunOrder(Function::RunOrder(runOrder));
    emit runOrderChanged(runOrder);
}

int ChaserEditor::direction() const
{
    if (m_chaser == NULL)
        return Function::Forward;

    return m_chaser->direction();
}

void ChaserEditor::setDirection(int direction)
{
    if (m_chaser == NULL || m_chaser->direction() == Function::Direction(direction))
        return;

    m_chaser->setDirection(Function::Direction(direction));
    emit directionChanged(direction);
}

/*********************************************************************
 * Steps speed mode
 *********************************************************************/

int ChaserEditor::tempoType() const
{
    if (m_chaser == NULL)
        return Speed::Ms;

    return m_chaser->commonSpeeds().tempoType();
}

void ChaserEditor::setTempoType(int tempoType)
{
    if (m_chaser == NULL || m_chaser->commonSpeeds().tempoType() == Speed::TempoType(tempoType))
        return;

    int beatDuration = m_doc->masterTimer()->beatTimeDuration();
    int index = 0;

    m_chaser->speedsEdit().setTempoType(Speed::TempoType(tempoType), beatDuration);
    m_chaser->commonSpeedsEdit().setTempoType(Speed::TempoType(tempoType), beatDuration);

    foreach(ChaserStep step, m_chaser->steps())
    {
        step.speeds.setTempoType(Speed::TempoType(tempoType), beatDuration);
        m_chaser->replaceStep(step, index);
        index++;
    }

    emit tempoTypeChanged(tempoType);
    updateStepsList();
}

int ChaserEditor::stepsFadeInMode() const
{
    if (m_chaser == NULL)
        return Chaser::Default;

    return m_chaser->fadeInMode();
}

void ChaserEditor::setStepsFadeInMode(int stepsFadeInMode)
{
    if (m_chaser == NULL || m_chaser->fadeInMode() == Chaser::SpeedMode(stepsFadeInMode))
        return;

    m_chaser->setFadeInMode(Chaser::SpeedMode(stepsFadeInMode));

    emit stepsFadeInModeChanged(stepsFadeInMode);
    updateStepsList();
}

int ChaserEditor::stepsFadeOutMode() const
{
    if (m_chaser == NULL)
        return Chaser::Default;

    return m_chaser->fadeOutMode();
}

void ChaserEditor::setStepsFadeOutMode(int stepsFadeOutMode)
{
    if (m_chaser == NULL || m_chaser->fadeOutMode() == Chaser::SpeedMode(stepsFadeOutMode))
        return;

    m_chaser->setFadeOutMode(Chaser::SpeedMode(stepsFadeOutMode));

    emit stepsFadeOutModeChanged(stepsFadeOutMode);
    updateStepsList();
}

int ChaserEditor::stepsDurationMode() const
{
    if (m_chaser == NULL)
        return Chaser::Default;

    return m_chaser->durationMode();
}

void ChaserEditor::setStepsDurationMode(int stepsDurationMode)
{
    if (m_chaser == NULL || m_chaser->durationMode() == Chaser::SpeedMode(stepsDurationMode))
        return;

    m_chaser->setDurationMode(Chaser::SpeedMode(stepsDurationMode));

    emit stepsDurationModeChanged(stepsDurationMode);
    updateStepsList();
}

void ChaserEditor::setStepSpeed(int index, int value, int type)
{
    if (m_chaser == NULL || index < 0 || index >= m_chaser->stepsCount())
        return;

    switch (FunctionSpeeds::SpeedComponentType(type))
    {
        case FunctionSpeeds::FadeIn:
        {
            if (m_chaser->fadeInMode() == Chaser::Common)
            {
                m_chaser->commonSpeedsEdit().setFadeIn(value);
                setSelectedValue(FunctionSpeeds::FadeIn, "fadeIn", uint(value), false);
            }
            else if (m_chaser->fadeInMode() == Chaser::PerStep)
                setSelectedValue(FunctionSpeeds::FadeIn, "fadeIn", uint(value));
        }
        break;
        case FunctionSpeeds::Hold:
            setSelectedValue(FunctionSpeeds::Hold, "hold", uint(value));
        break;
        case FunctionSpeeds::FadeOut:
            if (m_chaser->fadeOutMode() == Chaser::Common)
            {
                m_chaser->commonSpeedsEdit().setFadeOut(value);
                setSelectedValue(FunctionSpeeds::FadeOut, "fadeOut", uint(value), false);
            }
            else if (m_chaser->fadeOutMode() == Chaser::PerStep)
                setSelectedValue(FunctionSpeeds::FadeOut, "fadeOut", uint(value));
        break;
        case FunctionSpeeds::Duration:
            if (m_chaser->durationMode() == Chaser::Common)
            {
                m_chaser->commonSpeedsEdit().setDuration(value);
                setSelectedValue(FunctionSpeeds::Duration, "duration", uint(value), false);
            }
            else
                setSelectedValue(FunctionSpeeds::Duration, "duration", uint(value));
        break;
    }

}
