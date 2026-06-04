/*
  Q Light Controller Plus
  CubeItem.cpp

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

#include "qml/CubeItem.h"

CubeItem::CubeItem(QObject *parent)
    : MeshItem(parent)
{
}

void CubeItem::setBaseColor(const QVector3D &color)
{
    if (m_baseColor == color)
        return;
    m_baseColor = color;
    emit baseColorChanged();
    notifyParent();
}

void CubeItem::setEmissiveColor(const QVector3D &color)
{
    if (m_emissiveColor == color)
        return;
    m_emissiveColor = color;
    emit emissiveColorChanged();
    notifyParent();
}

void CubeItem::setMetalness(float metalness)
{
    if (qFuzzyCompare(m_metalness, metalness))
        return;
    m_metalness = metalness;
    emit metalnessChanged();
    notifyParent();
}

void CubeItem::setRoughness(float roughness)
{
    if (qFuzzyCompare(m_roughness, roughness))
        return;
    m_roughness = roughness;
    emit roughnessChanged();
    notifyParent();
}
