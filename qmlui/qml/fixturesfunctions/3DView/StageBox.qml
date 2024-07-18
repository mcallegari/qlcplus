/*
  Q Light Controller Plus
  StageBox.qml

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

import Qt3D.Core 2.0
import Qt3D.Render 2.0
import Qt3D.Input 2.0
import Qt3D.Extras 2.0

Entity
{
    id: stage

    property vector3d size: contextManager ? contextManager.environmentSize : Qt.vector3d(5, 3, 5)

    property Layer sceneLayer
    property Effect effect

    property Material material:
        Material
        {
            effect: stage.effect

            parameters: [
                Parameter { name: "diffuse"; value: "lightgray" },
                Parameter { name: "specular"; value: "black" },
                Parameter { name: "shininess"; value: 1.0 },
                Parameter { name: "bloom"; value: 0 }
            ]
        }

    CuboidMesh
    {
        id: groundMesh
        xExtent: size.x
        zExtent: size.z
        yExtent: 0.2
    }

    CuboidMesh
    {
        id: sideMesh
        xExtent: 0.2
        zExtent: size.z
        yExtent: size.y + 0.2
    }

    CuboidMesh
    {
        id: backMesh
        xExtent: size.x + 0.4
        zExtent: 0.2
        yExtent: size.y + 0.2
    }

    // ground mesh
    Entity
    {
        property Transform transform: Transform { translation: Qt.vector3d(0, - (groundMesh.yExtent / 2), 0) }

        ObjectPicker
        {
            id: groundPicker
            onClicked: contextManager.setPositionPickPoint(pick.worldIntersection)
        }

        components: [
            groundMesh,
            stage.material,
            transform,
            groundPicker,
            stage.sceneLayer
        ]
    }

    // left side mesh
    Entity
    {
        property Transform transform: Transform { translation: Qt.vector3d((-size.x / 2) - (groundMesh.yExtent / 2),
                                                                           (size.y / 2) - (groundMesh.yExtent / 2),
                                                                           0) }

        ObjectPicker
        {
            id: leftPicker
            onClicked: contextManager.setPositionPickPoint(pick.worldIntersection)
        }

        components: [
            sideMesh,
            stage.material,
            transform,
            leftPicker,
            stage.sceneLayer
        ]
    }

    // right side mesh
    Entity
    {
        property Transform transform: Transform { translation: Qt.vector3d((size.x / 2) + (groundMesh.yExtent / 2),
                                                                           (size.y / 2) - (groundMesh.yExtent / 2),
                                                                           0) }
        ObjectPicker
        {
            id: rightPicker
            onClicked: contextManager.setPositionPickPoint(pick.worldIntersection)
        }

        components: [
            sideMesh,
            stage.material,
            transform,
            rightPicker,
            stage.sceneLayer
        ]
    }

    // back mesh
    Entity
    {
        property Transform transform: Transform { translation: Qt.vector3d(0,
                                                                           (size.y / 2) - (groundMesh.yExtent / 2),
                                                                           (-size.z / 2) - (groundMesh.yExtent / 2)) }
        ObjectPicker
        {
            id: backPicker
            onClicked: contextManager.setPositionPickPoint(pick.worldIntersection)
        }

        components: [
            backMesh,
            stage.material,
            transform,
            backPicker,
            stage.sceneLayer
        ]
    }
}
