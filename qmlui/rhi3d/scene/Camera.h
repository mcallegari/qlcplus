/*
  Q Light Controller Plus
  Camera.h

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

#include <QtGui/QMatrix4x4>
#include <QtGui/QVector3D>

class Camera
{
public:
    void setPerspective(float fovYDegrees, float aspect, float nearPlane, float farPlane);
    void setPosition(const QVector3D &pos);
    void lookAt(const QVector3D &target, const QVector3D &up = QVector3D(0.0f, 1.0f, 0.0f));

    const QMatrix4x4 &viewMatrix() const { return m_view; }
    const QMatrix4x4 &projectionMatrix() const { return m_proj; }
    const QVector3D &position() const { return m_pos; }
    float nearPlane() const { return m_near; }
    float farPlane() const { return m_far; }
    float fovYDegrees() const { return m_fovY; }
    float aspect() const { return m_aspect; }
    bool isDirty() const { return m_dirty; }
    void clearDirty() { m_dirty = false; }

private:
    QMatrix4x4 m_view;
    QMatrix4x4 m_proj;
    QVector3D m_pos;
    float m_near = 0.1f;
    float m_far = 1000.0f;
    float m_fovY = 60.0f;
    float m_aspect = 1.0f;
    bool m_dirty = true;
};
