/*
  Q Light Controller Plus
  PixelBarItem.h

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

#include <QtCore/QVariant>

class PixelBarItem : public MeshItem
{
    Q_OBJECT
    Q_PROPERTY(int emitterCount READ emitterCount WRITE setEmitterCount NOTIFY emitterCountChanged)
    Q_PROPERTY(QVector3D baseColor READ baseColor WRITE setBaseColor NOTIFY baseColorChanged)
    Q_PROPERTY(QVector3D emissiveColor READ emissiveColor WRITE setEmissiveColor NOTIFY emissiveColorChanged)
    Q_PROPERTY(QVariantList emitterColors READ emitterColors WRITE setEmitterColors NOTIFY emitterColorsChanged)
    Q_PROPERTY(QVariantList emitterIntensities READ emitterIntensities WRITE setEmitterIntensities NOTIFY emitterIntensitiesChanged)

public:
    explicit PixelBarItem(QObject *parent = nullptr);

    MeshType type() const override { return MeshType::PixelBar; }

    int emitterCount() const { return m_emitterCount; }
    void setEmitterCount(int count);

    QVector3D baseColor() const { return m_baseColor; }
    void setBaseColor(const QVector3D &color);

    QVector3D emissiveColor() const { return m_emissiveColor; }
    void setEmissiveColor(const QVector3D &color);

    QVariantList emitterColors() const;
    void setEmitterColors(const QVariantList &colors);
    const QVector<QVector3D> &emitterColorsVector() const { return m_emitterColors; }

    QVariantList emitterIntensities() const;
    void setEmitterIntensities(const QVariantList &intensities);
    const QVector<float> &emitterIntensitiesVector() const { return m_emitterIntensities; }

Q_SIGNALS:
    void emitterCountChanged();
    void baseColorChanged();
    void emissiveColorChanged();
    void emitterColorsChanged();
    void emitterIntensitiesChanged();

private:
    int m_emitterCount = 1;
    QVector3D m_baseColor = QVector3D(0.64f, 0.64f, 0.64f);
    QVector3D m_emissiveColor = QVector3D(3.0f, 0.6f, 0.2f);
    QVector<QVector3D> m_emitterColors;
    QVector<float> m_emitterIntensities;
};
