/*
  Q Light Controller Plus
  PickingUtils.h

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

#include <QtGui/QVector3D>
#include <QtCore/QPointF>
#include <limits>

class RhiScene;
class QRhi;

namespace RhiQmlUtils
{

struct PickHit
{
    int meshIndex = -1;
    QVector3D worldPos;
    float distance = std::numeric_limits<float>::max();
};

enum class PickFilter
{
    All,
    SelectableOnly,
    GizmosOnly
};

bool computeRay(const RhiScene &scene, QRhi *rhi, const QPointF &normPos,
                QVector3D &origin, QVector3D &dir);
bool rayAabbIntersection(const QVector3D &origin, const QVector3D &dir,
                         const QVector3D &minV, const QVector3D &maxV,
                         float &tNear, float &tFar);
bool rayTriangleIntersection(const QVector3D &origin, const QVector3D &dir,
                             const QVector3D &v0, const QVector3D &v1, const QVector3D &v2,
                             float &tOut);
float closestAxisT(const QVector3D &rayOrigin, const QVector3D &rayDir,
                   const QVector3D &axisOrigin, const QVector3D &axisDir);
bool pickSceneMesh(const RhiScene &scene, QRhi *rhi, const QPointF &normPos,
                   PickFilter filter, PickHit &hit);
bool rayPlaneIntersection(const QVector3D &rayOrigin, const QVector3D &rayDir,
                          const QVector3D &planeOrigin, const QVector3D &planeNormal,
                          QVector3D &hit);

}
