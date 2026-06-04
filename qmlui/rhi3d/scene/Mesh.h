/*
  Q Light Controller Plus
  Mesh.h

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

#include <QtCore/QVector>
#include <QtCore/QString>
#include <QtGui/QMatrix4x4>
#include <QtGui/QVector2D>
#include <QtGui/QVector3D>
#include <rhi/qrhi.h>

#include "scene/Material.h"

struct Vertex
{
    float px, py, pz;
    float nx, ny, nz;
    float u, v;
};

struct Mesh
{
    enum class SelectionDomain : quint8
    {
        FixtureItem = 0,
        GenericItem = 1
    };

    QString name;
    QVector<Vertex> vertices;
    QVector<quint32> indices;
    QRhiBuffer *vertexBuffer = nullptr;
    QRhiBuffer *indexBuffer = nullptr;
    QRhiTexture *baseColorTexture = nullptr;
    QRhiTexture *normalTexture = nullptr;
    QRhiTexture *metallicRoughnessTexture = nullptr;
    QRhiTexture *occlusionTexture = nullptr;
    QRhiTexture *emissiveTexture = nullptr;
    QRhiSampler *baseColorSampler = nullptr;
    QRhiSampler *normalSampler = nullptr;
    QRhiSampler *metallicRoughnessSampler = nullptr;
    QRhiSampler *occlusionSampler = nullptr;
    QRhiSampler *emissiveSampler = nullptr;
    QRhiBuffer *modelUbo = nullptr;
    QRhiBuffer *materialUbo = nullptr;
    QRhiShaderResourceBindings *srb = nullptr;
    QRhiShaderResourceBindings *gizmoSrb = nullptr;
    QRhiShaderResourceBindings *shadowSrb = nullptr;
    QVector<QRhiShaderResourceBindings *> spotShadowSrbs;
    int indexCount = 0;
    QMatrix4x4 baseModelMatrix;
    QMatrix4x4 modelMatrix;
    QVector3D userOffset = QVector3D(0.0f, 0.0f, 0.0f);
    QVector3D boundsMin = QVector3D(0.0f, 0.0f, 0.0f);
    QVector3D boundsMax = QVector3D(0.0f, 0.0f, 0.0f);
    QVector3D worldBoundsMin = QVector3D(0.0f, 0.0f, 0.0f);
    QVector3D worldBoundsMax = QVector3D(0.0f, 0.0f, 0.0f);
    bool boundsValid = false;
    bool worldBoundsValid = false;
    bool worldBoundsDirty = true;
    bool selected = false;
    bool selectable = true;
    bool visible = true;
    bool modelDirty = true;
    bool materialDirty = true;
    bool selectionDirty = true;
    bool gpuReady = false;
    int gizmoAxis = -1;
    int gizmoType = 0;
    int selectionGroup = -1;
    SelectionDomain selectionDomain = SelectionDomain::FixtureItem;
    Material material;
};
