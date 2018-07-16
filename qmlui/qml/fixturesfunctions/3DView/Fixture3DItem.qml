/*
  Q Light Controller Plus
  Fixture3DItem.qml

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
    property alias itemSource: eSceneLoader.source
    property bool isSelected: false

    onItemIDChanged: isSelected = contextManager.isFixtureSelected(itemID)

    property int meshType: MainView3D.DefaultMeshType

    /* **************** Pan/Tilt properties **************** */
    property real panMaxDegrees: 360
    property real tiltMaxDegrees: 270
    property real totalDuration: 4000 // in milliseconds

    property real panRotation: 0
    property real tiltRotation: 0

    property Transform panTransform
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
    readonly property Layer spotlightShadingLayer: Layer { objectName: "spotlightShadingLayer" }
    readonly property Layer outputDepthLayer: Layer { objectName: "outputDepthLayer" }
    readonly property Layer spotlightScatteringLayer: Layer { objectName: "spotlightScatteringLayer" }

    property real coneBottomRadius: distCutoff * Math.tan(cutoffAngle) + coneTopRadius
    property real coneTopRadius: (0.24023 / 2) * transform.scale3D.x * 0.7 // (diameter / 2) * scale * magic number

    property real headLength: 
    {
        switch(meshType)
        {
            case MainView3D.ParMeshType: return 0.389005 * transform.scale3D.x
            case MainView3D.MovingHeadMeshType: return 0.63663 * transform.scale3D.x
        }
        console.log("UNSUPPORTED MESH TYPE " + meshType)
        return 0.5 * transform.scale3D.x
    }

    /* ********************* Light properties ********************* */
    /* ****** These are bound to uniforms in ScreenQuadEntity ***** */

    property int lightIndex
    property real lightIntensity: 1.0
    property real intensityOrigValue: lightIntensity
    property color lightColor: Qt.rgba(0, 0, 0, 1)
    property vector3d lightPos: Qt.vector3d(0, 0, 0)
    property vector3d lightDir: Math3D.getLightDirection(transform, panTransform, tiltTransform)

    /* ********************** Light matrices ********************** */
    property matrix4x4 lightMatrix
    property matrix4x4 lightViewMatrix: 
        Math3D.getLightViewMatrix(lightMatrix, panRotation, tiltRotation, lightPos)
    property matrix4x4 lightProjectionMatrix:
        Math3D.getLightProjectionMatrix(distCutoff, coneBottomRadius, coneTopRadius, headLength, cutoffAngle)
    property matrix4x4 lightViewProjectionMatrix: lightProjectionMatrix.times(lightViewMatrix)
    property matrix4x4 lightViewProjectionScaleAndOffsetMatrix:
        Math3D.getLightViewProjectionScaleOffsetMatrix(lightViewProjectionMatrix)

    //onPanTransformChanged: console.log("Pan transform changed " + panTransform)
    //onTiltTransformChanged: console.log("Tilt transform changed " + tiltTransform)

    //onPositionChanged: console.log("Light position changed: " + position)
    //onDirectionChanged: console.log("Light direction changed: " + direction)

    function bindPanTransform(t, maxDegrees)
    {
        console.log("Binding pan ----")
        fixtureEntity.panTransform = t
        fixtureEntity.panMaxDegrees = maxDegrees
        t.rotationY = Qt.binding(function() { return panRotation })
    }

    function bindTiltTransform(t, maxDegrees)
    {
        console.log("Binding tilt ----")
        fixtureEntity.tiltTransform = t
        fixtureEntity.tiltMaxDegrees = maxDegrees
        tiltRotation = maxDegrees / 2
        t.rotationX = Qt.binding(function() { return tiltRotation })
    }

    function setPosition(pan, tilt)
    {
        if (panMaxDegrees)
        {
            panAnim.stop()
            panAnim.from = panRotation
            panAnim.to = (panMaxDegrees / 0xFFFF) * pan
            panAnim.duration = Math.max((totalDuration / panMaxDegrees) * Math.abs(panAnim.to - panAnim.from), 300)
            panAnim.start()
        }

        if (tiltMaxDegrees)
        {
            tiltAnim.stop()
            tiltAnim.from = tiltRotation
            var degTo = parseInt(((tiltMaxDegrees / 0xFFFF) * tilt) - (tiltMaxDegrees / 2))
            //console.log("Tilt to " + degTo + ", max: " + tiltMaxDegrees)
            tiltAnim.to = -degTo
            tiltAnim.duration = Math.max((totalDuration / tiltMaxDegrees) * Math.abs(tiltAnim.to - tiltAnim.from), 300)
            tiltAnim.start()
        }
    }

    function setFocus(value)
    {
        cutoffAngle = (((((focusMaxDegrees - focusMinDegrees) / 255) * value) + focusMinDegrees) / 2) * (Math.PI / 180)
    }

    function setShutter(type, low, high)
    {
        //console.log("Shutter " + low + ", " + high)
        shutterAnim.stop()
        inPhase.duration = 0
        inPhase.easing.type = Easing.Linear
        highPhase.duration = 0
        outPhase.duration = 0
        outPhase.easing.type = Easing.Linear
        lowPhase.duration = low

        switch(type)
        {
            case QLCCapability.ShutterOpen:
                lightIntensity = intensityOrigValue
            break;

            case QLCCapability.ShutterClose:
                intensityOrigValue = lightIntensity
                lightIntensity = 0
            break;

            case QLCCapability.StrobeFastToSlow:
            case QLCCapability.StrobeSlowToFast:
            case QLCCapability.StrobeFrequency:
            case QLCCapability.StrobeFreqRange:
                highPhase.duration = high
                shutterAnim.start()
            break;

            case QLCCapability.PulseFastToSlow:
            case QLCCapability.PulseSlowToFast:
            case QLCCapability.PulseFrequency:
            case QLCCapability.PulseFreqRange:
                inPhase.duration = high / 2
                outPhase.duration = high / 2
                inPhase.easing.type = Easing.InOutCubic
                outPhase.easing.type = Easing.InOutCubic
                shutterAnim.start()
            break;

            case QLCCapability.RampUpFastToSlow:
            case QLCCapability.RampUpSlowToFast:
            case QLCCapability.RampUpFrequency:
            case QLCCapability.RampUpFreqRange:
                inPhase.duration = high
                shutterAnim.start()
            break;

            case QLCCapability.RampDownSlowToFast:
            case QLCCapability.RampDownFastToSlow:
            case QLCCapability.RampDownFrequency:
            case QLCCapability.RampDownFreqRange:
                outPhase.duration = high
                shutterAnim.start()
            break;
        }
    }

    function setupScattering(shadingLayer, scatteringLayer, depthLayer,
                             shadingEffect, scatteringEffect, depthEffect,
                             headEntity, sceneEntity)
    {
        if (sceneEntity.coneMesh.length !== distCutoff)
            sceneEntity.coneMesh.length = distCutoff

        shadingCone.coneLayer = shadingLayer
        shadingCone.coneEffect = shadingEffect
        shadingCone.coneMaterial.bindFixture(fixtureEntity)
        shadingCone.parent = sceneEntity
        shadingCone.spotlightConeMesh = sceneEntity.coneMesh

        scatteringCone.coneLayer = scatteringLayer
        scatteringCone.coneEffect = scatteringEffect
        scatteringCone.coneMaterial.bindFixture(fixtureEntity)
        scatteringCone.parent = sceneEntity
        scatteringCone.spotlightConeMesh = sceneEntity.coneMesh

        outDepthCone.coneLayer = depthLayer
        outDepthCone.coneEffect = depthEffect
        outDepthCone.coneMaterial.bindFixture(fixtureEntity)
        outDepthCone.parent = sceneEntity
        outDepthCone.spotlightConeMesh = sceneEntity.coneMesh
    }

    QQ2.NumberAnimation on panRotation
    {
        id: panAnim
        running: false
        easing.type: Easing.Linear
    }

    QQ2.NumberAnimation on tiltRotation
    {
        id: tiltAnim
        running: false
        easing.type: Easing.Linear
    }

    // strobe/pulse effect
    QQ2.SequentialAnimation on lightIntensity
    {   
        id: shutterAnim
        running: false
        loops: QQ2.Animation.Infinite
        QQ2.NumberAnimation { id: inPhase; from: 0; to: intensityOrigValue; duration: 0; easing.type: Easing.Linear }
        QQ2.NumberAnimation { id: highPhase; from: intensityOrigValue; to: intensityOrigValue; duration: 200; easing.type: Easing.Linear }
        QQ2.NumberAnimation { id: outPhase; from: intensityOrigValue; to: 0; duration: 0; easing.type: Easing.Linear }
        QQ2.NumberAnimation { id: lowPhase; from: 0; to: 0; duration: 800; easing.type: Easing.Linear }
    }

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
                            width: 1024
                            height: 1024
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

    property Texture2D goboTexture: Texture2D { }
    property Transform transform: Transform { }

    /* Cone meshes used for scattering. These get re-parented to a head mesh via setupScattering */
    SpotlightConeEntity { id: shadingCone }
    SpotlightConeEntity { id: scatteringCone }
    SpotlightConeEntity { id: outDepthCone }

    SceneLoader
    {
        id: eSceneLoader

        onStatusChanged:
        {
            if (status === SceneLoader.Ready)
                View3D.initializeFixture(itemID, fixtureEntity, eSceneLoader)
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
        //onPressed: lastPos = pick.worldIntersection
        //onReleased: console.log("Item release")
        //onEntered: console.log("Item entered")
        //onExited: console.log("Item exited")
/*
        onMoved:
        {
            //console.log("Pick pos: " + pick.worldIntersection)
            //var x = pick.worldIntersection.x - lastPos
            contextManager.setFixturePosition("3D", itemID,
                                              pick.worldIntersection.x * 1000.0,
                                              pick.worldIntersection.y * 1000.0,
                                              pick.worldIntersection.z * 1000.0)
        }
*/
    }

    components: [ eSceneLoader, transform, eObjectPicker ]
}


