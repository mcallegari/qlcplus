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
    property size gridSize: Qt.size(1, 1)

    /** The resize constrain to fill the view */
    property int fillDirection: Qt.Horizontal

    /** The minimum size in pixels of a cell */
    property int mininumCellSize: 0

    /** An array of data organized as follows: Item ID | Absolute Index | isOdd | item type */
    property variant gridData

    /** An array of data organized as follows: Item ID | Absolute Index | Length | Label */
    property variant gridLabels

    property real labelsFontSize: cellSize / 3

    /** The size in pixels of a grid cell */
    property real cellSize

    /** The number of indices to display (typically 512) */
    property int showIndices: 0

    /** During a drag operation, this indicates if the selection is valid (green) or not (red) */
    property bool validSelection: true

    /** An array of absolute indices identifying the currently selected item(s) */
    property variant selectionData

    /** ID of the currently selected item */
    property int currentItemID

    /** A flag indicating if a drag from an external panel is in place */
    property bool externalDrag: itemDropArea.containsDrag

    onGridSizeChanged: calculateCellSize()
    onGridDataChanged: { selectionData = null; dataCanvas.requestPaint() }
    onGridLabelsChanged: gridCanvas.requestPaint()
    onWidthChanged: repaintTimer.restart()
    onValidSelectionChanged: dataCanvas.requestPaint()

    signal pressed(int xPos, int yPos, int mods)
    signal positionChanged(int xPos, int yPos, int offset, int mods)
    signal released(int xPos, int yPos, int offset, int mods)

    /** Signal emitted when an external drag item enters the grid */
    signal dragEntered(int xPos, int yPos, var dragEvent)

    /** Signal emitted when an external drag changes position */
    signal dragPositionChanged(int xPos, int yPos, int offset, var dragEvent)

    /** Signal emitted when an external drag has been dropped */
    signal dragDropped(int xPos, int yPos, var dragEvent)

    property color oddColor: "#2D84B0"
    property color evenColor: "#2C58B0"

    function calculateCellSize()
    {
        if (width <= 0 || height <= 0)
            return

        var horSize = width / gridSize.width
        var vertSize = height / gridSize.height

        if (mininumCellSize)
        {
            horSize = Math.max(horSize, mininumCellSize)
            vertSize = Math.max(vertSize, mininumCellSize)
        }

        if (fillDirection == Qt.Vertical)
            cellSize = vertSize
        if (fillDirection == Qt.Horizontal)
            cellSize = horSize
        else
            cellSize = Math.min(Math.min(horSize, vertSize), UISettings.bigItemHeight)

        // autosize width/height only if it needs to exceed the initial height
        if (cellSize * gridSize.height > height)
            height = cellSize * gridSize.height
        if (cellSize * gridSize.width > width)
            width = cellSize * gridSize.width

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
            // an array acting as map for quick lookup of item types per absolute index
            var typesMap = []

            context.fillStyle = "#7f7f7f"
            context.clearRect(0, 0, width, height)
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
                    //console.log("Item ID: " + gridData[idx] + ", subIndex: " + gridData[idx + 1] + ", type: " + gridData[idx + 3]);
                    if (checkIconCache(gridData[idx], subIndex, gridData[idx + 3]) === true)
                        needRepaint = true

                    typesMap[gridData[idx + 1]] = gridData[idx + 3]

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
                    for (var cIdx = 0; cIdx < selectionData.length; cIdx++)
                    {
                        var clearIdx = selectionData[cIdx]
                        fillCell(clearIdx, "#7f7f7f", -1)
                    }
                }
                for (var selIdx = 0; selIdx < selectionData.length; selIdx++)
                {
                    var gridIdx = selectionData[selIdx] + selectionOffset
                    //console.log("Update selection gridIdx: " + gridIdx)

                    // retrieve the item type from the types map array,
                    // unless it comes from an external drag
                    var itemType = itemDropArea.containsDrag ? -1 : typesMap[selectionData[selIdx]] //gridData[(selectionData[selIdx] * 4) + 3]
                    if (validSelection)
                        fillCell(gridIdx, "green", itemType)
                    else
                        fillCell(gridIdx, "red", itemType)
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
                    gridRoot.pressed(dataCanvas.mouseLastX, dataCanvas.mouseLastY, mouse.modifiers)
                    if (currentItemID === -1)
                        return
                    dataCanvas.setSelectionOffset()
                    console.log("Clicked item ID: " + currentItemID)
                    dataCanvas.movingSelection = true
                }
                else
                {
                    // handle outside click
                    gridRoot.pressed(-1, -1, 0);
                }
            }
            onPositionChanged:
            {
                if (dataCanvas.movingSelection && dataCanvas.checkPosition(mouse))
                {
                    dataCanvas.setSelectionOffset()
                    gridRoot.positionChanged(dataCanvas.mouseLastX, dataCanvas.mouseLastY,
                                             dataCanvas.selectionOffset, mouse.modifiers)
                    dataCanvas.requestPaint()
                }
            }
            onReleased:
            {
                gridRoot.released(dataCanvas.mouseLastX, dataCanvas.mouseLastY,
                                  dataCanvas.selectionOffset, mouse.modifiers)

                dataCanvas.mouseOrigX = -1
                dataCanvas.mouseOrigY = -1
                dataCanvas.mouseLastX = -1
                dataCanvas.mouseLastY = -1
                dataCanvas.selectionOffset = 0
                dataCanvas.movingSelection = false
                validSelection = true
            }
        }

        DropArea
        {
            id: itemDropArea
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
                    gridRoot.dragPositionChanged(dataCanvas.mouseLastX, dataCanvas.mouseLastY,
                                                 dataCanvas.selectionOffset, drag)
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

            onDropped:
            {
                gridRoot.dragDropped(dataCanvas.mouseLastX, dataCanvas.mouseLastY, drag)
                dataCanvas.mouseOrigX = -1
                dataCanvas.mouseOrigY = -1
                dataCanvas.mouseLastX = -1
                dataCanvas.mouseLastY = -1
                dataCanvas.selectionOffset = 0
                selectionData = []
            }
        }
    }

    // a top layer Canvas that draws only the things that don't change:
    // 1) a grid with a size determined by gridSize
    // 2) DMX channel numbers
    // 3) Item labels if present in gridLabels
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

        function drawLabel(x, y, text, fontSize, color)
        {
            context.fillStyle = color

            //console.log("absIdx: " + absIdx + ", x: " + xCell + ", y: " + yCell + ", label: " + text)

            var yOffset = fontSize
            var labelLines = text.split("\n")
            for (var l = 0; l < labelLines.length; l++)
            {
                context.fillText(labelLines[l], x + 2, y + yOffset)
                yOffset += fontSize
            }
        }

        onPaint:
        {
            if (repaintTimer.running === true)
                return

            var context = gridCanvas.getContext('2d')
            context.fillStyle = "#1A1A1A"
            context.strokeStyle = "black"
            context.lineWidth = 1

            context.rect(0, 0, cellSize * gridSize.width, cellSize * gridSize.height)
            context.clearRect(0, 0, cellSize * gridSize.width, cellSize * gridSize.height)

            var cWidth = cellSize * gridSize.width
            context.font = labelsFontSize + "px \"" + UISettings.robotoFontName + "\""

            context.beginPath()
            /* Paint the grid vertical lines */
            for (var vl = 1; vl < gridSize.width; vl++)
            {
                var xPos = cellSize * vl
                context.moveTo(xPos, 0)
                context.lineTo(xPos, cellSize * gridSize.height)
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
                {
                    var yCell = parseInt(gridLabels[li + 1] / gridSize.width)
                    var xCell = gridLabels[li + 1] - (yCell * gridSize.width)
                    var x = xCell * cellSize
                    var y = yCell * cellSize

                    context.save()
                    context.beginPath()
                    // create a clipping region in case the label is too long
                    context.rect(x, y, cellSize * gridLabels[li + 2], cellSize)
                    context.clip()
                    drawLabel(x, y, gridLabels[li + 3], labelsFontSize, "white")
                    context.restore()
                }
            }
        }
    }
}
