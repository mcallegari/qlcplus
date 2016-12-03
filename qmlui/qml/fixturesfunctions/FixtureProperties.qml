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
            height: UISettings.listItemHeight * 0.8
            width: parent.width
            color: UISettings.highlightPressed
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
            id: propsGrid
            x: 4
            width: parent.width - 8
            columns: 4
            columnSpacing: 5
            rowSpacing: 4

            property real itemsHeight: UISettings.listItemHeight
            property real itemsFontSize: UISettings.textSizeDefault * 0.75

            // row 1
            RobotoText
            {
                id: fxNameLabel
                height: propsGrid.itemsHeight
                //anchors.verticalCenter: parent.verticalCenter
                label: qsTr("Name")
                fontSize: propsGrid.itemsFontSize
            }

            CustomTextEdit
            {
                id: fxNameTextEdit
                inputText: fxName
                Layout.columnSpan: 3
                Layout.fillWidth: true
                onInputTextChanged:
                {
                    //console.log("Text changed !!")
                    fxProps.fxName = inputText
                }
            }

            // row 2
            RobotoText
            {
                id: fxUniverseLabel
                height: propsGrid.itemsHeight
                label: qsTr("Universe")
                fontSize: propsGrid.itemsFontSize
            }
            CustomComboBox
            {
                id: fxUniverseCombo
                height: propsGrid.itemsHeight
                Layout.columnSpan: 3
                Layout.fillWidth: true
                model: ioManager.universeNames
            }

            // row 3
            RobotoText
            {
                id: fxAddressLabel
                height: propsGrid.itemsHeight
                label: qsTr("Address")
                fontSize: propsGrid.itemsFontSize
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
                height: propsGrid.itemsHeight
                label: qsTr("Quantity")
                fontSize: propsGrid.itemsFontSize
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
                height: propsGrid.itemsHeight
                label: qsTr("Channels")
                fontSize: propsGrid.itemsFontSize
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
                height: propsGrid.itemsHeight
                label: qsTr("Gap")
                fontSize: propsGrid.itemsFontSize
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
                height: propsGrid.itemsHeight
                label: qsTr("Mode")
                fontSize: propsGrid.itemsFontSize
            }

            Rectangle
            {
                color: "transparent"
                Layout.columnSpan: 3
                height: propsGrid.itemsHeight
                Layout.fillWidth: true

                RowLayout
                {
                    width: parent.width

                    CustomComboBox
                    {
                        id: fxModesCombo
                        height: propsGrid.itemsHeight
                        Layout.fillWidth: true
                        model: fixtureBrowser.modes(fxManufacturer, fxModel)
                        onModelChanged: currentIndex = 0
                        onCurrentTextChanged: fxProps.fxMode = currentText
                    }
                    IconButton
                    {
                        id: fxModeInfo
                        width: propsGrid.itemsHeight
                        height: width
                        imgSource: "qrc:/info.svg"
                        checkable: true
                        onToggled:
                        {

                        }
                    }
                }
            }
        }
    }
}
