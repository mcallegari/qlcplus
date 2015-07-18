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

Rectangle
{
    id: fixtureItem
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

    property bool isSelected: false
    property bool showLabel: false

    onWidthChanged: calculateHeadSize();
    onHeightChanged: calculateHeadSize();
    onHeadsNumberChanged: calculateHeadSize();

    function calculateHeadSize()
    {
        var areaSqrt = Math.sqrt((width * height) / headsNumber)
        var columns = parseInt((width / areaSqrt) + 0.5);
        var rows = parseInt((height / areaSqrt) + 0.5);

        // dirty workaround to correctly display right columns on one row
        if (rows === 1) columns = headsNumber;
        if (columns === 1) rows = headsNumber;

        if (columns > headsNumber)
            columns = headsNumber;

        if (rows < 1) rows = 1;
        if (columns < 1) columns = 1;

        var cellWidth = width / columns;
        var cellHeight = height / rows;
        headSide = (cellWidth < cellHeight) ? cellWidth : cellHeight;
        headColumns = columns;
        headRows = rows;
    }

    function setHeadIntensity(headIndex, intensity)
    {
        console.log("headIdx: " + headIndex + ", int: " + intensity)
        headsRepeater.itemAt(headIndex).headLevel = intensity
    }

    function setHeadColor(headIndex, color)
    {
        headsRepeater.itemAt(headIndex).headColor = color
    }

    function setGoboPicture(headIndex, resource)
    {
        headsRepeater.itemAt(headIndex).goboSource = "file:/" + resource
    }

    x: (gridCellSize * mmXPos) / gridUnits
    y: (gridCellSize * mmYPos) / gridUnits
    z: 2
    width: (gridCellSize * mmWidth) / gridUnits
    height: (gridCellSize * mmHeight) / gridUnits

    color: "#2A2A2A"
    border.width: isSelected ? 2 : 1
    border.color: isSelected ? "yellow" : "#AAA"

    Drag.active: fxMouseArea.drag.active

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

                    width: fixtureItem.headSide - 1
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

    Rectangle
    {
        id: fixtureLabel
        y: parent.height + 2
        x: -10
        width: parent.width + 20
        height: 28
        color: "#444"
        visible: showLabel

        RobotoText
        {
            width: parent.width
            height: parent.height
            label: fixtureName
            //labelColor: "black"
            fontSize: 8
            wrapText: true
            textAlign: Text.AlignHCenter
        }
    }

    MouseArea
    {
        id: fxMouseArea
        anchors.fill: parent
        hoverEnabled: true
        preventStealing: false

        onEntered: fixtureLabel.visible = true
        onExited: showLabel ? fixtureLabel.visible = true : fixtureLabel.visible = false

        onPressAndHold:
        {
            drag.target = fixtureItem
            console.log("drag started");
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
        onClicked:
        {
            isSelected = !isSelected
            contextManager.setFixtureSelection(fixtureID, isSelected)
        }
    }
}
