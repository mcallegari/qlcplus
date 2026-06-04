/*
  Q Light Controller Plus
  MeshUtils.cpp

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

#include "qml/MeshUtils.h"

#include <QtMath>

namespace RhiQmlUtils
{

Mesh createUnitCubeMesh()
{
    Mesh mesh;
    const float h = 0.5f;
    mesh.vertices = {
        // +Z
        { -h, -h,  h,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f },
        {  h, -h,  h,  0.0f, 0.0f, 1.0f,  1.0f, 0.0f },
        {  h,  h,  h,  0.0f, 0.0f, 1.0f,  1.0f, 1.0f },
        { -h,  h,  h,  0.0f, 0.0f, 1.0f,  0.0f, 1.0f },
        // -Z
        {  h, -h, -h,  0.0f, 0.0f, -1.0f, 0.0f, 0.0f },
        { -h, -h, -h,  0.0f, 0.0f, -1.0f, 1.0f, 0.0f },
        { -h,  h, -h,  0.0f, 0.0f, -1.0f, 1.0f, 1.0f },
        {  h,  h, -h,  0.0f, 0.0f, -1.0f, 0.0f, 1.0f },
        // +X
        {  h, -h,  h,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f },
        {  h, -h, -h,  1.0f, 0.0f, 0.0f,  1.0f, 0.0f },
        {  h,  h, -h,  1.0f, 0.0f, 0.0f,  1.0f, 1.0f },
        {  h,  h,  h,  1.0f, 0.0f, 0.0f,  0.0f, 1.0f },
        // -X
        { -h, -h, -h, -1.0f, 0.0f, 0.0f,  0.0f, 0.0f },
        { -h, -h,  h, -1.0f, 0.0f, 0.0f,  1.0f, 0.0f },
        { -h,  h,  h, -1.0f, 0.0f, 0.0f,  1.0f, 1.0f },
        { -h,  h, -h, -1.0f, 0.0f, 0.0f,  0.0f, 1.0f },
        // +Y
        { -h,  h,  h,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f },
        {  h,  h,  h,  0.0f, 1.0f, 0.0f,  1.0f, 0.0f },
        {  h,  h, -h,  0.0f, 1.0f, 0.0f,  1.0f, 1.0f },
        { -h,  h, -h,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f },
        // -Y
        { -h, -h, -h,  0.0f, -1.0f, 0.0f, 0.0f, 0.0f },
        {  h, -h, -h,  0.0f, -1.0f, 0.0f, 1.0f, 0.0f },
        {  h, -h,  h,  0.0f, -1.0f, 0.0f, 1.0f, 1.0f },
        { -h, -h,  h,  0.0f, -1.0f, 0.0f, 0.0f, 1.0f }
    };
    mesh.indices = {
        0, 1, 2, 0, 2, 3,
        4, 5, 6, 4, 6, 7,
        8, 9, 10, 8, 10, 11,
        12, 13, 14, 12, 14, 15,
        16, 17, 18, 16, 18, 19,
        20, 21, 22, 20, 22, 23
    };
    mesh.indexCount = mesh.indices.size();
    mesh.modelMatrix.setToIdentity();
    mesh.baseModelMatrix = mesh.modelMatrix;
    mesh.boundsMin = QVector3D(-h, -h, -h);
    mesh.boundsMax = QVector3D(h, h, h);
    mesh.boundsValid = true;
    return mesh;
}

Mesh createUnitQuadMesh()
{
    Mesh mesh;
    const float h = 0.5f;
    mesh.vertices = {
        { -h, -h, 0.0f,  0.0f, 0.0f, 1.0f,  0.0f, 1.0f },
        {  h, -h, 0.0f,  0.0f, 0.0f, 1.0f,  1.0f, 1.0f },
        {  h,  h, 0.0f,  0.0f, 0.0f, 1.0f,  1.0f, 0.0f },
        { -h,  h, 0.0f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f }
    };
    mesh.indices = { 0, 1, 2, 0, 2, 3 };
    mesh.indexCount = mesh.indices.size();
    mesh.modelMatrix.setToIdentity();
    mesh.baseModelMatrix = mesh.modelMatrix;
    mesh.boundsMin = QVector3D(-h, -h, 0.0f);
    mesh.boundsMax = QVector3D(h, h, 0.0f);
    mesh.boundsValid = true;
    return mesh;
}

Mesh createSphereMesh(float radius, int rings, int sectors)
{
    Mesh mesh;
    rings = qMax(2, rings);
    sectors = qMax(3, sectors);
    const int ringCount = rings + 1;
    const int sectorCount = sectors + 1;
    mesh.vertices.reserve(ringCount * sectorCount);
    mesh.indices.reserve(rings * sectors * 6);

    for (int r = 0; r <= rings; ++r)
    {
        const float v = float(r) / float(rings);
        const float phi = v * float(M_PI);
        const float y = qCos(phi);
        const float sinPhi = qSin(phi);
        for (int s = 0; s <= sectors; ++s)
        {
            const float u = float(s) / float(sectors);
            const float theta = u * float(M_PI * 2.0);
            const float x = qCos(theta) * sinPhi;
            const float z = qSin(theta) * sinPhi;
            mesh.vertices.push_back({ x * radius, y * radius, z * radius,
                                      x, y, z, u, v });
        }
    }

    for (int r = 0; r < rings; ++r)
    {
        for (int s = 0; s < sectors; ++s)
        {
            const int row = sectorCount;
            const int a = r * row + s;
            const int b = a + 1;
            const int c = (r + 1) * row + s;
            const int d = c + 1;
            mesh.indices.push_back(a);
            mesh.indices.push_back(c);
            mesh.indices.push_back(b);
            mesh.indices.push_back(b);
            mesh.indices.push_back(c);
            mesh.indices.push_back(d);
        }
    }

    mesh.indexCount = mesh.indices.size();
    mesh.modelMatrix.setToIdentity();
    mesh.baseModelMatrix = mesh.modelMatrix;
    mesh.boundsMin = QVector3D(-radius, -radius, -radius);
    mesh.boundsMax = QVector3D(radius, radius, radius);
    mesh.boundsValid = true;
    return mesh;
}

Mesh createArcMesh(float majorRadius, float tubeRadius,
                   float startAngle, float endAngle,
                   int segments, int sides)
{
    Mesh mesh;
    const int ringCount = segments + 1;
    const int sideCount = sides + 1;
    mesh.vertices.reserve(ringCount * sideCount);
    mesh.indices.reserve(segments * sides * 6);

    for (int i = 0; i < ringCount; ++i)
    {
        const float t = startAngle + (endAngle - startAngle) * (float(i) / float(segments));
        const float ct = qCos(t);
        const float st = qSin(t);
        for (int j = 0; j < sideCount; ++j)
        {
            const float p = float(M_PI * 2.0) * (float(j) / float(sides));
            const float cp = qCos(p);
            const float sp = qSin(p);
            const float r = majorRadius + tubeRadius * cp;
            const float x = r * ct;
            const float y = r * st;
            const float z = tubeRadius * sp;
            const QVector3D n(ct * cp, st * cp, sp);
            const float u = float(i) / float(segments);
            const float v = float(j) / float(sides);
            mesh.vertices.push_back({ x, y, z, n.x(), n.y(), n.z(), u, v });
        }
    }

    for (int i = 0; i < segments; ++i)
    {
        for (int j = 0; j < sides; ++j)
        {
            const int row = sideCount;
            const int a = i * row + j;
            const int b = a + 1;
            const int c = (i + 1) * row + j;
            const int d = c + 1;
            mesh.indices.push_back(a);
            mesh.indices.push_back(c);
            mesh.indices.push_back(b);
            mesh.indices.push_back(b);
            mesh.indices.push_back(c);
            mesh.indices.push_back(d);
        }
    }

    mesh.indexCount = mesh.indices.size();
    mesh.modelMatrix.setToIdentity();
    mesh.baseModelMatrix = mesh.modelMatrix;
    const float extent = majorRadius + tubeRadius;
    mesh.boundsMin = QVector3D(-extent, -extent, -tubeRadius);
    mesh.boundsMax = QVector3D(extent, extent, tubeRadius);
    mesh.boundsValid = true;
    return mesh;
}

}
