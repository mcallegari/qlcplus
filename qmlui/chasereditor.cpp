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
#include "chaser.h"

ChaserEditor::ChaserEditor(QQuickView *view, Doc *doc, QObject *parent)
    : FunctionEditor(view, doc, parent)
    , m_chaser(NULL)
{
    m_view->rootContext()->setContextProperty("chaserEditor", this);
}

void ChaserEditor::setFunctionID(quint32 ID)
{
    m_chaser = qobject_cast<Chaser *>(m_doc->function(ID));
    FunctionEditor::setFunctionID(ID);
}

QVariant ChaserEditor::stepsList() const
{
    QVariantList stepList;

    if (m_chaser != NULL)
    {
        foreach(ChaserStep step, m_chaser->steps())
        {
            QVariantMap stepMap;
            stepMap.insert("funcID", step.fid);

            switch (m_chaser->fadeInMode())
            {
                case Chaser::Common:
                    stepMap.insert("fadeIn", Function::speedToString(m_chaser->fadeInSpeed()));
                break;
                case Chaser::PerStep:
                    stepMap.insert("fadeIn", Function::speedToString(step.fadeIn));
                break;
                default:
                    stepMap.insert("fadeIn", QString());
                break;
            }

            switch (m_chaser->fadeOutMode())
            {
                case Chaser::Common:
                    stepMap.insert("fadeOut", Function::speedToString(m_chaser->fadeOutSpeed()));
                break;
                case Chaser::PerStep:
                    stepMap.insert("fadeOut", Function::speedToString(step.fadeOut));
                    break;
                default:
                    stepMap.insert("fadeOut", QString());
                break;
            }

            switch (m_chaser->durationMode())
            {
                default:
                case Chaser::Common:
                    step.duration = m_chaser->duration();
                    step.hold = Function::speedSubstract(step.duration, step.fadeIn);
                case Chaser::PerStep:
                    stepMap.insert("hold", Function::speedToString(step.hold));
                    stepMap.insert("duration", Function::speedToString(step.duration));
                break;
            }

            stepMap.insert("note", step.note);
            stepList.append(stepMap);
        }
    }

    return QVariant::fromValue(stepList);
}

QString ChaserEditor::chaserName() const
{
    if (m_chaser == NULL)
        return "";
    return m_chaser->name();
}

void ChaserEditor::setChaserName(QString chaserName)
{
    if (m_chaser == NULL || m_chaser->name() == chaserName)
        return;

    m_chaser->setName(chaserName);
    emit chaserNameChanged(chaserName);
}

bool ChaserEditor::addFunction(quint32 fid, int insertIndex)
{
    if (m_chaser == NULL)
        return false;

    ChaserStep step(fid);
    m_chaser->addStep(step, insertIndex);
    emit stepsListChanged();
    return true;
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

int ChaserEditor::stepsFadeIn() const
{
    if (m_chaser == NULL)
        return Chaser::Default;

    return m_chaser->fadeInMode();
}

void ChaserEditor::setStepsFadeIn(int stepsFadeIn)
{
    if (m_chaser == NULL || m_chaser->fadeInMode() == Chaser::SpeedMode(stepsFadeIn))
        return;

    m_chaser->setFadeInMode(Chaser::SpeedMode(stepsFadeIn));

    emit stepsFadeInChanged(stepsFadeIn);
    emit stepsListChanged();
}

int ChaserEditor::stepsFadeOut() const
{
    if (m_chaser == NULL)
        return Chaser::Default;

    return m_chaser->fadeOutMode();
}

void ChaserEditor::setStepsFadeOut(int stepsFadeOut)
{
    if (m_chaser == NULL || m_chaser->fadeOutMode() == Chaser::SpeedMode(stepsFadeOut))
        return;

    m_chaser->setFadeOutMode(Chaser::SpeedMode(stepsFadeOut));

    emit stepsFadeOutChanged(stepsFadeOut);
    emit stepsListChanged();
}

int ChaserEditor::stepsDuration() const
{
    if (m_chaser == NULL)
        return Chaser::Default;

    return m_chaser->durationMode();
}

void ChaserEditor::setStepsDuration(int stepsDuration)
{
    if (m_chaser == NULL || m_chaser->durationMode() == Chaser::SpeedMode(stepsDuration))
        return;

    m_chaser->setDurationMode(Chaser::SpeedMode(stepsDuration));

    emit stepsDurationChanged(stepsDuration);
    emit stepsListChanged();
}
