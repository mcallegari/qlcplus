/*
  Q Light Controller Plus
  Fixture3DItem.qml

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

import QtQuick 2.7 as QQ2

import Qt3D.Core 2.0
import Qt3D.Render 2.0
import Qt3D.Extras 2.0

import org.qlcplus.classes 1.0
import "."

Entity
{
    id: fixtureEntity

    property int fixtureID: fixtureManager.invalidFixture()
    property alias itemSource: eSceneLoader.source
    property bool isSelected: false

    property int panMaxDegrees: 0
    property int tiltMaxDegrees: 0

    property real panRotation: 0
    property real tiltRotation: 0

    property Transform panTransform
    property Transform tiltTransform

    /* Light properties. These are bound to uniforms in ScreenQuadEntity */
    property int lightIndex
    property real intensity: 0.0
    property color lightColor: Qt.rgba(0, 0, 0, 1)
    property vector3d lightPosition: Qt.vector3d(0, 0, 0)
    property vector3d direction: Qt.vector3d(0, -1, 0)
    property real cutOff: 15.0

    onFixtureIDChanged: isSelected = contextManager.isFixtureSelected(fixtureID)

    //onPanTransformChanged: console.log("Pan transform changed " + panTransform)
    //onTiltTransformChanged: console.log("Tilt transform changed " + tiltTransform)

    //onPositionChanged: console.log("Light position changed: " + position)
    //onDirectionChanged: console.log("Light direction changed: " + direction)

    function setPosition(pan, tilt)
    {
        //console.log("[3Ditem] set position " + pan + ", " + tilt)
        if (panMaxDegrees)
        {
            panAnim.stop()
            panAnim.from = panRotation
            panAnim.to = (panMaxDegrees / 0xFFFF) * pan
            panAnim.start()
        }

        if (tiltMaxDegrees)
        {
            tiltAnim.stop()
            tiltAnim.from = tiltRotation
            var degTo = parseInt(((tiltMaxDegrees / 0xFFFF) * tilt) - (tiltMaxDegrees / 2))
            //console.log("Tilt to " + degTo + ", max: " + tiltMaxDegrees)
            tiltAnim.to = -degTo
            tiltAnim.start()
        }
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

    QQ2.NumberAnimation on panRotation
    {
        id: panAnim
        duration: 2000
        easing.type: Easing.Linear
    }

    QQ2.NumberAnimation on tiltRotation
    {
        id: tiltAnim
        duration: 2000
        easing.type: Easing.Linear
    }

    property Transform transform: Transform { }

    SceneLoader
    {
        id: eSceneLoader

        onStatusChanged:
        {
            if (status == SceneLoader.Ready)
                View3D.initializeFixture(fixtureID, fixtureEntity, eObjectPicker, eSceneLoader)
        }
    }

    components: [ eSceneLoader, transform ]

    /* This gets re-parented and activated on initializeFixture */
    ObjectPicker
    {
        id: eObjectPicker
        //hoverEnabled: true
        dragEnabled: true

        property var lastPos

        onClicked:
        {
            console.log("Item clicked")
            isSelected = !isSelected
            contextManager.setFixtureSelection(fixtureID, isSelected)
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
            contextManager.setFixturePosition("3D", fixtureID,
                                              pick.worldIntersection.x * 1000.0,
                                              pick.worldIntersection.y * 1000.0,
                                              pick.worldIntersection.z * 1000.0)
        }
*/
    }
}


