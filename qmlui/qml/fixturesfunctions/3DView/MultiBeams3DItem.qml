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

import QtQuick

import Qt3D.Core
import Qt3D.Render
import Qt3D.Extras

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
    property real coneTopRadius: transform ? (0.24023 / 2) * transform.scale3D.x * 0.7 : 0.0 // (diameter / 2) * scale * magic number

    property real headLength: 0.5 * transform.scale3D.x

    /* ********************* Light properties ********************* */
    /* ****** These are bound to uniforms in ScreenQuadEntity ***** */

    property real shutterValue: sAnimator.shutterValue
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

            // Bind the shared beam properties to the parent so that render
            // quality, zoom, shutter and tilt keep updating every head after
            // creation. coneBottomRadius is derived inside LightEntity.
            var headNode = component.createObject(fixtureEntity,
            {
                "headIndex": i,
                "lightDir": Qt.binding(function() { return fixtureEntity.lightDir }),
                "shutterValue": Qt.binding(function() { return fixtureEntity.shutterValue }),
                "raymarchSteps": Qt.binding(function() { return fixtureEntity.raymarchSteps }),
                "cutoffAngle": Qt.binding(function() { return fixtureEntity.cutoffAngle }),
                "tiltRotation": Qt.binding(function() { return fixtureEntity.tiltRotation }),
                "distCutoff": distCutoff,
                "headLength": headLength,
                "coneTopRadius": coneTopRadius,
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

    function cleanupScattering()
    {
        for (var i = 0; i < headsList.length; i++)
        {
            var headItem = headsList[i]
            if (headItem && headItem.cleanupScattering)
                headItem.cleanupScattering()
        }
    }

    function getHead(headIndex)
    {
        return headsList[headIndex]
    }

    // The C++ side computes a single emitter position/orientation for the whole
    // bar (headIndex is always 0). Spread the beams evenly across the bar's
    // physical width, centered on the fixture origin and rotated by the bar's
    // current orientation matrix.
    function setHeadLightProps(headIndex, pos, matrix)
    {
        var n = headsList.length
        if (n === 0)
            return

        var spacing = (n > 1) ? (phySize.x / n) : 0
        for (var h = 0; h < n; h++)
        {
            var localX = (h - (n - 1) / 2) * spacing
            var worldOffset = matrix.times(Qt.vector4d(localX, 0, 0, 0)).toVector3d()
            var head = headsList[h]
            head.lightPos = pos.plus(worldOffset)
            head.lightMatrix = matrix
        }
    }

    function setHeadIntensity(headIndex, intensity)
    {
        if (headIndex >= 0 && headIndex < headsList.length)
            headsList[headIndex].dimmerValue = intensity
    }

    function setHeadRGBColor(headIndex, color)
    {
        if (headIndex >= 0 && headIndex < headsList.length)
            headsList[headIndex].lightColor = color
    }

    // See Fixture3DItem: timestamp of the previous setPosition() call, used to pace
    // animations to the target-arrival rate instead of the full physical slew time.
    // 0 means "no previous update yet".
    property real lastPositionTime: 0

    // Gap (ms) above which two updates are treated as a one-shot move (use physical
    // slew time) rather than a continuous effect (pace to the interval).
    readonly property real continuousUpdateGap: 1000

    function setPosition(pan, tilt)
    {
        var now = Date.now()
        var elapsed = (lastPositionTime > 0) ? (now - lastPositionTime) : continuousUpdateGap + 1
        var oneShot = (elapsed <= 0 || elapsed >= continuousUpdateGap)
        lastPositionTime = now

        if (tiltMaxDegrees)
        {
            var degTo = parseInt(((tiltMaxDegrees / 0xFFFF) * tilt) - (tiltMaxDegrees / 2))
            //console.log("Tilt to " + degTo + ", max: " + tiltMaxDegrees)
            tiltAnim.stop()
            tiltAnim.from = tiltRotation
            tiltAnim.to = -degTo
            var tiltPhysical = (tiltSpeed / tiltMaxDegrees) * Math.abs(tiltAnim.to - tiltAnim.from)
            tiltAnim.duration = animationDuration(elapsed, tiltPhysical, oneShot)
            tiltAnim.start()
        }
    }

    // Pick the animation duration for one position update.
    //  - one-shot (first move / after a gap): realistic physical slew time (min 300 ms).
    //  - continuous (rapid updates, e.g. an EFX): pace to the update interval but
    //    NEVER shorter than the physical slew time, otherwise a large per-tick step
    //    would be crossed in a few ms, i.e. the head would teleport. When the effect
    //    ticks faster than the head can move, it runs at physical speed and lags.
    function animationDuration(elapsed, physical, oneShot)
    {
        if (oneShot)
            return Math.max(physical, 300)
        return Math.max(elapsed, physical, 1)
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
                Parameter { name: "diffuse"; value: Qt.color("gray") },
                Parameter { name: "specular"; value: Qt.color("black") },
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

    components: [
        baseMesh,
        headEntity,
        transform,
        material,
        sceneLayer
    ]
}
