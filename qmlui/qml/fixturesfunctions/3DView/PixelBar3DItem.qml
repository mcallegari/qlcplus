/*
  Q Light Controller Plus
  PixelBar3DItem.qml

  Copyright (c) Massimo Callegari
  Copyright (c) Eric Arnebäck

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

import QtQuick 2.7 as QQ2

import Qt3D.Core 2.0
import Qt3D.Render 2.0
import Qt3D.Extras 2.0

import org.qlcplus.classes 1.0
import "Math3DView.js" as Math3D
import "."

Entity
{
    id: fixtureEntity
    objectName: "fixture3DItem"

    property int itemID: fixtureManager.invalidFixture()
    property bool isSelected: false
    property int headsNumber: 0
    property size headsLayout: Qt.size(1, 1)
    property vector3d phySize: Qt.vector3d(1, 0.1, 0.1)
    property bool useScattering: false
    property bool useShadows: false
    property real shutterValue: sAnimator.shutterValue

    onItemIDChanged:
    {
        isSelected = contextManager.isFixtureSelected(itemID)
        headsRepeater.model = headsNumber
    }

    function getHead(headIndex)
    {
        return headsRepeater.objectAt(headIndex)
    }

    function setHeadIntensity(headIndex, intensity)
    {
        headsRepeater.objectAt(headIndex).dimmerValue = intensity
    }

    function setHeadRGBColor(headIndex, color)
    {
        headsRepeater.objectAt(headIndex).lightColor = color
    }

    function setShutter(type, low, high)
    {
        sAnimator.setShutter(type, low, high)
    }

    ShutterAnimator { id: sAnimator }

    /* Main transform of the whole fixture item */
    property Transform transform: Transform { }

    property Layer sceneLayer
    property Effect sceneEffect

    property Material material:
        Material
        {
            effect: sceneEffect

            parameters: [
                Parameter { name: "diffuse"; value: "gray" },
                Parameter { name: "specular"; value: "black" },
                Parameter { name: "shininess"; value: 1.0 },
                Parameter { name: "bloom"; value: 0 }
            ]
        }

    CuboidMesh
    {
        id: baseMesh
        xExtent: phySize.x
        yExtent: phySize.y
        zExtent: phySize.z
    }

    NodeInstantiator
    {
        id: headsRepeater
        //model: fixtureEntity.headsNumber

        onObjectAdded:
        {
            console.log("Head " + index + " added ----------------")
            if (index == fixtureEntity.headsNumber - 1)
                View3D.initializeFixture(itemID, fixtureEntity, null)
        }

        delegate:
            Entity
            {
                id: headDelegate
                property real dimmerValue: 0
                property real lightIntensity: dimmerValue * shutterValue
                property real headWidth: phySize.x / headsLayout.width
                property real headHeight: phySize.z / headsLayout.height
                property color lightColor: Qt.rgba(0, 0, 0, 1)

                enabled: lightIntensity === 0 || lightColor === Qt.rgba(0, 0, 0, 1) ? false : true

                PlaneMesh
                {
                    id: headMesh
                    width: headWidth
                    height: headHeight
                    meshResolution: Qt.size(2, 2)
                }

                property Transform headTransform:
                    Transform
                    {
                        translation: {
                            var row = Math.floor(index / headsLayout.width)
                            var column = index % headsLayout.width
                            var xPos = (column * headWidth) + (headWidth / 2)
                            var zPos = (row * headHeight) + (headHeight / 2)

                            return Qt.vector3d(-(phySize.x / 2) + xPos, (phySize.y / 2) + 0.001, -(phySize.z / 2) + zPos)
                        }
                    }

                property Material headMaterial:
                    Material
                    {
                        effect: sceneEffect

                        parameters: [
                            Parameter {
                                name: "diffuse"
                                value: Qt.rgba(lightColor.r * lightIntensity, lightColor.g * lightIntensity, lightColor.b * lightIntensity, 1)
                            },
                            Parameter { name: "specular"; value: "black" },
                            Parameter { name: "shininess"; value: 1.0 },
                            Parameter { name: "bloom"; value: 1 }
                        ]
                    }

                components: [
                    headMesh,
                    headTransform,
                    headMaterial,
                    fixtureEntity.sceneLayer
                ]
            }
    }

    ObjectPicker
    {
        id: eObjectPicker
        //hoverEnabled: true
        dragEnabled: true

        property var lastPos

        onClicked:
        {
            console.log("3D item clicked")
            isSelected = !isSelected
            contextManager.setItemSelection(itemID, isSelected, pick.modifiers)
        }
    }

    components: [
        baseMesh,
        transform,
        material,
        sceneLayer,
        eObjectPicker
    ]
}
