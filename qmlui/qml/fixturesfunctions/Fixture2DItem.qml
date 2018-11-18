/*
  Q Light Controller Plus
  Fixture2DItem.qml

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

import QtQuick 2.10

import org.qlcplus.classes 1.0
import "CanvasDrawFunctions.js" as DrawFuncs
import "."

Rectangle
{
    id: fixtureItem
    x: ((gridCellSize * mmXPos) / gridUnits) + gridPosition.x
    y: ((gridCellSize * mmYPos) / gridUnits) + gridPosition.y
    z: 2
    width: (gridCellSize * mmWidth) / gridUnits
    height: (gridCellSize * mmHeight) / gridUnits

    color: "#2A2A2A"
    border.width: isSelected ? 2 : 1
    border.color: isSelected ? UISettings.selection : UISettings.fgLight

    //Drag.active: fxMouseArea.drag.active

    property int itemID: fixtureManager.invalidFixture()
    property string fixtureName: ""

    property real gridCellSize: View2D.cellPixels
    property real gridUnits: View2D.gridUnits === MonitorProperties.Meters ? 1000.0 : 304.8
    property point gridPosition: View2D.gridPosition

    property real mmXPos: 0
    property real mmYPos: 0
    property real mmHeight: 300
    property real mmWidth: 300

    property int headsNumber: 1
    property real headSide: 10
    property size headsLayout: Qt.size(1, 1)

    property int panMaxDegrees: 0
    property int tiltMaxDegrees: 0

    property bool isSelected: false
    property bool showLabel: false

    onWidthChanged: calculateHeadSize();
    onHeightChanged: calculateHeadSize();
    //onHeadsNumberChanged: calculateHeadSize();

    function calculateHeadSize()
    {
        var columns, rows
        if (headsLayout !== Qt.size(1, 1))
        {
            columns = headsLayout.width
            rows = headsLayout.height
            //console.log("" + fixtureName + ": layout provided - " + columns + "x" + rows)
        }
        else
        {
            // fallback to guessing based on heads number
            //console.log("Guessing heads layout...")
            var areaSqrt = Math.sqrt((width * height) / headsNumber)
            columns = parseInt((width / areaSqrt) + 0.5)
            rows = parseInt((height / areaSqrt) + 0.5)

            // dirty workaround to correctly display right columns on one row
            if (rows === 1) columns = headsNumber
            if (columns === 1) rows = headsNumber

            if (columns > headsNumber)
                columns = headsNumber

            if (rows < 1) rows = 1
            if (columns < 1) columns = 1
        }
        var cellWidth = width / columns
        var cellHeight = height / rows
        headSide = Math.min(cellWidth, cellHeight) - 1

        var hHeadsWidth = headSide * columns
        var hSpacing = (width - hHeadsWidth) / (columns - 1)
        headsBox.columnSpacing = parseInt(hSpacing)
        headsBox.columns = columns
        headsBox.rows = rows
        headsBox.width = hHeadsWidth + (hSpacing * (columns - 1))
        headsBox.height = (headSide + 2) * rows
        //console.log("size: " + width + " x " + height + ", head size: " + headSide + ", spacing: " + hSpacing)
    }

    function setHeadIntensity(headIndex, intensity)
    {
        //console.log("headIdx: " + headIndex + ", int: " + intensity)
        headsRepeater.itemAt(headIndex).dimmerValue = intensity
    }

    function setHeadRGBColor(headIndex, color)
    {
        var headItem = headsRepeater.itemAt(headIndex)
        headItem.isWheelColor = false
        headItem.headColor1 = color
    }

    function setShutter(type, low, high)
    {
        for (var i = 0; i < headsRepeater.count; i++)
            headsRepeater.itemAt(i).setShutter(type, low, high);
    }

    function setPosition(pan, tilt)
    {
        if (panMaxDegrees)
            positionLayer.panDegrees = (panMaxDegrees / 0xFFFF) * pan

        if (tiltMaxDegrees)
            positionLayer.tiltDegrees = (tiltMaxDegrees / 0xFFFF) * tilt

        positionLayer.requestPaint()
    }

    function setWheelColor(headIndex, col1, col2)
    {
        var headItem = headsRepeater.itemAt(headIndex)
        headItem.headColor1 = col1
        if (col2 !== Qt.rgba(0,0,0,1))
        {
            headItem.isWheelColor = true
            headItem.headColor2 = col2
        }
    }

    function setGoboPicture(headIndex, resource)
    {
        if (Qt.platform.os === "android")
            headsRepeater.itemAt(headIndex).goboSource = resource
        else
            headsRepeater.itemAt(headIndex).goboSource = "file:/" + resource
    }

    Grid
    {
        id: headsBox
        //width: headSide * headsLayout.width
        //height: headSide * headsLayout.height
        anchors.centerIn: parent

        Repeater
        {
            id: headsRepeater
            model: headsBox.columns * headsBox.rows // fixtureItem.headsNumber
            delegate:
                Rectangle
                {
                    id: headDelegate
                    property real intensity: dimmerValue * shutterValue
                    property real dimmerValue: 0
                    property real shutterValue: sAnimator.shutterValue
                    property bool isWheelColor: false
                    property color headColor1: "black"
                    property color headColor2: "black"
                    property string goboSource: ""

                    width: fixtureItem.headSide
                    height: width
                    color: "black"
                    radius: fixtureItem.headSide / 2
                    border.width: 1
                    border.color: "#AAA"

                    function setShutter(type, low, high)
                    {
                        sAnimator.setShutter(type, low, high)
                    }

                    ShutterAnimator { id: sAnimator }

                    MultiColorBox
                    {
                        x: 1
                        y: 1
                        width: parent.width - 2
                        height: parent.height - 2
                        radius: parent.radius - 2
                        opacity: headDelegate.intensity
                        biColor: headDelegate.isWheelColor
                        primary: headDelegate.headColor1
                        secondary: headDelegate.headColor2
                    }

                    Image
                    {
                        anchors.fill: parent
                        sourceSize: Qt.size(parent.width, parent.height)
                        source: headDelegate.goboSource
                    }
                }
        }
    }

    Canvas
    {
        id: positionLayer
        anchors.fill: parent
        visible: (panMaxDegrees || tiltMaxDegrees) ? true : false
        contextType: "2d"

        property int panDegrees: 0
        property int tiltDegrees: 0

        property int tiltWidth: ((positionLayer.width / 3) < 30) ? (positionLayer.width / 3) : 30
        property int panHeight: ((positionLayer.height / 4) < 30) ? (positionLayer.height / 4) : 30
        property int cursorRadius: tiltWidth / 2

        onPaint:
        {
            var context = getContext("2d")
            if (positionLayer.visible == false)
                return;

            context.globalAlpha = 0.7;
            context.lineWidth = 1;

            context.clearRect(0, 0, width, height)

            if (tiltMaxDegrees)
            {
                // draw TILT curve
                context.strokeStyle = "#2E77FF";
                DrawFuncs.drawEllipse(context, width / 2, height / 2, tiltWidth, height)
            }
            if (panMaxDegrees)
            {
                // draw PAN curve
                context.strokeStyle = "#19438F"
                DrawFuncs.drawEllipse(context, width / 2, height / 2, width, panHeight)
            }

            context.lineWidth = 1;
            context.strokeStyle = "white";

            if (tiltMaxDegrees)
            {
                // draw TILT cursor position
                context.fillStyle = "red";
                DrawFuncs.drawCursor(context, width / 2, height / 2, tiltWidth, height, tiltDegrees + 135, cursorRadius)
            }
            if (panMaxDegrees)
            {
                // draw PAN cursor position
                context.fillStyle = "green";
                DrawFuncs.drawCursor(context, width / 2, height / 2, width, panHeight, panDegrees + 90, cursorRadius)
            }
        }
    }

    Rectangle
    {
        id: fixtureLabel
        y: parent.height + 2
        x: -10
        width: parent.width + 20
        height: UISettings.listItemHeight
        color: "#444"
        visible: showLabel

        RobotoText
        {
            anchors.fill: parent
            label: fixtureName
            //labelColor: "black"
            fontSize: UISettings.textSizeDefault / 2
            wrapText: true
            textHAlign: Text.AlignHCenter
        }
    }

    MouseArea
    {
        id: fxMouseArea
        anchors.fill: parent
        hoverEnabled: true
        propagateComposedEvents: true

        onEntered: fixtureLabel.visible = true
        onExited: showLabel ? fixtureLabel.visible = true : fixtureLabel.visible = false

        onPressed:
        {
            // do not accept this event to propagate it to the drag rectangle
            mouse.accepted = false
        }
    }
}
