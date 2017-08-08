/*
  Q Light Controller Plus
  SceneEntity.qml

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

import QtQuick 2.0

import Qt3D.Core 2.0
import Qt3D.Render 2.0
import Qt3D.Input 2.0
import Qt3D.Extras 2.0

Entity
{
    id: sceneRootEntity
    objectName: "sceneRootEntity"

    readonly property Camera camera: camera
    readonly property Layer layer: sceneLayer
    readonly property Effect effect: geometryPassEffect

    // Global elements
    Camera
    {
        id: camera
        projectionType: CameraLens.PerspectiveProjection
        fieldOfView: 45
        aspectRatio: 16/9
        nearPlane : 1.0
        farPlane : 1000.0
        position: Qt.vector3d(0.0, 0.0, -5.0)
        upVector: Qt.vector3d(0.0, 1.0, 0.0)
        viewCenter: Qt.vector3d(0.0, 0.0, 0.0)
    }

    GeometryPassEffect { id: geometryPassEffect }

    Layer { id: sceneLayer }

    Entity
    {
        id: ground

        property real xSize: 5.0
        property real zSize: 5.0

        property Material material:
            Material
            {
                effect: geometryPassEffect
                parameters: Parameter { name : "meshColor"; value : "lightgray" }
            }

        CuboidMesh
        {
            id: groundMesh
            yzMeshResolution: Qt.size(2, 2)
            xzMeshResolution: Qt.size(2, 2)
            xyMeshResolution: Qt.size(2, 2)
            xExtent: ground.xSize
            zExtent: ground.zSize
            yExtent: 0.2
        }
        
        property Transform transform: Transform { translation: Qt.vector3d(0, -1, 0) }

        components: [
            groundMesh,
            ground.material,
            ground.transform,
            sceneLayer
        ]
    }
}
