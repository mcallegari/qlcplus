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
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.2

Rectangle {
    width: 150
    height: 350
    color: "#333"
    border.color: "#666"
    border.width: 2

    property bool dmxValues: true
    property int currentValue: 0

    onCurrentValueChanged: {
        if (dmxValues)
            fixtureManager.setIntensityValue(currentValue)
        else
            fixtureManager.setIntensityValue(currentValue * 2.55)
    }

    RobotoText {
        id: titleBox
        y: 7
        height: 20
        anchors.horizontalCenter: parent.horizontalCenter
        label: qsTr("Intensity")
        fontSize: 15
        fontBold: true
    }

    Image {
        id: intBackgroundImg
        x:30
        y: 30
        width: 100
        height: 256
        source: "qrc:/dimmer-back.svg"
        sourceSize: Qt.size(100, 256)
    }

    Rectangle {
        id: rectMask
        color: "transparent"
        x: 30
        width: 100
        y: 30 + (255 - (dmxValues ? slider.value : (slider.value * 2.55)))
        height: 285 - y
        clip: true

        Image {
            id: intForegroundImg
            y: parent.height - 256
            width: 100
            height: 256
            source: "qrc:/dimmer-fill.svg"
            sourceSize: Qt.size(100, 256)
        }
    }

    Slider {
        id: slider
        x: 20
        y: 30
        width: 110
        height: 256
        orientation: Qt.Vertical
        minimumValue: 0
        maximumValue: 255
        stepSize: 1.0

        style: SliderStyle {
            groove: Rectangle { color: "transparent" }
            handle: Rectangle { color: "transparent" }
        }
        onValueChanged: currentValue = slider.value
    }

    CustomSpinBox {
        id: spinBox
        x: 10
        y: 300
        width: 75
        height: 40
        minimumValue: 0
        maximumValue: slider.maximumValue
        value: slider.value

        onValueChanged: slider.value = value
    }

    Rectangle {
        x: 90
        y: 300
        width: 50
        height: 40
        border.width: 2
        border.color: "white"
        radius: 5
        color: "#1E476E"

        RobotoText {
            height: 40
            anchors.horizontalCenter: parent.horizontalCenter
            label: dmxValues ? "DMX" : "%"
            fontSize: 15
            fontBold: true
        }

        MouseArea {
            anchors.fill: parent
            onClicked: {
                dmxValues = !dmxValues
                var newVal;
                if (dmxValues == false)
                {
                    newVal = (slider.value / 255) * 100
                    slider.maximumValue = 100
                }
                else
                {
                    newVal = (slider.value / 100) * 255
                    slider.maximumValue = 255
                }
                slider.value = newVal
            }
        }
    }
}

