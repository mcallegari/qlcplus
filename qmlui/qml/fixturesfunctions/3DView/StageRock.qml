/*
  Q Light Controller Plus
  StageRock.qml

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

    property Layer sceneLayer
    property Effect effect

    // if the model changes, this has to be changed accordingly
    property real trussHalfSize: 0.15

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

        var columnsArray = defaultColumnPositions()

        // create the four truss columns
        var i, len, vecPos
        for (i = 0; i < 4; i++)
        {
            len = size.y
            vecPos = columnsArray[i]

            while (len > 0)
            {
                if (len >= 2)
                {
                    vecPos.y = size.y - len + 1
                    createMesh(truss2mMesh, vecPos, Qt.vector3d(90, 0, 0))
                    len -= 2
                }
                else
                {
                    vecPos.y = size.y - len + 0.5
                    createMesh(truss1mMesh, vecPos, Qt.vector3d(90, 0, 0))
                    len -= 1
                }
            }
        }

        columnsArray = defaultColumnPositions()

        // create upper side trusses
        for (i = 0; i < 2; i++)
        {
            len = size.z
            vecPos = columnsArray[i]

            while (len > 0)
            {
                if (len >= 2)
                {
                    vecPos.z = (size.z / 2) - len + 1
                    createMesh(truss2mMesh, vecPos, Qt.vector3d(0, 0, 0))
                    len -= 2
                }
                else
                {
                    vecPos.z = (size.z / 2) - len + 0.5
                    createMesh(truss1mMesh, vecPos, Qt.vector3d(0, 0, 0))
                    len -= 1
                }
            }
        }

        columnsArray = defaultColumnPositions()

        // create upper front/rear trusses
        for (i = 0; i < 2; i++)
        {
            len = size.x
            vecPos = columnsArray[i * 2]

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
        }
    }

    function defaultColumnPositions()
    {
        var array = [ Qt.vector3d(-size.x / 2 - trussHalfSize, size.y + trussHalfSize, size.z / 2 + trussHalfSize),  // front left
                      Qt.vector3d(size.x / 2 + trussHalfSize, size.y + trussHalfSize, size.z / 2 + trussHalfSize),   // front right
                      Qt.vector3d(-size.x / 2 - trussHalfSize, size.y + trussHalfSize, -size.z / 2 - trussHalfSize), // rear left
                      Qt.vector3d(size.x / 2 + trussHalfSize, size.y + trussHalfSize, -size.z / 2 - trussHalfSize) ] // rear right
        return array
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

    Mesh
    {
        id: cornerMesh
        source: View3D.meshDirectory + "stage/truss_square_corner.obj"
    }

    Mesh { id: truss2mMesh }
    Mesh { id: truss1mMesh }

    Entity
    {
        CuboidMesh
        {
            id: groundMesh
            xExtent: size.x
            zExtent: size.z
            yExtent: 0.2
        }

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

    // front left corner
    Entity
    {
        id: flCorner
        property Transform transform: Transform { translation: Qt.vector3d(-size.x / 2 - trussHalfSize,
                                                                           size.y + trussHalfSize,
                                                                           size.z / 2 + trussHalfSize) }
        components: [
            cornerMesh,
            stage.material,
            transform,
            stage.sceneLayer
        ]
    }

    // front right corner
    Entity
    {
        property Transform transform: Transform { translation: Qt.vector3d(size.x / 2 + trussHalfSize,
                                                                           size.y + trussHalfSize,
                                                                           size.z / 2 + trussHalfSize) }
        components: [
            cornerMesh,
            stage.material,
            transform,
            stage.sceneLayer
        ]
    }

    // rear left corner
    Entity
    {
        property Transform transform: Transform { translation: Qt.vector3d(-size.x / 2 - trussHalfSize,
                                                                           size.y + trussHalfSize,
                                                                           -size.z / 2 - trussHalfSize) }
        components: [
            cornerMesh,
            stage.material,
            transform,
            stage.sceneLayer
        ]
    }

    // rear right corner
    Entity
    {
        property Transform transform: Transform { translation: Qt.vector3d(size.x / 2 + trussHalfSize,
                                                                           size.y + trussHalfSize,
                                                                           -size.z / 2 - trussHalfSize) }
        components: [
            cornerMesh,
            stage.material,
            transform,
            stage.sceneLayer
        ]
    }

}
