/*
  Q Light Controller Plus
  BeamBarItem.cpp

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

#include "qml/BeamBarItem.h"

#include <QtGui/QColor>
#include <QtMath>

BeamBarItem::BeamBarItem(QObject *parent)
    : MeshItem(parent)
{
}

void BeamBarItem::setEmitterCount(int count)
{
    const int next = qMax(1, count);
    if (m_emitterCount == next)
        return;
    m_emitterCount = next;
    emit emitterCountChanged();
    notifyParent();
}

void BeamBarItem::setHeadsLayout(const QSize &layout)
{
    const int w = qMax(1, layout.width());
    const int h = qMax(1, layout.height());
    const QSize next(w, h);
    if (m_headsLayout == next)
        return;
    m_headsLayout = next;
    emit headsLayoutChanged();
    notifyParent();
}

void BeamBarItem::setBaseColor(const QVector3D &color)
{
    if (m_baseColor == color)
        return;
    m_baseColor = color;
    emit baseColorChanged();
    notifyParent();
}

void BeamBarItem::setColor(const QVector3D &color)
{
    if (m_color == color)
        return;
    m_color = color;
    emit colorChanged();
    notifyParent();
}

void BeamBarItem::setIntensity(float intensity)
{
    if (qFuzzyCompare(m_intensity, intensity))
        return;
    m_intensity = intensity;
    emit intensityChanged();
    notifyParent();
}

void BeamBarItem::setRange(float range)
{
    if (qFuzzyCompare(m_range, range))
        return;
    m_range = range;
    emit rangeChanged();
    notifyParent();
}

void BeamBarItem::setBeamRadius(float radius)
{
    if (qFuzzyCompare(m_beamRadius, radius))
        return;
    m_beamRadius = radius;
    emit beamRadiusChanged();
    notifyParent();
}

void BeamBarItem::setCastShadows(bool cast)
{
    if (m_castShadows == cast)
        return;
    m_castShadows = cast;
    emit castShadowsChanged();
    notifyParent();
}

void BeamBarItem::setPan(float panDegrees)
{
    if (qFuzzyCompare(m_pan, panDegrees))
        return;
    m_pan = panDegrees;
    emit panChanged();
    notifyParent();
}

void BeamBarItem::setTilt(float tiltDegrees)
{
    if (qFuzzyCompare(m_tilt, tiltDegrees))
        return;
    m_tilt = tiltDegrees;
    emit tiltChanged();
    notifyParent();
}

void BeamBarItem::setPanRangeDegrees(float degrees)
{
    if (qFuzzyCompare(m_panRangeDegrees, degrees))
        return;
    m_panRangeDegrees = degrees;
    emit panRangeDegreesChanged();
    notifyParent();
}

void BeamBarItem::setTiltRangeDegrees(float degrees)
{
    if (qFuzzyCompare(m_tiltRangeDegrees, degrees))
        return;
    m_tiltRangeDegrees = degrees;
    emit tiltRangeDegreesChanged();
    notifyParent();
}

void BeamBarItem::setPanTilt(const QVariant &panTarget, const QVariant &tiltTarget)
{
    setPan(float(panTarget.toDouble()));
    setTilt(float(tiltTarget.toDouble()));
}

void BeamBarItem::setPositionSpeed(const QVariant &panDuration, const QVariant &tiltDuration)
{
    Q_UNUSED(panDuration)
    Q_UNUSED(tiltDuration)
}

QVariantList BeamBarItem::emitterColors() const
{
    QVariantList list;
    list.reserve(m_emitterColors.size());
    for (const QVector3D &color : m_emitterColors)
        list.push_back(QVariant::fromValue(color));
    return list;
}

void BeamBarItem::setEmitterColors(const QVariantList &colors)
{
    QVector<QVector3D> next;
    next.reserve(colors.size());
    for (const QVariant &value : colors)
    {
        if (value.canConvert<QVector3D>())
        {
            next.push_back(value.value<QVector3D>());
            continue;
        }
        if (value.canConvert<QColor>())
        {
            const QColor color = value.value<QColor>();
            next.push_back(QVector3D(color.redF(), color.greenF(), color.blueF()));
        }
    }
    if (m_emitterColors == next)
        return;
    m_emitterColors = next;
    emit emitterColorsChanged();
    notifyParent();
}

QVariantList BeamBarItem::emitterIntensities() const
{
    QVariantList list;
    list.reserve(m_emitterIntensities.size());
    for (float intensity : m_emitterIntensities)
        list.push_back(intensity);
    return list;
}

void BeamBarItem::setEmitterIntensities(const QVariantList &intensities)
{
    QVector<float> next;
    next.reserve(intensities.size());
    for (const QVariant &value : intensities)
        next.push_back(float(value.toDouble()));
    if (m_emitterIntensities == next)
        return;
    m_emitterIntensities = next;
    emit emitterIntensitiesChanged();
    notifyParent();
}

Light BeamBarItem::toLight() const
{
    Light light;
    light.type = Light::Type::Spot;
    light.position = QVector3D(0.0f, 0.0f, 0.0f);
    light.direction = QVector3D(0.0f, -1.0f, 0.0f);
    light.color = m_color;
    light.intensity = m_intensity;
    light.range = m_range;
    const float coneAngle = qDegreesToRadians(1.0f);
    light.outerCone = coneAngle;
    light.innerCone = coneAngle * 0.8f;
    light.castShadows = m_castShadows;
    light.beamRadius = m_beamRadius;
    light.beamShape = Light::BeamShapeType::BeamShape;
    return light;
}
