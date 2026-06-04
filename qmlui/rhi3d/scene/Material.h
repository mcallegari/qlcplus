/*
  Q Light Controller Plus
  Material.h

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

#include <QtCore/QString>
#include <QtGui/QImage>
#include <QtGui/QVector3D>

struct Material
{
    QVector3D baseColor = QVector3D(1.0f, 1.0f, 1.0f);
    float baseAlpha = 1.0f;
    float metalness = 0.0f;
    float roughness = 0.5f;
    float occlusion = 1.0f;
    QVector3D emissive = QVector3D(0.0f, 0.0f, 0.0f);
    float alphaCutoff = 0.5f;
    enum class AlphaMode
    {
        Opaque = 0,
        Mask = 1,
        Blend = 2
    };
    AlphaMode alphaMode = AlphaMode::Opaque;
    bool doubleSided = false;
    QString baseColorMapPath;
    QImage baseColorMap;
    QString normalMapPath;
    QImage normalMap;
    QString metallicRoughnessMapPath;
    QImage metallicRoughnessMap;
    QString occlusionMapPath;
    QImage occlusionMap;
    QString emissiveMapPath;
    QImage emissiveMap;
};
