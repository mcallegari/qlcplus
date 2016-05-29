/*
  Q Light Controller Plus
  GridEditor.qml

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

Rectangle
{
    id: gridRoot
    width: 640
    height: 480

    color: "transparent"

    /** The size of the universe grid to render */
    property size gridSize: Qt.size(1,1)

    /** An array of data organized as follows: Fixture ID | DMX address | isOdd | channel type */
    property variant gridData

    /** The size in pixels of a grid cell */
    property real cellSize

    /** The number of universe DMX addresses to display (typically 512) */
    property int showIndices: 0

    /** During a drag operation, this indicates if the selection is valid (green) or not (red) */
    property bool validSelection: true

    /** An array of DMX addresses composing the currently selected fixtures */
    property variant selectionData

    property int currentFixtureID

    onGridSizeChanged: calculateCellSize()
    onGridDataChanged: { selectionData = null; dataCanvas.requestPaint() }
    onWidthChanged: calculateCellSize()
    onValidSelectionChanged: dataCanvas.requestPaint()

    signal pressed(int xPos, int yPos, int mods)
    signal positionChanged(int xPos, int yPos, int mods)
    signal released(int xPos, int yPos, int offset, int mods)

    signal dragEntered(int xPos, int yPos, var dragEvent)
    signal dragPositionChanged(int xPos, int yPos, var dragEvent)

    property color oddColor: "#2D84B0"
    property color evenColor: "#2C58B0"

    function calculateCellSize()
    {
        if (width <= 0 || height <= 0)
            return;
        cellSize = width / gridSize.width;
        if (cellSize > 50)
            cellSize = 50;

        //gridRoot.width = cellSize * gridSize.width
        gridRoot.height = cellSize * gridSize.height

        //console.log("Grid View cell size: " + cellSize)
    }

    function setSelectionData(data)
    {
        selectionData = data
        dataCanvas.requestPaint()
    }

    Canvas
    {
        id: dataCanvas
        width: parent.width
        height: parent.height
        x: 0
        y: 0
        z: 0

        property int selectionOffset: 0
        property int mouseOrigX: -1
        property int mouseOrigY: -1
        property int mouseLastX: -1
        property int mouseLastY: -1
        property bool movingSelection: false

        function checkPosition(mouse)
        {
            var xCell = parseInt(mouse.x / cellSize)
            var yCell = parseInt(mouse.y / cellSize)

            if (xCell < 0 || xCell >= gridSize.width || yCell < 0 || yCell >= gridSize.height)
                return

            if (mouseOrigX == -1 || mouseOrigY == -1)
            {
                mouseOrigX = xCell
                mouseOrigY = yCell
            }

            if (xCell !== mouseLastX || yCell !== mouseLastY)
            {
                mouseLastX = xCell
                mouseLastY = yCell
                return true
            }

            return false
        }

        function getPressedFixtureID(uniAddress)
        {
            if (gridData === null)
                return -1

            for (var idx = 0; idx < gridData.length; idx+=4)
            {
                if (gridData[idx + 1] === uniAddress)
                    return gridData[idx]
            }
            return -1
        }

        function setSelectionOffset()
        {
            dataCanvas.selectionOffset =
                    ((dataCanvas.mouseLastY - dataCanvas.mouseOrigY) * gridSize.width) +
                      dataCanvas.mouseLastX - dataCanvas.mouseOrigX
        }

        function fillCell(ctx, absIdx, color)
        {
            var yCell = parseInt(absIdx / gridSize.width)
            var xCell = absIdx - (yCell * gridSize.width)
            ctx.fillStyle = color
            ctx.fillRect(xCell * cellSize, yCell * cellSize, cellSize, cellSize)
        }

        onPaint:
        {
            var ctx = dataCanvas.getContext('2d');
            ctx.fillStyle = "#7f7f7f"
            ctx.fillRect(0, 0, cellSize * gridSize.width, cellSize * gridSize.height)

            if (gridData && gridData.length)
            {
                console.log("[GridEditor] Full repaint data length: " + gridData.length)

                for (var idx = 0; idx < gridData.length; idx+=4)
                {
                    //console.log("Fixture ID: " + gridData[idx] + ", address: " + gridData[idx + 1]);

                    if (gridData[idx + 2])
                        fillCell(ctx, gridData[idx + 1], oddColor)
                    else
                        fillCell(ctx, gridData[idx + 1], evenColor)
                }
                if (selectionData && selectionData.length)
                {
                    // if the selection has a offset, clear the original position first
                    if (selectionOffset != 0)
                    {
                        for (var cIdx = 0; cIdx < selectionData.length; cIdx++)
                        {
                            var clearIdx = selectionData[cIdx]
                            fillCell(ctx, clearIdx, "#7f7f7f")
                        }
                    }
                    for (var selIdx = 0; selIdx < selectionData.length; selIdx++)
                    {
                        var gridIdx = selectionData[selIdx] + selectionOffset
                        //console.log("Update selection gridIdx: " + gridIdx + " x: " + xCell + " y: " + yCell)
                        if (validSelection)
                            fillCell(ctx, gridIdx, "green")
                        else
                            fillCell(ctx, gridIdx, "red")
                    }
                }
            }
        }

        MouseArea
        {
            anchors.fill: parent

            onPressed:
            {
                if (dataCanvas.checkPosition(mouse))
                {
                    var uniAddress = (dataCanvas.mouseLastY * gridSize.width) + dataCanvas.mouseLastX
                    currentFixtureID = dataCanvas.getPressedFixtureID(uniAddress)
                    if (currentFixtureID === -1)
                        return;

                    console.log("Clicked Fixture ID: " + currentFixtureID)
                    gridRoot.pressed(dataCanvas.mouseLastX, dataCanvas.mouseLastY, mouse.modifiers)
                    dataCanvas.movingSelection = true
                }
            }
            onPositionChanged:
            {
                if (dataCanvas.movingSelection && dataCanvas.checkPosition(mouse))
                {
                    dataCanvas.setSelectionOffset()
                    gridRoot.positionChanged(dataCanvas.mouseLastX, dataCanvas.mouseLastY, mouse.modifiers)
                    dataCanvas.requestPaint()
                }
            }
            onReleased:
            {
                if (dataCanvas.selectionOffset != 0 && selectionData && selectionData.length)
                {
                    var absPosition = (dataCanvas.mouseLastY * gridSize.width) + dataCanvas.mouseLastX
                    var offset = selectionData[0] + dataCanvas.selectionOffset - absPosition
                    console.log("Offset with mouse: " + offset)
                    gridRoot.released(dataCanvas.mouseLastX, dataCanvas.mouseLastY, offset, mouse.modifiers)
                    dataCanvas.mouseOrigX = -1
                    dataCanvas.mouseOrigY = -1
                    dataCanvas.mouseLastX = -1
                    dataCanvas.mouseLastY = -1
                    dataCanvas.selectionOffset = 0
                    dataCanvas.movingSelection = false
                }
            }
        }

        DropArea
        {
            anchors.fill: parent
            onEntered:
            {
                dataCanvas.checkPosition(drag)
                gridRoot.dragEntered(dataCanvas.mouseLastX, dataCanvas.mouseLastY, drag)
            }

            onPositionChanged:
            {
                if (dataCanvas.checkPosition(drag))
                {
                    console.log("Drag position changed" + dataCanvas.mouseLastX + " " + dataCanvas.mouseLastY)
                    dataCanvas.setSelectionOffset()
                    gridRoot.dragPositionChanged(dataCanvas.mouseLastX, dataCanvas.mouseLastY, drag)
                    dataCanvas.requestPaint()
                }
            }

            onExited:
            {
                dataCanvas.mouseOrigX = -1
                dataCanvas.mouseOrigY = -1
                dataCanvas.mouseLastX = -1
                dataCanvas.mouseLastY = -1
                dataCanvas.selectionOffset = 0
                selectionData = []
                dataCanvas.requestPaint()
            }
        }
    }

    // a top layer Canvas that draws only a grid with
    // a size determined by gridSize
    Canvas
    {
        id: gridCanvas
        width: parent.width
        height: parent.height
        x: 0
        y: 0
        z: 1

        antialiasing: true

        onPaint:
        {
            var ctx = gridCanvas.getContext('2d');
            ctx.strokeStyle = "#1A1A1A";
            ctx.fillStyle = "transparent";
            ctx.lineWidth = 1;

            ctx.beginPath();
            var cWidth = cellSize * gridSize.width

            for (var vl = 1; vl < gridSize.width; vl++)
            {
                var xPos = cellSize * vl;
                ctx.moveTo(xPos, 0);
                ctx.lineTo(xPos, height);
            }
            for (var hl = 1; hl < gridSize.height; hl++)
            {
                var yPos = cellSize * hl;
                ctx.moveTo(0, yPos);
                ctx.lineTo(cWidth, yPos);
            }
            ctx.closePath();
            ctx.stroke();

            if (showIndices)
            {
                ctx.font = '10px "Roboto Condensed"'
                var xpos = 5
                var ypos = cellSize - 10
                var rowCount = 0;

                for(var idx = 1; idx <= showIndices; idx++)
                {
                    ctx.strokeText(idx, xpos, ypos)
                    xpos += cellSize
                    rowCount++
                    if (rowCount === gridSize.width)
                    {
                        xpos = 5
                        rowCount = 0
                        ypos += cellSize
                    }
                }

            }
        }
    }
}
