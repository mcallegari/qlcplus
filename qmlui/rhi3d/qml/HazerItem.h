/*
  Q Light Controller Plus
  HazerItem.h

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

class HazerItem : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVector3D position READ position WRITE setPosition NOTIFY positionChanged)
    Q_PROPERTY(QVector3D direction READ direction WRITE setDirection NOTIFY directionChanged)
    Q_PROPERTY(float length READ length WRITE setLength NOTIFY lengthChanged)
    Q_PROPERTY(float radius READ radius WRITE setRadius NOTIFY radiusChanged)
    Q_PROPERTY(float density READ density WRITE setDensity NOTIFY densityChanged)
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)

public:
    explicit HazerItem(QObject *parent = nullptr);

    QVector3D position() const { return m_position; }
    void setPosition(const QVector3D &position);

    QVector3D direction() const { return m_direction; }
    void setDirection(const QVector3D &direction);

    float length() const { return m_length; }
    void setLength(float length);

    float radius() const { return m_radius; }
    void setRadius(float radius);

    float density() const { return m_density; }
    void setDensity(float density);

    bool enabled() const { return m_enabled; }
    void setEnabled(bool enabled);

Q_SIGNALS:
    void positionChanged();
    void directionChanged();
    void lengthChanged();
    void radiusChanged();
    void densityChanged();
    void enabledChanged();

private:
    void notifyParent();

    QVector3D m_position = QVector3D(0.0f, 0.0f, 0.0f);
    QVector3D m_direction = QVector3D(0.0f, 1.0f, 0.0f);
    float m_length = 3.0f;
    float m_radius = 1.0f;
    float m_density = 0.6f;
    bool m_enabled = true;
};
