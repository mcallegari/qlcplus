/*
  Q Light Controller Plus
  SettingsView3D.qml

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
        x: 5
        width: settingsRoot.width - 10
        columns: 2
        columnSpacing: 5
        rowSpacing: 5

        // row 1
        Rectangle
        {
            visible: !fxPropsVisible
            height: UISettings.listItemHeight
            Layout.fillWidth: true
            color: "transparent"
            Layout.columnSpan: 2

            RobotoText
            {
                x: 5
                anchors.verticalCenter: parent.verticalCenter
                label: qsTr("Select some fixtures first")
            }
        }

        // row 1
        Rectangle
        {
            visible: fxPropsVisible
            height: UISettings.listItemHeight
            Layout.fillWidth: true
            color: UISettings.sectionHeader
            Layout.columnSpan: 2

            RobotoText
            {
                x: 5
                anchors.verticalCenter: parent.verticalCenter
                label: qsTr("Position")
            }
        }

        // row 2
        RobotoText { visible: fxPropsVisible; label: "X" }
        CustomSpinBox
        {
            id: fxXPos
            visible: fxPropsVisible
            height: UISettings.listItemHeight
            width: settingsRoot.width * 0.75
            from: -10000
            to: 100000
            suffix: "mm"
            value: contextManager.fixturesPosition.x * 1000
            onValueChanged:
            {
                if (settingsRoot.visible)
                    contextManager.fixturesPosition = Qt.vector3d(value, fxYPos.value, fxZPos.value)
            }
        }

        // row 3
        RobotoText { visible: fxPropsVisible; label: "Y" }
        CustomSpinBox
        {
            id: fxYPos
            visible: fxPropsVisible
            height: UISettings.listItemHeight
            width: settingsRoot.width * 0.75
            from: -10000
            to: 100000
            suffix: "mm"
            value: contextManager.fixturesPosition.y * 1000
            onValueChanged:
            {
                if (settingsRoot.visible)
                    contextManager.fixturesPosition = Qt.vector3d(fxXPos.value, value, fxZPos.value)
            }
        }

        // row 4
        RobotoText { visible: fxPropsVisible; label: "Z" }
        CustomSpinBox
        {
            id: fxZPos
            visible: fxPropsVisible
            height: UISettings.listItemHeight
            width: settingsRoot.width * 0.75
            from: -10000
            to: 100000
            suffix: "mm"
            value: contextManager.fixturesPosition.z * 1000
            onValueChanged:
            {
                if (settingsRoot.visible)
                    contextManager.fixturesPosition = Qt.vector3d(fxXPos.value, fxYPos.value, value)
            }
        }

        // row 5
        Rectangle
        {
            visible: fxPropsVisible
            height: UISettings.listItemHeight
            Layout.fillWidth: true
            color: UISettings.sectionHeader
            Layout.columnSpan: 2

            RobotoText
            {
                x: 5
                anchors.verticalCenter: parent.verticalCenter
                label: qsTr("Rotation")
            }
        }

        // row 6
        RobotoText { visible: fxPropsVisible; label: "X" }
        CustomSpinBox
        {
            id: fxXRot
            visible: fxPropsVisible
            height: UISettings.listItemHeight
            width: settingsRoot.width * 0.75
            from: -359
            to: 359
            suffix: "°"
            value: contextManager.fixturesRotation.x
            onValueChanged:
            {
                if (settingsRoot.visible)
                    contextManager.fixturesRotation = Qt.vector3d(value, fxYRot.value, fxZRot.value)
            }
        }

        // row 7
        RobotoText { visible: fxPropsVisible; label: "Y" }
        CustomSpinBox
        {
            id: fxYRot
            visible: fxPropsVisible
            height: UISettings.listItemHeight
            width: settingsRoot.width * 0.75
            from: -359
            to: 359
            suffix: "°"
            value: contextManager.fixturesRotation.y
            onValueChanged:
            {
                if (settingsRoot.visible)
                    contextManager.fixturesRotation = Qt.vector3d(fxXRot.value, value, fxZRot.value)
            }
        }

        // row 8
        RobotoText { visible: fxPropsVisible; label: "Z" }
        CustomSpinBox
        {
            id: fxZRot
            visible: fxPropsVisible
            height: UISettings.listItemHeight
            width: settingsRoot.width * 0.75
            from: -359
            to: 359
            suffix: "°"
            value: contextManager.fixturesRotation.z
            onValueChanged:
            {
                if (settingsRoot.visible)
                    contextManager.fixturesRotation = Qt.vector3d(fxXRot.value, fxYRot.value, value)
            }
        }
    }
}
