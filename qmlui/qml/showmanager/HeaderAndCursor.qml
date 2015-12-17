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

import "TimeUtils.js" as TimeUtils
import "."

Rectangle
{
    id: tlHeaderCursorLayer
    height: parent.height
    width: parent.width
    color: "transparent"

    property int duration: -1
    property int headerHeight: 40
    property int cursorHeight: 0
    property real timeScale: 1.0
    property int currentTime: showManager.currentTime

    onCurrentTimeChanged:
    {
        cursor.x = TimeUtils.timeToSize(currentTime, timeScale)
    }

    onDurationChanged:
    {
        width = TimeUtils.timeToSize(duration + 300000, timeScale)
        console.log("New header width: " + width)
    }

    Rectangle
    {
        id: cursor
        height: cursorHeight
        width: 1
        color: "transparent"
        z: 2

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
        width: parent.width
        height: headerHeight
        //antialiasing: true

        // tick size is the main time divider
        // on a timeScale equal to 1.0 it is 100 pixels
        property real tickSize: 100

        function calculateTickSize()
        {

        }

        onPaint:
        {
            var ctx = timeHeader.getContext('2d')
            ctx.globalAlpha = 1.0
            ctx.lineWidth = 1

            ctx.clearRect(0, 0, width, height)

            ctx.fillStyle = "black"
            ctx.strokeStyle = "white"
            //ctx.font = '12px RobotoCondensed'
            ctx.fillRect(0, 0, width, headerHeight)

            var divNum = width / tickSize
            var xPos = 0
            var msTime = 0

            for (var i = 0; i < divNum; i++)
            {
                ctx.moveTo(xPos, 0)
                ctx.lineTo(xPos, height)

                ctx.strokeText(TimeUtils.msToString(msTime), xPos + 3, height - 20)

                xPos += tickSize
                msTime += timeScale * 1000
            }
            ctx.closePath()
            ctx.stroke()
        }
    }
}
