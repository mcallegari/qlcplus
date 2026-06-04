/*
  Q Light Controller Plus
  BeamBarItem.h

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
#include "scene/Scene.h"

#include <QtCore/QVariant>
#include <QtCore/QSize>

class BeamBarItem : public MeshItem
{
    Q_OBJECT
    Q_PROPERTY(int emitterCount READ emitterCount WRITE setEmitterCount NOTIFY emitterCountChanged)
    Q_PROPERTY(QSize headsLayout READ headsLayout WRITE setHeadsLayout NOTIFY headsLayoutChanged)
    Q_PROPERTY(QVector3D baseColor READ baseColor WRITE setBaseColor NOTIFY baseColorChanged)
    Q_PROPERTY(QVector3D color READ color WRITE setColor NOTIFY colorChanged)
    Q_PROPERTY(float intensity READ intensity WRITE setIntensity NOTIFY intensityChanged)
    Q_PROPERTY(float range READ range WRITE setRange NOTIFY rangeChanged)
    Q_PROPERTY(float beamRadius READ beamRadius WRITE setBeamRadius NOTIFY beamRadiusChanged)
    Q_PROPERTY(bool castShadows READ castShadows WRITE setCastShadows NOTIFY castShadowsChanged)
    Q_PROPERTY(float pan READ pan WRITE setPan NOTIFY panChanged)
    Q_PROPERTY(float tilt READ tilt WRITE setTilt NOTIFY tiltChanged)
    Q_PROPERTY(float panRangeDegrees READ panRangeDegrees WRITE setPanRangeDegrees NOTIFY panRangeDegreesChanged)
    Q_PROPERTY(float tiltRangeDegrees READ tiltRangeDegrees WRITE setTiltRangeDegrees NOTIFY tiltRangeDegreesChanged)
    Q_PROPERTY(QVariantList emitterColors READ emitterColors WRITE setEmitterColors NOTIFY emitterColorsChanged)
    Q_PROPERTY(QVariantList emitterIntensities READ emitterIntensities WRITE setEmitterIntensities NOTIFY emitterIntensitiesChanged)

public:
    explicit BeamBarItem(QObject *parent = nullptr);

    MeshType type() const override { return MeshType::BeamBar; }

    int emitterCount() const { return m_emitterCount; }
    void setEmitterCount(int count);

    QSize headsLayout() const { return m_headsLayout; }
    void setHeadsLayout(const QSize &layout);

    QVector3D baseColor() const { return m_baseColor; }
    void setBaseColor(const QVector3D &color);

    QVector3D color() const { return m_color; }
    void setColor(const QVector3D &color);

    float intensity() const { return m_intensity; }
    void setIntensity(float intensity);

    float range() const { return m_range; }
    void setRange(float range);

    float beamRadius() const { return m_beamRadius; }
    void setBeamRadius(float radius);

    bool castShadows() const { return m_castShadows; }
    void setCastShadows(bool cast);

    float pan() const { return m_pan; }
    void setPan(float panDegrees);

    float tilt() const { return m_tilt; }
    void setTilt(float tiltDegrees);

    float panRangeDegrees() const { return m_panRangeDegrees; }
    void setPanRangeDegrees(float degrees);

    float tiltRangeDegrees() const { return m_tiltRangeDegrees; }
    void setTiltRangeDegrees(float degrees);

    Q_INVOKABLE void setPanTilt(const QVariant &panTarget, const QVariant &tiltTarget);
    Q_INVOKABLE void setPositionSpeed(const QVariant &panDuration, const QVariant &tiltDuration);

    QVariantList emitterColors() const;
    void setEmitterColors(const QVariantList &colors);
    const QVector<QVector3D> &emitterColorsVector() const { return m_emitterColors; }

    QVariantList emitterIntensities() const;
    void setEmitterIntensities(const QVariantList &intensities);
    const QVector<float> &emitterIntensitiesVector() const { return m_emitterIntensities; }

    Light toLight() const;

Q_SIGNALS:
    void emitterCountChanged();
    void headsLayoutChanged();
    void baseColorChanged();
    void colorChanged();
    void intensityChanged();
    void rangeChanged();
    void beamRadiusChanged();
    void castShadowsChanged();
    void panChanged();
    void tiltChanged();
    void panRangeDegreesChanged();
    void tiltRangeDegreesChanged();
    void emitterColorsChanged();
    void emitterIntensitiesChanged();

private:
    int m_emitterCount = 1;
    QSize m_headsLayout = QSize(1, 1);
    QVector3D m_baseColor = QVector3D(0.64f, 0.64f, 0.64f);
    QVector3D m_color = QVector3D(1.0f, 1.0f, 1.0f);
    float m_intensity = 1.0f;
    float m_range = 30.0f;
    float m_beamRadius = 0.05f;
    bool m_castShadows = true;
    float m_pan = 0.0f;
    float m_tilt = 0.0f;
    float m_panRangeDegrees = 360.0f;
    float m_tiltRangeDegrees = 270.0f;
    QVector<QVector3D> m_emitterColors;
    QVector<float> m_emitterIntensities;
};
