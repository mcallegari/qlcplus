/*
  Q Light Controller Plus
  StageTheatre.qml

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

import QtQuick 2.7

import Qt3D.Core 2.0
import Qt3D.Render 2.0
import Qt3D.Input 2.0
import Qt3D.Extras 2.0

Entity
{
    id: stage

    property vector3d size: contextManager ? contextManager.environmentSize : Qt.vector3d(5, 3, 5)
    property int sideSpace: 3 // 3 extra meters on each side of the stage
    property int twiceSideSpace: sideSpace * 2
    property int frontSpace: 2 // 2 extra meters of front space
    property real dividerThickness: 0.1
    // if the model changes, this has to be changed accordingly
    property real trussHalfSize: 0.15
    property real trussDistance: 2

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

    onSizeChanged:
    {
        if (size.x > 1 || size.y > 1 || size.z > 1)
            truss2mMesh.source = View3D.meshDirectory + "stage/truss_square_2m.obj"

        if (size.x % 2 || size.y % 2 || size.z % 2)
            truss1mMesh.source = View3D.meshDirectory + "stage/truss_square_1m.obj"

        // clean up any previously created truss entities
        View3D.resetStage(stage)

        var zPos = size.z / 2 - trussHalfSize

        // create upper front/rear trusses
        for (var i = 0; i < (size.z / trussDistance); i++)
        {
            var len = size.x
            var vecPos = Qt.vector3d(0, size.y + trussHalfSize, zPos)

            while (len > 0)
            {
                if (len >= 2)
                {
                    vecPos.x = (size.x / 2) - len + 1
                    createMesh(truss2mMesh, vecPos, Qt.vector3d(0, 90, 0))
                    len -= 2
                }
                else
                {
                    vecPos.x = (size.x / 2) - len + 0.5
                    createMesh(truss1mMesh, vecPos, Qt.vector3d(0, 90, 0))
                    len -= 1
                }
            }

            zPos -= trussDistance
        }
    }

    function createMesh(obj, pos, rot)
    {
        return meshComp.createObject(stage, { mesh: obj, position: pos, vec3Rotation: rot } )
    }

    Component
    {
        id: meshComp
        Entity
        {
            property bool isDynamic: true
            property Mesh mesh
            property vector3d position
            property vector3d vec3Rotation

            property Transform transform:
                Transform
                {
                    translation: position
                    rotationX: vec3Rotation.x
                    rotationY: vec3Rotation.y
                    rotationZ: vec3Rotation.z
                }

            components: [
                mesh,
                stage.material,
                transform,
                stage.sceneLayer
            ]
        }
    }

    CuboidMesh
    {
        id: groundMesh
        xExtent: size.x + twiceSideSpace
        zExtent: size.z + frontSpace
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
        id: frontMesh
        xExtent: sideSpace
        zExtent: 0.2
        yExtent: size.y + 0.2
    }

    CuboidMesh
    {
        id: backMesh
        xExtent: size.x + twiceSideSpace + 0.4
        zExtent: 0.2
        yExtent: size.y + 0.2
    }

    Mesh { id: truss2mMesh }
    Mesh { id: truss1mMesh }

    // ground mesh
    Entity
    {
        property Transform transform: Transform { translation: Qt.vector3d(0, - (groundMesh.yExtent / 2), 0) }

        ObjectPicker
        {
            id: stagePicker
            onClicked: contextManager.setPositionPickPoint(pick.worldIntersection)
        }

        components: [
            groundMesh,
            stage.material,
            transform,
            stagePicker,
            stage.sceneLayer
        ]
    }

    // left side mesh
    Entity
    {
        property Transform transform: Transform { translation: Qt.vector3d((-(size.x + twiceSideSpace) / 2) - (groundMesh.yExtent / 2),
                                                                           (size.y / 2) - (groundMesh.yExtent / 2),
                                                                           0) }

        components: [
            sideMesh,
            stage.material,
            transform,
            stage.sceneLayer
        ]
    }

    // left side front mesh
    Entity
    {
        property Transform transform: Transform { translation: Qt.vector3d((-(size.x + sideSpace + 0.2) / 2) - (groundMesh.yExtent / 2),
                                                                           (size.y / 2) - (groundMesh.yExtent / 2),
                                                                           ((size.z - frontMesh.zExtent) / 2)  ) }

        components: [
            frontMesh,
            stage.material,
            transform,
            stage.sceneLayer
        ]
    }

    // right side mesh
    Entity
    {
        property Transform transform: Transform { translation: Qt.vector3d(((size.x + twiceSideSpace) / 2) + (groundMesh.yExtent / 2),
                                                                           (size.y / 2) - (groundMesh.yExtent / 2),
                                                                           0) }

        components: [
            sideMesh,
            stage.material,
            transform,
            stage.sceneLayer
        ]
    }

    // right side front mesh
    Entity
    {
        property Transform transform: Transform { translation: Qt.vector3d(((size.x + sideSpace + 0.2) / 2) - (groundMesh.yExtent / 2),
                                                                           (size.y / 2) - (groundMesh.yExtent / 2),
                                                                           ((size.z - frontMesh.zExtent) / 2)  ) }

        components: [
            frontMesh,
            stage.material,
            transform,
            stage.sceneLayer
        ]
    }

    // back mesh
    Entity
    {
        property Transform transform: Transform { translation: Qt.vector3d(0,
                                                                           (size.y / 2) - (groundMesh.yExtent / 2),
                                                                           (-size.z / 2) - (groundMesh.yExtent / 2)) }

        components: [
            backMesh,
            stage.material,
            transform,
            stage.sceneLayer
        ]
    }
}

