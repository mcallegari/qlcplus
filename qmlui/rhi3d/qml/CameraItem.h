/*
  Q Light Controller Plus
  CameraItem.h

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

#include <QtCore/QObject>
#include <QtGui/QVector3D>

class CameraItem : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVector3D position READ position WRITE setPosition NOTIFY positionChanged)
    Q_PROPERTY(QVector3D target READ target WRITE setTarget NOTIFY targetChanged)
    Q_PROPERTY(float fov READ fov WRITE setFov NOTIFY fovChanged)
    Q_PROPERTY(float nearPlane READ nearPlane WRITE setNearPlane NOTIFY nearPlaneChanged)
    Q_PROPERTY(float farPlane READ farPlane WRITE setFarPlane NOTIFY farPlaneChanged)

public:
    explicit CameraItem(QObject *parent = nullptr);

    QVector3D position() const { return m_position; }
    void setPosition(const QVector3D &position);

    QVector3D target() const { return m_target; }
    void setTarget(const QVector3D &target);

    float fov() const { return m_fov; }
    void setFov(float fov);

    float nearPlane() const { return m_nearPlane; }
    void setNearPlane(float nearPlane);

    float farPlane() const { return m_farPlane; }
    void setFarPlane(float farPlane);

Q_SIGNALS:
    void positionChanged();
    void targetChanged();
    void fovChanged();
    void nearPlaneChanged();
    void farPlaneChanged();

private:
    void notifyParent();

    QVector3D m_position = QVector3D(0.0f, 1.0f, 5.0f);
    QVector3D m_target = QVector3D(0.0f, 0.0f, 0.0f);
    float m_fov = 60.0f;
    float m_nearPlane = 0.1f;
    float m_farPlane = 200.0f;
};
