/*
  Q Light Controller Plus
  Camera.cpp

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

#include "scene/Camera.h"

void Camera::setPerspective(float fovYDegrees, float aspect, float nearPlane, float farPlane)
{
    if (m_fovY == fovYDegrees && m_aspect == aspect && m_near == nearPlane && m_far == farPlane)
    {
        return;
    }
    m_fovY = fovYDegrees;
    m_aspect = aspect;
    m_near = nearPlane;
    m_far = farPlane;
    m_proj.setToIdentity();
    m_proj.perspective(fovYDegrees, aspect, nearPlane, farPlane);
    m_dirty = true;
}

void Camera::setPosition(const QVector3D &pos)
{
    if (m_pos == pos)
        return;
    m_pos = pos;
    m_dirty = true;
}

void Camera::lookAt(const QVector3D &target, const QVector3D &up)
{
    QMatrix4x4 view;
    view.setToIdentity();
    view.lookAt(m_pos, target, up);
    if (view == m_view)
        return;
    m_view = view;
    m_dirty = true;
}
