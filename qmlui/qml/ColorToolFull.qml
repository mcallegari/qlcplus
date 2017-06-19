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
import "."

Rectangle
{
    id: rootBox
    width: 330
    height: 370
    color: "#444"
    border.color: "#222"
    border.width: 2

    property int colorsMask: 0
    property color selectedColor

    property int whiteValue: 0
    property int amberValue: 0
    property int uvValue: 0

    property int slHandleSize: UISettings.listItemHeight * 0.8

    signal colorChanged(real r, real g, real b, int w, int a, int uv)
    signal released()

    onSelectedColorChanged: emitCurrentColor()
    onWhiteValueChanged: emitCurrentColor()
    onAmberValueChanged: emitCurrentColor()
    onUvValueChanged: emitCurrentColor()

    function emitCurrentColor()
    {
        colorChanged(selectedColor.r, selectedColor.g, selectedColor.b, whiteValue, amberValue, uvValue)
    }

    function setHTMLColor(r, g, b)
    {
        htmlText.inputText = "#" + ("0" + r.toString(16)).slice(-2) +
                ("0" + g.toString(16)).slice(-2) + ("0" + b.toString(16)).slice(-2)
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

        function getHTMLColor(r, g, b)
        {
            var color = r << 16 | g << 8 | b;
            var colStr = color.toString(16);
            return "#" + "000000".substr(0, 6 - colStr.length) + colStr;
            //return "#" + r.toString(16) + g.toString(16) + b.toString(16);
        }

        function fillWithGradient(r, g, b, xPos)
        {
            context.beginPath();
            var grad = context.createLinearGradient(xPos, 0, xPos, 255);
            grad.addColorStop(0, 'black');
            grad.addColorStop(0.5, getHTMLColor(r,g,b));
            grad.addColorStop(1, 'white');
            context.strokeStyle = grad;
            context.moveTo(xPos, 0);
            context.lineTo(xPos, 255);
            context.closePath();
            context.stroke();
        }

        onPaint:
        {
            context.globalAlpha = 1.0;
            var i = 0;
            var x = 0;
            var r = 0xFF;
            var g = 0;
            var b = 0;
            context.lineWidth = 1;

            var baseColors = [ 0xFF0000, 0xFFFF00, 0x00FF00, 0x00FFFF, 0x0000FF, 0xFF00FF, 0xFF0000 ];

            for (var c = 0; c < 6; c++)
            {
                r = (baseColors[c] >> 16) & 0x00FF;
                g = (baseColors[c] >> 8) & 0x00FF;
                b = baseColors[c] & 0x00FF;
                var nr = (baseColors[c + 1] >> 16) & 0x00FF;
                var ng = (baseColors[c + 1] >> 8) & 0x00FF;
                var nb = baseColors[c + 1] & 0x00FF;
                var rD = (nr - r) / 42;
                var gD = (ng - g) / 42;
                var bD = (nb - b) / 42;

                for (i = x; i < x + 42; i++)
                {
                    fillWithGradient(r, g, b, i);
                    r+=rD;
                    g+=gD;
                    b+=bD;
                }
                x+=42;
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
                rSpin.value = r
                gSpin.value = g
                bSpin.value = b
                setHTMLColor(r, g, b)

                selectedColor = Qt.rgba(r / 256, g / 256, b / 256, 1.0)
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
            onValueChanged:
            {
                selectedColor = Qt.rgba(rSpin.value / 256, gSpin.value / 256, bSpin.value / 256, 1.0)
                setHTMLColor(rSpin.value, gSpin.value, bSpin.value)
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
            onValueChanged:
            {
                selectedColor = Qt.rgba(rSpin.value / 256, gSpin.value / 256, bSpin.value / 256, 1.0)
                setHTMLColor(rSpin.value, gSpin.value, bSpin.value)
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
            onValueChanged:
            {
                selectedColor = Qt.rgba(rSpin.value / 256, gSpin.value / 256, bSpin.value / 256, 1.0)
                setHTMLColor(rSpin.value, gSpin.value, bSpin.value)
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
            visible: colorsMask & App.White
            height: UISettings.listItemHeight
            label: qsTr("White");
        }

        Slider
        {
            id: wSlider
            visible: colorsMask & App.White
            Layout.fillWidth: true
            orientation: Qt.Horizontal
            from: 0
            to: 255
            value: whiteValue
            handle: Rectangle {
                x: wSlider.leftPadding + wSlider.visualPosition * (wSlider.availableWidth - width)
                y: wSlider.topPadding + wSlider.availableHeight / 2 - height / 2
                implicitWidth: slHandleSize
                implicitHeight: slHandleSize
                radius: slHandleSize / 5
            }

            onPositionChanged: whiteValue = valueAt(position)
        }

        CustomSpinBox
        {
            visible: colorsMask & App.White
            width: UISettings.bigItemHeight * 0.7
            height: UISettings.listItemHeight
            from: 0
            to: 255
            value: whiteValue
            onValueChanged: whiteValue = value
        }

        RobotoText
        {
            visible: colorsMask & App.Amber
            height: UISettings.listItemHeight
            label: qsTr("Amber");
        }

        Slider
        {
            id: aSlider
            visible: colorsMask & App.Amber
            Layout.fillWidth: true
            orientation: Qt.Horizontal
            from: 0
            to: 255
            value: amberValue
            handle: Rectangle {
                x: aSlider.leftPadding + aSlider.visualPosition * (aSlider.availableWidth - width)
                y: aSlider.topPadding + aSlider.availableHeight / 2 - height / 2
                implicitWidth: slHandleSize
                implicitHeight: slHandleSize
                radius: slHandleSize / 5
            }

            onPositionChanged: amberValue = valueAt(position)
        }

        CustomSpinBox
        {
            visible: colorsMask & App.Amber
            width: UISettings.bigItemHeight * 0.7
            height: UISettings.listItemHeight
            from: 0
            to: 255
            value: amberValue
            onValueChanged: amberValue = value
        }

        RobotoText
        {
            visible: colorsMask & App.UV
            height: UISettings.listItemHeight
            label: qsTr("UV");
        }

        Slider
        {
            id: uvSlider
            visible: colorsMask & App.UV
            Layout.fillWidth: true
            orientation: Qt.Horizontal
            from: 0
            to: 255
            value: uvValue
            handle: Rectangle {
                x: uvSlider.leftPadding + uvSlider.visualPosition * (uvSlider.availableWidth - width)
                y: uvSlider.topPadding + uvSlider.availableHeight / 2 - height / 2
                implicitWidth: slHandleSize
                implicitHeight: slHandleSize
                radius: slHandleSize / 5
            }

            onPositionChanged: uvValue = valueAt(position)
        }

        CustomSpinBox
        {
            visible: colorsMask & App.UV
            width: UISettings.bigItemHeight * 0.7
            height: UISettings.listItemHeight
            from: 0
            to: 255
            value: uvValue
            onValueChanged: uvValue = value
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
            label: qsTr("Selected color");
        }
        Rectangle
        {
            width: UISettings.mediumItemHeight
            height: UISettings.listItemHeight
            color: selectedColor
        }
    }
}
