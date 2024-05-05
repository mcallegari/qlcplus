/*
  Q Light Controller Plus
  ColorToolFull.qml

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
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.1

import org.qlcplus.classes 1.0
import "GenericHelpers.js" as Helpers
import "."

Rectangle
{
    id: rootBox
    width: 330
    height: 370
    color: UISettings.bgMedium
    border.color: "#222"
    border.width: 2

    property int colorsMask: 0
    property color currentRGB
    property color currentWAUV

    property int slHandleSize: UISettings.listItemHeight * 0.8

    signal colorChanged(real r, real g, real b, real w, real a, real uv)
    signal released()

    onCurrentRGBChanged:
    {
        htmlText.text = Helpers.getHTMLColor(currentRGB.r * 255, currentRGB.g * 255, currentRGB.b * 255)
    }

    Canvas
    {
        id: colorBox
        x: 5
        y: 5
        width: 256
        height: 256
        transformOrigin: Item.TopLeft
        // try to scale always to a little bit more
        // than half of the tool width
        scale: (rootBox.width / 1.75) / 256
        contextType: "2d"

        function fillWithGradient(r, g, b, xPos)
        {
            context.beginPath()
            var grad = context.createLinearGradient(xPos, 0, xPos, 255)
            grad.addColorStop(0, 'black')
            grad.addColorStop(0.5, Helpers.getHTMLColor(r,g,b))
            grad.addColorStop(1, 'white')
            context.strokeStyle = grad
            context.moveTo(xPos, 0)
            context.lineTo(xPos, 255)
            context.closePath()
            context.stroke()
        }

        onPaint:
        {
            context.globalAlpha = 1.0
            var i = 0
            var x = 0
            var r = 0xFF
            var g = 0
            var b = 0
            context.lineWidth = 1

            var baseColors = [ 0xFF0000, 0xFFFF00, 0x00FF00, 0x00FFFF, 0x0000FF, 0xFF00FF, 0xFF0000 ]

            for (var c = 0; c < 6; c++)
            {
                r = (baseColors[c] >> 16) & 0x00FF
                g = (baseColors[c] >> 8) & 0x00FF
                b = baseColors[c] & 0x00FF
                var nr = (baseColors[c + 1] >> 16) & 0x00FF
                var ng = (baseColors[c + 1] >> 8) & 0x00FF
                var nb = baseColors[c + 1] & 0x00FF
                var rD = (nr - r) / 42
                var gD = (ng - g) / 42
                var bD = (nb - b) / 42

                for (i = x; i < x + 42; i++)
                {
                    fillWithGradient(r, g, b, i)
                    r += rD
                    g += gD
                    b += bD
                }
                x += 42
            }
        }

        MouseArea
        {
            anchors.fill: parent

            function setPickedColor(mouse)
            {
                var imgData = colorBox.context.getImageData(mouse.x, mouse.y, 1, 1).data
                var r = imgData[0]
                var g = imgData[1]
                var b = imgData[2]
                /*rSpin.value = r
                gSpin.value = g
                bSpin.value = b*/

                currentRGB = Qt.rgba(r / 255, g / 255, b / 255, 1.0)
                colorChanged(currentRGB.r, currentRGB.g, currentRGB.b, currentWAUV.r, currentWAUV.g, currentWAUV.b)
            }

            onPressed: setPickedColor(mouse)
            onPositionChanged: setPickedColor(mouse)
            onReleased: rootBox.released()
        }
    }

    Grid
    {
        id: tColumn
        x: colorBox.x + (colorBox.width * colorBox.scale) + 5
        y: 5
        columns: 2
        columnSpacing: 5

        RobotoText
        {
            height: UISettings.listItemHeight
            label: qsTr("Red")
        }

        CustomSpinBox
        {
            id: rSpin
            width: UISettings.bigItemHeight * 0.7
            height: UISettings.listItemHeight
            from: 0
            to: 255
            value: currentRGB.r * 255
            onValueModified: colorChanged(value / 255, currentRGB.g, currentRGB.b,
                                          currentWAUV.r, currentWAUV.g, currentWAUV.b)
        }

        RobotoText
        {
            height: UISettings.listItemHeight
            label: qsTr("Green")
        }

        CustomSpinBox
        {
            id: gSpin
            width: UISettings.bigItemHeight * 0.7
            height: UISettings.listItemHeight
            from: 0
            to: 255
            value: currentRGB.g * 255
            onValueModified: colorChanged(currentRGB.r, value / 255, currentRGB.b,
                                          currentWAUV.r, currentWAUV.g, currentWAUV.b)
        }

        RobotoText
        {
            height: UISettings.listItemHeight
            label: qsTr("Blue")
        }

        CustomSpinBox
        {
            id: bSpin
            width: UISettings.bigItemHeight * 0.7
            height: UISettings.listItemHeight
            from: 0
            to: 255
            value: currentRGB.b * 255
            onValueModified: colorChanged(currentRGB.r, currentRGB.g, value / 255,
                                          currentWAUV.r, currentWAUV.g, currentWAUV.b)
        }

        RobotoText
        {
            height: UISettings.listItemHeight
            label: "HTML"
        }

        CustomTextEdit
        {
            id: htmlText
            width: UISettings.bigItemHeight * 0.7
            height: UISettings.listItemHeight
        }
    }

    GridLayout
    {
        x: 5
        width: parent.width - 10
        y: colorBox.y + (colorBox.height * colorBox.scale)
        columns: 3
        columnSpacing: 5

        RobotoText
        {
            visible: colorsMask & App.White
            height: UISettings.listItemHeight
            label: qsTr("White")
        }

        CustomSlider
        {
            id: wSlider
            visible: colorsMask & App.White
            Layout.fillWidth: true
            from: 0
            to: 255
            value: currentWAUV.r * 255
            onMoved: colorChanged(currentRGB.r, currentRGB.g, currentRGB.b,
                                  valueAt(position) / 255, currentWAUV.g, currentWAUV.b)
        }

        CustomSpinBox
        {
            id: wSpin
            visible: colorsMask & App.White
            width: UISettings.bigItemHeight * 0.7
            height: UISettings.listItemHeight
            from: 0
            to: 255
            value: currentWAUV.r * 255
            onValueModified: colorChanged(currentRGB.r, currentRGB.g, currentRGB.b,
                                          value / 255, currentWAUV.g, currentWAUV.b)
        }

        RobotoText
        {
            visible: colorsMask & App.Amber
            height: UISettings.listItemHeight
            label: qsTr("Amber")
        }

        CustomSlider
        {
            id: aSlider
            visible: colorsMask & App.Amber
            Layout.fillWidth: true
            from: 0
            to: 255
            value: currentWAUV.g * 255
            onMoved: colorChanged(currentRGB.r, currentRGB.g, currentRGB.b,
                                  currentWAUV.r, valueAt(position) / 255, currentWAUV.b)
        }

        CustomSpinBox
        {
            id: aSpin
            visible: colorsMask & App.Amber
            width: UISettings.bigItemHeight * 0.7
            height: UISettings.listItemHeight
            from: 0
            to: 255
            value: currentWAUV.g * 255
            onValueModified: colorChanged(currentRGB.r, currentRGB.g, currentRGB.b,
                                          currentWAUV.r, value / 255, currentWAUV.b)
        }

        RobotoText
        {
            visible: colorsMask & App.UV
            height: UISettings.listItemHeight
            label: qsTr("UV")
        }

        CustomSlider
        {
            id: uvSlider
            visible: colorsMask & App.UV
            Layout.fillWidth: true
            from: 0
            to: 255
            value: currentWAUV.b * 255
            onMoved: colorChanged(currentRGB.r, currentRGB.g, currentRGB.b,
                                  currentWAUV.r, currentWAUV.g, valueAt(position) / 255)
        }

        CustomSpinBox
        {
            id: uvSpin
            visible: colorsMask & App.UV
            width: UISettings.bigItemHeight * 0.7
            height: UISettings.listItemHeight
            from: 0
            to: 255
            value: currentWAUV.b * 255
            onValueModified: colorChanged(currentRGB.r, currentRGB.g, currentRGB.b,
                                          currentWAUV.r, currentWAUV.g, value / 255)
        }
    }

    Row
    {
        x: 5
        y: rootBox.height - UISettings.listItemHeight - 10
        spacing: 20
        RobotoText
        {
            height: UISettings.listItemHeight
            label: qsTr("Selected color")
        }

        MultiColorBox
        {
            width: UISettings.mediumItemHeight
            height: UISettings.listItemHeight
            primary: currentRGB
            secondary: currentWAUV
        }
    }
}
