/*
  Q Light Controller Plus
  HeaderAndCursor.qml

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

import org.qlcplus.classes 1.0

import "TimeUtils.js" as TimeUtils
import "."

Rectangle
{
    id: tlHeaderCursorLayer
    height: parent.height
    width: parent.width
    color: "transparent"

    property int visibleWidth
    property real visibleX
    property int duration: -1
    property int headerHeight: UISettings.iconSizeMedium
    property int cursorHeight: 0
    property real timeScale: showManager.timeScale
    property real tickSize: showManager.tickSize
    property int currentTime: showManager.currentTime
    property int timeDivision: showManager.timeDivision
    property int bpmNumber: ioManager.bpmNumber
    property int beatsDivision: showManager.beatsDivision
    property bool showTimeMarkers: true

    signal clicked(int mouseX, int mouseY)

    onVisibleWidthChanged:
    {
        console.log("Visible width changed to: " + visibleWidth)
        timeHeader.x = ((visibleX / visibleWidth) * visibleWidth) - visibleWidth
        timeHeader.requestPaint()
    }

    onVisibleXChanged:
    {
        /** To avoid consuming a massive amount of memory, and to workaround a
          * nasty Canvas bug (QTBUG-51530), the adopted rendering strategy is the following:
          * the timeHeader Canvas is 3 times the size of the visible
          * screen area. Basically it is treated as 3 chunks, where the 2nd chunk
          * is the visible one, the 1st chunk is for scrolling left and the 3rd chunk
          * for scrolling right.
          * Here, it is necessary to monitor the Flickable scroll position to properly
          * shift and render the Canvas.
          */

        if (visibleX < timeHeader.x + visibleWidth || visibleX > timeHeader.x + (visibleWidth * 2))
        {
            //console.log("Shift the canvas. X before: " + timeHeader.x);
            timeHeader.x = (parseInt(visibleX / visibleWidth) * visibleWidth) - visibleWidth
            timeHeader.requestPaint()
            //console.log("Shift the canvas. X after: " + timeHeader.x);
        }
    }

    onTimeDivisionChanged: timeHeader.requestPaint()

    onCurrentTimeChanged:
    {
        if (cursorHeight)
        {
            if (timeDivision === Show.Time)
                cursor.x = TimeUtils.timeToSize(currentTime, timeScale, tickSize)
            else
                cursor.x = TimeUtils.timeToBeatPosition(currentTime, tickSize, bpmNumber, beatsDivision)
        }
    }

    onDurationChanged:
    {
        width = parseInt(TimeUtils.timeToSize(duration + 300000, timeScale, tickSize))
        //console.log("New header width: " + width)
    }

    onTimeScaleChanged:
    {
        if (cursorHeight)
            cursor.x = TimeUtils.timeToSize(currentTime, timeScale, tickSize)
        width = parseInt(TimeUtils.timeToSize(duration + 300000, timeScale, tickSize))
        timeHeader.requestPaint()
        //console.log("New header width: " + width)
    }

    Rectangle
    {
        id: cursor
        height: cursorHeight
        width: 1
        color: "transparent"
        z: 1
        visible: cursorHeight ? (x >= visibleX ? true : false) : false

        Rectangle
        {
            height: 10
            width: 10
            x: -5
            y: headerHeight - 10
            color: UISettings.selection
        }

        Rectangle
        {
            width: 1
            height: cursorHeight - headerHeight
            y: headerHeight
            color: UISettings.selection
        }
    }

    Canvas
    {
        id: timeHeader
        x: -visibleWidth
        width: visibleWidth * 3
        height: headerHeight
        antialiasing: true
        contextType: "2d"

        onPaint:
        {
            var fontSize = headerHeight * 0.55
            var subDividers = showManager.beatsDivision
            context.globalAlpha = 1.0
            context.lineWidth = 1

            if (showTimeMarkers)
            {
                context.strokeStyle = "white"
                context.fillStyle = "black"
                context.font = fontSize + "px \"" + UISettings.robotoFontName + "\""
                context.fillRect(0, 0, width, headerHeight)
            }
            else
            {
                context.strokeStyle = UISettings.bgLight
                context.clearRect(0, 0, width, headerHeight)
            }

            var divNum = width / tickSize
            var xPos = parseInt((x + width) / tickSize) * tickSize
            var msTime = TimeUtils.posToMs(xPos, timeScale, tickSize)
            var barNumber = parseInt(xPos / tickSize)
            xPos -= x

            //console.log("xPos: " + xPos + ", msTime: " + msTime)

            context.beginPath()
            context.fillStyle = "white"

            // paint bars and text markers from the end to the beginning
            for (var i = 0; i < divNum; i++)
            {
                // don't even bother to paint if we're outside the timeline
                if (msTime >= 0)
                {
                    if (subDividers > 1)
                    {
                        var subX = xPos - (tickSize / subDividers)
                        for (var s = 0; s < subDividers - 1; s++)
                        {
                            context.moveTo(subX, height / 2)
                            context.lineTo(subX, height)
                            subX -= (tickSize / subDividers)
                        }
                    }

                    context.moveTo(xPos, 0)
                    context.lineTo(xPos, height)

                    if (showTimeMarkers)
                    {
                        if (timeDivision === Show.Time)
                            context.fillText(TimeUtils.msToString(msTime), xPos + 3, height - fontSize)
                        else
                            context.fillText(barNumber, xPos + 3, height - fontSize)
                    }
                }
                xPos -= tickSize
                msTime -= timeScale * 1000
                barNumber--

                //console.log("xPos: " + xPos + ", msTime: " + msTime)
            }
            context.closePath()
            context.stroke()
        }
    }

    MouseArea
    {
        enabled: showTimeMarkers
        anchors.fill: parent
        onClicked: tlHeaderCursorLayer.clicked(mouse.x, mouse.y)
    }
}
