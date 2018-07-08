/*
  Q Light Controller Plus
  RGBMatrixPreview.qml

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

import QtQuick 2.0

import "."

Rectangle
{
    id: matrixBox
    width: 400
    height: 400

    color: "black"

    property size matrixSize: Qt.size(0, 0)
    property variant matrixData
    property bool circleItems: false
    property int maximumHeight: 0

    onMatrixSizeChanged: matrix.calculateCellSize()
    onMatrixDataChanged: matrix.requestPaint()
    onWidthChanged: matrix.calculateCellSize()
    onMaximumHeightChanged: matrix.calculateCellSize()

    Canvas
    {
        id: matrix
        width: parent.width
        height: parent.height
        x: 0
        y: 0
        z: 0
        contextType: "2d"

        property int cellSize

        function calculateCellSize()
        {
            var cWidth = matrixBox.width / matrixSize.width
            var cHeight = (maximumHeight ? maximumHeight : matrixBox.width) / matrixSize.height

            cellSize = Math.min(cWidth, cHeight)

            width = cellSize * matrixSize.width
            x = (matrixBox.width - width) / 2

            height = cellSize * matrixSize.height
            matrixBox.height = height + 10
            y = (matrixBox.height - height) / 2

            //console.log("Width: " + matrixBox.width + ", height: " + matrixBox.height)
            //console.log("RGB Matrix cell size: " + cellSize + " " + matrixSize.width + "x" + matrixSize.height)
        }

        onPaint:
        {
            context.globalAlpha = 1.0
            context.strokeStyle = UISettings.bgLight
            context.fillStyle = "black"
            context.lineWidth = 1

            context.clearRect(0, 0, width, height)
            context.fillRect(0, 0, width, height)

            var yPos = 0
            var xPos = 0
            var twoPi = Math.PI * 2

            for (var yp = 0; yp < matrixSize.height; yp++)
            {
                if (matrixBox.circleItems)
                {
                    xPos = cellSize / 2
                    yPos = (yp * cellSize) + (cellSize / 2)
                }
                else
                {
                    xPos = 0
                    yPos = yp * cellSize
                }

                for (var xp = 0; xp < matrixSize.width; xp++)
                {
                    var col = matrixData[(yp * matrixSize.width) + xp]
                    if (col === undefined || col.a === 0)
                    {
                        xPos += cellSize
                        continue
                    }

                    context.fillStyle = col

                    if (matrixBox.circleItems)
                    {
                        context.beginPath()
                        context.arc(xPos, yPos, cellSize / 2, 0, twoPi, false)
                        context.fill()
                        context.stroke()
                        context.closePath()
                    }
                    else
                    {
                        context.fillRect(xPos, yPos, cellSize, cellSize)
                        context.strokeRect(xPos, yPos, cellSize, cellSize)
                    }

                    xPos += cellSize
                }
            }
        }
    }
    MouseArea
    {
        anchors.fill: parent
        onClicked: circleItems = !circleItems
    }
}
