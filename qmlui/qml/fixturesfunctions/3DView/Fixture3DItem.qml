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
    property int headsNumber: 1

    onItemIDChanged: isSelected = contextManager.isFixtureSelected(itemID)

    property int meshType: MainView3D.NoMeshType

    /* **************** Pan/Tilt properties **************** */
    property real panMaxDegrees: 360
    property real tiltMaxDegrees: 270
    property bool invertedPan: false
    property bool invertedTilt: false
    property real panSpeed: 4000 // in milliseconds
    property real tiltSpeed: 4000 // in milliseconds

    property real panRotation: 0
    property real tiltRotation: 0

    property Transform panTransform
    property Transform tiltTransform

    /* **************** Focus properties **************** */
    property real focusMinDegrees: 15
    property real focusMaxDegrees: 30
    property real distCutoff: 40.0
    property real cutoffAngle: (focusMinDegrees / 2.0) * (Math.PI / 180.0)

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
    readonly property Layer spotlightShadingLayer: Layer { }
    readonly property Layer outputDepthLayer: Layer { }
    readonly property Layer spotlightScatteringLayer: Layer { }

    property real coneBottomRadius: distCutoff * Math.tan(cutoffAngle) + coneTopRadius
    property real coneTopRadius: (0.24023 / 2) * transform.scale3D.x * 0.7 // (diameter / 2) * scale * magic number

    property real headLength:
    {
        switch(meshType)
        {
            case MainView3D.NoMeshType: return 0;
            case MainView3D.ParMeshType: return 0.389005 * transform.scale3D.x
            case MainView3D.MovingHeadMeshType: return 0.63663 * transform.scale3D.x
        }
        console.log("UNSUPPORTED MESH TYPE " + meshType)
        return 0.5 * transform.scale3D.x
    }

    /* ********************* Light properties ********************* */
    /* ****** These are bound to uniforms in ScreenQuadEntity ***** */

    property real lightIntensity: dimmerValue * shutterValue
    property real dimmerValue: 0
    property real shutterValue: sAnimator.shutterValue
    property color lightColor: Qt.rgba(0, 0, 0, 1)
    property vector3d lightPos: Qt.vector3d(0, 0, 0)
    property vector3d lightDir: Math3D.getLightDirection(transform, panTransform, tiltTransform)

    /* ********************** Light matrices ********************** */
    property matrix4x4 lightMatrix
    property matrix4x4 lightViewMatrix:
        Math3D.getLightViewMatrix(lightMatrix,
                                  invertedPan ? panMaxDegrees - panRotation : panRotation,
                                  invertedTilt ? tiltMaxDegrees - tiltRotation : tiltRotation,
                                  lightPos)
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
        t.rotationY = Qt.binding(function() {
            return invertedPan ? panMaxDegrees - panRotation : panRotation
        })
    }

    function bindTiltTransform(t, maxDegrees)
    {
        console.log("Binding tilt ----")
        fixtureEntity.tiltTransform = t
        fixtureEntity.tiltMaxDegrees = maxDegrees
        tiltRotation = maxDegrees / 2
        t.rotationX = Qt.binding(function() {
            return invertedTilt ? tiltMaxDegrees - tiltRotation : tiltRotation
        })
    }

    function getHead(headIndex)
    {
        return fixtureEntity
    }

    function setHeadLightProps(headIndex, pos, matrix)
    {
        lightPos = pos
        lightMatrix = matrix
    }

    function setHeadIntensity(headIndex, intensity)
    {
        dimmerValue = intensity
    }

    function setHeadRGBColor(headIndex, color)
    {
        lightColor = color
    }

    function setPosition(pan, tilt)
    {
        if (panMaxDegrees)
        {
            panAnim.stop()
            panAnim.from = panRotation
            panAnim.to = (panMaxDegrees / 0xFFFF) * pan
            panAnim.duration = Math.max((panSpeed / panMaxDegrees) * Math.abs(panAnim.to - panAnim.from), 300)
            panAnim.start()
        }

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
        if (panDuration !== -1)
            panSpeed = panDuration
        if (tiltDuration !== -1)
            tiltSpeed = tiltDuration
    }

    function setShutter(type, low, high)
    {
        sAnimator.setShutter(type, low, high)
    }

    function setZoom(value)
    {
        cutoffAngle = (((((focusMaxDegrees - focusMinDegrees) / 255.0) * value) + focusMinDegrees) / 2.0) * (Math.PI / 180.0)
    }

    function setupScattering(sceneEntity)
    {
        if (sceneEntity.coneMesh.length !== distCutoff)
            sceneEntity.coneMesh.length = distCutoff

        shadingCone.coneEffect = sceneEntity.spotlightShadingEffect
        shadingCone.parent = sceneEntity
        shadingCone.spotlightConeMesh = sceneEntity.coneMesh

        scatteringCone.coneEffect = sceneEntity.spotlightScatteringEffect
        scatteringCone.parent = sceneEntity
        scatteringCone.spotlightConeMesh = sceneEntity.coneMesh

        outDepthCone.coneEffect = sceneEntity.outputFrontDepthEffect
        outDepthCone.parent = sceneEntity
        outDepthCone.spotlightConeMesh = sceneEntity.coneMesh
    }

    ShutterAnimator { id: sAnimator }

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

    property Texture2D depthTex:
        Texture2D
        {
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

    property RenderTarget shadowMap:
        RenderTarget
        {
            attachments: [
                RenderTargetOutput
                {
                    attachmentPoint: RenderTargetOutput.Depth
                    texture: depthTex
                }
            ] // attachments
        }

    /* **************** Gobo properties **************** */
    property Texture2D goboTexture: Texture2D { }
    property real goboRotation: 0

    function setGoboSpeed(cw, speed)
    {
        //console.log("Gobo clockwise: " + (cw ? "yes" : "no") + " speed: " + speed)
        goboAnim.stop()
        goboAnim.from = cw ? 0 : Math.PI * 2
        goboAnim.to = cw ? Math.PI * 2 : 0
        if (speed !== 0)
        {
            goboAnim.duration = speed
            goboAnim.restart()
        }
    }

    QQ2.NumberAnimation on goboRotation
    {
        id: goboAnim
        running: false
        duration: 0
        easing.type: Easing.Linear
        from: 0
        to: 360
        loops: QQ2.Animation.Infinite
    }

    /* Cone meshes used for scattering. These get re-parented to
       the main Scene entity via setupScattering */
    SpotlightConeEntity
    {
        id: shadingCone
        coneLayer: spotlightShadingLayer
        fxEntity: fixtureEntity
    }
    SpotlightConeEntity
    {
        id: scatteringCone
        coneLayer: spotlightScatteringLayer
        fxEntity: fixtureEntity
    }
    SpotlightConeEntity
    {
        id: outDepthCone
        coneLayer: outputDepthLayer
        fxEntity: fixtureEntity
    }

    SceneLoader
    {
        id: eSceneLoader

        onStatusChanged:
        {
            if (status === SceneLoader.Ready)
                View3D.initializeFixture(itemID, fixtureEntity, eSceneLoader)
        }
    }

    /* Main transform of the whole fixture item */
    property Transform transform: Transform { }

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


