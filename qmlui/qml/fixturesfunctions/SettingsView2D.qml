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
    width: 300
    height: parent.height

    color: UISettings.bgMedium
    border.width: 1
    border.color: "#222"

    property bool fxPropsVisible: contextManager.hasSelectedFixtures

    GridLayout
    {
        columns: 2
        columnSpacing: 5
        rowSpacing: 5

        // row 1
        Rectangle
        {
            width: settingsRoot.width
            height: 38
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
            width: 180
            height: 40
            color: "transparent"
            Row
            {
                spacing: 3
                CustomSpinBox
                {
                    id: gWidthSpin
                    width: 80
                    height: 38
                    minimumValue: 1
                    maximumValue: 50
                    decimals: 0
                    value: View2D.gridSize.width
                    onValueChanged:
                    {
                        if (settingsRoot.visible)
                        View2D.gridSize = Qt.size(value, gHeightSpin.value)
                    }
                }
                RobotoText { label: "x" }
                CustomSpinBox
                {
                    id: gHeightSpin
                    width: 80
                    height: 38
                    minimumValue: 1
                    maximumValue: 50
                    decimals: 0
                    value: View2D.gridSize.height
                    onValueChanged:
                    {
                        if (settingsRoot.visible)
                            View2D.gridSize = Qt.size(gWidthSpin.value, value)
                    }
                }
            }
        }

        // row 3
        RobotoText { label: qsTr("Grid units") }
        CustomComboBox
        {
            width: 120
            height: 38
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
            width: settingsRoot.width
            height: 38
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
            width: 80
            height: 38
            minimumValue: -359
            maximumValue: 359
            decimals: 0
            value: contextManager.fixturesRotation
            onValueChanged:
            {
                if (settingsRoot.visible)
                    contextManager.fixturesRotation = value
            }
        }
    } // GridLayout
}
