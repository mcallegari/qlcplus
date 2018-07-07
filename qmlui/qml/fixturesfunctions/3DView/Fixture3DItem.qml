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
import "."

Entity
{
    id: fixtureEntity
    objectName: "fixture3DItem"

    property int itemID: fixtureManager.invalidFixture()
    property alias itemSource: eSceneLoader.source
    property bool isSelected: false

    property real panMaxDegrees: 360
    property real tiltMaxDegrees: 270
    property real focusMinDegrees: 15
    property real focusMaxDegrees: 30
    property real totalDuration: 4000 // in milliseconds

    property bool useScattering : true

    property real panRotation: 0
    property real tiltRotation: 0

    property Transform panTransform
    property Transform tiltTransform

   // property real cutOff: focusMinDegrees / 2 // TODO: degrees or radians ? That is the question
    property real distCutoff: 40.0
    property real cutoffAngle: {
        return (focusMinDegrees / 2) * (Math.PI / 180)
    }

    property int raymarchSteps: 40

    // spotlight cone radius
    property real coneRadius: Math.tan(cutoffAngle) * distCutoff

    property matrix4x4 lightMatrix

    readonly property Layer spotlightShadingLayer: Layer { objectName: "spotlightShadingLayer" }
    readonly property Layer outputDepthLayer: Layer { objectName: "outputDepthLayer" }
    readonly property Layer spotlightScatteringLayer: Layer { objectName: "spotlightScatteringLayer" }

    /* Light properties. These are bound to uniforms in ScreenQuadEntity */
    property int lightIndex
    property real lightIntensity: 1.0
    property real intensityOrigValue: lightIntensity
    property color lightColor: Qt.rgba(0, 0, 0, 1)
    property vector3d lightPos: Qt.vector3d(0, 0, 0)


    property vector3d lightDir: {
        return getLightDir()
    }


    property matrix4x4 lightViewMatrix: {
        return lookAt(lightPos,  lightPos.plus(getLightDir()), Qt.vector3d(1.0, 0.0, 0.0))
    }
    property matrix4x4 lightProjectionMatrix:perspective( cutoffAngle, 1.0, 0.1, 40.0 )
    property matrix4x4 lightViewProjectionMatrix: lightProjectionMatrix.times(lightViewMatrix)
    property matrix4x4 lightViewProjectionScaleAndOffsetMatrix: Qt.matrix4x4(
        0.5, 0.0, 0.0, 0.5,
        0.0, 0.5, 0.0, 0.5,
        0.0, 0.0, 0.5, 0.5,
        0.0, 0.0, 0.0, 1.0).times(lightViewProjectionMatrix)

    onItemIDChanged: isSelected = contextManager.isFixtureSelected(itemID)

    //onPanTransformChanged: console.log("Pan transform changed " + panTransform)
    //onTiltTransformChanged: console.log("Tilt transform changed " + tiltTransform)

    //onPositionChanged: console.log("Light position changed: " + position)
    //onDirectionChanged: console.log("Light direction changed: " + direction)

    function lookAt(eye, center, up)
    {
        // compute the basis vectors.
        var forward = (center.minus(eye)).normalized() // forward vector.
        var left = (forward.crossProduct(up)).normalized() // left vector.
        var u = (left.crossProduct(forward)).normalized() // up vector.

        return Qt.matrix4x4(
            left.x, u.x, -forward.x, 0.0,
            left.y, u.y, -forward.y, 0.0,
            left.z, u.z, -forward.z, 0.0,
            -left.dotProduct(eye), -u.dotProduct(eye), forward.dotProduct(eye), 1.0).transposed()
    }

    function getLightDir()
    {
        var lightMatrix2 = transform.matrix
        if (panTransform)
            lightMatrix2 = transform.matrix.times(panTransform.matrix)
        if (tiltTransform)
            lightMatrix2 = lightMatrix2.times(tiltTransform.matrix)
        lightMatrix2 = lightMatrix2.times(Qt.vector4d(0.0, -1.0, 0.0, 0.0))
   
        return (lightMatrix2.toVector3d().normalized())
    }

    function perspective(fovy, aspect, zNear, zFar)
    {
        var ymax = zNear * Math.tan(fovy)
        var xmax = ymax * aspect;
        var left = -xmax;
        var right = +xmax;
        var bottom = -ymax;
        var top = +ymax;
        var f1, f2, f3, f4;
        f1 = 2.0 * zNear;
        f2 = right - left;
        f3 = top - bottom;
        f4 = zFar - zNear;
        return Qt.matrix4x4(
            f1 / f2, 0.0, 0.0, 0.0,
            0.0, f1/f3, 0.0, 0.0,
            (right + left) / f2, (top + bottom) / f3, (-zFar - zNear) / f4, -1.0,
            0.0, 0.0, (-zFar * f1) / f4, 0.0).transposed()
    }

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
        //console.log("[3Ditem] set position " + pan + ", " + tilt)
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
        console.log("Shutter " + low + ", " + high)
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

    function setRaymarchSteps(value)
    {
        raymarchSteps = value
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

    SceneLoader
    {
        id: eSceneLoader

        onStatusChanged:
        {
            if (status === SceneLoader.Ready)
                View3D.initializeFixture(itemID, fixtureEntity, eSceneLoader)
        }
    }

    components: [ eSceneLoader, transform, eObjectPicker ]

    function setupScattering(shadingLayer, scatteringLayer, depthLayer,
                             shadingEffect, scatteringEffect, depthEffect,
                             headEntity, sceneEntity)
    {
        shadingCone.coneLayer = shadingLayer
        shadingCone.coneEffect = shadingEffect
        shadingCone.coneMaterial.bindFixture(fixtureEntity)
        shadingCone.parent = sceneEntity

        scatteringCone.coneLayer = scatteringLayer
        scatteringCone.coneEffect = scatteringEffect
        scatteringCone.coneMaterial.bindFixture(fixtureEntity)
        scatteringCone.parent = sceneEntity

        outDepthCone.coneLayer = depthLayer
        outDepthCone.coneEffect = depthEffect
        outDepthCone.coneMaterial.bindFixture(fixtureEntity)
        outDepthCone.parent = sceneEntity
    }

    /* Cone meshes used for scattering. These get re-parented to a head mesh via setupScattering */
    SpotlightConeEntity { id: shadingCone }
    SpotlightConeEntity { id: scatteringCone }
    SpotlightConeEntity { id: outDepthCone }

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
}


