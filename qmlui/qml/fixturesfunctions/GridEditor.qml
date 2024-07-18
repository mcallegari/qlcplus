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
import QtQuick.Layouts 1.2
import QtQuick.Controls 2.13

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
    property int minimumCellSize: 0

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
    onGridDataChanged:
    {
        setSelectionData(null)
        updateViewData()
    }
    onGridLabelsChanged: updateViewLabels()
    onWidthChanged: repaintTimer.restart()

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

        if (minimumCellSize)
        {
            horSize = Math.max(horSize, minimumCellSize)
            vertSize = Math.max(vertSize, minimumCellSize)
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

        console.log("Grid View cell size: " + cellSize)
        updateViewData()
        updateViewLabels()
    }

    function updateViewData()
    {
        var isOdd = 0
        var subIndex = 0
        var dataIdx = 0

        for (var i = 0; i < gridItems.count; i++)
        {
            var item = gridItems.itemAt(i)
            if (item === null)
                continue;

            if (dataIdx < gridData.length && i === gridData[dataIdx + 1])
            {
                if (gridData[dataIdx + 2] !== isOdd)
                {
                    subIndex = 0
                    isOdd = gridData[dataIdx + 2]
                }

                var itemType = gridData[dataIdx + 3]

                item.itemID = gridData[dataIdx]
                item.color = isOdd ? oddColor : evenColor
                if (itemType >= 0)
                    item.icon = getItemIcon(item.itemID, subIndex)

                subIndex++
                dataIdx += 4
            }
            else
            {
                item.color = "#7F7F7F"
                item.icon = ""
                item.itemID = -1
            }
        }
    }

    function updateViewLabels()
    {
        var dataIdx = 0

        for (var i = 0; i < gridItems.count; i++)
        {
            var item = gridItems.itemAt(i)
            if (item === null)
                continue;

            if (dataIdx < gridLabels.length && i === gridLabels[dataIdx + 1])
            {
                item.label = gridLabels[dataIdx + 3]
                item.labelWidth = cellSize * gridLabels[dataIdx + 2]
                dataIdx += 4
            }
            else
                item.label = ""
        }
    }

    function setSelectionData(data)
    {
        selectionData = data
        updateViewSelection(0)
    }

    function updateViewSelection(offset)
    {
        var dataIdx = 0

        for (var i = 0; i < gridItems.count; i++)
        {
            var item = gridItems.itemAt(i)
            if (item === null)
                continue;

            if (selectionData !== null &&
                dataIdx < selectionData.length &&
                i === selectionData[dataIdx] + offset)
            {
                item.overColor = validSelection ? "green" : "red"
                dataIdx++
            }
            else
            {
                item.overColor = "transparent"
            }
        }
    }

    Timer
    {
        id: repaintTimer
        repeat: false
        running: false
        interval: 100
        onTriggered: calculateCellSize()
    }

    Text
    {
        id: ttText

        property string tooltipText: ""
        visible: false
        x: gridMouseArea.mouseX
        y: gridMouseArea.mouseY
        ToolTip.visible: ttText.visible
        ToolTip.timeout: 3000
        ToolTip.text: tooltipText
    }

    Timer
    {
        id: ttTimer
        repeat: false
        interval: 1000
        running: false
        onTriggered:
        {
            var xPos = parseInt(gridMouseArea.mouseX / cellSize)
            var yPos = parseInt(gridMouseArea.mouseY / cellSize)
            ttText.tooltipText = getTooltip(xPos, yPos)
            ttText.visible = true
        }
    }

    GridLayout
    {
        width: cellSize * gridSize.width
        height: cellSize * gridSize.height
        columns: gridSize.width
        rows: gridSize.height
        columnSpacing: 1
        rowSpacing: 1

        Repeater
        {
            id: gridItems
            model: showIndices ? showIndices : gridSize.width * gridSize.height

            delegate:
                Rectangle
                {
                    color: "#7F7F7F"
                    implicitHeight: cellSize
                    implicitWidth: cellSize
                    z: (gridSize.width * gridSize.height) - index

                    property int itemID: -1
                    property alias overColor: colorLayer.color
                    property alias icon: itemIcon.source
                    property alias label: itemLabel.label
                    property alias labelWidth: itemLabel.width

                    Rectangle
                    {
                        id: colorLayer
                        anchors.fill: parent
                        color: "transparent"
                    }

                    RobotoText
                    {
                        id: itemLabel
                        x: 1
                        height: cellSize * 0.5
                        fontSize: labelsFontSize
                    }

                    RobotoText
                    {
                        visible: showIndices
                        anchors.bottom: parent.bottom
                        anchors.right: parent.right
                        height: labelsFontSize
                        label: index + 1
                        labelColor: "black"
                        fontSize: labelsFontSize
                    }

                    Image
                    {
                        id: itemIcon
                        x: 2
                        y: parent.height - height
                        width: gridRoot.cellSize * 0.5
                        height: gridRoot.cellSize * 0.5
                        sourceSize: Qt.size(width, height)
                    }
                }
        } // Repeater
    } // GridLayout

    MouseArea
    {
        id: gridMouseArea
        anchors.fill: parent
        hoverEnabled: true

        property int startX: -1
        property int startY: -1
        property int lastX: -1
        property int lastY: -1
        property bool movingSelection: false
        property int selectionOffset: 0

        onPressed:
        {
            startX = parseInt(mouse.x / cellSize)
            startY = parseInt(mouse.y / cellSize)
            var absStart = (startY * gridSize.width) + startX
            var item = gridItems.itemAt(absStart)
            currentItemID = item !== null ? item.itemID : -1
            selectionOffset = 0
            movingSelection = true
            gridRoot.pressed(startX, startY, mouse.modifiers)
        }

        onReleased:
        {
            if (selectionOffset != 0)
                gridRoot.released(lastX, lastY, selectionOffset, mouse.modifiers)
            else
                gridRoot.released(-1, -1, 0, mouse.modifiers)

            movingSelection = false
            validSelection = true
            startX = startY = lastX = lastY = -1
            updateViewSelection(0)
        }

        onPositionChanged:
        {
            ttText.visible = false
            ttTimer.restart()

            if (movingSelection == false)
                return

            lastX = parseInt(mouse.x / cellSize)
            lastY = parseInt(mouse.y / cellSize)
            var absLast = (lastY * gridSize.width) + lastX
            var absStart = (startY * gridSize.width) + startX

            if (absLast - absStart == selectionOffset)
                return

            selectionOffset = absLast - absStart
            //console.log("Mouse entered on " + lastX + ", " + lastY + ", offset: " + selectionOffset)

            // now find the real starting point of the selection
            var realAbs = selectionData[0] + selectionOffset
            var realY = parseInt(realAbs / gridSize.width)
            var realX = realAbs - (realY * gridSize.width)

            gridRoot.positionChanged(realX, realY, selectionOffset, mouse.modifiers)
            updateViewSelection(selectionOffset)
        }

        onExited: ttTimer.stop()
    }

    DropArea
    {
        id: itemDropArea
        anchors.fill: parent

        property int startX: -1
        property int startY: -1
        property int lastX: -1
        property int lastY: -1
        property bool movingSelection: false
        property int selectionOffset: 0

        onEntered:
        {
            startX = parseInt(drag.x / cellSize)
            startY = parseInt(drag.y / cellSize)
            gridRoot.dragEntered(startX, startY, drag)

            selectionOffset = 0
            movingSelection = true
        }

        onPositionChanged:
        {
            if (movingSelection == false)
                return

            lastX = parseInt(drag.x / cellSize)
            lastY = parseInt(drag.y / cellSize)
            var absLast = (lastY * gridSize.width) + lastX
            var absStart = (startY * gridSize.width) + startX

            if (absLast - absStart == selectionOffset)
                return

            selectionOffset = absLast - absStart
            gridRoot.dragPositionChanged(lastX, lastY, selectionOffset, drag)
            updateViewSelection(selectionOffset)
        }

        onExited:
        {
            startX = startY = lastX = lastY = -1
            selectionOffset = 0
            selectionData = null
            updateViewSelection(0)
        }

        onDropped:
        {
            gridRoot.dragDropped(lastX, lastY, drag)
            startX = startY = lastX = lastY = -1
            selectionOffset = 0
            selectionData = null
            updateViewSelection(0)
        }
    }
}
