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

    property vector3d fxPosition: contextManager.fixturesPosition
    property vector3d fxRotation: contextManager.fixturesRotation

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
            visible: fxPropsVisible
            height: UISettings.listItemHeight
            Layout.fillWidth: true
            from: -10000
            to: 100000
            suffix: "mm"
            value: fxPosition.x
            onValueChanged:
            {
                if (settingsRoot.visible)
                    contextManager.fixturesPosition = Qt.vector3d(value, fxPosition.y, fxPosition.z)
            }
        }

        // row 3
        RobotoText { visible: fxPropsVisible; label: "Y" }
        CustomSpinBox
        {
            visible: fxPropsVisible
            height: UISettings.listItemHeight
            Layout.fillWidth: true
            from: -10000
            to: 100000
            suffix: "mm"
            value: fxPosition.y
            onValueChanged:
            {
                if (settingsRoot.visible)
                    contextManager.fixturesPosition = Qt.vector3d(fxPosition.x, value, fxPosition.z)
            }
        }

        // row 4
        RobotoText { visible: fxPropsVisible; label: "Z" }
        CustomSpinBox
        {
            visible: fxPropsVisible
            height: UISettings.listItemHeight
            Layout.fillWidth: true
            from: -10000
            to: 100000
            suffix: "mm"
            value: fxPosition.z
            onValueChanged:
            {
                if (settingsRoot.visible)
                    contextManager.fixturesPosition = Qt.vector3d(fxPosition.x, fxPosition.y, value)
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
            visible: fxPropsVisible
            height: UISettings.listItemHeight
            Layout.fillWidth: true
            from: -359
            to: 359
            suffix: "°"
            value: fxRotation.x
            onValueChanged:
            {
                if (settingsRoot.visible)
                    contextManager.fixturesRotation = Qt.vector3d(value, fxRotation.y, fxRotation.y)
            }
        }

        // row 7
        RobotoText { visible: fxPropsVisible; label: "Y" }
        CustomSpinBox
        {
            visible: fxPropsVisible
            height: UISettings.listItemHeight
            Layout.fillWidth: true
            from: -359
            to: 359
            suffix: "°"
            value: fxRotation.y
            onValueChanged:
            {
                if (settingsRoot.visible)
                    contextManager.fixturesRotation = Qt.vector3d(fxRotation.x, value, fxRotation.z)
            }
        }

        // row 8
        RobotoText { visible: fxPropsVisible; label: "Z" }
        CustomSpinBox
        {
            visible: fxPropsVisible
            height: UISettings.listItemHeight
            Layout.fillWidth: true
            from: -359
            to: 359
            suffix: "°"
            value: fxRotation.z
            onValueChanged:
            {
                if (settingsRoot.visible)
                    contextManager.fixturesRotation = Qt.vector3d(fxRotation.x, fxRotation.y, value)
            }
        }
    }
}
