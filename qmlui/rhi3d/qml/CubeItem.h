/*
  Q Light Controller Plus
  CubeItem.h

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

#pragma once

#include "qml/MeshItem.h"

class CubeItem : public MeshItem
{
    Q_OBJECT
    Q_PROPERTY(QVector3D baseColor READ baseColor WRITE setBaseColor NOTIFY baseColorChanged)
    Q_PROPERTY(QVector3D emissiveColor READ emissiveColor WRITE setEmissiveColor NOTIFY emissiveColorChanged)
    Q_PROPERTY(float metalness READ metalness WRITE setMetalness NOTIFY metalnessChanged)
    Q_PROPERTY(float roughness READ roughness WRITE setRoughness NOTIFY roughnessChanged)

public:
    explicit CubeItem(QObject *parent = nullptr);

    MeshType type() const override { return MeshType::Cube; }

    QVector3D baseColor() const { return m_baseColor; }
    void setBaseColor(const QVector3D &color);
    QVector3D emissiveColor() const { return m_emissiveColor; }
    void setEmissiveColor(const QVector3D &color);
    float metalness() const { return m_metalness; }
    void setMetalness(float metalness);
    float roughness() const { return m_roughness; }
    void setRoughness(float roughness);

Q_SIGNALS:
    void baseColorChanged();
    void emissiveColorChanged();
    void metalnessChanged();
    void roughnessChanged();

private:
    QVector3D m_baseColor = QVector3D(0.7f, 0.7f, 0.7f);
    QVector3D m_emissiveColor = QVector3D(0.0f, 0.0f, 0.0f);
    float m_metalness = 0.0f;
    float m_roughness = 0.5f;
};
