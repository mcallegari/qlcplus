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

Rectangle {
    id: fixtureItem
    property int fixtureID: fixtureManager.invalidFixture()

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

    onWidthChanged: calculateHeadSize();
    onHeightChanged: calculateHeadSize();
    onHeadsNumberChanged: calculateHeadSize();

    function calculateHeadSize() {
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

    x: (gridCellSize * mmXPos) / gridUnits
    y: (gridCellSize * mmYPos) / gridUnits
    width: (gridCellSize * mmWidth) / gridUnits
    height: (gridCellSize * mmHeight) / gridUnits

    color: "#2A2A2A";
    border.width: 1
    border.color: isSelected ? "yellow" : "#AAA"

    Flow {
        width: headSide * headColumns
        height: headSide * headRows
        anchors.centerIn: parent
        Repeater {
            model: fixtureItem.headsNumber
            delegate:
                Rectangle {
                    objectName: "head" + index
                    width: fixtureItem.headSide - 1
                    height: width
                    color: "black"
                    radius: fixtureItem.headSide / 2
                    border.width: 1
                    border.color: "#AAA"
                }
        }
    }

    MouseArea {
        anchors.fill: parent
        onClicked: isSelected = !isSelected
    }
}
