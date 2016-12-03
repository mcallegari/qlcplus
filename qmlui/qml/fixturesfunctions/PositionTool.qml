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
import QtQuick.Layouts 1.0
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.2

import "CanvasDrawFunctions.js" as DrawFuncs
import "."

Rectangle
{
    id: posToolRoot
    width: UISettings.bigItemHeight * 2
    height: UISettings.bigItemHeight * 3
    color: UISettings.bgMedium
    border.color: "#666"
    border.width: 2

    property int panMaxDegrees: 360
    property int tiltMaxDegrees: 270

    property int panDegrees: 0
    property int tiltDegrees: 0

    onPanDegreesChanged: fixtureManager.setPanValue(panDegrees)
    onTiltDegreesChanged: fixtureManager.setTiltValue(tiltDegrees)

    Rectangle
    {
        id: posToolBar
        width: parent.width
        height: UISettings.listItemHeight
        z: 10
        gradient:
            Gradient
            {
                GradientStop { position: 0; color: UISettings.toolbarStartSub }
                GradientStop { position: 1; color: UISettings.toolbarEnd }
            }

        RobotoText
        {
            id: titleBox
            y: 7
            height: parent.height
            anchors.horizontalCenter: parent.horizontalCenter
            label: qsTr("Position")
            fontSize: UISettings.textSizeDefault
            fontBold: true
        }
        // allow the tool to be dragged around
        // by holding it on the title bar
        MouseArea
        {
            anchors.fill: parent
            drag.target: posToolRoot
        }
    }

    Rectangle
    {
        id: rotateButton
        x: parent.width - width
        y: posToolBar.height
        width: UISettings.iconSizeDefault
        height: width
        z: 2

        radius: 3
        color: rotMouseArea.pressed ? UISettings.bgLight : UISettings.bgMedium
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
            id: rotMouseArea
            anchors.fill: parent

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
        y: posToolBar.height + 5
        rotation: 0

        antialiasing: true

        onPaint:
        {
            var ctx = gCanvas.getContext('2d');
            //ctx.save();
            ctx.globalAlpha = 1.0;
            ctx.fillStyle = "#111";
            ctx.lineWidth = 1;

            ctx.fillRect(0, 0, width, height)
            // draw head basement
            DrawFuncs.drawBasement(ctx, width, height);

            ctx.lineWidth = 5;
            // draw TILT curve
            ctx.strokeStyle = "#2E77FF";
            DrawFuncs.drawEllipse(ctx, width / 2, height / 2, UISettings.iconSizeDefault, height - 30)
            // draw PAN curve
            ctx.strokeStyle = "#19438F"
            DrawFuncs.drawEllipse(ctx, width / 2, height / 2, width - 30, UISettings.iconSizeDefault)

            ctx.lineWidth = 1;
            ctx.strokeStyle = "white";

            // draw TILT cursor position
            ctx.fillStyle = "red";
            DrawFuncs.drawCursor(ctx, width / 2, height / 2, UISettings.iconSizeDefault, height - 30, tiltDegrees + 135, UISettings.iconSizeMedium / 2)

            // draw PAN cursor position
            ctx.fillStyle = "green";
            DrawFuncs.drawCursor(ctx, width / 2, height / 2, width - 30, UISettings.iconSizeDefault, panDegrees + 90, UISettings.iconSizeMedium / 2)
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

    GridLayout
    {
        x: 10
        y: gCanvas.y + gCanvas.height + 5
        width: parent.width - 20
        columns: 2
        rows: 2
        //rowsSpacing: 10
        //columnsSpacing: 10

        // row 1
        RobotoText
        {
            label: "Pan"
        }

        CustomSpinBox
        {
            id: panSpinBox
            Layout.fillWidth: true
            minimumValue: 0
            maximumValue: panMaxDegrees
            value: 0
            suffix: "°"

            onValueChanged:
            {
                panDegrees = value
                gCanvas.requestPaint()
            }
        }

        // row 2
        RobotoText
        {
            label: "Tilt"
        }

        CustomSpinBox
        {
            id: tiltSpinBox
            Layout.fillWidth: true
            minimumValue: 0
            maximumValue: tiltMaxDegrees
            value: 0
            suffix: "°"

            onValueChanged:
            {
                tiltDegrees = value
                gCanvas.requestPaint()
            }
        }
    }
}
