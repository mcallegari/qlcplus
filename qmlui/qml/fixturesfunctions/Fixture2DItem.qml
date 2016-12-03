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

import QtQuick 2.2

import "CanvasDrawFunctions.js" as DrawFuncs
import "."

Rectangle
{
    id: fixtureItem
    x: (gridCellSize * mmXPos) / gridUnits
    y: (gridCellSize * mmYPos) / gridUnits
    z: 2
    width: (gridCellSize * mmWidth) / gridUnits
    height: (gridCellSize * mmHeight) / gridUnits

    color: "#2A2A2A"
    border.width: isSelected ? 2 : 1
    border.color: isSelected ? UISettings.selection : UISettings.fgLight

    Drag.active: fxMouseArea.drag.active

    property int fixtureID: fixtureManager.invalidFixture()
    property string fixtureName: ""

    property real gridCellSize: parent ? parent.cellSize : 100
    property int gridUnits: parent ? parent.gridUnits : 1000

    property real mmXPos: 0
    property real mmYPos: 0
    property real mmHeight: 300
    property real mmWidth: 300

    property int headsNumber: 1
    property real headSide: 10
    property int headColumns: 1
    property int headRows: 1

    property int panMaxDegrees: 0
    property int tiltMaxDegrees: 0

    property bool isSelected: false
    property bool isDragging: false
    property bool showLabel: false

    onWidthChanged: calculateHeadSize();
    onHeightChanged: calculateHeadSize();
    onHeadsNumberChanged: calculateHeadSize();

    function calculateHeadSize()
    {
        var areaSqrt = Math.sqrt((width * height) / headsNumber)
        var columns = parseInt((width / areaSqrt) + 0.5)
        var rows = parseInt((height / areaSqrt) + 0.5)

        // dirty workaround to correctly display right columns on one row
        if (rows === 1) columns = headsNumber
        if (columns === 1) rows = headsNumber

        if (columns > headsNumber)
            columns = headsNumber

        if (rows < 1) rows = 1
        if (columns < 1) columns = 1

        var cellWidth = width / columns
        var cellHeight = height / rows
        headSide = parseInt(Math.min(cellWidth, cellHeight))
        headColumns = columns
        headRows = rows
    }

    function setHeadIntensity(headIndex, intensity)
    {
        //console.log("headIdx: " + headIndex + ", int: " + intensity)
        headsRepeater.itemAt(headIndex).headLevel = intensity
    }

    function setHeadColor(headIndex, color)
    {
        headsRepeater.itemAt(headIndex).headColor = color
    }

    function setPosition(pan, tilt)
    {
        if (panMaxDegrees)
            positionLayer.panDegrees = (panMaxDegrees / 0xFFFF) * pan

        if (tiltMaxDegrees)
            positionLayer.tiltDegrees = (tiltMaxDegrees / 0xFFFF) * tilt

        positionLayer.requestPaint()
    }

    function setGoboPicture(headIndex, resource)
    {
        if (Qt.platform.os === "android")
            headsRepeater.itemAt(headIndex).goboSource = resource
        else
            headsRepeater.itemAt(headIndex).goboSource = "file:/" + resource
    }

    Flow
    {
        id: headsBox
        width: headSide * headColumns
        height: headSide * headRows
        anchors.centerIn: parent

        Repeater
        {
            id: headsRepeater
            model: fixtureItem.headsNumber
            delegate:
                Rectangle
                {
                    id: headDelegate
                    property color headColor: "black"
                    property real headLevel: 0.0
                    property real whiteLevel: 0.0
                    property real amberLevel: 0.0
                    property real uvLevel: 0.0
                    property string goboSource: ""

                    width: fixtureItem.headSide
                    height: width
                    color: "black"
                    radius: fixtureItem.headSide / 2
                    border.width: 1
                    border.color: "#AAA"

                    Rectangle
                    {
                        id: headMainLayer
                        x: 1
                        y: 1
                        width: parent.width - 2
                        height: parent.height - 2
                        radius: parent.radius - 2
                        color: headDelegate.headColor
                        opacity: headDelegate.headLevel

                        Rectangle
                        {
                            id: headWhiteLayer
                            anchors.fill: parent
                            radius: parent.radius
                            color: "white"
                            opacity: headDelegate.whiteLevel
                        }
                        Rectangle
                        {
                            id: headAmberLayer
                            anchors.fill: parent
                            radius: parent.radius
                            color: "#FF7E00"
                            opacity: headDelegate.amberLevel
                        }
                        Rectangle
                        {
                            id: headUVLayer
                            anchors.fill: parent
                            radius: parent.radius
                            color: "#9400D3"
                            opacity: headDelegate.uvLevel
                        }
                    }
                    Image
                    {
                        id: headGoboLayer
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

        property int panDegrees: 0
        property int tiltDegrees: 0

        property int tiltWidth: ((positionLayer.width / 3) < 30) ? (positionLayer.width / 3) : 30
        property int panHeight: ((positionLayer.height / 4) < 30) ? (positionLayer.height / 4) : 30
        property int cursorRadius: tiltWidth / 2

        onPaint:
        {
            if (positionLayer.visible == false)
                return;

            var ctx = positionLayer.getContext('2d');
            //ctx.save();
            ctx.globalAlpha = 0.7;
            ctx.lineWidth = 1;

            ctx.clearRect(0, 0, width, height)

            if (tiltMaxDegrees)
            {
                // draw TILT curve
                ctx.strokeStyle = "#2E77FF";
                DrawFuncs.drawEllipse(ctx, width / 2, height / 2, tiltWidth, height)
            }
            if (panMaxDegrees)
            {
                // draw PAN curve
                ctx.strokeStyle = "#19438F"
                DrawFuncs.drawEllipse(ctx, width / 2, height / 2, width, panHeight)
            }

            ctx.lineWidth = 1;
            ctx.strokeStyle = "white";

            if (tiltMaxDegrees)
            {
                // draw TILT cursor position
                ctx.fillStyle = "red";
                DrawFuncs.drawCursor(ctx, width / 2, height / 2, tiltWidth, height, tiltDegrees + 135, cursorRadius)
            }
            if (panMaxDegrees)
            {
                // draw PAN cursor position
                ctx.fillStyle = "green";
                DrawFuncs.drawCursor(ctx, width / 2, height / 2, width, panHeight, panDegrees + 90, cursorRadius)
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
        preventStealing: false

        drag.threshold: UISettings.iconSizeDefault

        onEntered: fixtureLabel.visible = true
        onExited: showLabel ? fixtureLabel.visible = true : fixtureLabel.visible = false

        onPressed:
        {
            isSelected = !isSelected
            contextManager.setFixtureSelection(fixtureID, isSelected)
        }

        onPositionChanged:
        {
            if (!fxMouseArea.pressed)
                return
            drag.target = fixtureItem
            isSelected = true
        }

        onReleased:
        {
            if (drag.target !== null)
            {
                console.log("drag finished");
                mmXPos = (fixtureItem.x * gridUnits) / gridCellSize;
                mmYPos = (fixtureItem.y * gridUnits) / gridCellSize;
                contextManager.setFixturePosition(fixtureID, mmXPos, mmYPos)
                drag.target = null
            }
        }
    }
}
