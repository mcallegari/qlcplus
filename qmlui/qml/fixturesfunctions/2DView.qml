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

Rectangle
{
    anchors.fill: parent
    color: "black"
    clip: true

    property string contextName: "2D"
    property alias contextItem: twoDView

    onWidthChanged: twoDView.calculateCellSize()
    onHeightChanged: twoDView.calculateCellSize()

    Component.onDestruction: contextManager.enableContext("2D", false, twoDView)

    function setZoom(amount)
    {
        if (View2D.gridScale + amount < 1.0)
            View2D.gridScale = 1.0
        else
            View2D.gridScale += amount

        twoDView.calculateCellSize()
    }

    function hasSettings()
    {
        return true;
    }

    function showSettings(show)
    {
        twoDSettings.visible = show
        twoDView.calculateCellSize()
    }

    Flickable
    {
        id: twoDView
        objectName: "twoDView"
        anchors.fill: parent
        z: 1
        boundsBehavior: Flickable.StopAtBounds
        //contentWidth: parent.width
        //contentHeight: parent.height

        property size gridSize: View2D.gridSize
        property real gridUnits: View2D.gridUnits

        Component.onCompleted:
        {
            calculateCellSize()
            contextManager.enableContext("2D", true, twoDView)
        }

        onGridSizeChanged: calculateCellSize()
        onGridUnitsChanged: calculateCellSize()

        function calculateCellSize()
        {
            if (width <= 0 || height <= 0)
                return;
            var w = twoDSettings.visible ? (width - twoDSettings.width) : width
            var xDiv = w / gridSize.width
            var yDiv = height / gridSize.height
            twoDContents.x = 0
            twoDContents.y = 0

            if (yDiv < xDiv)
                View2D.cellPixels = yDiv * View2D.gridScale
            else if (xDiv < yDiv)
                View2D.cellPixels = xDiv * View2D.gridScale

            //console.log("Cell size calculated: " + View2D.cellPixels)

            contentWidth = View2D.cellPixels * gridSize.width;
            contentHeight = View2D.cellPixels * gridSize.height;

            if (contentWidth < w)
                twoDContents.x = (w - contentWidth) / 2;
            if (contentHeight < height)
                twoDContents.y = (height - contentHeight) / 2;

            if (View2D.cellPixels > 0)
                twoDContents.requestPaint();
        }

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

        Canvas
        {
            id: twoDContents
            objectName: "twoDContents"
            width: twoDView.contentWidth
            height: twoDView.contentHeight
            x: 0
            y: 0
            z: 0

            antialiasing: true
            contextType: "2d"

            property real cellSize: View2D.cellPixels
            property int gridUnits: twoDView.gridUnits

            function setFlickableStatus(status)
            {
                console.log("Flickable interaction set to: " + status)
                twoDView.interactive = status
            }

            onPaint:
            {
                context.globalAlpha = 1.0
                context.strokeStyle = "#5F5F5F"
                context.fillStyle = "black"
                context.lineWidth = 1

                context.beginPath()
                context.clearRect(0, 0, width, height)
                context.fillRect(0, 0, width, height)
                context.rect(0, 0, width, height)

                for (var vl = 1; vl < twoDView.gridSize.width; vl++)
                {
                    var xPos = cellSize * vl
                    context.moveTo(xPos, 0)
                    context.lineTo(xPos, height)
                }
                for (var hl = 1; hl < twoDView.gridSize.height; hl++)
                {
                    var yPos = cellSize * hl
                    context.moveTo(0, yPos)
                    context.lineTo(width, yPos)
                }
                context.closePath()
                context.stroke()
            }

            MouseArea
            {
                // set the size to cover the whole twoDView
                x: -twoDContents.x
                y: -twoDContents.y
                width: twoDSettings.visible ? twoDView.width - twoDSettings.width : twoDView.width
                height: twoDView.height
                z: 1

                property int initialXPos
                property int initialYPos

                onPressed:
                {
                    console.log("button: " + mouse.button + ", mods: " + mouse.modifiers)

                    if (mouse.button === Qt.LeftButton && mouse.modifiers & Qt.ShiftModifier)
                    {
                        //console.log("Flickable shift-clicked !")
                        // initialize local variables to determine the selection orientation
                        initialXPos = mouse.x
                        initialYPos = mouse.y

                        twoDView.interactive = false
                        selectionRect.x = mouse.x
                        selectionRect.y = mouse.y
                        selectionRect.width = 0
                        selectionRect.height = 0
                        selectionRect.visible = true
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
                    if (selectionRect.visible === true)
                    {
                        var rx = selectionRect.x - twoDContents.x
                        var ry = selectionRect.y - twoDContents.y
                        var rw = selectionRect.width
                        var rh = selectionRect.height
                        switch (selectionRect.rotation)
                        {
                            case 0: contextManager.setRectangleSelection(rx, ry, rw, rh); break;
                            case -180: contextManager.setRectangleSelection(rx - rw, ry - rh, rw, rh); break;
                            case 90: contextManager.setRectangleSelection(rx - rh, ry, rh, rw); break;
                            case -90: contextManager.setRectangleSelection(rx, ry - rw, rh, rw); break;
                        }
                    }

                    selectionRect.visible = false
                    twoDView.interactive = true
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
            DropArea
            {
                anchors.fill: parent
            }
        }
    }

    SettingsView2D
    {
        id: twoDSettings
        visible: false
        x: parent.width - width
        z: 5
    }
}
