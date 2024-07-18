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
#include "tardis.h"

ChaserEditor::ChaserEditor(QQuickView *view, Doc *doc, QObject *parent)
    : FunctionEditor(view, doc, parent)
    , m_chaser(nullptr)
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
        disconnect(m_chaser, &Chaser::currentStepChanged, this, &ChaserEditor::slotStepIndexChanged);

    m_chaser = qobject_cast<Chaser *>(m_doc->function(ID));
    FunctionEditor::setFunctionID(ID);
    if (m_chaser != nullptr)
        connect(m_chaser, &Chaser::currentStepChanged, this, &ChaserEditor::slotStepIndexChanged);

    updateStepsList(m_doc, m_chaser, m_stepsList);
    emit stepsListChanged();
}

bool ChaserEditor::isSequence() const
{
    if (m_chaser != nullptr && m_chaser->type() == Function::SequenceType)
        return true;

    return false;
}

QVariant ChaserEditor::stepsList() const
{
    return QVariant::fromValue(m_stepsList);
}

bool ChaserEditor::addFunctions(QVariantList idsList, int insertIndex)
{
    if (m_chaser == nullptr || idsList.count() == 0)
        return false;

    if (insertIndex == -1)
        insertIndex = m_chaser->stepsCount();

    for (QVariant vID : idsList) // C++11
    {
        quint32 fid = vID.toUInt();

        // do not allow a Chaser to add itself as step
        if (fid == m_chaser->id())
            continue;

        ChaserStep step(fid);
        if (m_chaser->durationMode() == Chaser::PerStep)
        {
            Function *func = m_doc->function(fid);
            if (func == nullptr)
                continue;

            step.duration = func->totalDuration();
            if (step.duration == 0)
                step.duration = 1000;
            step.hold = step.duration;
        }
        m_chaser->addStep(step, insertIndex++);

        Tardis::instance()->enqueueAction(Tardis::ChaserAddStep, m_chaser->id(), QVariant(),
                                          Tardis::instance()->actionToByteArray(Tardis::ChaserAddStep, m_chaser->id(), insertIndex - 1));
    }

    updateStepsList(m_doc, m_chaser, m_stepsList);
    emit stepsListChanged();

    return true;
}

bool ChaserEditor::addStep(int insertIndex)
{
    if (m_chaser == nullptr || m_chaser->type() != Function::SequenceType)
    {
        qDebug() << "This is not a Sequence";
        return false;
    }

    Sequence *sequence = qobject_cast<Sequence*>(m_chaser);
    ChaserStep step(sequence->boundSceneID());
    Scene *currScene = qobject_cast<Scene*> (m_doc->function(sequence->boundSceneID()));

    if (currScene == nullptr)
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

    m_chaser->addStep(step, insertIndex++);

    Tardis::instance()->enqueueAction(Tardis::ChaserAddStep, m_chaser->id(), QVariant(),
                                      Tardis::instance()->actionToByteArray(Tardis::ChaserAddStep, m_chaser->id(), insertIndex));

    addStepToListModel(m_doc, m_chaser, m_stepsList, &step);
    //setPlaybackIndex(insertIndex);

    return true;
}

bool ChaserEditor::moveSteps(QVariantList indicesList, int insertIndex)
{
    if (m_chaser == nullptr || indicesList.count() == 0)
        return false;

    QVector<int>sortedList;
    bool firstDecreased = false;

    if (insertIndex == -1)
        insertIndex = m_chaser->stepsCount() - 1;

    int insIdx = insertIndex;

    // create a list of ordered step indices
    for (QVariant &vIndex : indicesList)
    {
        int idx = vIndex.toInt();
        sortedList.append(idx);
    }
    std::sort(sortedList.begin(), sortedList.end());

    for (int i = 0; i < sortedList.count(); i++)
    {
        int index = sortedList.at(i);

        // when moving an item down, every other step with index < destination
        // needs to have their index decreased by one
        if (index < insIdx)
        {
            if (firstDecreased == false)
            {
                insIdx--;
                firstDecreased = true;
            }

            for (int j = i + 1; j < sortedList.count(); j++)
                sortedList[j]--;
            insertIndex--;
        }

        qDebug() << "Moving step from" << index << "to" << insIdx;
        Tardis::instance()->enqueueAction(Tardis::ChaserMoveStep, m_chaser->id(), index, insIdx);
        m_chaser->moveStep(index, insIdx);

        if (index > insIdx)
            insIdx++;
    }

    updateStepsList(m_doc, m_chaser, m_stepsList);
    emit stepsListChanged();

    QQuickItem *chaserWidget = qobject_cast<QQuickItem*>(m_view->rootObject()->findChild<QObject *>("chaserEditorWidget"));

    for (int i = 0; i < indicesList.length(); i++)
    {
        QMetaObject::invokeMethod(chaserWidget, "selectStep",
                Q_ARG(QVariant, insertIndex + i),
                Q_ARG(QVariant, i == 0 ? false : true));
    }

    return true;
}

bool ChaserEditor::duplicateSteps(QVariantList indicesList)
{
    if (m_chaser == nullptr || indicesList.count() == 0)
        return false;

    for (QVariant &vIndex : indicesList)
    {
        int stepIndex = vIndex.toInt();
        ChaserStep *sourceStep = m_chaser->stepAt(stepIndex);
        ChaserStep step(*sourceStep);
        m_chaser->addStep(step);

        Tardis::instance()->enqueueAction(Tardis::ChaserAddStep, m_chaser->id(), QVariant(),
            Tardis::instance()->actionToByteArray(Tardis::ChaserAddStep, m_chaser->id(), m_chaser->stepsCount() - 1));
    }

    updateStepsList(m_doc, m_chaser, m_stepsList);
    emit stepsListChanged();

    return true;
}

void ChaserEditor::setSequenceStepValue(SceneValue &scv)
{
    if (m_chaser == nullptr || m_chaser->type() != Function::SequenceType)
        return;

    if (m_playbackIndex < 0 || m_playbackIndex >= m_chaser->stepsCount())
        return;

    ChaserStep *cs = m_chaser->stepAt(m_playbackIndex);
    cs->setValue(scv);
}

int ChaserEditor::playbackIndex() const
{
    return m_playbackIndex;
}

void ChaserEditor::setPlaybackIndex(int playbackIndex)
{
    if (m_playbackIndex == playbackIndex)
        return;

    if (m_chaser != nullptr && m_previewEnabled == false &&
        m_chaser->type() == Function::SequenceType && playbackIndex >= 0)
    {
        Sequence *sequence = qobject_cast<Sequence*>(m_chaser);
        Scene *currScene = qobject_cast<Scene*> (m_doc->function(sequence->boundSceneID()));

        if (currScene != nullptr)
        {
            for (SceneValue &scv : m_chaser->stepAt(playbackIndex)->values)
                currScene->setValue(scv);
        }
    }

    m_playbackIndex = playbackIndex;
    emit playbackIndexChanged(playbackIndex);
}

void ChaserEditor::setPreviewEnabled(bool enable)
{
    if (m_chaser != nullptr && m_playbackIndex >= 0)
    {
        ChaserAction action;
        action.m_action = ChaserSetStepIndex;
        action.m_stepIndex = m_playbackIndex;
        action.m_masterIntensity = 1.0;
        action.m_stepIntensity = 1.0;
        action.m_fadeMode = Chaser::FromFunction;
        m_chaser->setAction(action);
    }

    FunctionEditor::setPreviewEnabled(enable);
}

void ChaserEditor::gotoPreviousStep()
{
    ChaserAction action;
    action.m_action = ChaserPreviousStep;
    action.m_masterIntensity = 1.0;
    action.m_stepIntensity = 1.0;
    action.m_fadeMode = Chaser::FromFunction;
    m_chaser->setAction(action);
}

void ChaserEditor::gotoNextStep()
{
    ChaserAction action;
    action.m_action = ChaserNextStep;
    action.m_masterIntensity = 1.0;
    action.m_stepIntensity = 1.0;
    action.m_fadeMode = Chaser::FromFunction;
    m_chaser->setAction(action);
}

void ChaserEditor::deleteItems(QVariantList list)
{
    if (m_chaser == nullptr)
        return;

    std::sort(list.begin(), list.end(),
              [](QVariant a, QVariant b) {
                  return a.toUInt() < b.toUInt();
              });

    qDebug() << "Chaser delete list" << list;

    while (!list.isEmpty())
    {
        Tardis::instance()->enqueueAction(Tardis::ChaserRemoveStep, m_chaser->id(),
                                          Tardis::instance()->actionToByteArray(Tardis::ChaserRemoveStep,
                                                                                m_chaser->id(), list.last().toUInt()),
                                          QVariant());

        m_chaser->removeStep(list.last().toInt());
        list.removeLast();
    }

    updateStepsList(m_doc, m_chaser, m_stepsList);
    emit stepsListChanged();
}

void ChaserEditor::removeFixtures(QVariantList list)
{
    if (m_chaser == nullptr)
        return;

    Sequence *sequence = qobject_cast<Sequence *>(m_chaser);
    Scene *scene = qobject_cast<Scene *>(m_doc->function(sequence->boundSceneID()));
    if (scene == nullptr)
        return;

    // transform the list of fixture indices into a list of fixture IDs
    QList<quint32> sceneFixtureList = scene->fixtures();
    QList<quint32> fixtureIdList;
    for (QVariant &fIndex : list)
        fixtureIdList.append(sceneFixtureList.at(fIndex.toInt()));

    // run though steps and search for matching fixture IDs
    for (int i = 0; i < m_chaser->stepsCount(); i++)
    {
        ChaserStep *step = m_chaser->stepAt(i);
        QMutableListIterator<SceneValue> it(step->values);
        while (it.hasNext())
        {
            SceneValue scv = it.next();
            if (fixtureIdList.contains(scv.fxi))
                it.remove();
        }
    }
}

void ChaserEditor::slotStepIndexChanged(int index)
{
    setPlaybackIndex(index);
}

QVariantMap ChaserEditor::stepDataMap(Doc *doc, Chaser *chaser, ChaserStep *step)
{
    QVariantMap stepMap;
    Function *func = doc->function(step->fid);

    stepMap.insert("funcID", step->fid);
    stepMap.insert("isSelected", false);

    switch (chaser->fadeInMode())
    {
        case Chaser::Common:
            stepMap.insert("fadeIn", chaser->fadeInSpeed());
        break;
        case Chaser::PerStep:
            stepMap.insert("fadeIn", step->fadeIn);
        break;
        default:
            stepMap.insert("fadeIn", func->fadeInSpeed());
        break;
    }

    switch (chaser->fadeOutMode())
    {
        case Chaser::Common:
            stepMap.insert("fadeOut", chaser->fadeOutSpeed());
        break;
        case Chaser::PerStep:
            stepMap.insert("fadeOut", step->fadeOut);
        break;
        default:
            stepMap.insert("fadeOut", func->fadeOutSpeed());
        break;
    }

    switch (chaser->durationMode())
    {
        case Chaser::Common:
            step->duration = chaser->duration();
            step->hold = Function::speedSubtract(step->duration, step->fadeIn);
            stepMap.insert("hold", int(step->hold));
            stepMap.insert("duration", int(step->duration));
        break;
        case Chaser::PerStep:
            stepMap.insert("hold", int(step->hold));
            stepMap.insert("duration", int(step->duration));
        break;
        default:
            step->duration = func->totalDuration();
            step->hold = Function::speedSubtract(func->totalDuration(), func->fadeInSpeed());
            stepMap.insert("hold", int(step->hold));
            stepMap.insert("duration", int(step->duration));
        break;
    }

    stepMap.insert("note", step->note);

    return stepMap;
}

void ChaserEditor::addStepToListModel(Doc *doc, Chaser *chaser, ListModel *stepsList, ChaserStep *step)
{
    QVariantMap stepMap = stepDataMap(doc, chaser, step);
    stepsList->addDataMap(stepMap);
}

void ChaserEditor::updateStepInListModel(Doc *doc, Chaser *chaser, ListModel *stepsList, ChaserStep *step, int index)
{
    QVariantMap stepMap = stepDataMap(doc, chaser, step);
    QModelIndex idx = stepsList->index(index, 0, QModelIndex());
    stepsList->setDataMap(idx, stepMap);
}

void ChaserEditor::updateStepsList(Doc *doc, Chaser *chaser, ListModel *stepsList)
{
    if (doc == nullptr || chaser == nullptr || stepsList == nullptr)
        return;

    stepsList->clear();

    if (chaser == nullptr)
        return;

    for (int i = 0; i < chaser->stepsCount(); i++)
    {
        ChaserStep *step = chaser->stepAt(i);
        addStepToListModel(doc, chaser, stepsList, step);
    }
}

void ChaserEditor::setSelectedValue(Function::PropType type, QString param, uint value, bool selectedOnly)
{
    if (m_chaser == nullptr)
        return;

    for (quint32 i = 0; i < quint32(m_chaser->stepsCount()); i++)
    {
        QModelIndex idx = m_stepsList->index(int(i), 0, QModelIndex());
        QVariant isSelected = true;

        if (selectedOnly == true)
            isSelected = m_stepsList->data(idx, "isSelected");

        if (isSelected.isValid() && isSelected.toBool() == true)
        {
            m_stepsList->setDataWithRole(idx, param, int(value));

            ChaserStep step = m_chaser->steps().at(int(i));
            uint duration = m_chaser->durationMode() == Chaser::Common ? m_chaser->duration() : step.duration;

            /* Now update also the Chaser step */
            switch(type)
            {
                case Function::FadeIn:
                {
                    UIntPair oldFadeIn(i, step.fadeIn);
                    step.fadeIn = value;
                    Tardis::instance()->enqueueAction(Tardis::ChaserSetStepFadeIn, m_chaser->id(), QVariant::fromValue(oldFadeIn),
                                                      QVariant::fromValue(UIntPair(i, step.fadeIn)));

                    if (m_chaser->durationMode() == Chaser::Common)
                    {
                        UIntPair oldHold(i, step.hold);
                        step.hold = Function::speedSubtract(duration, step.fadeIn);
                        Tardis::instance()->enqueueAction(Tardis::ChaserSetStepHold, m_chaser->id(), QVariant::fromValue(oldHold),
                                                          QVariant::fromValue(UIntPair(i, step.hold)));
                        m_stepsList->setDataWithRole(idx, "hold", int(step.hold));
                    }
                    else
                    {
                        UIntPair oldDuration(i, step.duration);
                        step.duration = Function::speedAdd(step.fadeIn, step.hold);
                        Tardis::instance()->enqueueAction(Tardis::ChaserSetStepDuration, m_chaser->id(), QVariant::fromValue(oldDuration),
                                                          QVariant::fromValue(UIntPair(i, step.duration)));
                        m_stepsList->setDataWithRole(idx, "duration", int(step.duration));
                    }
                }
                break;
                case Function::Hold:
                {
                    UIntPair oldHold(i, step.hold);
                    step.hold = value;
                    Tardis::instance()->enqueueAction(Tardis::ChaserSetStepHold, m_chaser->id(), QVariant::fromValue(oldHold),
                                                      QVariant::fromValue(UIntPair(i, step.hold)));

                    if (m_chaser->durationMode() == Chaser::Common)
                    {
                        UIntPair oldFadeIn(i, step.fadeIn);
                        step.fadeIn = Function::speedSubtract(duration, step.hold);
                        Tardis::instance()->enqueueAction(Tardis::ChaserSetStepFadeIn, m_chaser->id(), QVariant::fromValue(oldFadeIn),
                                                          QVariant::fromValue(UIntPair(i, step.fadeIn)));
                        m_stepsList->setDataWithRole(idx, "fadeIn", int(step.hold));
                    }
                    else
                    {
                        UIntPair oldDuration(i, step.duration);
                        step.duration = Function::speedAdd(step.fadeIn, step.hold);
                        Tardis::instance()->enqueueAction(Tardis::ChaserSetStepDuration, m_chaser->id(), QVariant::fromValue(oldDuration),
                                                          QVariant::fromValue(UIntPair(i, step.duration)));
                        m_stepsList->setDataWithRole(idx, "duration", int(step.duration));
                    }
                }
                break;
                case Function::FadeOut:
                {
                    UIntPair oldFadeOut(i, step.fadeOut);
                    step.fadeOut = value;
                    Tardis::instance()->enqueueAction(Tardis::ChaserSetStepFadeOut, m_chaser->id(), QVariant::fromValue(oldFadeOut),
                                                      QVariant::fromValue(UIntPair(i, step.fadeOut)));
                    m_stepsList->setDataWithRole(idx, "fadeOut", int(step.fadeOut));
                }
                break;
                case Function::Duration:
                {
                    UIntPair oldDuration(i, step.duration);
                    UIntPair oldHold(i, step.hold);
                    step.duration = duration = value;
                    step.hold = Function::speedSubtract(duration, step.fadeIn);

                    Tardis::instance()->enqueueAction(Tardis::ChaserSetStepDuration, m_chaser->id(), QVariant::fromValue(oldDuration),
                                                      QVariant::fromValue(UIntPair(i, step.duration)));
                    Tardis::instance()->enqueueAction(Tardis::ChaserSetStepHold, m_chaser->id(), QVariant::fromValue(oldHold),
                                                      QVariant::fromValue(UIntPair(i, step.hold)));

                    m_stepsList->setDataWithRole(idx, "hold", int(step.hold));
                }
                break;
                default:
                break;
            }

            m_chaser->replaceStep(step, int(i));
        }
    }
}

/*********************************************************************
 * Steps speed mode
 *********************************************************************/
int ChaserEditor::tempoType() const
{
    if (m_chaser == nullptr)
        return Function::Time;

    return m_chaser->tempoType();
}

void ChaserEditor::setTempoType(int tempoType)
{
    if (m_chaser == nullptr || m_chaser->tempoType() == Function::TempoType(tempoType))
        return;

    m_chaser->setTempoType(Function::TempoType(tempoType));

    int beatDuration = m_doc->masterTimer()->beatTimeDuration();
    quint32 index = 0;

    for (ChaserStep &step : m_chaser->steps())
    {
        UIntPair oldDuration(index, step.duration);
        UIntPair oldFadeIn(index, step.fadeIn);
        UIntPair oldFadeOut(index, step.fadeOut);
        UIntPair oldHold(index, step.hold);

        // Time -> Beats
        if (tempoType == Function::Beats)
        {
            step.fadeIn = Function::timeToBeats(step.fadeIn, beatDuration);
            step.hold = Function::timeToBeats(step.hold, beatDuration);
            step.fadeOut = Function::timeToBeats(step.fadeOut, beatDuration);
        }
        // Beats -> Time
        else
        {
            step.fadeIn = Function::beatsToTime(step.fadeIn, beatDuration);
            step.hold = Function::beatsToTime(step.hold, beatDuration);
            step.fadeOut = Function::beatsToTime(step.fadeOut, beatDuration);
        }

        Tardis::instance()->enqueueAction(Tardis::ChaserSetStepFadeIn, m_chaser->id(), QVariant::fromValue(oldFadeIn),
                                          QVariant::fromValue(UIntPair(index, step.fadeIn)));
        Tardis::instance()->enqueueAction(Tardis::ChaserSetStepHold, m_chaser->id(), QVariant::fromValue(oldHold),
                                          QVariant::fromValue(UIntPair(index, step.hold)));
        Tardis::instance()->enqueueAction(Tardis::ChaserSetStepFadeOut, m_chaser->id(), QVariant::fromValue(oldFadeOut),
                                          QVariant::fromValue(UIntPair(index, step.fadeOut)));

        step.duration = step.fadeIn + step.hold;
        Tardis::instance()->enqueueAction(Tardis::ChaserSetStepDuration, m_chaser->id(), QVariant::fromValue(oldDuration),
                                          QVariant::fromValue(UIntPair(index, step.duration)));

        m_chaser->replaceStep(step, int(index));
        index++;
    }

    emit tempoTypeChanged(tempoType);
    updateStepsList(m_doc, m_chaser, m_stepsList);
    emit stepsListChanged();
}

int ChaserEditor::stepsFadeIn() const
{
    if (m_chaser == nullptr)
        return Chaser::Default;

    return m_chaser->fadeInMode();
}

void ChaserEditor::setStepsFadeIn(int stepsFadeIn)
{
    if (m_chaser == nullptr || m_chaser->fadeInMode() == Chaser::SpeedMode(stepsFadeIn))
        return;

    m_chaser->setFadeInMode(Chaser::SpeedMode(stepsFadeIn));

    emit stepsFadeInChanged(stepsFadeIn);
    updateStepsList(m_doc, m_chaser, m_stepsList);
    emit stepsListChanged();
}

int ChaserEditor::stepsFadeOut() const
{
    if (m_chaser == nullptr)
        return Chaser::Default;

    return m_chaser->fadeOutMode();
}

void ChaserEditor::setStepsFadeOut(int stepsFadeOut)
{
    if (m_chaser == nullptr || m_chaser->fadeOutMode() == Chaser::SpeedMode(stepsFadeOut))
        return;

    m_chaser->setFadeOutMode(Chaser::SpeedMode(stepsFadeOut));

    emit stepsFadeOutChanged(stepsFadeOut);
    updateStepsList(m_doc, m_chaser, m_stepsList);
    emit stepsListChanged();
}

int ChaserEditor::stepsDuration() const
{
    if (m_chaser == nullptr)
        return Chaser::Default;

    return m_chaser->durationMode();
}

void ChaserEditor::setStepsDuration(int stepsDuration)
{
    if (m_chaser == nullptr || m_chaser->durationMode() == Chaser::SpeedMode(stepsDuration))
        return;

    m_chaser->setDurationMode(Chaser::SpeedMode(stepsDuration));

    emit stepsDurationChanged(stepsDuration);
    updateStepsList(m_doc, m_chaser, m_stepsList);
    emit stepsListChanged();
}

void ChaserEditor::setStepSpeed(int index, int value, int type)
{
    if (m_chaser == nullptr || index < 0 || index >= m_chaser->stepsCount())
        return;

    switch (Function::PropType(type))
    {
        case Function::FadeIn:
        {
            if (m_chaser->fadeInMode() == Chaser::Common)
            {
                Tardis::instance()->enqueueAction(Tardis::FunctionSetFadeIn, m_chaser->id(), m_chaser->fadeInSpeed(), value);
                m_chaser->setFadeInSpeed(value);
                setSelectedValue(Function::FadeIn, "fadeIn", uint(value), false);
            }
            else if (m_chaser->fadeInMode() == Chaser::PerStep)
            {
                setSelectedValue(Function::FadeIn, "fadeIn", uint(value));
            }
        }
        break;
        case Function::Hold:
            if (m_chaser->durationMode() == Chaser::Common)
            {
                Tardis::instance()->enqueueAction(Tardis::FunctionSetDuration, m_chaser->id(), m_chaser->duration(), value);
                m_chaser->setDuration(value);
                setSelectedValue(Function::Duration, "hold", uint(value), false);
                setSelectedValue(Function::Duration, "duration", uint(value), false);
            }
            else
            {
                setSelectedValue(Function::Hold, "hold", uint(value));
            }
        break;
        case Function::FadeOut:
            if (m_chaser->fadeOutMode() == Chaser::Common)
            {
                Tardis::instance()->enqueueAction(Tardis::FunctionSetFadeOut, m_chaser->id(), m_chaser->fadeOutSpeed(), value);
                m_chaser->setFadeOutSpeed(value);
                setSelectedValue(Function::FadeOut, "fadeOut", uint(value), false);
            }
            else if (m_chaser->fadeOutMode() == Chaser::PerStep)
            {
                setSelectedValue(Function::FadeOut, "fadeOut", uint(value));
            }
        break;
        case Function::Duration:
            if (m_chaser->durationMode() == Chaser::Common)
            {
                Tardis::instance()->enqueueAction(Tardis::FunctionSetDuration, m_chaser->id(), m_chaser->duration(), value);
                m_chaser->setDuration(value);
                setSelectedValue(Function::Duration, "duration", uint(value), false);
            }
            else
            {
                setSelectedValue(Function::Duration, "duration", uint(value));
            }
        break;
        default:
        break;
    }

}

void ChaserEditor::setStepNote(int index, QString text)
{
    if (m_chaser == nullptr || index < 0 || index >= m_chaser->stepsCount())
        return;

    ChaserStep step = m_chaser->steps().at(int(index));
    step.note = text;
    m_chaser->replaceStep(step, index);

    QModelIndex mIdx = m_stepsList->index(index, 0, QModelIndex());
    m_stepsList->setDataWithRole(mIdx, "note", text);
}
