/*
  Q Light Controller Plus
  functioneditor.cpp

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

#include "functioneditor.h"
#include "tardis.h"
#include "doc.h"

FunctionEditor::FunctionEditor(QQuickView *view, Doc *doc, QObject *parent)
    : QObject(parent)
    , m_view(view)
    , m_doc(doc)
    , m_functionID(Function::invalidId())
    , m_previousID(-1)
    , m_function(nullptr)
    , m_functionType(Function::Undefined)
    , m_previewEnabled(false)
{
}

FunctionEditor::~FunctionEditor()
{

}

void FunctionEditor::setFunctionID(quint32 ID)
{
    bool wasRunning = false;

    if (m_function != nullptr && m_function->isRunning())
    {
        wasRunning = true;
        m_function->stop(FunctionParent::master());
    }

    m_functionID = ID;
    m_function = m_doc->function(ID);
    if (m_function != nullptr)
        m_functionType = m_function->type();

    if (wasRunning)
        m_function->start(m_doc->masterTimer(), FunctionParent::master());
}

quint32 FunctionEditor::functionID() const
{
    return m_functionID;
}

Function::Type FunctionEditor::functionType() const
{
    return m_functionType;
}

bool FunctionEditor::previewEnabled() const
{
    return m_previewEnabled;
}

void FunctionEditor::setPreviewEnabled(bool enable)
{
    if (m_previewEnabled == enable)
        return;

    m_previewEnabled = enable;

    if (m_function == nullptr)
        return;

    if (m_previewEnabled)
    {
        if (m_function->isRunning() == false)
            m_function->start(m_doc->masterTimer(), FunctionParent::master());
    }
    else
    {
        if (m_function->isRunning())
            m_function->stop(FunctionParent::master());
    }
    emit previewEnabledChanged(enable);
}

QString FunctionEditor::functionName() const
{
    if (m_function == nullptr)
        return "";

    return m_function->name();
}

void FunctionEditor::setFunctionName(QString functionName)
{
    if (m_function == nullptr || m_function->name() == functionName)
        return;

    Tardis::instance()->enqueueAction(Tardis::FunctionSetName, m_function->id(), m_function->name(), functionName);

    m_function->setName(functionName);
    emit functionNameChanged(functionName);
}

int FunctionEditor::previousID() const
{
    return m_previousID;
}

void FunctionEditor::setPreviousID(int previousID)
{
    qDebug() << "Previous ID" << previousID;
    if (m_previousID == previousID)
        return;

    m_previousID = previousID;
    emit previousIDChanged(m_previousID);
}

/************************************************************************
 * Speed
 ************************************************************************/

int FunctionEditor::tempoType() const
{
    if (m_function == nullptr)
        return Function::Time;

    return m_function->tempoType();
}

void FunctionEditor::setTempoType(int tempoType)
{
    if (m_function == nullptr || m_function->tempoType() == Function::TempoType(tempoType))
        return;

    Tardis::instance()->enqueueAction(Tardis::FunctionSetTempoType, m_function->id(), m_function->tempoType(), tempoType);

    m_function->setTempoType(Function::TempoType(tempoType));

    int beatDuration = m_doc->masterTimer()->beatTimeDuration();

    // Time -> Beats
    if (tempoType == Function::Beats)
    {
        uint fadeIn = Function::timeToBeats(m_function->fadeInSpeed(), beatDuration);
        uint fadeOut = Function::timeToBeats(m_function->fadeOutSpeed(), beatDuration);
        uint duration = Function::timeToBeats(m_function->duration(), beatDuration);

        Tardis::instance()->enqueueAction(Tardis::FunctionSetFadeIn, m_function->id(), m_function->fadeInSpeed(), fadeIn);
        Tardis::instance()->enqueueAction(Tardis::FunctionSetDuration, m_function->id(), m_function->duration(), duration);
        Tardis::instance()->enqueueAction(Tardis::FunctionSetFadeOut, m_function->id(), m_function->fadeOutSpeed(), fadeOut);

        m_function->setFadeInSpeed(fadeIn);
        m_function->setDuration(duration);
        m_function->setFadeOutSpeed(fadeOut);
    }
    // Beats -> Time
    else
    {
        uint fadeIn = Function::beatsToTime(m_function->fadeInSpeed(), beatDuration);
        uint fadeOut = Function::beatsToTime(m_function->fadeOutSpeed(), beatDuration);
        uint duration = Function::beatsToTime(m_function->duration(), beatDuration);

        Tardis::instance()->enqueueAction(Tardis::FunctionSetFadeIn, m_function->id(), m_function->fadeInSpeed(), fadeIn);
        Tardis::instance()->enqueueAction(Tardis::FunctionSetDuration, m_function->id(), m_function->duration(), duration);
        Tardis::instance()->enqueueAction(Tardis::FunctionSetFadeOut, m_function->id(), m_function->fadeOutSpeed(), fadeOut);

        m_function->setFadeInSpeed(fadeIn);
        m_function->setDuration(duration);
        m_function->setFadeOutSpeed(fadeOut);
    }

    emit tempoTypeChanged(tempoType);
}

int FunctionEditor::fadeInSpeed() const
{
    if (m_function == nullptr)
        return Function::defaultSpeed();

    return m_function->fadeInSpeed();
}

void FunctionEditor::setFadeInSpeed(int fadeInSpeed)
{
    if (m_function == nullptr)
        return;

    if (m_function->fadeInSpeed() == (uint)fadeInSpeed)
        return;

    Tardis::instance()->enqueueAction(Tardis::FunctionSetFadeIn, m_function->id(), m_function->fadeInSpeed(), fadeInSpeed);
    m_function->setFadeInSpeed(fadeInSpeed);
    emit fadeInSpeedChanged(fadeInSpeed);
}

int FunctionEditor::holdSpeed() const
{
    if (m_function == nullptr)
        return Function::defaultSpeed();

    return m_function->duration() - m_function->fadeInSpeed();
}

void FunctionEditor::setHoldSpeed(int holdSpeed)
{
    if (m_function == nullptr)
        return;

    if (m_function->duration() - m_function->fadeInSpeed() == (uint)holdSpeed)
        return;

    uint duration = Function::speedAdd(m_function->fadeInSpeed(), holdSpeed);
    Tardis::instance()->enqueueAction(Tardis::FunctionSetDuration, m_function->id(), m_function->duration(), duration);
    m_function->setDuration(duration);

    emit holdSpeedChanged(holdSpeed);
    emit durationChanged(duration);
}

int FunctionEditor::fadeOutSpeed() const
{
    if (m_function == nullptr)
        return Function::defaultSpeed();

    return m_function->fadeOutSpeed();
}

void FunctionEditor::setFadeOutSpeed(int fadeOutSpeed)
{
    if (m_function == nullptr)
        return;

    if (m_function->fadeOutSpeed() == (uint)fadeOutSpeed)
        return;

    Tardis::instance()->enqueueAction(Tardis::FunctionSetFadeOut, m_function->id(), m_function->fadeOutSpeed(), fadeOutSpeed);
    m_function->setFadeOutSpeed(fadeOutSpeed);
    emit fadeOutSpeedChanged(fadeOutSpeed);
}

int FunctionEditor::duration() const
{
    if (m_function == nullptr)
        return Function::defaultSpeed();

    return m_function->duration();
}

/************************************************************************
 * Run order and direction
 ************************************************************************/

int FunctionEditor::runOrder() const
{
    if (m_function == nullptr)
        return Function::Loop;

    return m_function->runOrder();
}

void FunctionEditor::setRunOrder(int runOrder)
{
    if (m_function == nullptr || m_function->runOrder() == Function::RunOrder(runOrder))
        return;

    Tardis::instance()->enqueueAction(Tardis::FunctionSetRunOrder, m_function->id(), m_function->runOrder(), runOrder);

    m_function->setRunOrder(Function::RunOrder(runOrder));
    emit runOrderChanged(runOrder);
}

int FunctionEditor::direction() const
{
    if (m_function == nullptr)
        return Function::Forward;

    return m_function->direction();
}

void FunctionEditor::setDirection(int direction)
{
    if (m_function == nullptr || m_function->direction() == Function::Direction(direction))
        return;

    Tardis::instance()->enqueueAction(Tardis::FunctionSetDirection, m_function->id(), m_function->direction(), direction);

    m_function->setDirection(Function::Direction(direction));
    emit directionChanged(direction);
}

void FunctionEditor::deleteItems(QVariantList list)
{
    Q_UNUSED(list)
}

