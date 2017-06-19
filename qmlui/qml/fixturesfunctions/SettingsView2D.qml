/*
  Q Light Controller Plus
  SettingsView2D.qml

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

import "."

Rectangle
{
    id: settingsRoot
    width: mainView.width / 5
    height: parent.height

    color: UISettings.bgMedium
    border.width: 1
    border.color: "#222"

    property bool fxPropsVisible: contextManager.hasSelectedFixtures

    GridLayout
    {
        width: settingsRoot.width
        columns: 2
        columnSpacing: 5
        rowSpacing: 5

        // row 1
        Rectangle
        {
            height: UISettings.listItemHeight
            Layout.fillWidth: true
            color: UISettings.sectionHeader
            Layout.columnSpan: 2

            RobotoText
            {
                x: 5
                anchors.verticalCenter: parent.verticalCenter
                label: qsTr("Global")
            }
        }

        // row 2
        RobotoText { label: qsTr("Grid size") }
        Rectangle
        {
            Layout.fillWidth: true
            height: UISettings.listItemHeight
            color: "transparent"

            CustomSpinBox
            {
                id: gWidthSpin
                width: parent.width * 0.45
                height: UISettings.listItemHeight
                from: 1
                to: 50
                value: View2D.gridSize.width
                onValueChanged:
                {
                    if (settingsRoot.visible)
                    View2D.gridSize = Qt.size(value, gHeightSpin.value)
                }
            }

            RobotoText
            {
                anchors.centerIn: parent
                label: "x"
            }

            CustomSpinBox
            {
                id: gHeightSpin
                x: parent.width - width
                width: parent.width * 0.45
                height: UISettings.listItemHeight
                from: 1
                to: 50
                value: View2D.gridSize.height
                onValueChanged:
                {
                    if (settingsRoot.visible)
                        View2D.gridSize = Qt.size(gWidthSpin.value, value)
                }
            }

        }

        // row 3
        RobotoText { label: qsTr("Grid units") }
        CustomComboBox
        {
            Layout.fillWidth: true
            height: UISettings.listItemHeight
            model: [ qsTr("Meters"), qsTr("Feet") ]
            onCurrentIndexChanged:
            {
                if (settingsRoot.visible)
                {
                    if(currentIndex == 1)
                        View2D.gridUnits = 304.8
                    else
                        View2D.gridUnits = 1000.0
                }
            }
        }

        // row 4
        Rectangle
        {
            Layout.fillWidth: true
            height: UISettings.listItemHeight
            color: UISettings.sectionHeader
            visible: fxPropsVisible
            Layout.columnSpan: 2

            RobotoText
            {
                x: 5
                anchors.verticalCenter: parent.verticalCenter
                label: qsTr("Selected fixtures")
            }
        }

        // row 5
        RobotoText { visible: fxPropsVisible; label: qsTr("Rotation") }
        CustomSpinBox
        {
            id: fxRotSpin
            visible: fxPropsVisible
            Layout.fillWidth: true
            height: UISettings.listItemHeight
            from: -359
            to: 359
            suffix: "°"
            value: contextManager.fixturesRotation.y
            onValueChanged:
            {
                if (settingsRoot.visible)
                    contextManager.fixturesRotation = Qt.vector3d(0.0, value, 0.0)
            }
        }

        RobotoText { visible: fxPropsVisible; label: qsTr("Alignment") }

        Row
        {
            Layout.fillWidth: true
            visible: fxPropsVisible

            IconButton
            {
                id: alignLeftBtn
                width: UISettings.iconSizeDefault
                height: width
                bgColor: UISettings.bgLighter
                imgSource: "qrc:/align-left.svg"
                tooltip: qsTr("Align the selected items to the left")
                onClicked: contextManager.setFixturesAlignment(Qt.AlignLeft)
            }
            IconButton
            {
                id: alignTopBtn
                width: UISettings.iconSizeDefault
                height: width
                bgColor: UISettings.bgLighter
                imgSource: "qrc:/align-top.svg"
                tooltip: qsTr("Align the selected items to the top")
                onClicked: contextManager.setFixturesAlignment(Qt.AlignTop)
            }
        }
    } // GridLayout
}
