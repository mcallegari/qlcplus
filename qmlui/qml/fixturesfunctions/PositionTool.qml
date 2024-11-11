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
import QtQuick.Layouts 1.1

import org.qlcplus.classes 1.0
import "CanvasDrawFunctions.js" as DrawFuncs
import "."

Rectangle
{
    id: posToolRoot
    width: UISettings.bigItemHeight * 2.2
    height: (UISettings.bigItemHeight * 3.2) + paletteBox.height
    color: UISettings.bgMedium
    //border.color: UISettings.bgLight
    //border.width: 2

    property int panMaxDegrees: 360
    property int tiltMaxDegrees: 270

    property alias panDegrees: panSpinBox.realValue
    property int previousPanDegrees: 0
    property bool relativePanValue: false

    property alias tiltDegrees: tiltSpinBox.realValue
    property int previousTiltDegrees: 0
    property bool relativeTiltValue: false

    property alias showPalette: paletteBox.visible
    property bool isLoading: false

    signal close()

    function updatePanTiltDegrees()
    {
        previousPanDegrees = 0
        previousTiltDegrees = 0

        var pan = contextManager.getCurrentValue(QLCChannel.Pan, true)
        if (pan === -1)
        {
            relativePanValue = true
            panDegrees = 0
        }
        else
        {
            relativePanValue = false
            panDegrees = pan * Math.pow(10, panSpinBox.decimals)
        }

        var tilt = contextManager.getCurrentValue(QLCChannel.Tilt, true)
        if (tilt === -1)
        {
            relativeTiltValue = true
            tiltDegrees = 0
        }
        else
        {
            relativeTiltValue = false
            tiltDegrees = tilt * Math.pow(10, tiltSpinBox.decimals)
        }
    }

    onVisibleChanged:
    {
        if (visible)
        {
            updatePanTiltDegrees()
        }
        else
        {
            paletteBox.checked = false
        }
    }

    onPanDegreesChanged:
    {
        if (isLoading)
            return

        if (relativePanValue)
        {
            contextManager.setPositionValue(QLCChannel.Pan, panDegrees - previousPanDegrees, true)
            previousPanDegrees = panDegrees
        }
        else
        {
            paletteBox.updateValues(panDegrees, tiltDegrees)

            if (paletteBox.isEditing || paletteBox.checked)
                paletteBox.updatePreview()
            else
                contextManager.setPositionValue(QLCChannel.Pan, panDegrees, false)
        }
    }

    onTiltDegreesChanged:
    {
        if (isLoading)
            return

        if (relativeTiltValue)
        {
            contextManager.setPositionValue(QLCChannel.Tilt, tiltDegrees - previousTiltDegrees, true)
            previousTiltDegrees = tiltDegrees
        }
        else
        {
            paletteBox.updateValues(panDegrees, tiltDegrees)

            if (paletteBox.isEditing || paletteBox.checked)
                paletteBox.updatePreview()
            else
                contextManager.setPositionValue(QLCChannel.Tilt, tiltDegrees, false)
        }
    }

    onPanMaxDegreesChanged: gCanvas.requestPaint()
    onTiltMaxDegreesChanged: gCanvas.requestPaint()

    function tiltPositionsArray()
    {
        var halfTilt = tiltMaxDegrees / 2
        var array = [ 0,
                      halfTilt - 90,
                      halfTilt - 45,
                      halfTilt,
                      halfTilt + 45,
                      halfTilt + 90,
                      tiltMaxDegrees ]
        return array
    }

    function loadPalette(id)
    {
        isLoading = true

        var palette = paletteManager.getPalette(id)
        if (palette)
        {
            posToolBar.visible = false
            paletteToolbar.visible = true
            paletteToolbar.text = palette.name
            paletteBox.editPalette(palette)

            if (palette.type === QLCPalette.Pan)
            {
                panDegrees = palette.intValue1
            }
            else if (palette.type === QLCPalette.Tilt)
            {
                tiltDegrees = palette.intValue1
            }
            else if (palette.type === QLCPalette.PanTilt)
            {
                panDegrees = palette.intValue1
                tiltDegrees = palette.intValue2
            }
        }
        isLoading = false
    }

    MouseArea
    {
        anchors.fill: parent
        onWheel: { return false }
    }

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
        GenericButton
        {
            width: height
            height: parent.height
            anchors.right: parent.right
            border.color: UISettings.bgMedium
            useFontawesome: true
            label: FontAwesome.fa_times
            onClicked: posToolRoot.close()
        }
    }

    EditorTopBar
    {
        id: paletteToolbar
        visible: false
        onBackClicked: posToolRoot.parent.dismiss()
        onTextChanged: paletteBox.setName(text)
    }

    IconButton
    {
        x: parent.width - width - 2
        y: posToolBar.height
        z: 2
        imgSource: "qrc:/rotate-right.svg"
        tooltip: qsTr("Rotate preview 90° clockwise")
        onClicked:
        {
            gCanvas.rotation += 90
            if (gCanvas.rotation == 360)
                gCanvas.rotation = 0
        }
    }

    Timer
    {
        id: updTimer
        running: false
        interval: 100
        repeat: false
        onTriggered: updatePanTiltDegrees()
    }

    IconButton
    {
        x: parent.width - width - 2
        y: gCanvas.y + gCanvas.height - width
        z: 2
        faSource: FontAwesome.fa_bullseye
        tooltip: qsTr("Center Pan/Tilt halfway")
        onClicked:
        {
            contextManager.setPositionCenter()
            updTimer.restart()
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
        contextType: "2d"

        onPaint:
        {
            context.globalAlpha = 1.0
            context.fillStyle = "#111"
            context.lineWidth = 1

            context.fillRect(0, 0, width, height)
            // draw head basement
            DrawFuncs.drawBasement(context, width, height)

            context.lineWidth = 5
            // draw TILT curve
            context.strokeStyle = "#2E77FF"
            DrawFuncs.drawEllipse(context, width / 2, height / 2, UISettings.iconSizeDefault, height - 30)
            // draw PAN curve
            context.strokeStyle = "#19438F"
            DrawFuncs.drawEllipse(context, width / 2, height / 2, width - 30, UISettings.iconSizeDefault)

            context.lineWidth = 1
            context.strokeStyle = "white"

            // draw TILT cursor position
            context.fillStyle = "red"
            DrawFuncs.drawCursor(context, width / 2, height / 2, UISettings.iconSizeDefault, height - 30,
                                 tiltDegrees + 90 + (180 - tiltMaxDegrees / 2), UISettings.iconSizeMedium / 2)

            // draw PAN cursor position
            context.fillStyle = "green"
            DrawFuncs.drawCursor(context, width / 2, height / 2, width - 30, UISettings.iconSizeDefault,
                                 panDegrees + 90, UISettings.iconSizeMedium / 2)
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
                        panDegrees++
                    else
                        panDegrees--
                }
                else
                {
                    if (mouse.y < initialYPos)
                        tiltDegrees++
                    else
                        tiltDegrees--
                }
            }
        }
    }

    GridLayout
    {
        x: 10
        y: gCanvas.y + gCanvas.height + 5
        width: parent.width - 20
        columns: 4
        rows: 2

        // row 1
        RobotoText
        {
            height: UISettings.listItemHeight
            label: "Pan"
        }

        CustomDoubleSpinBox
        {
            id: panSpinBox
            Layout.fillWidth: true
            realFrom: relativePanValue ? -panMaxDegrees : 0
            realTo: panMaxDegrees
            realStep: 0.1
            value: 0
            suffix: "°"

            onValueChanged: gCanvas.requestPaint()
        }

        IconButton
        {
            width: UISettings.iconSizeMedium
            height: width
            imgSource: "qrc:/back.svg"
            tooltip: qsTr("Snap to the previous value")
            onClicked:
            {
                var prev = (parseInt(panSpinBox.realValue / 45) * 45) - 45
                console.log("---- PREV PAN " + prev)
                if (prev >= 0)
                    panSpinBox.setValue(prev * 100)
            }
        }
        IconButton
        {
            width: UISettings.iconSizeMedium
            height: width
            imgSource: "qrc:/forward.svg"
            tooltip: qsTr("Snap to the next value")
            onClicked:
            {
                var next = (parseInt(panSpinBox.realValue / 45) * 45) + 45
                console.log("---- NEXT PAN " + next)
                if (next <= panMaxDegrees)
                    panSpinBox.setValue(next * 100)
            }
        }

        // row 2
        RobotoText
        {
            height: UISettings.listItemHeight
            label: "Tilt"
        }

        CustomDoubleSpinBox
        {
            id: tiltSpinBox
            Layout.fillWidth: true
            realFrom: relativeTiltValue ? -tiltMaxDegrees : 0
            realTo: tiltMaxDegrees
            realStep: 0.1
            value: 0
            suffix: "°"

            onValueChanged: gCanvas.requestPaint()
        }

        IconButton
        {
            width: UISettings.iconSizeMedium
            height: width
            imgSource: "qrc:/back.svg"
            tooltip: qsTr("Snap to the previous value")
            onClicked:
            {
                var fixedPos = tiltPositionsArray()
                for (var i = fixedPos.length - 1; i >= 0; i--)
                {
                    if (parseInt(fixedPos[i]) < tiltDegrees)
                    {
                        tiltSpinBox.setValue(parseInt(fixedPos[i]) * 100)
                        break;
                    }
                }
            }
        }
        IconButton
        {
            width: UISettings.iconSizeMedium
            height: width
            imgSource: "qrc:/forward.svg"
            tooltip: qsTr("Snap to the next value")
            onClicked:
            {
                var fixedPos = tiltPositionsArray()
                for (var i = 0; i < fixedPos.length; i++)
                {
                    if (tiltDegrees < parseInt(fixedPos[i]))
                    {
                        tiltSpinBox.setValue(parseInt(fixedPos[i]) * 100)
                        break;
                    }
                }
            }
        }

        // row 3
        PaletteFanningBox
        {
            id: paletteBox
            Layout.columnSpan: 4
            Layout.fillWidth: true
            paletteType: QLCPalette.PanTilt
        }
    } // GridLayout
}
