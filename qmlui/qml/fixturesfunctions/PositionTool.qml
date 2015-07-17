/*
  Q Light Controller Plus
  PositionTool.qml

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
import QtQuick.Controls.Styles 1.2

Rectangle
{
    id: posToolRoot
    width: 200
    height: 300
    color: "#333"
    border.color: "#666"
    border.width: 2

    property bool dmxValues: false
    property int panMaxDegrees: 360
    property int tiltMaxDegrees: 270

    property int panValue: 0 // DMX value
    property int tiltValue: 0 // DMX value

    property int panDegrees: 0
    property int tiltDegrees: 0

    Rectangle
    {
        id: rotateButton
        x: parent.width - 40
        width: 40
        height: 40
        z: 2

        radius: 3
        color: "#333"
        border.color: "#666"
        border.width: 2

        Image
        {
            anchors.fill: parent
            source: "qrc:/rotate-right.svg"
            sourceSize: Qt.size(width, height)
        }
        MouseArea
        {
            anchors.fill: parent
            onPressed: rotateButton.color = "#555"
            onReleased: rotateButton.color = "#333"
            onClicked:
            {
                gCanvas.rotation += 90
                if (gCanvas.rotation == 360)
                    gCanvas.rotation = 0
            }
        }
    }

    Canvas
    {
        id: gCanvas
        width: posToolRoot.width - 20
        height: width
        x: 10
        y: 10
        rotation: 0

        antialiasing: true

        function drawEllipse (ctx, eX, eY, eWidth, eHeight)
        {
            var step = 2*Math.PI/30;
            var r = eWidth / 2;
            var xFactor = 1.0;
            var yFactor = 1.0;
            if (eWidth > eHeight)
                yFactor = eHeight / eWidth;
            if (eHeight > eWidth) {
                xFactor = eWidth / eHeight;
                r = eHeight / 2;
            }

            ctx.beginPath();
            for(var theta = 0; theta < 2*Math.PI; theta+=step)
            {
               var x = eX + xFactor * r * Math.cos(theta) ;
               var y = eY - yFactor * r * Math.sin(theta) ;
               ctx.lineTo(x,y);
            }

            ctx.closePath();     //close the end to the start point
            ctx.stroke();
        }

        function degToRad(degrees)
        {
            return degrees * (Math.PI / 180);
        }

        function drawCursor(ctx, eX, eY, eWidth, eHeight, degrees)
        {
            var r = eWidth / 2;
            var xFactor = 1.0;
            var yFactor = 1.0;
            if (eWidth > eHeight)
                yFactor = eHeight / eWidth;
            if (eHeight > eWidth)
            {
                xFactor = eWidth / eHeight;
                r = eHeight / 2;
            }

            var radPos = degToRad(degrees);
            var x = eX + xFactor * r * Math.cos(radPos) ;
            var y = eY + yFactor * r * Math.sin(radPos) ;

            ctx.beginPath();
            ctx.ellipse(x - 8, y - 8, 16, 16);
            ctx.fill();
            ctx.closePath();     //close the end to the start point
            ctx.stroke();
        }

        function drawBasement(ctx, eWidth, eHeight)
        {
            ctx.fillStyle = "#222";
            ctx.strokeStyle = "#333"
            ctx.beginPath();
            var halfWidth = eWidth / 2;
            ctx.moveTo(halfWidth - 80, height);
            ctx.lineTo(halfWidth - 75, height - 35);
            ctx.lineTo(halfWidth + 75, height - 35);
            ctx.lineTo(halfWidth + 80, height);
            ctx.lineTo(halfWidth - 80, height);

            ctx.fill();
            ctx.closePath();     //close the end to the start point
            ctx.stroke();
        }

        onPaint:
        {
            var ctx = gCanvas.getContext('2d');
            //ctx.save();
            ctx.globalAlpha = 1.0;
            ctx.fillStyle = "#111";
            ctx.lineWidth = 1;

            ctx.fillRect(0, 0, width, height)
            // draw head basement
            drawBasement(ctx, width, height);

            ctx.lineWidth = 5;
            // draw TILT curve
            ctx.strokeStyle = "#2E77FF";
            drawEllipse(ctx, width / 2, height / 2, 40, height - 30)
            // draw PAN curve
            ctx.strokeStyle = "#19438F"
            drawEllipse(ctx, width / 2, height / 2, width - 30, 50)

            ctx.lineWidth = 1;
            ctx.strokeStyle = "white";

            // draw TILT cursor position
            ctx.fillStyle = "red";
            drawCursor(ctx, width / 2, height / 2, 40, height - 30, tiltDegrees + 135)

            // draw PAN cursor position
            ctx.fillStyle = "green";
            drawCursor(ctx, width / 2, height / 2, width - 30, 50, panDegrees + 90)
        }

        MouseArea
        {
            anchors.fill: parent

            property int initialXPos
            property int initialYPos

            onPressed:
            {
                // initialize local variables to determine the selection orientation
                initialXPos = mouse.x
                initialYPos = mouse.y
            }
            onPositionChanged:
            {
                if (Math.abs(mouse.x - initialXPos) > Math.abs(mouse.y - initialYPos))
                {
                    if (mouse.x < initialXPos)
                        panSpinBox.value++
                    else
                        panSpinBox.value--
                }
                else
                {
                    if (mouse.y < initialYPos)
                        tiltSpinBox.value++
                    else
                        tiltSpinBox.value--
                }
            }

        }
    }

    Row
    {
        x: 10
        y: gCanvas.height + 20
        width: parent.width - 20
        height: 40
        spacing: 5

        RobotoText
        {
            label: "Pan"
            width: 40
            height: 40
        }
        CustomSpinBox
        {
            id: panSpinBox
            width: 75
            height: 40
            minimumValue: 0
            maximumValue: dmxValues ? 255 : panMaxDegrees
            value: 0

            onValueChanged:
            {
                if (dmxValues)
                    panDegrees = (panMaxDegrees * value) / 255
                else
                    panDegrees = value
                gCanvas.requestPaint()
            }
        }
        Rectangle
        {
            width: 50
            height: 40
            border.width: 2
            border.color: "white"
            radius: 5
            color: "#1E476E"

            RobotoText
            {
                height: 40
                anchors.horizontalCenter: parent.horizontalCenter
                label: dmxValues ? "DMX" : "°"
                fontSize: 15
                fontBold: true
            }

            MouseArea
            {
                anchors.fill: parent
                onClicked:
                {
                    dmxValues = !dmxValues
                    // perform DMX <-> degrees calculation
                    var newVal;
                    if (dmxValues == false)
                        newVal = (panSpinBox.value / 255) * panMaxDegrees
                    else
                        newVal = (panSpinBox.value / panMaxDegrees) * 255
                    panSpinBox.value = newVal
                }
            }
        }
    }

    Row
    {
        x: 10
        y: gCanvas.height + 65
        width: parent.width - 20
        height: 40
        spacing: 5

        RobotoText
        {
            label: "Tilt"
            width: 40
            height: 40
        }
        CustomSpinBox
        {
            id: tiltSpinBox
            width: 75
            height: 40
            minimumValue: 0
            maximumValue: dmxValues ? 255 : tiltMaxDegrees
            value: 0

            onValueChanged:
            {
                if (dmxValues)
                    tiltDegrees = (tiltMaxDegrees * value) / 255
                else
                    tiltDegrees = value
                gCanvas.requestPaint()
            }
        }
        Rectangle
        {
            width: 50
            height: 40
            border.width: 2
            border.color: "white"
            radius: 5
            color: "#1E476E"

            RobotoText
            {
                height: 40
                anchors.horizontalCenter: parent.horizontalCenter
                label: dmxValues ? "DMX" : "°"
                fontSize: 15
                fontBold: true
            }

            MouseArea
            {
                anchors.fill: parent
                onClicked:
                {
                    dmxValues = !dmxValues
                    // perform DMX <-> degrees calculation
                    var newVal;
                    if (dmxValues == false)
                        newVal = (tiltSpinBox.value / 255) * tiltMaxDegrees
                    else
                        newVal = (tiltSpinBox.value / tiltMaxDegrees) * 255
                    tiltSpinBox.value = newVal
                }
            }
        }
    }
}
