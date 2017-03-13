/*
  Q Light Controller Plus
  efxeditor.cpp

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

#include <QTimer>
#include <QDebug>

#include "efxeditor.h"

#include "efx.h"
#include "doc.h"

EFXEditor::EFXEditor(QQuickView *view, Doc *doc, QObject *parent)
    : FunctionEditor(view, doc, parent)
    , m_efx(NULL)
{
    m_view->rootContext()->setContextProperty("efxEditor", this);
}

EFXEditor::~EFXEditor()
{
    m_view->rootContext()->setContextProperty("efxEditor", NULL);
}

void EFXEditor::setFunctionID(quint32 id)
{
    if (id == Function::invalidId())
    {
        m_efx = NULL;
        return;
    }

    m_efx = qobject_cast<EFX *>(m_doc->function(id));
    if (m_efx != NULL)
    {
        updateAlgorithmData();
        emit algorithmIndexChanged();
    }

    FunctionEditor::setFunctionID(id);
}

/************************************************************************
 * Algorithm
 ************************************************************************/

QStringList EFXEditor::algorithms() const
{
    return EFX::algorithmList();
}

int EFXEditor::algorithmIndex() const
{
    if (m_efx == NULL)
        return 0;

    QStringList algoList = algorithms();
    return algoList.indexOf(EFX::algorithmToString(m_efx->algorithm()));
}

void EFXEditor::setAlgorithmIndex(int algoIndex)
{
    if (m_efx == NULL)
        return;

    if (algoIndex == m_efx->algorithm())
        return;

    m_efx->setAlgorithm(EFX::Algorithm(algoIndex));
    emit algorithmIndexChanged();
    updateAlgorithmData();
}

bool EFXEditor::isRelative() const
{
    if (m_efx == NULL)
        return false;

    return m_efx->isRelative();
}

void EFXEditor::setIsRelative(bool val)
{
    if (m_efx == NULL || val == m_efx->isRelative())
        return;

    m_efx->setIsRelative(val);
    emit isRelativeChanged();
    updateAlgorithmData();
}

int EFXEditor::algorithmWidth() const
{
    if (m_efx == NULL)
        return 0;

    return m_efx->width();
}

void EFXEditor::setAlgorithmWidth(int algorithmWidth)
{
    if (m_efx == NULL || algorithmWidth == m_efx->width())
        return;

    m_efx->setWidth(algorithmWidth);
    emit algorithmWidthChanged();
    updateAlgorithmData();
}

int EFXEditor::algorithmHeight() const
{
    if (m_efx == NULL)
        return 0;

    return m_efx->height();
}

void EFXEditor::setAlgorithmHeight(int algorithmHeight)
{
    if (m_efx == NULL || algorithmHeight == m_efx->height())
        return;

    m_efx->setHeight(algorithmHeight);
    emit algorithmHeightChanged();
    updateAlgorithmData();
}

int EFXEditor::algorithmXOffset() const
{
    if (m_efx == NULL)
        return 0;

    return m_efx->xOffset();
}

void EFXEditor::setAlgorithmXOffset(int algorithmXOffset)
{
    if (m_efx == NULL || algorithmXOffset == m_efx->xOffset())
        return;

    m_efx->setXOffset(algorithmXOffset);
    emit algorithmXOffsetChanged();
    updateAlgorithmData();
}

int EFXEditor::algorithmYOffset() const
{
    if (m_efx == NULL)
        return 0;

    return m_efx->yOffset();
}

void EFXEditor::setAlgorithmYOffset(int algorithmYOffset)
{
    if (m_efx == NULL || algorithmYOffset == m_efx->yOffset())
        return;

    m_efx->setYOffset(algorithmYOffset);
    emit algorithmYOffsetChanged();
    updateAlgorithmData();
}

int EFXEditor::algorithmRotation() const
{
    if (m_efx == NULL)
        return 0;

    return m_efx->rotation();
}

void EFXEditor::setAlgorithmRotation(int algorithmRotation)
{
    if (m_efx == NULL || algorithmRotation == m_efx->rotation())
        return;

    m_efx->setRotation(algorithmRotation);
    emit algorithmRotationChanged();
    updateAlgorithmData();
}

int EFXEditor::algorithmStartOffset() const
{
    if (m_efx == NULL)
        return 0;

    return m_efx->startOffset();
}

void EFXEditor::setAlgorithmStartOffset(int algorithmStartOffset)
{
    if (m_efx == NULL || algorithmStartOffset == m_efx->startOffset())
        return;

    m_efx->setStartOffset(algorithmStartOffset);
    emit algorithmStartOffsetChanged();
    updateAlgorithmData();
}

int EFXEditor::algorithmXFrequency() const
{
    if (m_efx == NULL)
        return 0;

    return m_efx->xFrequency();
}

void EFXEditor::setAlgorithmXFrequency(int algorithmXFrequency)
{
    if (m_efx == NULL || algorithmXFrequency == m_efx->xFrequency())
        return;

    m_efx->setXFrequency(algorithmXFrequency);
    emit algorithmXFrequencyChanged();
    updateAlgorithmData();
}

int EFXEditor::algorithmYFrequency() const
{
    if (m_efx == NULL)
        return 0;

    return m_efx->yFrequency();
}

void EFXEditor::setAlgorithmYFrequency(int algorithmYFrequency)
{
    if (m_efx == NULL || algorithmYFrequency == m_efx->yFrequency())
        return;

    m_efx->setYFrequency(algorithmYFrequency);
    emit algorithmYFrequencyChanged();
    updateAlgorithmData();
}

int EFXEditor::algorithmXPhase() const
{
    if (m_efx == NULL)
        return 0;

    return m_efx->xPhase();
}

void EFXEditor::setAlgorithmXPhase(int algorithmXPhase)
{
    if (m_efx == NULL || algorithmXPhase == m_efx->xPhase())
        return;

    m_efx->setXPhase(algorithmXPhase);
    emit algorithmXPhaseChanged();
    updateAlgorithmData();
}

int EFXEditor::algorithmYPhase() const
{
    if (m_efx == NULL)
        return 0;

    return m_efx->yPhase();
}

void EFXEditor::setAlgorithmYPhase(int algorithmYPhase)
{
    if (m_efx == NULL || algorithmYPhase == m_efx->yPhase())
        return;

    m_efx->setYPhase(algorithmYPhase);
    emit algorithmYPhaseChanged();
    updateAlgorithmData();
}

/************************************************************************
 * Run order and direction
 ************************************************************************/

int EFXEditor::runOrder() const
{
    if (m_efx == NULL)
        return Function::Loop;

    return m_efx->runOrder();
}

void EFXEditor::setRunOrder(int runOrder)
{
    if (m_efx == NULL || m_efx->runOrder() == Function::RunOrder(runOrder))
        return;

    m_efx->setRunOrder(Function::RunOrder(runOrder));
    emit runOrderChanged(runOrder);
}

int EFXEditor::direction() const
{
    if (m_efx == NULL)
        return Function::Forward;

    return m_efx->direction();
}

void EFXEditor::setDirection(int direction)
{
    if (m_efx == NULL || m_efx->direction() == Function::Direction(direction))
        return;

    m_efx->setDirection(Function::Direction(direction));
    emit directionChanged(direction);
}

/************************************************************************
 * Preview
 ************************************************************************/

QVariantList EFXEditor::algorithmData()
{
    return m_algorithmData;
}

void EFXEditor::updateAlgorithmData()
{
    if (m_efx == NULL)
        return;

    QPolygonF polygon;
    m_efx->preview(polygon);

    m_algorithmData.clear();

    for (int i = 0; i < polygon.size(); i++)
    {
        QPointF pt = polygon.at(i);
        m_algorithmData.append(pt.x());
        m_algorithmData.append(pt.y());
    }

    emit algorithmDataChanged();
}


