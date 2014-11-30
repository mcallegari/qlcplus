/*
  Q Light Controller Plus
  FixtureProperties.qml

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

import QtQuick 2.3
import QtQuick.Controls 1.2
import QtQuick.Layouts 1.1

Rectangle {
    id: fxProps
    width: 400
    height: columnContainer.height + 8
    color: "#555555"
    radius: 4
    border.color: "#444"

    property string fxManufacturer: ""
    property string fxModel: ""
    property string fxMode: fxModesCombo.currentText
    property string fxName: ""
    property int fxUniverseIndex: fxUniverseCombo.currentIndex
    property int fxAddress: fxAddressSpin.value
    property int fxChannels: fxModeChSpin.value
    property int fxQuantity: fxQuantitySpin.value
    property int fxGap: fxGapSpin.value

    property variant fxModes: null

    Column {
        id: columnContainer
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 1
        spacing: 4

        Rectangle {
            height: 24
            width: parent.width
            color: "#0d235b"
            radius: 3

            Text {
                id: fxPropsTitle
                anchors.fill: parent
                color: "#ffffff"
                text: qsTr("Fixture properties")
                horizontalAlignment: Text.AlignHCenter
                font.pixelSize: 18
            }
        }

        RowLayout {
            anchors.horizontalCenter: parent.horizontalCenter
            width: parent.width - 14
            spacing: 4

            Text {
                id: fxNameLabel
                anchors.verticalCenter: parent.verticalCenter
                color: "#ffffff"
                text: qsTr("Name")
                font.pixelSize: 15
            }

            CustomTextEdit {
                id: fxNameTextEdit
                height: 30
                inputText: fxName
                Layout.fillWidth: true
                onInputTextChanged: {
                    console.log("Text changed !!")
                    fxProps.fxName = inputText
                }
            }
        }

        RowLayout {
            anchors.horizontalCenter: parent.horizontalCenter
            width: parent.width - 14
            spacing: 4

            Text {
                id: fxUniverse
                anchors.verticalCenter: parent.verticalCenter
                color: "#ffffff"
                text: qsTr("Universe")
                font.pixelSize: 15
            }
            CustomComboBox {
                id: fxUniverseCombo
                height: 30
                Layout.fillWidth: true
                model: ioManager.universes
            }
        }

        RowLayout {
            anchors.horizontalCenter: parent.horizontalCenter
            width: parent.width - 14
            spacing: 4

            Text {
                id: fxAddress
                anchors.verticalCenter: parent.verticalCenter
                color: "#ffffff"
                text: qsTr("Address")
                font.pixelSize: 15
            }
            SpinBox {
                id: fxAddressSpin
                //width: (parent.width - fxAddress.width - fxQuantity.width) / 2
                height: 30
                minimumValue: 1
                maximumValue: 512
                decimals: 0
                Layout.fillWidth: true
            }
            Text {
                id: fxQuantity
                anchors.verticalCenter: parent.verticalCenter
                color: "#ffffff"
                text: qsTr("Quantity")
                font.pixelSize: 15
            }
            SpinBox {
                id: fxQuantitySpin
                //width: (parent.width - fxAddress.width - fxQuantity.width) / 2
                height: 30
                minimumValue: 1
                maximumValue: 512
                decimals: 0
                Layout.fillWidth: true
            }
        }

        RowLayout {
            anchors.horizontalCenter: parent.horizontalCenter
            width: parent.width - 14
            spacing: 4

            Text {
                id: fxModeCh
                anchors.verticalCenter: parent.verticalCenter
                color: "#ffffff"
                text: qsTr("Channels")
                font.pixelSize: 15
            }
            SpinBox {
                id: fxModeChSpin
                height: 30
                Layout.fillWidth: true
                minimumValue: 1
                maximumValue: 512
                decimals: 0
                value: fixtureBrowser.modeChannels
            }
            Text {
                id: fxGap
                anchors.verticalCenter: parent.verticalCenter
                color: "#ffffff"
                text: qsTr("Gap")
                font.pixelSize: 15
            }

            SpinBox {
                id: fxGapSpin
                height: 30
                Layout.fillWidth: true
                minimumValue: 0
                maximumValue: 511
                decimals: 0
            }
        }

        RowLayout {
            anchors.horizontalCenter: parent.horizontalCenter
            width: parent.width - 14
            spacing: 4

            Text {
                id: fxMode
                anchors.verticalCenter: parent.verticalCenter
                color: "#ffffff"
                text: qsTr("Mode")
                font.pixelSize: 15
            }

            CustomComboBox {
                id: fxModesCombo
                height: 30
                model: fxModes
                Layout.fillWidth: true
                onCurrentIndexChanged: {
                    fixtureBrowser.mode = currentText
                }
            }
        }
    }
}
