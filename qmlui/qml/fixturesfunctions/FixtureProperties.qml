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

import "."

Rectangle
{
    id: fxProps
    width: 400
    height: columnContainer.height + 8
    color: UISettings.bgLight
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

    onFxModeChanged:
    {
        console.log("Mode changed: " + fxMode)
        updateAvailableAddress()
    }
    onFxCountChanged: updateAvailableAddress()

    function updateAvailableAddress()
    {
        fxAddressSpin.value =
                fixtureBrowser.availableChannel(fxUniverseIndex, fxChannels,
                                                fxQuantity, fxGap, fxAddressSpin.value - 1) + 1
    }

    Column
    {
        id: columnContainer
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 1
        spacing: 4

        Rectangle
        {
            height: 24
            width: parent.width
            color: "#0d235b"
            radius: 3

            RobotoText
            {
                id: fxPropsTitle
                anchors.centerIn: parent
                label: qsTr("Fixture properties")
            }
        }

        GridLayout
        {
            x: 4
            width: parent.width - 8
            columns: 4
            columnSpacing: 5
            rowSpacing: 4

            // row 1
            RobotoText
            {
                id: fxNameLabel
                height: 30
                //anchors.verticalCenter: parent.verticalCenter
                label: qsTr("Name")
                fontSize: 14
            }

            CustomTextEdit
            {
                id: fxNameTextEdit
                inputText: fxName
                Layout.columnSpan: 3
                Layout.fillWidth: true
                onInputTextChanged:
                {
                    console.log("Text changed !!")
                    fxProps.fxName = inputText
                }
            }

            // row 2
            RobotoText
            {
                id: fxUniverseLabel
                height: 30
                label: qsTr("Universe")
                fontSize: 14
            }
            CustomComboBox
            {
                id: fxUniverseCombo
                height: 30
                Layout.columnSpan: 3
                Layout.fillWidth: true
                model: ioManager.universeNames
            }

            // row 3
            RobotoText
            {
                id: fxAddressLabel
                height: 30
                label: qsTr("Address")
                fontSize: 14
            }
            CustomSpinBox
            {
                id: fxAddressSpin
                //width: (parent.width - fxAddress.width - fxQuantity.width) / 2
                Layout.fillWidth: true
                minimumValue: 1
                maximumValue: 512
                decimals: 0
            }
            RobotoText
            {
                id: fxQuantityLabel
                height: 30
                label: qsTr("Quantity")
                fontSize: 14
            }
            CustomSpinBox
            {
                id: fxQuantitySpin
                //width: (parent.width - fxAddress.width - fxQuantity.width) / 2
                Layout.fillWidth: true
                minimumValue: 1
                maximumValue: 512
                decimals: 0
            }

            // row 4
            RobotoText
            {
                id: fxModeChLabel
                height: 30
                label: qsTr("Channels")
                fontSize: 14
            }
            CustomSpinBox
            {
                id: fxModeChSpin
                Layout.fillWidth: true
                minimumValue: 1
                maximumValue: 512
                decimals: 0
                value: fixtureBrowser.modeChannels(fxMode)
            }
            RobotoText
            {
                id: fxGapLabel
                height: 30
                label: qsTr("Gap")
                fontSize: 14
            }

            CustomSpinBox
            {
                id: fxGapSpin
                Layout.fillWidth: true
                minimumValue: 0
                maximumValue: 511
                decimals: 0
            }

            // row 5
            RobotoText
            {
                id: fxModeLabel
                height: 30
                label: qsTr("Mode")
                fontSize: 14
            }

            Rectangle
            {
                color: "transparent"
                Layout.columnSpan: 3
                height: 30
                Layout.fillWidth: true

                RowLayout
                {
                    width: parent.width

                    CustomComboBox
                    {
                        id: fxModesCombo
                        height: 30
                        Layout.fillWidth: true
                        model: fixtureBrowser.modes(fxManufacturer, fxModel)
                        onModelChanged: currentIndex = 0
                        onCurrentTextChanged: fxProps.fxMode = currentText
                    }
                    IconButton
                    {
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
    }
}
