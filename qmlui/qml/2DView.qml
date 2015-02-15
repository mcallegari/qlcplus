/*
  Q Light Controller Plus
  2DView.qml

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

import QtQuick 2.3

Rectangle {
    anchors.fill: parent
    color: "black"

    onWidthChanged: twoDView.calculateCellSize()

    Flickable {
        id: twoDView
        objectName: "twoDView"
        anchors.fill: parent
        z: 1
        boundsBehavior: Flickable.StopAtBounds
        contentWidth: parent.width
        contentHeight: parent.height

        property int gridWidth: 5
        property int gridHeight: 5
        property real gridScale: 1.0
        property int gridUnits: 1000

        property real baseCellSize

        Component.onCompleted: calculateCellSize()

        onGridWidthChanged: calculateCellSize();
        onGridHeightChanged: calculateCellSize();
        onGridScaleChanged: calculateCellSize();
        onGridUnitsChanged: calculateCellSize();

        function calculateCellSize() {
            var xDiv = width / gridWidth;
            var yDiv = height / gridHeight;
            twoDContents.x = 0;
            twoDContents.y = 0;
            if (yDiv < xDiv)
            {
                twoDView.baseCellSize = yDiv * gridScale;
                if (gridScale == 1.0)
                    twoDContents.x = (width - (yDiv * gridWidth)) / 2;
            }
            else if (xDiv < yDiv)
            {
                baseCellSize = xDiv * gridScale;
                if (gridScale == 1.0)
                    twoDContents.y = (height - (xDiv * gridHeight)) / 2;
            }
            contentWidth = baseCellSize * gridWidth;
            contentHeight = baseCellSize * gridHeight;

            console.log("Cell size calculated: " + baseCellSize)
            twoDContents.requestPaint();
        }

        Canvas {
            id: twoDContents
            objectName: "twoDContents"
            width: twoDView.contentWidth
            height: twoDView.contentHeight
            x: 0
            y: 0
            z: 0

            antialiasing: true

            property real cellSize: twoDView.baseCellSize
            property int gridUnits: twoDView.gridUnits

            onPaint: {
                var ctx = twoDContents.getContext('2d');
                //ctx.save();
                ctx.globalAlpha = 1.0;
                ctx.strokeStyle = "#1A1A1A";
                ctx.fillStyle = "black";
                ctx.lineWidth = 1;

                ctx.beginPath();
                ctx.clearRect(0, 0, width, height);
                ctx.fillRect(0, 0, width, height)
                ctx.rect(0, 0, width, height)

                for (var vl = 1; vl < twoDView.gridWidth; vl++)
                {
                    var xPos = cellSize * vl;
                    ctx.moveTo(xPos, 0);
                    ctx.lineTo(xPos, height);
                }
                for (var hl = 1; hl < twoDView.gridHeight; hl++)
                {
                    var yPos = cellSize * hl;
                    ctx.moveTo(0, yPos);
                    ctx.lineTo(width, yPos);
                }
                ctx.closePath();
                ctx.stroke();
                //ctx.restore();
            }

            MouseArea {
                anchors.fill: parent
                onWheel: {
                    console.log("Wheel delta: " + wheel.angleDelta.y)
                    if (wheel.angleDelta.y > 0)
                        twoDView.gridScale += 0.5;
                    else {
                        if (twoDView.gridScale > 1.0)
                            twoDView.gridScale -= 0.5;
                    }
                }
            }
            DropArea {
                anchors.fill: parent

            }
        }
    }
}
