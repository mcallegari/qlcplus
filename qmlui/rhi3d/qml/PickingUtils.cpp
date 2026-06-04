/*
  Q Light Controller Plus
  PickingUtils.cpp

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

#include "qml/PickingUtils.h"

#include "scene/Scene.h"
#include "scene/Mesh.h"
#include <rhi/qrhi.h>
#include <QtMath>
#include <algorithm>
#include <limits>

namespace RhiQmlUtils
{

bool computeRay(const RhiScene &scene, QRhi *rhi, const QPointF &normPos,
                QVector3D &origin, QVector3D &dir)
{
    if (!rhi)
        return false;

    QMatrix4x4 viewProj = rhi->clipSpaceCorrMatrix()
            * scene.camera().projectionMatrix()
            * scene.camera().viewMatrix();
    bool invertible = false;
    const QMatrix4x4 invViewProj = viewProj.inverted(&invertible);
    if (!invertible)
        return false;

    const float x = (2.0f * float(normPos.x()) - 1.0f);
    const float y = 1.0f - (2.0f * float(normPos.y()));
    const float zNear = rhi->isClipDepthZeroToOne() ? 0.0f : -1.0f;
    const float zFar = 1.0f;

    QVector4D nearClip(x, y, zNear, 1.0f);
    QVector4D farClip(x, y, zFar, 1.0f);
    QVector4D nearWorld = invViewProj * nearClip;
    QVector4D farWorld = invViewProj * farClip;
    if (qFuzzyIsNull(nearWorld.w()) || qFuzzyIsNull(farWorld.w()))
        return false;

    nearWorld /= nearWorld.w();
    farWorld /= farWorld.w();

    origin = nearWorld.toVector3D();
    dir = (farWorld - nearWorld).toVector3D().normalized();
    if (dir.isNull())
        return false;

    return true;
}

bool rayAabbIntersection(const QVector3D &origin, const QVector3D &dir,
                         const QVector3D &minV, const QVector3D &maxV,
                         float &tNear, float &tFar)
{
    tNear = 0.0f;
    tFar = std::numeric_limits<float>::max();

    for (int axis = 0; axis < 3; ++axis)
    {
        const float o = axis == 0 ? origin.x() : (axis == 1 ? origin.y() : origin.z());
        const float d = axis == 0 ? dir.x() : (axis == 1 ? dir.y() : dir.z());
        const float minA = axis == 0 ? minV.x() : (axis == 1 ? minV.y() : minV.z());
        const float maxA = axis == 0 ? maxV.x() : (axis == 1 ? maxV.y() : maxV.z());

        if (qFuzzyIsNull(d))
        {
            if (o < minA || o > maxA)
                return false;
            continue;
        }

        float t1 = (minA - o) / d;
        float t2 = (maxA - o) / d;
        if (t1 > t2)
            std::swap(t1, t2);
        tNear = qMax(tNear, t1);
        tFar = qMin(tFar, t2);
        if (tNear > tFar)
            return false;
    }
    return tFar >= 0.0f;
}

bool rayTriangleIntersection(const QVector3D &origin, const QVector3D &dir,
                             const QVector3D &v0, const QVector3D &v1, const QVector3D &v2,
                             float &tOut)
{
    const float eps = 1e-6f;
    const QVector3D e1 = v1 - v0;
    const QVector3D e2 = v2 - v0;
    const QVector3D p = QVector3D::crossProduct(dir, e2);
    const float det = QVector3D::dotProduct(e1, p);
    if (qAbs(det) < eps)
        return false;
    const float invDet = 1.0f / det;
    const QVector3D t = origin - v0;
    const float u = QVector3D::dotProduct(t, p) * invDet;
    if (u < 0.0f || u > 1.0f)
        return false;
    const QVector3D q = QVector3D::crossProduct(t, e1);
    const float v = QVector3D::dotProduct(dir, q) * invDet;
    if (v < 0.0f || (u + v) > 1.0f)
        return false;
    const float tHit = QVector3D::dotProduct(e2, q) * invDet;
    if (tHit <= eps)
        return false;
    tOut = tHit;
    return true;
}

float closestAxisT(const QVector3D &rayOrigin, const QVector3D &rayDir,
                   const QVector3D &axisOrigin, const QVector3D &axisDir)
{
    const float a = QVector3D::dotProduct(rayDir, rayDir);
    const float b = QVector3D::dotProduct(rayDir, axisDir);
    const float c = QVector3D::dotProduct(axisDir, axisDir);
    const QVector3D w0 = rayOrigin - axisOrigin;
    const float d = QVector3D::dotProduct(rayDir, w0);
    const float e = QVector3D::dotProduct(axisDir, w0);
    const float denom = a * c - b * b;
    if (qFuzzyIsNull(denom))
        return -e / c;
    return (a * e - b * d) / denom;
}

bool pickSceneMesh(const RhiScene &scene, QRhi *rhi, const QPointF &normPos,
                   PickFilter filter, PickHit &hit)
{
    QVector3D origin;
    QVector3D dir;
    if (!computeRay(scene, rhi, normPos, origin, dir))
        return false;

    const auto &meshes = scene.meshes();
    for (int meshIndex = 0; meshIndex < meshes.size(); ++meshIndex)
    {
        const Mesh &mesh = meshes[meshIndex];
        if (!mesh.visible)
            continue;
        if (mesh.gizmoAxis >= 0 && filter != PickFilter::GizmosOnly)
            continue;
        if (filter == PickFilter::SelectableOnly && !mesh.selectable)
            continue;
        if (filter == PickFilter::GizmosOnly && mesh.gizmoAxis < 0)
            continue;
        if (mesh.vertices.isEmpty())
            continue;

        bool invOk = false;
        const QMatrix4x4 invModel = mesh.modelMatrix.inverted(&invOk);
        if (!invOk)
            continue;

        const QVector3D originLocal = (invModel * QVector4D(origin, 1.0f)).toVector3D();
        QVector3D dirLocal = (invModel * QVector4D(dir, 0.0f)).toVector3D();
        if (dirLocal.isNull())
            continue;
        dirLocal.normalize();

        QVector3D minV = mesh.boundsMin;
        QVector3D maxV = mesh.boundsMax;
        if (!mesh.boundsValid)
        {
            minV = QVector3D(mesh.vertices[0].px, mesh.vertices[0].py, mesh.vertices[0].pz);
            maxV = minV;
            for (const Vertex &v : mesh.vertices)
            {
                minV.setX(qMin(minV.x(), v.px));
                minV.setY(qMin(minV.y(), v.py));
                minV.setZ(qMin(minV.z(), v.pz));
                maxV.setX(qMax(maxV.x(), v.px));
                maxV.setY(qMax(maxV.y(), v.py));
                maxV.setZ(qMax(maxV.z(), v.pz));
            }
        }

        float tNear = 0.0f;
        float tFar = 0.0f;
        if (!rayAabbIntersection(originLocal, dirLocal, minV, maxV, tNear, tFar))
            continue;

        auto getVertex = [&](int idx)
        {
            const Vertex &v = mesh.vertices[idx];
            return QVector3D(v.px, v.py, v.pz);
        };

        auto testTriangle = [&](const QVector3D &v0, const QVector3D &v1, const QVector3D &v2)
        {
            float tHit = 0.0f;
            if (!rayTriangleIntersection(originLocal, dirLocal, v0, v1, v2, tHit))
                return;
            const QVector3D localHit = originLocal + dirLocal * tHit;
            const QVector3D worldHit = (mesh.modelMatrix * QVector4D(localHit, 1.0f)).toVector3D();
            const float dist = (worldHit - origin).length();
            if (dist < hit.distance)
            {
                hit.distance = dist;
                hit.meshIndex = meshIndex;
                hit.worldPos = worldHit;
            }
        };

        if (!mesh.indices.isEmpty())
        {
            for (int i = 0; i + 2 < mesh.indices.size(); i += 3)
            {
                const QVector3D v0 = getVertex(int(mesh.indices[i]));
                const QVector3D v1 = getVertex(int(mesh.indices[i + 1]));
                const QVector3D v2 = getVertex(int(mesh.indices[i + 2]));
                testTriangle(v0, v1, v2);
            }
        }
        else
        {
            for (int i = 0; i + 2 < mesh.vertices.size(); i += 3)
            {
                const QVector3D v0 = getVertex(i);
                const QVector3D v1 = getVertex(i + 1);
                const QVector3D v2 = getVertex(i + 2);
                testTriangle(v0, v1, v2);
            }
        }
    }

    return hit.meshIndex >= 0;
}

bool rayPlaneIntersection(const QVector3D &rayOrigin, const QVector3D &rayDir,
                          const QVector3D &planeOrigin, const QVector3D &planeNormal,
                          QVector3D &hit)
{
    const float denom = QVector3D::dotProduct(rayDir, planeNormal);
    if (qAbs(denom) < 1e-6f)
        return false;
    const float t = QVector3D::dotProduct(planeOrigin - rayOrigin, planeNormal) / denom;
    if (t <= 0.0f)
        return false;
    hit = rayOrigin + rayDir * t;
    return true;
}

}
