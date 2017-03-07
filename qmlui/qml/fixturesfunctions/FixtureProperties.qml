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

    property string fxMode: fixtureBrowser.selectedMode
    property string fxName: fixtureBrowser.selectedModel
    property int fxUniverseIndex: fxUniverseCombo.currentIndex
    property int fxAddress: fxAddressSpin.value
    property int fxChannels: fixtureBrowser.modeChannelsCount
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
            color: UISettings.sectionHeader

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
                Layout.fillWidth: true
                from: 1
                to: 512
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
                Layout.fillWidth: true
                from: 1
                to: 512
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
                from: 1
                to: 512
                value: fxChannels
                onValueChanged: fixtureBrowser.modeChannelsCount = value
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
                from: 0
                to: 511
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
                        model: fixtureBrowser.modesList
                        currentText: fxMode
                        onModelChanged: currentIndex = 0
                        onCurrentTextChanged: fixtureBrowser.selectedMode = currentText
                    }
                    IconButton
                    {
                        id: fxModeInfo
                        width: propsGrid.itemsHeight
                        height: width
                        imgSource: "qrc:/info.svg"
                        checkable: true
                    }
                }
            }
        } // end of GridLayout

        Rectangle
        {
            visible: fxModeInfo.checked
            height: UISettings.bigItemHeight * 1.5
            width: parent.width - 8

            clip: true
            color: UISettings.bgMedium

            ListView
            {
                id: channelList
                anchors.fill: parent
                boundsBehavior: Flickable.StopAtBounds
                model: fixtureBrowser.modeChannelList
                delegate:
                    IconTextEntry
                    {
                        width: channelList.width
                        height: UISettings.listItemHeight
                        tLabel: modelData.mLabel
                        iSrc: modelData.mIcon
                    }
                ScrollBar { flickable: channelList }
            }
        }
    } // end of Column
}
