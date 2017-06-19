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

import QtQuick 2.7

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

    property int panMaxDegrees: 0
    property int tiltMaxDegrees: 0

    function setPosition(pan, tilt)
    {
        //console.log("[3Ditem] set position " + pan + ", " + tilt)
        if (panMaxDegrees)
        {
            panAnim.stop()
            panAnim.from = eSceneLoader.panRotation
            panAnim.to = (panMaxDegrees / 0xFFFF) * pan
            panAnim.start()
        }

        if (tiltMaxDegrees)
        {
            tiltAnim.stop()
            tiltAnim.from = eSceneLoader.tiltRotation
            var degTo = parseInt(((tiltMaxDegrees / 0xFFFF) * tilt) - (tiltMaxDegrees / 2))
            //console.log("Tilt to " + degTo + ", max: " + tiltMaxDegrees)
            tiltAnim.to = degTo
            tiltAnim.start()
        }
    }

    SceneLoader
    {
        id: eSceneLoader

        property real panRotation: 0
        property real tiltRotation: 0
        property matrix4x4 panMatrix

        function bindPanTransform(t, maxDegrees)
        {
            console.log("Binding pan ----")
            fixtureEntity.panMaxDegrees = maxDegrees
            t.rotationZ = Qt.binding(function() { return panRotation })

            panMatrix = t.matrix

            var origMatrix = t.matrix;
            var origTranslation = t.translation;
            var origRotation = t.rotation;
            var origScale = t.scale3D;
            console.log("origMatrix      = " + origMatrix);
            console.log("origTranslation = " + origTranslation);
            console.log("origRotation    = " + origRotation);
            console.log("origScale       = " + origScale);
        }

        function bindTiltTransform(t, maxDegrees)
        {
            console.log("Binding tilt ----")
            fixtureEntity.tiltMaxDegrees = maxDegrees
            //tiltRotation = maxDegrees / 2
            //t.rotationX = Qt.binding(function() { return tiltRotation })

            var origMatrix = t.matrix;
            var origTranslation = t.translation;
            var origRotation = t.rotation;
            var origScale = t.scale3D;
            console.log("origMatrix      = " + origMatrix);
            console.log("origTranslation = " + origTranslation);
            console.log("origRotation    = " + origRotation);
            console.log("origScale       = " + origScale);

            // Make a deep copy of the original translation. This will not be updated when the
            // transformation's translation property gets updated due to the animation.
            var constantTranslation = Qt.vector3d(origTranslation.x, origTranslation.y, origTranslation.z);
            var rotatedScale = Qt.vector3d(origScale.x, origScale.z, origScale.y);

            t.matrix = Qt.binding(function() {
                var m = Qt.matrix4x4();
                m.translate(constantTranslation);
                m.rotate(tiltRotation, Qt.vector3d(1, 0, 0));
                m.scale(rotatedScale);
                //console.log("Modified matrix = " + m)
                return m;

            })
        }

        onStatusChanged:
        {
            if (status == SceneLoader.Ready)
                View3D.initializeFixture(fixtureID, eObjectPicker, eSceneLoader)
        }

        NumberAnimation on panRotation
        {
            id: panAnim
            duration: 2000
            easing.type: Easing.Linear
        }

        NumberAnimation on tiltRotation
        {
            id: tiltAnim
            duration: 2000
            easing.type: Easing.Linear
        }
    }

    components: [ eSceneLoader ]

    /* This gets re-parented and activated on initializeFixture */
    ObjectPicker
    {
        id: eObjectPicker
        objectName: "ePicker"
        //hoverEnabled: true
        dragEnabled: true

        property var lastPos

        onClicked: contextManager.setFixtureSelection(fixtureID, true)
        onPressed: lastPos = pick.worldIntersection
        //onReleased: console.log("Item release")
        //onEntered: console.log("Item entered")
        //onExited: console.log("Item exited")
/*
        onMoved:
        {
            //console.log("Pick pos: " + pick.worldIntersection)
            //var x = pick.worldIntersection.x - lastPos
            contextManager.setFixturePosition(fixtureID,
                                              pick.worldIntersection.x * 1000.0,
                                              pick.worldIntersection.y * 1000.0,
                                              pick.worldIntersection.z * 1000.0)
        }
*/
    }
}


