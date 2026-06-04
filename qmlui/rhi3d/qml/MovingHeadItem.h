/*
  Q Light Controller Plus
  MovingHeadItem.h

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

class MovingHeadItem : public MeshItem
{
    Q_OBJECT
    Q_PROPERTY(QString path READ path WRITE setPath NOTIFY pathChanged)
    Q_PROPERTY(QVector3D color READ color WRITE setColor NOTIFY colorChanged)
    Q_PROPERTY(float intensity READ intensity WRITE setIntensity NOTIFY intensityChanged)
    Q_PROPERTY(float zoom READ zoom WRITE setZoom NOTIFY zoomChanged)
    Q_PROPERTY(bool beamMode READ beamMode WRITE setBeamMode NOTIFY beamModeChanged)
    Q_PROPERTY(float beamRadius READ beamRadius WRITE setBeamRadius NOTIFY beamRadiusChanged)
    Q_PROPERTY(float pan READ pan WRITE setPan NOTIFY panChanged)
    Q_PROPERTY(float tilt READ tilt WRITE setTilt NOTIFY tiltChanged)
    Q_PROPERTY(QString goboPath READ goboPath WRITE setGoboPath NOTIFY goboPathChanged)

public:
    explicit MovingHeadItem(QObject *parent = nullptr);

    MeshType type() const override { return MeshType::MovingHead; }

    QString path() const { return m_path; }
    void setPath(const QString &path);

    QVector3D color() const { return m_color; }
    void setColor(const QVector3D &color);

    float intensity() const { return m_intensity; }
    void setIntensity(float intensity);

    float zoom() const { return m_zoom; }
    void setZoom(float zoom);

    bool beamMode() const { return m_beamMode; }
    void setBeamMode(bool enabled);

    float beamRadius() const { return m_beamRadius; }
    void setBeamRadius(float radius);

    float pan() const { return m_pan; }
    void setPan(float panDegrees);

    float tilt() const { return m_tilt; }
    void setTilt(float tiltDegrees);

    QString goboPath() const { return m_goboPath; }
    void setGoboPath(const QString &path);

    Light toLight() const;

Q_SIGNALS:
    void pathChanged();
    void colorChanged();
    void intensityChanged();
    void zoomChanged();
    void beamModeChanged();
    void beamRadiusChanged();
    void panChanged();
    void tiltChanged();
    void goboPathChanged();

private:
    QString m_path;
    QVector3D m_color = QVector3D(1.0f, 1.0f, 1.0f);
    float m_intensity = 1.0f;
    float m_zoom = 25.0f;
    bool m_beamMode = false;
    float m_beamRadius = 0.05f;
    float m_pan = 0.0f;
    float m_tilt = 0.0f;
    QString m_goboPath;
};
