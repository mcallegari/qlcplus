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
import QtQuick.Controls 2.1

import org.qlcplus.classes 1.0
import "."

Rectangle
{
    anchors.fill: parent
    color: "black"
    clip: true

    property string contextName: "2D"
    property alias contextItem: twoDView

    onWidthChanged: twoDView.calculateCellSize()
    onHeightChanged: twoDView.calculateCellSize()

    Component.onDestruction:
    {
        monitorPOVPopup.close()
        if (contextManager) contextManager.enableContext("2D", false, twoDView)
    }

    function setZoom(amount)
    {
        var currentScale = View2D.gridScale
        if (amount < 0)
        {
            if (currentScale > 0.1)
            {
                if (currentScale <= 1)
                    View2D.gridScale -= 0.1
                else
                    View2D.gridScale += amount
            }
        }
        else
        {
            if (currentScale < 1)
                View2D.gridScale += 0.1
            else
                View2D.gridScale += amount
        }

        twoDView.calculateCellSize()
    }

    function hasSettings()
    {
        return true
    }

    function showSettings(show)
    {
        twoDSettings.visible = show
        twoDView.calculateCellSize()
    }

    /**
      * The stacking order of this view is very important and delicate
      * From bottom to top:
      * Flickable (z = 1): main scrollable area
      *   Image (z = 0): Main background image
      *   Canvas (z = 0): The actual grid graphics view
      *     DropArea (z = 0): allow to drop items from fixture browser
      *     Rectangle (z = 1): multiple drag layer as big as the Canvas layer
      *       MouseArea (z = 0): handles drag & drop of multiple fixture items
      *     MouseArea (z = 2): handles selection rectangle and mouse wheel for zooming
      *     Fixture2DItem (z = 2): the Fixture 2D items
      *       MouseArea (z = 0): handles only the press event for selecting the Fixture item,
      *                          but doesn't accept it so it can be forwarded for dragging
      * Selection rectangle (z = 99): visible only when a drag is started from an empty space
      * Popup (z = 1): point of view selection popup
      * SettingsView2D (z = 5): right side settings panel
     */

    Flickable
    {
        id: twoDView
        objectName: "twoDView"
        anchors.fill: parent
        z: 1
        interactive: false
        boundsBehavior: Flickable.StopAtBounds
        //contentWidth: parent.width
        //contentHeight: parent.height

        property size gridSize: View2D.gridSize
        property real gridUnits: View2D.gridUnits

        onGridSizeChanged: calculateCellSize()
        onGridUnitsChanged: calculateCellSize()

        function calculateCellSize()
        {
            if (width <= 0 || height <= 0)
                return;

            var w = twoDSettings.visible ? (width - twoDSettings.width) : width
            var xDiv = w / gridSize.width
            var yDiv = height / gridSize.height

            if (yDiv < xDiv)
                View2D.cellPixels = yDiv * View2D.gridScale
            else if (xDiv < yDiv)
                View2D.cellPixels = xDiv * View2D.gridScale

            //console.log("Cell size calculated: " + View2D.cellPixels)

            var gridWidth = View2D.cellPixels * gridSize.width
            var gridHeight = View2D.cellPixels * gridSize.height

            twoDContents.xOffset = (gridWidth < w) ? (w - gridWidth) / 2 : 0
            twoDContents.yOffset = (gridHeight < height) ? (height - gridHeight) / 2 : 0

            View2D.gridPosition = Qt.point(twoDContents.xOffset, twoDContents.yOffset)

            contentWidth = gridWidth + (twoDContents.xOffset * 2)
            contentHeight = gridHeight + (twoDContents.yOffset * 2)

            //console.log("Grid offset x: " + twoDContents.xOffset + ", y: " + twoDContents.yOffset)
            if (View2D.cellPixels > 0)
                twoDContents.requestPaint()
        }

        Image
        {
            x: twoDContents.xOffset
            y: twoDContents.yOffset
            width: twoDView.contentWidth - (x * 2)
            height: twoDView.contentHeight - (y * 2)
            source: View2D.backgroundImage ? "file://" + View2D.backgroundImage : ""
            sourceSize: Qt.size(width, height)
            fillMode: Image.PreserveAspectFit
        }

        Canvas
        {
            id: twoDContents
            objectName: "twoDContents"
            width: twoDView.contentWidth
            height: twoDView.contentHeight
            z: 0

            antialiasing: true
            contextType: "2d"

            property real cellSize: View2D.cellPixels
            property int gridUnits: twoDView.gridUnits
            property bool justSelected: false
            property real xOffset: 0
            property real yOffset: 0

            function showPovPopup()
            {
                monitorPOVPopup.open()
            }

            Component.onCompleted:
            {
                twoDView.calculateCellSize()
                contextManager.enableContext("2D", true, twoDView)
            }

            onPaint:
            {
                var context = getContext("2d")
                context.globalAlpha = 1.0
                context.strokeStyle = "#5F5F5F"
                context.fillStyle = "transparent"
                context.lineWidth = 1

                context.beginPath()
                context.clearRect(0, 0, width, height)
                context.fillRect(0, 0, width, height)
                context.rect(xOffset, yOffset, width - (xOffset * 2), height - (yOffset * 2))

                // draw the view grid
                for (var vl = 1; vl < twoDView.gridSize.width; vl++)
                {
                    var xPos = (cellSize * vl) + xOffset
                    context.moveTo(xPos, yOffset)
                    context.lineTo(xPos, height - yOffset)
                }
                for (var hl = 1; hl < twoDView.gridSize.height; hl++)
                {
                    var yPos = (cellSize * hl) + yOffset
                    context.moveTo(xOffset, yPos)
                    context.lineTo(width - xOffset, yPos)
                }

                context.closePath()
                context.stroke()
            }

            MouseArea
            {
                width: twoDView.contentWidth
                height: twoDView.contentHeight
                z: 2

                property int initialXPos
                property int initialYPos

                onPressed:
                {
                    console.log("button: " + mouse.button + ", mods: " + mouse.modifiers)
                    var itemID = View2D.itemIDAtPos(Qt.point(mouse.x, mouse.y))

                    // pressing on nothing starts to draw the selection rectangle
                    if (itemID === -1)
                    {
                        //console.log("Starting selection rectangle!")
                        // initialize local variables to determine the selection orientation
                        initialXPos = mouse.x
                        initialYPos = mouse.y

                        selectionRect.x = mouse.x
                        selectionRect.y = mouse.y
                        selectionRect.width = 0
                        selectionRect.height = 0
                        selectionRect.visible = true
                    }
                    else
                    {
                        if (contextManager.isFixtureSelected(itemID) === false)
                        {
                            twoDContents.justSelected = true

                            // select the Fixture in case a drag is starting on a deselected one
                            contextManager.setItemSelection(itemID, true, mouse.modifiers)
                        }

                        // forward the event to the drag area
                        mouse.accepted = false
                    }
                }

                onPositionChanged:
                {
                    if (selectionRect.visible == true)
                    {
                        if (mouse.x !== initialXPos || mouse.y !== initialYPos)
                        {
                            //console.log("startX: " + initialXPos + ", startY: " + initialYPos)
                            //console.log("mouseX: " + mouse.x + ", mouseY: " + mouse.y)
                            if (mouse.x >= initialXPos)
                            {
                                if (mouse.y >= initialYPos)
                                   selectionRect.rotation = 0
                                else
                                   selectionRect.rotation = -90
                            }
                            else
                            {
                                if (mouse.y >= initialYPos)
                                    selectionRect.rotation = 90
                                else
                                    selectionRect.rotation = -180
                            }
                            //console.log("Selection rotation: " + selectionRect.rotation)
                        }

                        if (selectionRect.rotation == 0 || selectionRect.rotation == -180)
                        {
                            selectionRect.width = Math.abs(mouse.x - selectionRect.x)
                            selectionRect.height = Math.abs(mouse.y - selectionRect.y)
                        }
                        else
                        {
                            selectionRect.width = Math.abs(mouse.y - selectionRect.y)
                            selectionRect.height = Math.abs(mouse.x - selectionRect.x)
                        }
                    }
                }

                onReleased:
                {
                    if (selectionRect.visible === true && selectionRect.width && selectionRect.height)
                    {
                        var rx = selectionRect.x
                        var ry = selectionRect.y
                        var rw = selectionRect.width
                        var rh = selectionRect.height
                        switch (selectionRect.rotation)
                        {
                            case 0: contextManager.setRectangleSelection(rx, ry, rw, rh, mouse.modifiers); break;
                            case -180: contextManager.setRectangleSelection(rx - rw, ry - rh, rw, rh, mouse.modifiers); break;
                            case 90: contextManager.setRectangleSelection(rx - rh, ry, rh, rw, mouse.modifiers); break;
                            case -90: contextManager.setRectangleSelection(rx, ry - rw, rh, rw, mouse.modifiers); break;
                        }
                        selectionRect.visible = false
                    }
                    else
                    {
                        contextManager.resetFixtureSelection()
                    }
                }

                onWheel:
                {
                    //console.log("Wheel delta: " + wheel.angleDelta.y)
                    if (wheel.angleDelta.y > 0)
                        setZoom(0.5)
                    else
                    {
                        if (View2D.gridScale > 1.0)
                            setZoom(-0.5)
                    }
                }
            }

            Rectangle
            {
                id: contentsDragArea
                objectName: "contentsDragArea"
                width: twoDView.contentWidth
                height: twoDView.contentHeight
                color: "transparent"
                /*
                // enable for debug
                color: "red"
                opacity: 0.3
                */
                z: 1

                Drag.active: dragMouseArea.drag.active

                MouseArea
                {
                    id: dragMouseArea
                    anchors.fill: parent
                    drag.threshold: 10
                    drag.target: parent

                    onReleased:
                    {
                        if (drag.active)
                        {
                            var units = View2D.gridUnits === MonitorProperties.Meters ? 1000.0 : 304.8
                            var xDelta = contentsDragArea.x + twoDContents.x
                            var yDelta = contentsDragArea.y + twoDContents.y

                            // transform pixels in millimeters
                            xDelta = (xDelta * units) / View2D.cellPixels;
                            yDelta = (yDelta * units) / View2D.cellPixels;

                            contextManager.setFixturesOffset(xDelta, yDelta)

                            contentsDragArea.x = 0
                            contentsDragArea.y = 0
                        }
                        else
                        {
                            if (twoDContents.justSelected == false)
                            {
                                // handle Fixture selection/deselection here
                                var itemID = View2D.itemIDAtPos(Qt.point(mouse.x, mouse.y))

                                console.log("Fixture ID on release " + itemID)
                                contextManager.setItemSelection(itemID, false, mouse.modifiers)
                            }
                            twoDContents.justSelected = false
                        }
                    }
                }
            }

            DropArea
            {
                anchors.fill: parent
            }
        } // Canvas

        ScrollBar.vertical: CustomScrollBar { }
        ScrollBar.horizontal : CustomScrollBar { orientation: Qt.Horizontal }
    } // Flickable

    Rectangle
    {
        id: selectionRect
        visible: false
        x: 0
        y: 0
        z: 99
        width: 0
        height: 0
        rotation: 0
        color: "#5F227CEB"
        border.width: 1
        border.color: "#103A6E"
        transformOrigin: Item.TopLeft
    }
/*
    CustomScrollBar
    {
        anchors.right: parent.right
        z: 2
        flickable: twoDView
        doubleBars: true
    }
    CustomScrollBar
    {
        anchors.bottom: parent.bottom
        z: 2
        flickable: twoDView
        orientation: Qt.Horizontal
    }
*/
    PopupMonitor
    {
        id: monitorPOVPopup

        onAccepted: View2D.pointOfView = selectedPov
    }

    SettingsView2D
    {
        id: twoDSettings
        visible: false
        x: parent.width - width
        z: 5
    }
}
