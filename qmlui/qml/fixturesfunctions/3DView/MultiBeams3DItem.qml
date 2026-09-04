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

    /* Number of emitters across the bar width and along its depth.

       The physical layout describes the cell grid of the fixture body, which is
       NOT required to agree with the number of heads the selected mode declares.
       The Pulse LED BAR 320 is an 8x1 bar whose 4 channel mode declares no head
       at all, so the engine synthesises a single one covering every channel:
       laying that one emitter out on an 8 wide grid puts it in cell 0, i.e. at
       one end of the bar instead of across it. Only trust the layout when it
       accounts for exactly the emitters we have; otherwise put them in one row. */
    readonly property bool layoutMatchesHeads:
        headsLayout.width * headsLayout.height === headsNumber
    readonly property int cellColumns:
        layoutMatchesHeads ? headsLayout.width : Math.max(1, headsNumber)
    readonly property int cellRows:
        layoutMatchesHeads ? headsLayout.height : 1

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

    /* Shadows are not optional for this renderer: spotlight_shading.frag bounds
       the beam with the emitter's shadow map and nothing else, so an emitter
       without one lights everything inside its cone projection, straight through
       the walls of the stage environment. The per emitter cost is bounded from
       two sides: RenderShadowMapFilter matches no render pass while a cell is
       dark, and LightEntity uses a smaller map than a single head fixture. */
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
        return 0
    }

    /* Ray march steps of a single beam. The volumetric scattering pass runs once
       per emitter, so a bar would otherwise cost as much as headsNumber separate
       spotlights. Spend the budget of about four spotlights on the whole bar
       instead, with a floor that still reads as a beam: the cones of a bar are
       thin, so they need far fewer samples than a wide spotlight cone. */
    readonly property int beamRaymarchSteps:
        raymarchSteps <= 0 ? 0
                           : Math.max(6, Math.min(raymarchSteps,
                                                  Math.round((raymarchSteps * 4) / Math.max(1, headsNumber))))

    /* **************** Spotlight cone properties **************** */
    /* Radius of a single cell, rather than the lens radius of a PAR mesh that
       this item does not have. The 0.7 factor matches Fixture3DItem, where it
       compensates the mesh lens being slightly larger than the emitting surface. */
    property real coneTopRadius:
        Math.max(0.005, 0.5 * 0.7 * Math.min(phySize.x / cellColumns, phySize.z / cellRows))
    property real coneBottomRadius: distCutoff * Math.tan(cutoffAngle) + coneTopRadius

    /* Depth of the emitter inside the fixture body. Fixture3DItem takes this from
       the loaded mesh; a bar is drawn as a plain cuboid, so its own height is the
       closest equivalent. */
    property real headLength: Math.max(0.01, phySize.y)

    /* ********************* Light properties ********************* */
    /* ****** These are bound to uniforms in ScreenQuadEntity ***** */

    property real shutterValue: sAnimator.shutterValue
    /* Lumens of a single emitter of this fixture, from the "Lumens" physical
       property of its mode. 0 when the fixture definition carries no data */
    property real bulbLumens: 0
    /* Relative output of this fixture: its lumens against the brightest emitter
       in the project, so the reference fixture stays at the brightness it has
       always rendered at and everything else falls in below it. 1.0 (unscaled)
       when the "Lumens" setting is off, when this definition has no lumens, or
       when no fixture in the project has any. */
    property real lumensScale:
        (View3D && View3D.useFixtureLumens && bulbLumens > 0 && View3D.referenceLumens > 0) ?
            Math.min(1.0, bulbLumens / View3D.referenceLumens) : 1.0

    property vector3d lightDir: Math3D.getLightDirection(transform, null, tiltTransform)

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

        // Delete the existing heads first. setupScattering() re-parents the three
        // cones of a head to the scene root, so they do not die with it: without
        // cleanupScattering() they would be left in the scene, still bound to a
        // destroyed emitter, and every rebuild would add three more.
        for (i = headsList.length - 1; i >= 0; i--)
        {
            headsList[i].cleanupScattering()
            headsList[i].destroy()
        }

        headsList = []

        // itemID is invalid while the item is being built - MainView3D sets the
        // real one right after creation - and MainView3D::resetItems() sets it
        // back to -1 when the 3D view is torn down. Building a set of heads in
        // either case is pure waste, and in the teardown case it allocates
        // emitters into a scene that is being destroyed.
        if (itemID < 0 || headsNumber <= 0)
            return

        var component = Qt.createComponent("LightEntity.qml")
        if (component.status !== Component.Ready)
        {
            console.warn("MultiBeams3DItem: cannot load LightEntity.qml:", component.errorString())
            return
        }

        for (i = 0; i < headsNumber; i++)
        {
            // Everything shared with the parent is bound rather than copied: most
            // of these are only known once MainView3D::initializeFixture() has run,
            // which happens below, and render quality, zoom, shutter and tilt keep
            // changing afterwards.
            var headNode = component.createObject(fixtureEntity,
            {
                "headIndex": i,
                "enabled": Qt.binding(function() { return fixtureEntity.enabled }),
                "lightDir": Qt.binding(function() { return fixtureEntity.lightDir }),
                "shutterValue": Qt.binding(function() { return fixtureEntity.shutterValue }),
                "lumensScale": Qt.binding(function() { return fixtureEntity.lumensScale }),
                "raymarchSteps": Qt.binding(function() { return fixtureEntity.beamRaymarchSteps }),
                "cutoffAngle": Qt.binding(function() { return fixtureEntity.cutoffAngle }),
                "tiltRotation": Qt.binding(function() { return fixtureEntity.tiltRotation }),
                "distCutoff": Qt.binding(function() { return fixtureEntity.distCutoff }),
                "headLength": Qt.binding(function() { return fixtureEntity.headLength }),
                "coneTopRadius": Qt.binding(function() { return fixtureEntity.coneTopRadius }),
                "goboTexture": Qt.binding(function() { return fixtureEntity.goboTexture })
            });

            if (headNode === null)
            {
                console.warn("MultiBeams3DItem: cannot create head", i, "of item", itemID)
                break
            }

            headsList.push(headNode)
        }

        // 3DView.qml walks headsNumber heads when it builds the frame graph, so
        // this must never promise more emitters than actually exist
        if (headsList.length !== headsNumber)
            headsNumber = headsList.length

        // initializeFixture() is what hands us phySize and the lens angles, so the
        // beam geometry logged below is only meaningful once it has returned
        View3D.initializeFixture(itemID, fixtureEntity, null)

        console.log("MultiBeams3DItem: item", itemID, "emitters:", headsList.length,
                    "layout:", headsLayout.width + "x" + headsLayout.height,
                    "used as:", cellColumns + "x" + cellRows,
                    "| lens:", focusMinDegrees + "-" + focusMaxDegrees + " deg",
                    "cone top/bottom:", coneTopRadius.toFixed(4) + "/" + coneBottomRadius.toFixed(2),
                    "| steps:", beamRaymarchSteps)
    }

    function setupScattering(sceneEntity)
    {
        if (sceneEntity.coneMesh.length !== distCutoff)
            sceneEntity.coneMesh.length = distCutoff

        for (var i = 0; i < headsList.length; i++)
            headsList[i].setupScattering(sceneEntity)
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
        if (headIndex < 0 || headIndex >= headsList.length)
            return null

        return headsList[headIndex]
    }

    // The C++ side computes a single emitter position and orientation for the
    // whole bar (headIndex is always 0), so spread the cells over the fixture
    // body here: evenly across its width and depth, centered on the origin and
    // rotated by the bar's current orientation matrix.
    function setHeadLightProps(headIndex, pos, matrix)
    {
        var count = headsList.length
        if (count === 0)
            return

        var cellWidth = phySize.x / cellColumns
        var cellDepth = phySize.z / cellRows

        for (var h = 0; h < count; h++)
        {
            var column = h % cellColumns
            var row = Math.floor(h / cellColumns)
            var localPos = Qt.vector4d(-(phySize.x / 2) + ((column + 0.5) * cellWidth), 0,
                                       -(phySize.z / 2) + ((row + 0.5) * cellDepth), 0)

            var head = headsList[h]
            head.lightPos = pos.plus(matrix.times(localPos).toVector3d())
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

    // Same signature as Fixture3DItem: MainView3D calls this with degrees == true
    // when the fixture has a fixed zoom set in the monitor properties
    function setZoom(value, degrees)
    {
        if (degrees)
            cutoffAngle = (value / 2) * (Math.PI / 180.0)
        else
            cutoffAngle = (((((focusMaxDegrees - focusMinDegrees) / 255.0) * value) + focusMinDegrees) / 2.0) * (Math.PI / 180.0)
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

    /* headEntity is NOT listed here: it is an Entity, not a Component, so QML
       rejected it with a "Cannot append ... to a QML list of QComponent*"
       warning on every item creation. It is already a child of this entity
       through the default property, which is how it gets drawn and how
       MainView3D finds it by object name. */
    components: [
        baseMesh,
        transform,
        material,
        sceneLayer
    ]
}
