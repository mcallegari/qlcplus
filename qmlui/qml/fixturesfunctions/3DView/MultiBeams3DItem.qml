/*
  Q Light Controller Plus
  MultiBeams3DItem.qml

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

import QtQuick 2.7

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
    property int headsNumber: 1
    property size headsLayout: Qt.size(1, 1)
    property vector3d phySize: Qt.vector3d(1, 0.1, 0.1)

    onItemIDChanged:
    {
        isSelected = contextManager.isFixtureSelected(itemID)
        updateHeads()
    }

    /* **************** Tilt properties (motorized bars) **************** */
    property real tiltMaxDegrees: 270
    property real tiltSpeed: 4000 // in milliseconds
    property real tiltRotation: 0

    property Transform tiltTransform

    /* **************** Focus properties **************** */
    property real focusMinDegrees: 5
    property real focusMaxDegrees: 5
    property real distCutoff: 40.0
    property real cutoffAngle: (focusMinDegrees / 2) * (Math.PI / 180)

    /* **************** Rendering quality properties **************** */
    property bool useScattering: View3D.renderQuality === MainView3D.LowQuality ? false : true
    property bool useShadows: View3D.renderQuality === MainView3D.LowQuality ? false : true
    property int raymarchSteps:
    {
        switch(View3D.renderQuality)
        {
            case MainView3D.LowQuality: return 0
            case MainView3D.MediumQuality: return 20
            case MainView3D.HighQuality: return 40
            case MainView3D.UltraQuality: return 80
        }
    }

    /* **************** Spotlight cone properties **************** */
    property real coneBottomRadius: distCutoff * Math.tan(cutoffAngle) + coneTopRadius
    property real coneTopRadius: (0.24023 / 2) * transform.scale3D.x * 0.7 // (diameter / 2) * scale * magic number

    property real headLength: 0.5 * transform.scale3D.x

    /* ********************* Light properties ********************* */
    /* ****** These are bound to uniforms in ScreenQuadEntity ***** */

    property real shutterValue: 1.0
    property vector3d lightDir: Math3D.getLightDirection(transform, 0, tiltTransform)

    property var headsList: []

    function bindTiltTransform(t, maxDegrees)
    {
        console.log("Binding tilt ----")
        fixtureEntity.tiltTransform = t
        fixtureEntity.tiltMaxDegrees = maxDegrees
        tiltRotation = maxDegrees / 2
        t.rotationX = Qt.binding(function() { return tiltRotation })
    }

    function updateHeads()
    {
        var i

        // delete existing heads first
        for (i = headsList.length - 1; i >= 0; i--)
            headsList[i].destroy()

        headsList = []

        for (i = 0; i < headsNumber; i++)
        {
            console.log("Item " + itemID + " creating head - " + i)

            var component = Qt.createComponent("LightEntity.qml");
            if (component.status === Component.Error)
                console.log("Error loading component:", component.errorString())

            var headNode = component.createObject(fixtureEntity,
            {
                "headIndex": i,
                "lightDir": lightDir,
                "shutterValue": shutterValue,
                "raymarchSteps": raymarchSteps,
                "cutoffAngle": cutoffAngle,
                "distCutoff": distCutoff,
                "headLength": headLength,
                "coneBottomRadius": coneBottomRadius,
                "coneTopRadius": coneTopRadius,
                "tiltRotation": tiltRotation,
                "goboTexture": goboTexture
            });

            headsList.push(headNode)
        }

        View3D.initializeFixture(itemID, fixtureEntity, null)
    }

    function setupScattering(sceneEntity)
    {
        if (sceneEntity.coneMesh.length !== distCutoff)
            sceneEntity.coneMesh.length = distCutoff

        for (var i = 0; i < headsList.length; i++)
        {
            headsList[i].setupScattering(sceneEntity)
        }
    }

    function getHead(headIndex)
    {
        return headsList[headIndex]
    }

    function setHeadLightProps(headIndex, pos, matrix)
    {
        for (var h = 0; h < headsNumber; h++)
        {
            var head = getHead(h)
            var hPos = Qt.vector3d(pos.x * h, pos.y, pos.z)
            head.lightPos = hPos
            head.lightMatrix = matrix
            console.log("Setting light info: " + hPos + ", m: " + matrix)
        }
    }

    function setHeadIntensity(headIndex, intensity)
    {
        headsList[headIndex].dimmerValue = intensity
    }

    function setHeadRGBColor(headIndex, color)
    {
        headsList[headIndex].lightColor = color
    }

    function setPosition(pan, tilt)
    {
        if (tiltMaxDegrees)
        {
            tiltAnim.stop()
            tiltAnim.from = tiltRotation
            var degTo = parseInt(((tiltMaxDegrees / 0xFFFF) * tilt) - (tiltMaxDegrees / 2))
            //console.log("Tilt to " + degTo + ", max: " + tiltMaxDegrees)
            tiltAnim.to = -degTo
            tiltAnim.duration = Math.max((tiltSpeed / tiltMaxDegrees) * Math.abs(tiltAnim.to - tiltAnim.from), 300)
            tiltAnim.start()
        }
    }

    function setPositionSpeed(panDuration, tiltDuration)
    {
        if (tiltDuration !== -1)
            tiltSpeed = tiltDuration
    }

    function setShutter(type, low, high)
    {
        sAnimator.setShutter(type, low, high)
    }

    function setZoom(value)
    {
        cutoffAngle = (((((focusMaxDegrees - focusMinDegrees) / 255) * value) + focusMinDegrees) / 2) * (Math.PI / 180)
    }

    NumberAnimation on tiltRotation
    {
        id: tiltAnim
        running: false
        easing.type: Easing.Linear
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
        yExtent: phySize.y * 0.5
        zExtent: phySize.z
    }

    Entity
    {
        id: headEntity
        objectName: "headEntity"

        CuboidMesh
        {
            id: headMesh
            xExtent: phySize.x
            yExtent: phySize.y * 0.5
            zExtent: phySize.z
        }

        property Transform tiltTransform:
            Transform
            {
                translation: Qt.vector3d(0, phySize.y * 0.5, 0)
            }

        components: [
            headMesh,
            tiltTransform,
            fixtureEntity.material,
            fixtureEntity.sceneLayer
        ]
    }

    property Texture2D goboTexture: Texture2D { }

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
        headEntity,
        transform,
        material,
        sceneLayer,
        eObjectPicker
    ]
}
