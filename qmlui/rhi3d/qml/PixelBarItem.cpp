/*
  Q Light Controller Plus
  PixelBarItem.cpp

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

#include "qml/PixelBarItem.h"

PixelBarItem::PixelBarItem(QObject *parent)
    : MeshItem(parent)
{
}

void PixelBarItem::setEmitterCount(int count)
{
    const int next = qMax(1, count);
    if (m_emitterCount == next)
        return;
    m_emitterCount = next;
    emit emitterCountChanged();
    notifyParent();
}

void PixelBarItem::setBaseColor(const QVector3D &color)
{
    if (m_baseColor == color)
        return;
    m_baseColor = color;
    emit baseColorChanged();
    notifyParent();
}

void PixelBarItem::setEmissiveColor(const QVector3D &color)
{
    if (m_emissiveColor == color)
        return;
    m_emissiveColor = color;
    emit emissiveColorChanged();
    notifyParent();
}

QVariantList PixelBarItem::emitterColors() const
{
    QVariantList list;
    list.reserve(m_emitterColors.size());
    for (const QVector3D &color : m_emitterColors)
        list.push_back(QVariant::fromValue(color));
    return list;
}

void PixelBarItem::setEmitterColors(const QVariantList &colors)
{
    QVector<QVector3D> next;
    next.reserve(colors.size());
    for (const QVariant &value : colors)
    {
        if (value.canConvert<QVector3D>())
            next.push_back(value.value<QVector3D>());
    }
    if (m_emitterColors == next)
        return;
    m_emitterColors = next;
    emit emitterColorsChanged();
    notifyParent();
}

QVariantList PixelBarItem::emitterIntensities() const
{
    QVariantList list;
    list.reserve(m_emitterIntensities.size());
    for (float intensity : m_emitterIntensities)
        list.push_back(intensity);
    return list;
}

void PixelBarItem::setEmitterIntensities(const QVariantList &intensities)
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
