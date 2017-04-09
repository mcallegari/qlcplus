/*
  Q Light Controller Plus
  RGBPanelProperties.qml

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

import com.qlcplus.classes 1.0
import "."

Rectangle
{
    id: panelProps
    width: 400
    height: columnContainer.height + 8
    color: UISettings.bgLight

    property string name: fixtureBrowser.fixtureName
    property int universeIndex: fxUniverseCombo.currentIndex
    property int address: fxAddressSpin.value
    property int components: componentsCombo.currentValue
    property int columns: widthSpin.value
    property int rows: heightSpin.value
    property int physicalWidth: phyWidthSpin.value
    property int physicalHeight: phyHeightSpin.value
    property int startCorner: cornerCombo.currentValue
    property int displacement: dispCombo.currentValue
    property int direction: directionCombo.currentValue

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
                id: panelPropsTitle
                anchors.centerIn: parent
                label: qsTr("RGB panel properties")
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
                height: propsGrid.itemsHeight
                label: qsTr("Name")
                fontSize: propsGrid.itemsFontSize
            }
            CustomTextEdit
            {
                id: fxNameTextEdit
                inputText: name
                Layout.columnSpan: 3
                Layout.fillWidth: true
                onInputTextChanged: fixtureBrowser.fixtureName = inputText
            }

            // row 2
            RobotoText
            {
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
                height: propsGrid.itemsHeight
                label: qsTr("Components")
                fontSize: propsGrid.itemsFontSize
            }
            CustomComboBox
            {
                id: componentsCombo
                height: propsGrid.itemsHeight
                Layout.fillWidth: true
                model: compModel

                ListModel
                {
                    id: compModel
                    ListElement { mLabel: "RGB"; mValue: Fixture.RGB }
                    ListElement { mLabel: "BGR"; mValue: Fixture.BGR }
                    ListElement { mLabel: "BRG"; mValue: Fixture.BRG }
                    ListElement { mLabel: "GBR"; mValue: Fixture.GBR }
                    ListElement { mLabel: "GRB"; mValue: Fixture.GRB }
                    ListElement { mLabel: "RGBW"; mValue: Fixture.RGBW }
                }
            }

            // row 4
            RobotoText
            {
                height: propsGrid.itemsHeight
                label: qsTr("Size")
                fontSize: propsGrid.itemsFontSize
            }

            CustomSpinBox
            {
                id: widthSpin
                Layout.fillWidth: true
                from: 1
                to: 170
                value: 10
            }

            RobotoText
            {
                height: propsGrid.itemsHeight
                label: "x"
                fontSize: propsGrid.itemsFontSize
            }

            CustomSpinBox
            {
                id: heightSpin
                Layout.fillWidth: true
                from: 1
                to: 999
                value: 10
            }

            // row 5
            RobotoText
            {
                height: propsGrid.itemsHeight
                label: qsTr("Physical")
                fontSize: propsGrid.itemsFontSize
            }

            CustomSpinBox
            {
                id: phyWidthSpin
                Layout.fillWidth: true
                from: 1
                to: 99999
                value: 1000
                suffix: "mm"
            }

            RobotoText
            {
                height: propsGrid.itemsHeight
                label: "x"
                fontSize: propsGrid.itemsFontSize
            }

            CustomSpinBox
            {
                id: phyHeightSpin
                Layout.fillWidth: true
                from: 1
                to: 99999
                value: 1000
                suffix: "mm"
            }

            // row 6
            RobotoText
            {
                height: propsGrid.itemsHeight
                label: qsTr("Start corner")
                fontSize: propsGrid.itemsFontSize
            }
            CustomComboBox
            {
                id: cornerCombo
                height: propsGrid.itemsHeight
                Layout.columnSpan: 3
                Layout.fillWidth: true
                model: cornerModel

                ListModel
                {
                    id: cornerModel
                    ListElement { mLabel: qsTr("Top-Left"); mValue: FixtureManager.TopLeft }
                    ListElement { mLabel: qsTr("Top-Right"); mValue: FixtureManager.TopRight }
                    ListElement { mLabel: qsTr("Bottom-Left"); mValue: FixtureManager.BottomLeft }
                    ListElement { mLabel: qsTr("Bottom-Right"); mValue: FixtureManager.BottomRight }
                }
            }

            // row 7
            RobotoText
            {
                height: propsGrid.itemsHeight
                label: qsTr("Displacement")
                fontSize: propsGrid.itemsFontSize
            }
            CustomComboBox
            {
                id: dispCombo
                height: propsGrid.itemsHeight
                Layout.fillWidth: true
                model: dispModel

                ListModel
                {
                    id: dispModel
                    ListElement { mLabel: qsTr("Snake"); mValue: FixtureManager.Snake }
                    ListElement { mLabel: qsTr("Zig Zag"); mValue: FixtureManager.ZigZag }
                }
            }
            RobotoText
            {
                height: propsGrid.itemsHeight
                label: qsTr("Direction")
                fontSize: propsGrid.itemsFontSize
            }
            CustomComboBox
            {
                id: directionCombo
                height: propsGrid.itemsHeight
                Layout.fillWidth: true
                model: dirModel

                ListModel
                {
                    id: dirModel
                    ListElement { mLabel: qsTr("Horizontal"); mValue: FixtureManager.Horizontal }
                    ListElement { mLabel: qsTr("Vertical"); mValue: FixtureManager.Vertical }
                }
            }

        } // end of GridLayout
    } // end of Column
}
