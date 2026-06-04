/*
  Q Light Controller Plus
  HazerItem.cpp

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

#include "qml/HazerItem.h"

#include <QtQuick/QQuickItem>

HazerItem::HazerItem(QObject *parent)
    : QObject(parent)
{
}

void HazerItem::setPosition(const QVector3D &position)
{
    if (m_position == position)
        return;
    m_position = position;
    emit positionChanged();
    notifyParent();
}

void HazerItem::setDirection(const QVector3D &direction)
{
    if (m_direction == direction)
        return;
    m_direction = direction;
    emit directionChanged();
    notifyParent();
}

void HazerItem::setLength(float length)
{
    if (qFuzzyCompare(m_length, length))
        return;
    m_length = length;
    emit lengthChanged();
    notifyParent();
}

void HazerItem::setRadius(float radius)
{
    if (qFuzzyCompare(m_radius, radius))
        return;
    m_radius = radius;
    emit radiusChanged();
    notifyParent();
}

void HazerItem::setDensity(float density)
{
    if (qFuzzyCompare(m_density, density))
        return;
    m_density = density;
    emit densityChanged();
    notifyParent();
}

void HazerItem::setEnabled(bool enabled)
{
    if (m_enabled == enabled)
        return;
    m_enabled = enabled;
    emit enabledChanged();
    notifyParent();
}

void HazerItem::notifyParent()
{
    auto *item = qobject_cast<QQuickItem *>(parent());
    if (item)
        item->update();
}
