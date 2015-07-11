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

    property string fxManufacturer
    property string fxModel
    property string fxMode: fxModesCombo.currentText
    property string fxName
    property int fxUniverseIndex: fxUniverseCombo.currentIndex
    property int fxAddress: fxAddressSpin.value
    property int fxChannels: fxModeChSpin.value
    property int fxQuantity: fxQuantitySpin.value
    property int fxGap: fxGapSpin.value

    property int fxCount: fixtureManager.fixturesCount

    onFxModeChanged: {
        console.log("Mode changed: " + fxMode)
        updateAvailableAddress()
    }
    onFxCountChanged: updateAvailableAddress()

    function updateAvailableAddress() {
        fxAddressSpin.value =
                fixtureBrowser.availableChannel(fxUniverseIndex, fxChannels, fxAddressSpin.value - 1) + 1
    }

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

            RobotoText {
                id: fxPropsTitle
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
                label: qsTr("Fixture properties")
                //horizontalAlignment: Text.AlignHCenter
                fontSize: 16
            }
        }

        RowLayout {
            anchors.horizontalCenter: parent.horizontalCenter
            width: parent.width - 14
            spacing: 4

            RobotoText {
                id: fxNameLabel
                height: 30
                anchors.verticalCenter: parent.verticalCenter
                label: qsTr("Name")
                fontSize: 14
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

            RobotoText {
                id: fxUniverseLabel
                height: 30
                anchors.verticalCenter: parent.verticalCenter
                label: qsTr("Universe")
                fontSize: 14
            }
            CustomComboBox {
                id: fxUniverseCombo
                height: 30
                Layout.fillWidth: true
                model: ioManager.universeNames
            }
        }

        RowLayout {
            anchors.horizontalCenter: parent.horizontalCenter
            width: parent.width - 14
            spacing: 4

            RobotoText {
                id: fxAddressLabel
                height: 30
                anchors.verticalCenter: parent.verticalCenter
                label: qsTr("Address")
                fontSize: 14
            }
            CustomSpinBox {
                id: fxAddressSpin
                //width: (parent.width - fxAddress.width - fxQuantity.width) / 2
                height: 30
                minimumValue: 1
                maximumValue: 512
                decimals: 0
                Layout.fillWidth: true
            }
            RobotoText {
                id: fxQuantityLabel
                height: 30
                anchors.verticalCenter: parent.verticalCenter
                label: qsTr("Quantity")
                fontSize: 14
            }
            CustomSpinBox {
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

            RobotoText {
                id: fxModeChLabel
                height: 30
                anchors.verticalCenter: parent.verticalCenter
                label: qsTr("Channels")
                fontSize: 14
            }
            CustomSpinBox {
                id: fxModeChSpin
                height: 30
                Layout.fillWidth: true
                minimumValue: 1
                maximumValue: 512
                decimals: 0
                value: fixtureBrowser.modeChannels(fxMode)
            }
            RobotoText {
                id: fxGapLabel
                height: 30
                anchors.verticalCenter: parent.verticalCenter
                label: qsTr("Gap")
                fontSize: 14
            }

            CustomSpinBox {
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

            RobotoText {
                id: fxModeLabel
                height: 30
                anchors.verticalCenter: parent.verticalCenter
                label: qsTr("Mode")
                fontSize: 14
            }

            CustomComboBox {
                id: fxModesCombo
                height: 30
                model: fixtureBrowser.modes(fxManufacturer, fxModel)
                Layout.fillWidth: true
                onCurrentIndexChanged: {
                    fxProps.fxMode = currentText
                }
            }
            IconButton {
                id: fxModeInfo
                width: 30
                height: 30
                imgSource: "qrc:/info.svg"
                checkable: true
                onToggled: {

                }
            }
        }
    }
}
