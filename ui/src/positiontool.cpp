/*
  Q Light Controller Plus
  positiontool.cpp

  Copyright (c) Jano Svitok

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
#include <QPoint>
#include <QSettings>

#include "positiontool.h"
#include "vcxypadarea.h"

#define SETTINGS_GEOMETRY "positiontool/geometry"

/*****************************************************************************
 * Initialization
 *****************************************************************************/

PositionTool::PositionTool(const QPointF & initial, QRectF degreesRange, QWidget* parent)
    : QDialog(parent)
{
    setupUi(this);
    m_area = new VCXYPadArea(this);
    setPosition(initial);
    m_area->setMode(Doc::Operate); // to activate the area
    m_area->setWindowTitle("");
    m_area->setDegreesRange(degreesRange);
    m_area->setFocus();
    m_gridLayout->addWidget(m_area, 0, 0);

    QSettings settings;
    QVariant geometrySettings = settings.value(SETTINGS_GEOMETRY);
    if (geometrySettings.isValid() == true)
        restoreGeometry(geometrySettings.toByteArray());

    connect(m_area, SIGNAL(positionChanged(const QPointF &)),
            this, SLOT(slotPositionChanged(const QPointF &)));
}

PositionTool::~PositionTool()
{
    QSettings settings;
    settings.setValue(SETTINGS_GEOMETRY, saveGeometry());
}

/*****************************************************************************
 * Current XY position
 *****************************************************************************/

QPointF PositionTool::position() const
{
    if (m_area == NULL)
        return QPointF(127, 127);

    return m_area->position();
}

void PositionTool::setPosition(const QPointF & position)
{
    m_area->setPosition(position);
}

void PositionTool::slotPositionChanged(const QPointF & position)
{
    emit currentPositionChanged(position);
}

