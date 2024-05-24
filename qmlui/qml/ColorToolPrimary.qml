/*
  Q Light Controller Plus
  ColorToolPrimary.qml

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
import "GenericHelpers.js" as Helpers
import "."

Rectangle
{
    id: boxRoot
    width: UISettings.bigItemHeight * 3
    height: UISettings.bigItemHeight
    color: UISettings.bgMedium
    border.color: "#222"
    border.width: 2

    property int targetColor: QLCChannel.NoColour
    property int currentValue: 0 // as DMX value
    property bool closeOnSelect: false
    property bool showPalette: false

    signal valueChanged(int value)
    signal close()

    Canvas
    {
        id: colorBox
        x: 5
        y: 5
        width: parent.width - 10
        height: parent.height - 10
        contextType: "2d"

        onPaint:
        {
            var xPos = 0.0
            var r = (targetColor >> 16) & 0x00FF
            var g = (targetColor >> 8) & 0x00FF
            var b = targetColor & 0x00FF
            var rD = r === 0 ? 0 : r / 255.0
            var gD = g === 0 ? 0 : g / 255.0
            var bD = b === 0 ? 0 : b / 255.0
            var gW = (width * 0.8) / 255.0
            var rectApprox = Math.ceil(gW)
            r = 0.0
            g = 0.0
            b = 0.0

            context.globalAlpha = 1.0
            context.lineWidth = 1
            context.fillStyle = "black"
            context.fillRect(0, 0, width, height)

            // leftmost 10% side is black
            context.strokeStyle = "black"
            context.fillRect(xPos, 0, width * 0.1, height)

            // central 80% part is the gradient
            for (xPos = width * 0.1; xPos < width * 0.9; xPos += gW)
            {
                context.fillStyle = Helpers.getHTMLColor(r, g, b)
                context.fillRect(xPos, 0, rectApprox, height)

                r += rD
                g += gD
                b += bD
            }

            // rightmost 10% side is target color
            context.strokeStyle = Helpers.getHTMLColor(r, g, b)
            context.fillRect(width * 0.9, 0, width * 0.1, height)
        }

        MouseArea
        {
            anchors.fill: parent

            function calculateValue(mouse)
            {
                var val = 0

                if (mouse.x < width * 0.1)
                {
                    val = 0
                }
                else if (mouse.x > width * 0.9)
                {
                    val = 255
                }
                else
                {
                    //mouse.x - (width * 0.1) : x = (width * 0.8) : 255
                    val = ((mouse.x - (width * 0.1)) * 255.0) / (width * 0.8)
                }

                boxRoot.currentValue = val
                boxRoot.valueChanged(val)
            }

            onPressed: calculateValue()
            onPositionChanged:
            {
                if (!pressed)
                    return

                calculateValue(mouse)
            }
            onReleased:
            {
                if (closeOnSelect)
                    boxRoot.close()
            }
        }
    }
}

