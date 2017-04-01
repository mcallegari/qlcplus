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
import "."

Rectangle
{
    id: gridRoot
    width: 640
    height: 480

    color: "transparent"

    /** The size of the universe grid to render */
    property size gridSize: Qt.size(1,1)

    /** The resize constrain to fill the view */
    property int fillDirection: Qt.Horizontal

    /** An array of data organized as follows: Item ID | DMX address | isOdd | item type */
    property variant gridData

    /** An array of data organized as follows: Item ID | Absolute Index | Length | Label */
    property variant gridLabels

    /** The size in pixels of a grid cell */
    property real cellSize

    /** The number of indices to display (typically 512) */
    property int showIndices: 0

    /** During a drag operation, this indicates if the selection is valid (green) or not (red) */
    property bool validSelection: true

    /** An array of data composing the currently selected item as follows: address | type */
    property variant selectionData

    property int currentItemID

    onGridSizeChanged: calculateCellSize()
    onGridDataChanged: { selectionData = null; dataCanvas.requestPaint() }
    onGridLabelsChanged: gridCanvas.requestPaint()
    onWidthChanged: repaintTimer.restart()
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
            return

        var horSize = width / gridSize.width
        var vertSize = height / gridSize.height

        if (fillDirection == Qt.Vertical)
            cellSize = vertSize
        if (fillDirection == Qt.Horizontal)
            cellSize = horSize
        else
            cellSize = Math.min(Math.min(horSize, vertSize), UISettings.bigItemHeight)

        gridRoot.height = cellSize * gridSize.height

        //console.log("Grid View cell size: " + cellSize)
        dataCanvas.requestPaint()
        gridCanvas.requestPaint()
    }

    function setSelectionData(data)
    {
        selectionData = data
        dataCanvas.requestPaint()
    }

    Timer
    {
        id: repaintTimer
        repeat: false
        running: false
        interval: 100
        onTriggered: calculateCellSize()
    }

    Canvas
    {
        id: dataCanvas
        width: parent.width
        height: parent.height
        x: 0
        y: 0
        z: 0
        contextType: "2d"

        property int selectionOffset: 0
        property int mouseOrigX: -1
        property int mouseOrigY: -1
        property int mouseLastX: -1
        property int mouseLastY: -1
        property bool movingSelection: false
        property variant iconsCache: []

        onWidthChanged: repaintTimer.restart()

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

        function getPressedItemID(itemAbsIndex)
        {
            if (gridData === null)
                return -1

            for (var idx = 0; idx < gridData.length; idx+=4)
            {
                if (gridData[idx + 1] === itemAbsIndex)
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

        function checkIconCache(itemID, subIndex, itemType)
        {
            if (itemID === -1)
                return false

            if (dataCanvas.iconsCache[itemType] === undefined)
            {
                var iconSource = getItemIcon(itemID, subIndex)
                var imgObject = Qt.createQmlObject('import QtQuick 2.0; Image { source: "' + iconSource + '"; ' +
                                                   'width: gridRoot.cellSize * 0.5; ' +
                                                   'height: gridRoot.cellSize * 0.5; ' +
                                                   'sourceSize: Qt.size(width, height); visible: false }', dataCanvas)

                console.log("Caching icon: " + iconSource)
                //dataCanvas.loadImage(imgObject)
                dataCanvas.iconsCache[itemType] = imgObject
                return true
            }
            return false
        }

        function fillCell(absIdx, color, itemType)
        {
            var yCell = parseInt(absIdx / gridSize.width)
            var xCell = absIdx - (yCell * gridSize.width)
            context.fillStyle = color
            context.fillRect(xCell * cellSize, yCell * cellSize, cellSize, cellSize)
            if (itemType >= 0)
            {
                context.drawImage(dataCanvas.iconsCache[itemType],
                              (xCell * cellSize) + (cellSize / 2), (yCell * cellSize) + (cellSize / 2),
                              cellSize * 0.5, cellSize * 0.5)
            }
        }

        onPaint:
        {
            if (repaintTimer.running === true)
                return

            var needRepaint = false

            context.fillStyle = "#7f7f7f"
            context.fillRect(0, 0, cellSize * gridSize.width, cellSize * gridSize.height)

            if (gridData && gridData.length)
            {
                console.log("[GridEditor] Full repaint data length: " + gridData.length)
                var isOdd = 0
                var subIndex = 0

                for (var idx = 0; idx < gridData.length; idx+=4)
                {
                    if (gridData[idx + 2] !== isOdd)
                    {
                        subIndex = 0
                        isOdd = gridData[idx + 2]
                    }
                    console.log("Item ID: " + gridData[idx] + ", subIndex: " + gridData[idx + 1]);
                    if (checkIconCache(gridData[idx], subIndex, gridData[idx + 3]) === true)
                        needRepaint = true

                    if (gridData[idx + 2])
                        fillCell(gridData[idx + 1], oddColor, gridData[idx + 3])
                    else
                        fillCell(gridData[idx + 1], evenColor, gridData[idx + 3])

                    subIndex++
                }
            }

            if (selectionData && selectionData.length)
            {
                // if the selection has an offset, clear the original position first
                if (selectionOffset != 0)
                {
                    for (var cIdx = 0; cIdx < selectionData.length; cIdx+=2)
                    {
                        var clearIdx = selectionData[cIdx]
                        fillCell(clearIdx, "#7f7f7f", -1)
                    }
                }
                for (var selIdx = 0; selIdx < selectionData.length; selIdx+=2)
                {
                    var gridIdx = selectionData[selIdx] + selectionOffset
                    //console.log("Update selection gridIdx: " + gridIdx)
                    if (validSelection)
                        fillCell(gridIdx, "green", selectionData[selIdx + 1])
                    else
                        fillCell(gridIdx, "red", selectionData[selIdx + 1])
                }
            }
            if (needRepaint === true)
            {
                console.log("A further repaint is needed after icon caching")
                repaintTimer.restart()
            }
        }

        MouseArea
        {
            anchors.fill: parent

            onPressed:
            {
                if (dataCanvas.checkPosition(mouse))
                {
                    var itemAbsIndex = (dataCanvas.mouseLastY * gridSize.width) + dataCanvas.mouseLastX
                    currentItemID = dataCanvas.getPressedItemID(itemAbsIndex)
                    if (currentItemID === -1)
                        return;

                    dataCanvas.setSelectionOffset()
                    console.log("Clicked item ID: " + currentItemID)
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
                }
                dataCanvas.mouseOrigX = -1
                dataCanvas.mouseOrigY = -1
                dataCanvas.mouseLastX = -1
                dataCanvas.mouseLastY = -1
                dataCanvas.selectionOffset = 0
                dataCanvas.movingSelection = false
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
                    console.log("Drag position changed: " + dataCanvas.mouseLastX + " " + dataCanvas.mouseLastY)
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

    // a top layer Canvas that draws only the things that don't change:
    // 1) a grid with a size determined by gridSize
    // 2) DMX channel numbers
    Canvas
    {
        id: gridCanvas
        width: parent.width
        height: parent.height
        x: 0
        y: 0
        z: 1

        antialiasing: true
        contextType: "2d"

        onWidthChanged: repaintTimer.restart()

        function drawLabel(absIdx, text, fontSize, color)
        {
            var yCell = parseInt(absIdx / gridSize.width)
            var xCell = absIdx - (yCell * gridSize.width)
            context.fillStyle = color

            //console.log("absIdx: " + absIdx + ", x: " + xCell + ", y: " + yCell + ", label: " + text)

            context.fillText(text, (xCell * cellSize) + 2, (yCell * cellSize) + fontSize)
        }

        onPaint:
        {
            if (repaintTimer.running === true)
                return

            var context = gridCanvas.getContext('2d')
            context.fillStyle = "#1A1A1A"
            context.strokeStyle = "black"
            context.lineWidth = 1

            context.clearRect(0, 0, cellSize * gridSize.width, cellSize * gridSize.height)

            var cWidth = cellSize * gridSize.width
            var fontPxSize = cellSize / 3

            context.beginPath()
            /* Paint the grid vertical lines */
            for (var vl = 1; vl < gridSize.width; vl++)
            {
                var xPos = cellSize * vl
                context.moveTo(xPos, 0)
                context.lineTo(xPos, height)
            }

            /* Paint the grid horizontal lines */
            for (var hl = 1; hl < gridSize.height; hl++)
            {
                var yPos = cellSize * hl
                context.moveTo(0, yPos)
                context.lineTo(cWidth, yPos)
            }

            context.closePath()
            context.stroke()

            /* Paint the indices if needed */
            if (showIndices)
            {
                var xpos = 1
                var ypos = cellSize - 10
                var rowCount = 0

                context.font = fontPxSize + "px \"" + UISettings.robotoFontName + "\""

                for(var idx = 1; idx <= showIndices; idx++)
                {
                    context.fillText(idx, xpos, ypos)
                    xpos += cellSize
                    rowCount++
                    if (rowCount === gridSize.width)
                    {
                        xpos = 1
                        rowCount = 0
                        ypos += cellSize
                    }
                }
            }

            /* Paint the labels if needed */
            if (gridLabels && gridLabels.length)
            {
                for (var li = 0; li < gridLabels.length; li += 4)
                    drawLabel(gridLabels[li + 1], gridLabels[li + 3], fontPxSize, "white")
            }
        }
    }
}
