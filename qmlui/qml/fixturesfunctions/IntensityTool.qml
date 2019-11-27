/*
  Q Light Controller Plus
  IntensityTool.qml

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
import QtQuick.Controls 2.0

import org.qlcplus.classes 1.0
import "."

Rectangle
{
    id: intRoot
    width: UISettings.bigItemHeight * 1.5
    height: UISettings.bigItemHeight * 3
    color: UISettings.bgMedium
    border.color: UISettings.bgLight
    border.width: 2

    property bool dmxValues: true
    property alias currentValue: spinBox.value

    onCurrentValueChanged:
    {
        if (paletteBox.checked)
        {
            paletteBox.updatePreview()
        }
        else
        {
            if (dmxValues)
                fixtureManager.setIntensityValue(currentValue)
            else
                fixtureManager.setIntensityValue(currentValue * 2.55)
        }
    }

    onVisibleChanged: if(!visible) paletteBox.checked = false

    MouseArea
    {
        anchors.fill: parent
        onWheel: { return false }
    }

    Column
    {
        width: parent.width
        height: parent.height
        spacing: 5

        // draggable topbar
        Rectangle
        {
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
                label: qsTr("Intensity")
                fontSize: UISettings.textSizeDefault
                fontBold: true
            }
            // allow the tool to be dragged around
            // by holding it on the title bar
            MouseArea
            {
                anchors.fill: parent
                drag.target: intRoot
            }
        }

        // main control 'widget'
        Rectangle
        {
            id: intControl
            color: "transparent"
            x: (parent.width - width) / 2
            width: intRoot.width * 0.75
            height: intRoot.height - (UISettings.listItemHeight * 2) - UISettings.iconSizeMedium - 20

            Image
            {
                id: intBackgroundImg
                anchors.fill: parent
                source: "qrc:/dimmer-back.svg"
                sourceSize: Qt.size(width, height)
            }

            Rectangle
            {
                id: rectMask
                color: "transparent"
                width: Math.round((parent.height * currentValue) / (dmxValues ? 256.0 : 100.0))
                y: parent.height
                height: parent.width
                transformOrigin: Item.TopLeft
                rotation: -90
                clip: true

                Image
                {
                    id: intForegroundImg
                    y: -height
                    width: intBackgroundImg.width
                    height: intBackgroundImg.height
                    transformOrigin: Item.BottomLeft
                    rotation: 90
                    source: "qrc:/dimmer-fill.svg"
                    sourceSize: Qt.size(width, height)
                }
            }

            Slider
            {
                id: slider
                anchors.fill: parent
                orientation: Qt.Vertical
                from: 0
                to: dmxValues ? 255 : 100
                stepSize: 1.0
                background: Rectangle { color: "transparent" }
                handle: Rectangle { color: "transparent" }
                value: currentValue

                onPositionChanged: currentValue = valueAt(position)
            }
        }

        RowLayout
        {
            x: 5
            height: UISettings.listItemHeight
            width: intRoot.width - 10
            spacing: 5

            CustomSpinBox
            {
                id: spinBox
                Layout.fillWidth: true
                height: UISettings.listItemHeight
                from: 0
                suffix: dmxValues ? "" : "%"
                to: dmxValues ? 255 : 100

                onValueChanged: currentValue = value
            }

            DMXPercentageButton
            {
                height: UISettings.listItemHeight
                width: intRoot.width / 3
                dmxMode: dmxValues
                onClicked:
                {
                    var slVal = currentValue
                    var newVal
                    dmxValues = !dmxValues
                    if (dmxValues == false)
                        newVal = Math.round((slVal / 255.0) * 100.0)
                    else
                        newVal = Math.round((slVal / 100.0) * 255.0)

                    currentValue = newVal
                }
            }
        }

        PaletteFanningBox
        {
            id: paletteBox
            x: 5
            width: intRoot.width - 10
            paletteType: QLCPalette.Dimmer
            value1: intRoot.currentValue
        }
    }
}

