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
#include "doc.h"

FunctionEditor::FunctionEditor(QQuickView *view, Doc *doc, QObject *parent)
    : QObject(parent)
    , m_view(view)
    , m_doc(doc)
    , m_functionID(Function::invalidId())
    , m_function(NULL)
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

    if (m_function != NULL && m_function->isRunning())
    {
        wasRunning = true;
        m_function->stop(FunctionParent::master());
    }

    m_functionID = ID;
    m_function = m_doc->function(ID);
    if (m_function != NULL)
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

    if (m_function == NULL)
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
    if (m_function == NULL)
        return "";

    return m_function->name();
}

void FunctionEditor::setFunctionName(QString functionName)
{
    if (m_function == NULL || m_function->name() == functionName)
        return;

    m_function->setName(functionName);
    emit functionNameChanged(functionName);
}

int FunctionEditor::tempoType() const
{
    if (m_function == NULL)
        return Function::Time;

    return m_function->tempoType();
}

void FunctionEditor::setTempoType(int tempoType)
{
    if (m_function == NULL || m_function->tempoType() == Function::TempoType(tempoType))
        return;

    m_function->setTempoType(Function::TempoType(tempoType));

    int beatDuration = m_doc->masterTimer()->beatTimeDuration();

    // Time -> Beats
    if (tempoType == Function::Beats)
    {
        m_function->setFadeInSpeed(Function::timeToBeats(m_function->fadeInSpeed(), beatDuration));
        m_function->setDuration(Function::timeToBeats(m_function->duration(), beatDuration));
        m_function->setFadeOutSpeed(Function::timeToBeats(m_function->fadeOutSpeed(), beatDuration));
    }
    // Beats -> Time
    else
    {
        m_function->setFadeInSpeed(Function::beatsToTime(m_function->fadeInSpeed(), beatDuration));
        m_function->setDuration(Function::beatsToTime(m_function->duration(), beatDuration));
        m_function->setFadeOutSpeed(Function::beatsToTime(m_function->fadeOutSpeed(), beatDuration));
    }

    emit tempoTypeChanged(tempoType);
}

void FunctionEditor::deleteItems(QVariantList list)
{
    Q_UNUSED(list)
}

