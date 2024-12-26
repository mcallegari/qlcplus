/*
  Q Light Controller Plus
  BeamTool.qml

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

import org.qlcplus.classes 1.0
import "."

Rectangle
{
    id: toolRoot
    width: UISettings.bigItemHeight * 3
    height: UISettings.bigItemHeight * 3
    color: UISettings.bgMedium
    border.color: UISettings.bgLight
    border.width: 2

    property real minDegrees: 0
    property real maxDegrees: 0
    property bool invertedZoom: false
    property real projectedDiameter: 0
    property bool isUpdating: false

    property alias currentDegrees: beamSpinBox.realValue
    property int previousDegrees: 0
    property bool relativeValue: false

    signal close()

    onMinDegreesChanged: gCanvas.requestPaint()
    onMaxDegreesChanged: gCanvas.requestPaint()

    onVisibleChanged:
    {
        if (visible)
        {
            previousDegrees = 0
            var val = contextManager.getCurrentValue(QLCChannel.Beam, true)
            isUpdating = true

            if (val === -1)
            {
                relativeValue = true
                currentDegrees = 0
            }
            else
            {
                relativeValue = false
                currentDegrees = val
            }
            beamSpinBox.value = currentDegrees * Math.pow(10, beamSpinBox.decimals)
            calculateProjection()
            isUpdating = false
        }
    }

    onCurrentDegreesChanged:
    {
        if (isUpdating)
            return

        var val = relativeValue ? currentDegrees - previousDegrees : currentDegrees
        previousDegrees = currentDegrees

        beamSpinBox.value = currentDegrees * Math.pow(10, beamSpinBox.decimals)
        contextManager.setBeamDegrees(val, relativeValue)
        calculateProjection()
        gCanvas.requestPaint()
    }

    function setZoomRange(min, max, inverted)
    {
        if (max === maxDegrees && min === minDegrees)
            return

        isUpdating = true
        maxDegrees = max
        minDegrees = min
        invertedZoom = inverted
        currentDegrees = inverted ? maxDegrees : minDegrees
        beamSpinBox.value = currentDegrees * Math.pow(10, beamSpinBox.decimals)
        isUpdating = false
    }

    function calculateProjection()
    {
        // calculate apothem
        var apothem = distSpinBox.realValue / Math.cos(Math.PI * beamSpinBox.realValue / 360.0)
        // calculate projection radius r=sqrt(a^2 - h^2)
        var radius = Math.sqrt(Math.pow(apothem, 2) - Math.pow(distSpinBox.realValue, 2))
        projectedDiameter = radius * 2.0
    }

    MouseArea
    {
        anchors.fill: parent
        onWheel: { return false }
    }

    Rectangle
    {
        id: toolbar
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
            label: qsTr("Beam")
            fontSize: UISettings.textSizeDefault
            fontBold: true
        }
        // allow the tool to be dragged around
        // by holding it on the title bar
        MouseArea
        {
            anchors.fill: parent
            drag.target: toolRoot
        }
        GenericButton
        {
            width: height
            height: parent.height
            anchors.right: parent.right
            border.color: UISettings.bgMedium
            useFontawesome: true
            label: FontAwesome.fa_times
            onClicked: toolRoot.close()
        }
    }

    Canvas
    {
        id: gCanvas
        width: toolRoot.width - 20
        height: UISettings.bigItemHeight * 1.2
        x: 10
        y: toolbar.height + 5
        rotation: 0
        antialiasing: true
        contextType: "2d"

        onPaint:
        {
            context.globalAlpha = 1.0
            context.fillStyle = UISettings.bgStronger
            context.lineWidth = 1

            context.fillRect(0, 0, width, height)

            // draw the max degrees background
            context.fillStyle = UISettings.bgLight
            var halfHeight = height / 2
            // angle calculation as if line is 100px long, then scale to the canvas width.
            // Degrees to radians is included in the X/Y calculation
            var compX = 100 * Math.cos(Math.PI * maxDegrees / 360.0)
            var compY = 100 * Math.sin(Math.PI * maxDegrees / 360.0)
            var scaledHeight = (width * compY) / compX // compX : width = compY : scaledHeight
            context.beginPath()
            context.moveTo(0, halfHeight)
            context.lineTo(width, halfHeight - scaledHeight)
            context.lineTo(width, halfHeight + scaledHeight)
            context.lineTo(0, halfHeight)
            context.closePath()
            context.fill()

            // draw the current beam degrees
            context.fillStyle = "yellow"
            compX = 100 * Math.cos(Math.PI * beamSpinBox.realValue / 360.0)
            compY = 100 * Math.sin(Math.PI * beamSpinBox.realValue / 360.0)
            scaledHeight = (width * compY) / compX
            context.beginPath()
            context.moveTo(0, halfHeight)
            context.lineTo(width, halfHeight - scaledHeight)
            context.lineTo(width, halfHeight + scaledHeight)
            context.lineTo(0, halfHeight)
            context.closePath()
            context.fill()
        }
    }

    GridLayout
    {
        y: gCanvas.y + gCanvas.height + 10
        x: 10
        width: toolRoot.width - 20
        height: UISettings.iconSizeDefault
        columns: 2

        RobotoText { label: qsTr("Beam degrees") }

        CustomDoubleSpinBox
        {
            id: beamSpinBox
            realFrom: relativeValue ? -maxDegrees : minDegrees
            realTo: maxDegrees
        }

        RobotoText { label: qsTr("Distance") }

        CustomDoubleSpinBox
        {
            id: distSpinBox
            realFrom: 1
            realTo: 1000
            realValue: 3
            suffix: "m"

            onRealValueChanged:
            {
                calculateProjection()
                //gCanvas.requestPaint()
            }
        }

        RobotoText { label: qsTr("Projected diameter") }

        RobotoText
        {
            label: relativeValue ? qsTr("N/A") : Number(toolRoot.projectedDiameter).toFixed(2) + "m"
            fontBold: true
            //labelColor: UISettings.highlight
        }
    }
}
