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

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

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
    property bool isPaletteEditing: false
    property bool showPaletteWAUV: (currentWAUV.r > 0 || currentWAUV.g > 0 || currentWAUV.b > 0)
    property bool showWhite: (colorsMask & App.White) || isPaletteEditing
    property bool showAmber: (colorsMask & App.Amber) || isPaletteEditing
    property bool showUV: (colorsMask & App.UV) || isPaletteEditing
    
    // Mixed color combining RGB + WAUV using additive mixing
    property color mixedColor: {
        var r = currentRGB.r
        var g = currentRGB.g  
        var b = currentRGB.b
        
        // Add White (brightens all channels equally)
        if (showWhite) {
            r = Math.min(1.0, r + currentWAUV.r)
            g = Math.min(1.0, g + currentWAUV.r)
            b = Math.min(1.0, b + currentWAUV.r)
        }
        
        // Add Amber (uses upstream's color 0xFFFF7E00 -> R=1.0, G=0.49, B=0.0)
        if (showAmber) {
            r = Math.min(1.0, r + currentWAUV.g * 1.0)
            g = Math.min(1.0, g + currentWAUV.g * 0.49)
            b = Math.min(1.0, b + currentWAUV.g * 0.0)
        }
        
        // Add UV (uses upstream's violet color 0xFF9400D3 -> R=0.58, G=0.0, B=0.83)
        if (showUV) {
            r = Math.min(1.0, r + currentWAUV.b * 0.58)
            g = Math.min(1.0, g + currentWAUV.b * 0.0)
            b = Math.min(1.0, b + currentWAUV.b * 0.83)
        }
        
        return Qt.rgba(r, g, b, 1.0)
    }

    // Crosshair tracking
    property real clickedX: 0
    property real clickedY: 0
    property bool settingFromRGB: false

    property int slHandleSize: UISettings.listItemHeight * 0.8

    signal toolColorChanged(real r, real g, real b, real w, real a, real uv)
    signal released()

    // Helper function to convert RGB to canvas position
    function rgbToCanvasPos(r, g, b) {
        var max = Math.max(r, g, b)
        var min = Math.min(r, g, b)
        var h = 0
        var l = (max + min) / 2

        if (max === min) {
            h = 0
        } else {
            var d = max - min
            if (max === r) {
                h = (g - b) / d
                if (g < b) h += 6
            } else if (max === g) {
                h = (b - r) / d + 2
            } else {
                h = (r - g) / d + 4
            }
        }

        var xPos = (h / 6) * 255
        var yPos = l * 255
        return { x: xPos, y: yPos }
    }

    function updateCrosshairFromRGB() {
        if (!settingFromRGB) {
            var pos = rgbToCanvasPos(currentRGB.r, currentRGB.g, currentRGB.b)
            clickedX = Math.max(0, Math.min(255, pos.x))
            clickedY = Math.max(0, Math.min(255, pos.y))
        }
    }

    onCurrentRGBChanged:
    {
        htmlText.text = Helpers.getHTMLColor(currentRGB.r * 255, currentRGB.g * 255, currentRGB.b * 255)
        updateCrosshairFromRGB()
    }

    Component.onCompleted: {
        updateCrosshairFromRGB()
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
                var scaledX = mouse.x / colorBox.scale
                var scaledY = mouse.y / colorBox.scale
                var imgData = colorBox.context.getImageData(scaledX, scaledY, 1, 1).data
                var r = imgData[0]
                var g = imgData[1]
                var b = imgData[2]
                /*rSpin.value = r
                gSpin.value = g
                bSpin.value = b*/

                currentRGB = Qt.rgba(r / 255, g / 255, b / 255, 1.0)
                toolColorChanged(currentRGB.r, currentRGB.g, currentRGB.b, currentWAUV.r, currentWAUV.g, currentWAUV.b)
            }

            onPressed: (mouse) => {
                setPickedColor(mouse)
                clickedX = mouse.x / colorBox.scale
                clickedY = mouse.y / colorBox.scale
            }
            onPositionChanged: (mouse) => {
                setPickedColor(mouse)
                clickedX = mouse.x / colorBox.scale
                clickedY = mouse.y / colorBox.scale
            }
            onReleased: rootBox.released()
        }

        // Crosshair markers
        Item
        {
            x: 0
            y: 0
            width: colorBox.width
            height: colorBox.height
            clip: true

            Rectangle
            {
                id: crosshairH
                x: 0
                y: clickedY - 1
                width: colorBox.width
                height: 2
                color: "white"
                opacity: 0.8
            }

            Rectangle
            {
                id: crosshairV
                x: clickedX - 1
                y: 0
                width: 2
                height: colorBox.height
                color: "white"
                opacity: 0.8
            }

            Rectangle
            {
                id: crosshairCenter
                x: clickedX - 3
                y: clickedY - 3
                width: 6
                height: 6
                color: "transparent"
                border.color: "white"
                border.width: 2
                radius: 3
                opacity: 0.8
            }
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
            onValueModified:
            {
                currentRGB = Qt.rgba(value / 255, currentRGB.g, currentRGB.b, 1.0)
                toolColorChanged(currentRGB.r, currentRGB.g, currentRGB.b,
                                 currentWAUV.r, currentWAUV.g, currentWAUV.b)
            }
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
            onValueModified:
            {
                currentRGB = Qt.rgba(currentRGB.r, value / 255, currentRGB.b, 1.0)
                toolColorChanged(currentRGB.r, currentRGB.g, currentRGB.b,
                                 currentWAUV.r, currentWAUV.g, currentWAUV.b)
            }
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
            onValueModified:
            {
                currentRGB = Qt.rgba(currentRGB.r, currentRGB.g, value / 255, 1.0)
                toolColorChanged(currentRGB.r, currentRGB.g, currentRGB.b,
                                 currentWAUV.r, currentWAUV.g, currentWAUV.b)
            }
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
            visible: showWhite
            height: UISettings.listItemHeight
            label: qsTr("White")
        }

        CustomSlider
        {
            id: wSlider
            visible: showWhite
            Layout.fillWidth: true
            from: 0
            to: 255
            stepSize: 1
            wheelEnabled: true
            value: currentWAUV.r * 255
            onMoved: {
                currentWAUV.r = valueAt(position) / 255
                toolColorChanged(currentRGB.r, currentRGB.g, currentRGB.b,
                               currentWAUV.r, currentWAUV.g, currentWAUV.b)
            }
        }

        CustomSpinBox
        {
            id: wSpin
            visible: showWhite
            width: UISettings.bigItemHeight * 0.7
            height: UISettings.listItemHeight
            from: 0
            to: 255
            value: currentWAUV.r * 255
            onValueModified: {
                currentWAUV.r = value / 255
                toolColorChanged(currentRGB.r, currentRGB.g, currentRGB.b,
                               currentWAUV.r, currentWAUV.g, currentWAUV.b)
            }
        }

        RobotoText
        {
            visible: showAmber
            height: UISettings.listItemHeight
            label: qsTr("Amber")
        }

        CustomSlider
        {
            id: aSlider
            visible: showAmber
            Layout.fillWidth: true
            from: 0
            to: 255
            stepSize: 1
            wheelEnabled: true
            value: currentWAUV.g * 255
            onMoved: {
                currentWAUV.g = valueAt(position) / 255
                toolColorChanged(currentRGB.r, currentRGB.g, currentRGB.b,
                               currentWAUV.r, currentWAUV.g, currentWAUV.b)
            }
        }

        CustomSpinBox
        {
            id: aSpin
            visible: showAmber
            width: UISettings.bigItemHeight * 0.7
            height: UISettings.listItemHeight
            from: 0
            to: 255
            value: currentWAUV.g * 255
            onValueModified: {
                currentWAUV.g = value / 255
                toolColorChanged(currentRGB.r, currentRGB.g, currentRGB.b,
                               currentWAUV.r, currentWAUV.g, currentWAUV.b)
            }
        }

        RobotoText
        {
            visible: showUV
            height: UISettings.listItemHeight
            label: qsTr("UV")
        }

        CustomSlider
        {
            id: uvSlider
            visible: showUV
            Layout.fillWidth: true
            from: 0
            to: 255
            stepSize: 1
            wheelEnabled: true
            value: currentWAUV.b * 255
            onMoved: {
                currentWAUV.b = valueAt(position) / 255
                toolColorChanged(currentRGB.r, currentRGB.g, currentRGB.b,
                               currentWAUV.r, currentWAUV.g, currentWAUV.b)
            }
        }

        CustomSpinBox
        {
            id: uvSpin
            visible: showUV
            width: UISettings.bigItemHeight * 0.7
            height: UISettings.listItemHeight
            from: 0
            to: 255
            value: currentWAUV.b * 255
            onValueModified: {
                currentWAUV.b = value / 255
                toolColorChanged(currentRGB.r, currentRGB.g, currentRGB.b,
                               currentWAUV.r, currentWAUV.g, currentWAUV.b)
            }
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
            id: selectedColorBox
            width: UISettings.bigItemHeight
            height: UISettings.listItemHeight
            primary: mixedColor
            secondary: "black"
        }
    }
}
