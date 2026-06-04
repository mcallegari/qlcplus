/*
  Q Light Controller Plus
  LightItem.cpp

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

#include "qml/LightItem.h"

#include <QtMath>
#include <QtQuick/QQuickItem>

LightItem::LightItem(QObject *parent)
    : QObject(parent)
{
}

void LightItem::setType(Type type)
{
    if (m_type == type)
        return;
    m_type = type;
    emit typeChanged();
    notifyParent();
}

void LightItem::setPosition(const QVector3D &position)
{
    if (m_position == position)
        return;
    m_position = position;
    emit positionChanged();
    notifyParent();
}

void LightItem::setDirection(const QVector3D &direction)
{
    if (m_direction == direction)
        return;
    m_direction = direction;
    emit directionChanged();
    notifyParent();
}

void LightItem::setColor(const QVector3D &color)
{
    if (m_color == color)
        return;
    m_color = color;
    emit colorChanged();
    notifyParent();
}

void LightItem::setIntensity(float intensity)
{
    if (qFuzzyCompare(m_intensity, intensity))
        return;
    m_intensity = intensity;
    emit intensityChanged();
    notifyParent();
}

void LightItem::setRange(float range)
{
    if (qFuzzyCompare(m_range, range))
        return;
    m_range = range;
    emit rangeChanged();
    notifyParent();
}

void LightItem::setConeAngle(float angle)
{
    if (qFuzzyCompare(m_coneAngle, angle))
        return;
    m_coneAngle = angle;
    emit coneAngleChanged();
    notifyParent();
}

void LightItem::setCastShadows(bool cast)
{
    if (m_castShadows == cast)
        return;
    m_castShadows = cast;
    emit castShadowsChanged();
    notifyParent();
}

void LightItem::setQualitySteps(int steps)
{
    if (m_qualitySteps == steps)
        return;
    m_qualitySteps = steps;
    emit qualityStepsChanged();
    notifyParent();
}

void LightItem::setSize(const QSizeF &size)
{
    if (m_size == size)
        return;
    m_size = size;
    emit sizeChanged();
    notifyParent();
}

void LightItem::setGoboPath(const QString &path)
{
    if (m_goboPath == path)
        return;
    m_goboPath = path;
    emit goboPathChanged();
    notifyParent();
}

void LightItem::setBeamRadius(float radius)
{
    if (qFuzzyCompare(m_beamRadius, radius))
        return;
    m_beamRadius = radius;
    emit beamRadiusChanged();
    notifyParent();
}

void LightItem::setBeamShape(BeamShapeType shape)
{
    if (m_beamShape == shape)
        return;
    m_beamShape = shape;
    emit beamShapeChanged();
    notifyParent();
}

Light LightItem::toLight() const
{
    Light light;
    switch (m_type)
    {
    case Ambient:
        light.type = Light::Type::Point;
        break;
    case Directional:
        light.type = Light::Type::Directional;
        break;
    case Point:
        light.type = Light::Type::Point;
        break;
    case Spotlight:
        light.type = Light::Type::Spot;
        break;
    case Area:
        light.type = Light::Type::Area;
        break;
    }
    light.position = m_position;
    light.direction = m_direction;
    light.color = m_color;
    light.intensity = m_intensity;
    light.range = m_range;
    const float coneAngle = qDegreesToRadians(m_coneAngle);
    light.outerCone = coneAngle;
    light.innerCone = coneAngle * 0.8f;
    light.castShadows = m_castShadows;
    light.qualitySteps = m_qualitySteps;
    light.areaSize = QVector2D(float(m_size.width()), float(m_size.height()));
    light.goboPath = m_goboPath;
    light.beamRadius = m_beamRadius;
    light.beamShape = static_cast<Light::BeamShapeType>(m_beamShape);
    return light;
}

void LightItem::notifyParent()
{
    QObject *p = parent();
    while (p && !qobject_cast<QQuickItem *>(p))
        p = p->parent();
    if (auto *item = qobject_cast<QQuickItem *>(p))
        item->update();
}
