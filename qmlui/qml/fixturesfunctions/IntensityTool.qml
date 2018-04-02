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
import QtQuick.Controls 2.0

import "."

Rectangle
{
    id: intRoot
    width: UISettings.bigItemHeight * 1.5
    height: UISettings.bigItemHeight * 3
    color: UISettings.bgMedium
    border.color: "#666"
    border.width: 2

    property bool dmxValues: true
    property alias currentValue: spinBox.value

    onCurrentValueChanged:
    {
        if (dmxValues)
            fixtureManager.setIntensityValue(currentValue)
        else
            fixtureManager.setIntensityValue(currentValue * 2.55)
    }

    Rectangle
    {
        id: intToolBar
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

    Rectangle
    {
        color: "transparent"
        x: 30
        y: intToolBar.height + 5
        width: intRoot.width - 60
        height: intRoot.height - (UISettings.listItemHeight * 2) - 10

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
            width: (parent.height * currentValue) / (dmxValues ? 256 : 100)
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

    Row
    {
        x: 10
        y: intRoot.height - UISettings.listItemHeight - 10
        spacing: 5

        CustomSpinBox
        {
            id: spinBox
            width: intRoot.width / 2
            height: UISettings.listItemHeight
            from: 0
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
                    newVal = (slVal / 255) * 100
                else
                    newVal = (slVal / 100) * 255

                currentValue = newVal
            }
        }
    }
}

