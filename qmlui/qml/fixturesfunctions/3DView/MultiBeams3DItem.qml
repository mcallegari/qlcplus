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
    property int headsNumber: 1
    property vector3d phySize: Qt.vector3d(1, 0.1, 0.1)

    onItemIDChanged:
    {
        isSelected = contextManager.isFixtureSelected(itemID)
        headsRepeater.model = headsNumber
    }

    /* **************** Tilt properties (motorized bars) **************** */
    property real tiltMaxDegrees: 270
    property real tiltSpeed: 4000 // in milliseconds
    property real tiltRotation: 0

    property Transform tiltTransform

    /* **************** Focus properties **************** */
    property real focusMinDegrees: 15
    property real focusMaxDegrees: 30
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

    function bindTiltTransform(t, maxDegrees)
    {
        console.log("Binding tilt ----")
        fixtureEntity.tiltTransform = t
        fixtureEntity.tiltMaxDegrees = maxDegrees
        tiltRotation = maxDegrees / 2
        t.rotationX = Qt.binding(function() { return tiltRotation })
    }

    function getHead(headIndex)
    {
        return headsRepeater.objectAt(headIndex)
    }

    function setHeadLightProps(headIndex, pos, matrix)
    {
        headsRepeater.objectAt(headIndex).lightPos = pos
        headsRepeater.objectAt(headIndex).lightMatrix = matrix
    }

    function setHeadIntensity(headIndex, intensity)
    {
        headsRepeater.objectAt(headIndex).dimmerValue = intensity
    }

    function setHeadRGBColor(headIndex, color)
    {
        headsRepeater.objectAt(headIndex).lightColor = color
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

    QQ2.NumberAnimation on tiltRotation
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
                Parameter { name: "diffuse"; value: "lightgray" },
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

    function setupScattering(shadingEffect, scatteringEffect, depthEffect, sceneEntity)
    {
        if (sceneEntity.coneMesh.length !== distCutoff)
            sceneEntity.coneMesh.length = distCutoff

        for (var i = 0; i < headsRepeater.count; i++)
        {
            var item = headsRepeater.objectAt(i)
            item.shadingCone.coneEffect = shadingEffect
            item.shadingCone.parent = sceneEntity
            item.shadingCone.spotlightConeMesh = sceneEntity.coneMesh

            item.scatteringCone.coneEffect = scatteringEffect
            item.scatteringCone.parent = sceneEntity
            item.scatteringCone.spotlightConeMesh = sceneEntity.coneMesh

            item.outDepthCone.coneEffect = depthEffect
            item.outDepthCone.parent = sceneEntity
            item.outDepthCone.spotlightConeMesh = sceneEntity.coneMesh
        }
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

                property color lightColor: Qt.rgba(0, 0, 0, 1)
                property vector3d lightPos: Qt.vector3d(0, 0, 0)

                readonly property Layer spotlightShadingLayer: Layer { }
                readonly property Layer outputDepthLayer: Layer { }
                readonly property Layer spotlightScatteringLayer: Layer { }

                /* ********************** Light matrices ********************** */
                property matrix4x4 lightMatrix
                property matrix4x4 lightViewMatrix:
                    Math3D.getLightViewMatrix(lightMatrix, 0, tiltRotation, lightPos)
                property matrix4x4 lightProjectionMatrix:
                    Math3D.getLightProjectionMatrix(distCutoff, coneBottomRadius, coneTopRadius, headLength, cutoffAngle)
                property matrix4x4 lightViewProjectionMatrix: lightProjectionMatrix.times(lightViewMatrix)
                property matrix4x4 lightViewProjectionScaleAndOffsetMatrix:
                    Math3D.getLightViewProjectionScaleOffsetMatrix(lightViewProjectionMatrix)

                property Transform transform: Transform { translation: Qt.vector3d(0, 0.1 * index, 0) }

                property RenderTarget shadowMap:
                    RenderTarget
                    {
                        property alias depth: depthAttachment

                        attachments: [
                            RenderTargetOutput
                            {
                                attachmentPoint: RenderTargetOutput.Depth
                                texture:
                                    Texture2D
                                    {
                                        id: depthAttachment
                                        width: 512
                                        height: 512
                                        format: Texture.D32F
                                        generateMipMaps: false
                                        magnificationFilter: Texture.Linear
                                        minificationFilter: Texture.Linear
                                        wrapMode
                                        {
                                            x: WrapMode.ClampToEdge
                                            y: WrapMode.ClampToEdge
                                        }
                                    }
                            }
                        ] // outputs
                    }

                /* Cone meshes used for scattering. These get re-parented to
                   the main Scene entity via setupScattering */
                property SpotlightConeEntity shadingCone:
                    SpotlightConeEntity
                    {
                        coneLayer: spotlightShadingLayer
                        fxEntity: headDelegate
                    }
                property SpotlightConeEntity scatteringCone:
                    SpotlightConeEntity
                    {
                        coneLayer: spotlightScatteringLayer
                        fxEntity: headDelegate
                    }
                property SpotlightConeEntity outDepthCone:
                    SpotlightConeEntity
                    {
                        coneLayer: outputDepthLayer
                        fxEntity: headDelegate
                    }
            } // Entity
    } // Repeater

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
