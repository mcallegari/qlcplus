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
import QtQuick.Controls 1.2

Rectangle
{
    id: rootBox
    width: parent.width
    height: parent.height
    color: "#444"
    border.color: "#222"
    border.width: 2

    property color selectedColor
    property bool hasWhiteChannel: false
    property bool hasAmberChannel: false
    property bool hasUVChannel: false

    signal colorChanged(real r, real g, real b, real white, real amber, real uv)
    signal released()

    onSelectedColorChanged:
    {
        colorChanged(selectedColor.r, selectedColor.g, selectedColor.b, 0, 0, 0)
    }

    Canvas
    {
        id: colorBox
        x: 5
        y: 5
        width: 256
        height: 256

        function getHTMLColor(r, g, b)
        {
            var color = r << 16 | g << 8 | b;
            var colStr = color.toString(16);
            return "#" + "000000".substr(0, 6 - colStr.length) + colStr;
            //return "#" + r.toString(16) + g.toString(16) + b.toString(16);
        }

        function fillWithGradient(ctx, r, g, b, xPos)
        {
            ctx.beginPath();
            var grad = ctx.createLinearGradient(xPos, 0, xPos, 255);
            grad.addColorStop(0, 'black');
            grad.addColorStop(0.5, getHTMLColor(r,g,b));
            grad.addColorStop(1, 'white');
            ctx.strokeStyle = grad;
            ctx.moveTo(xPos, 0);
            ctx.lineTo(xPos, 255);
            ctx.closePath();
            ctx.stroke();
        }

        onPaint:
        {
            var ctx = colorBox.getContext('2d');
            //ctx.save();
            ctx.globalAlpha = 1.0;
            var i = 0;
            var x = 0;
            var r = 0xFF;
            var g = 0;
            var b = 0;
            ctx.lineWidth = 1;

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
                    fillWithGradient(ctx, r, g, b, i);
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
                var ctx = colorBox.getContext('2d');
                var imgData = ctx.getImageData(mouse.x, mouse.y, 1, 1).data;
                var r = imgData[0];
                var g = imgData[1];
                var b = imgData[2];
                rSpin.value = r;
                gSpin.value = g;
                bSpin.value = b;
                htmlText.inputText = "#" + ("0" + r.toString(16)).slice(-2) +
                        ("0" + g.toString(16)).slice(-2) + ("0" + b.toString(16)).slice(-2);

                selectedColor = Qt.rgba(r / 256, g / 256, b / 256, 1.0);
            }

            onPressed: setPickedColor(mouse)
            onPositionChanged: setPickedColor(mouse)
            onReleased: rootBox.released()
        }
    }

    Column
    {
        id: tColumn
        x: colorBox.width + 10
        y: 5
        height: 256

        RobotoText { height: 40; fontSize: 12; label: qsTr("Red"); }
        RobotoText { height: 40; fontSize: 12; label: qsTr("Green"); }
        RobotoText { height: 40; fontSize: 12; label: qsTr("Blue"); }
        RobotoText { height: 40; fontSize: 12; label: "HTML"; }
    }

    Rectangle
    {
        x: rootBox.width - 80
        y: 5
        height: 256
        width: 75
        color: "transparent"

        CustomSpinBox
        {
            id: rSpin
            width: 75
            height: 38
            minimumValue: 0
            maximumValue: 255
            decimals: 0
        }
        CustomSpinBox
        {
            id: gSpin
            y: 40
            width: 75
            height: 38
            minimumValue: 0
            maximumValue: 255
            decimals: 0;
        }
        CustomSpinBox
        {
            id: bSpin
            y: 80
            width: 75
            height: 38
            minimumValue: 0
            maximumValue: 255
            decimals: 0;
        }
        CustomTextEdit
        {
            id: htmlText
            y: 120
            width: 75
            height: 38
        }
    }


    Row
    {
        x: 5
        y: 350
        spacing: 20
        RobotoText
        {
            label: qsTr("Selected color");
        }
        Rectangle
        {
            width: 70
            height: 40
            color: selectedColor
        }
    }
}
