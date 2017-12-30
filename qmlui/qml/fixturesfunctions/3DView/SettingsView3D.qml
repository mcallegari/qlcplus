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

    property vector3d envSize: contextManager.environmentSize
    property real ambientIntensity: View3D.ambientIntensity

    property int selFixturesCount: contextManager.selectedFixturesCount
    property bool fxPropsVisible: selFixturesCount ? true : false
    property vector3d fxPosition: selFixturesCount === 1 ? contextManager.fixturesPosition : lastPosition
    property vector3d lastPosition
    property vector3d fxRotation: selFixturesCount === 1 ? contextManager.fixturesRotation : lastRotation
    property vector3d lastRotation

    onSelFixturesCountChanged:
    {
        if (selFixturesCount > 1)
        {
            lastPosition = Qt.vector3d(0, 0, 0)
            lastRotation = Qt.vector3d(0, 0, 0)
        }
    }

    function updatePosition(x, y, z)
    {
        if (visible == false)
            return;

        if (selFixturesCount == 1)
        {
            contextManager.fixturesPosition = Qt.vector3d(x, y, z)
        }
        else
        {
            contextManager.fixturesPosition = Qt.vector3d(x - lastPosition.x, y - lastPosition.y, z - lastPosition.z)
            lastPosition = Qt.vector3d(x, y, z)
        }
    }

    function updateRotation(x, y, z)
    {
        if (visible == false)
            return;

        if (selFixturesCount == 1)
        {
            contextManager.fixturesRotation = Qt.vector3d(x, y, z)
        }
        else
        {
            contextManager.fixturesRotation = Qt.vector3d(x - lastRotation.x, y - lastRotation.y, z - lastRotation.z)
            lastRotation = Qt.vector3d(x, y, z)
        }
    }

    GridLayout
    {
        x: 5
        width: settingsRoot.width - 10
        columns: 2
        columnSpacing: 5
        rowSpacing: 0

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
                label: qsTr("Environment")
            }
        }

        // row 2
        RobotoText { label: qsTr("Type") }
        CustomComboBox
        {
            Layout.fillWidth: true
            height: UISettings.listItemHeight

            model: View3D.stagesList
            currentIndex: View3D.stageIndex
            onCurrentIndexChanged: View3D.stageIndex = currentIndex
        }

        // row 3
        RobotoText { label: qsTr("Width") }
        CustomSpinBox
        {
            height: UISettings.listItemHeight
            Layout.fillWidth: true
            from: 1
            to: 50
            suffix: "m"
            value: envSize.x
            onValueChanged:
            {
                if (settingsRoot.visible)
                    contextManager.environmentSize = Qt.vector3d(value, envSize.y, envSize.z)
            }
        }

        // row 4
        RobotoText { label: qsTr("Height") }
        CustomSpinBox
        {
            height: UISettings.listItemHeight
            Layout.fillWidth: true
            from: 1
            to: 50
            suffix: "m"
            value: envSize.y
            onValueChanged:
            {
                if (settingsRoot.visible)
                    contextManager.environmentSize = Qt.vector3d(envSize.x, value, envSize.z)
            }
        }

        // row 5
        RobotoText { label: qsTr("Depth") }
        CustomSpinBox
        {
            height: UISettings.listItemHeight
            Layout.fillWidth: true
            from: 1
            to: 100
            suffix: "m"
            value: envSize.z
            onValueChanged:
            {
                if (settingsRoot.visible)
                    contextManager.environmentSize = Qt.vector3d(envSize.x, envSize.y, value)
            }
        }

        // row 6
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
                label: qsTr("Ambient light")
            }
        }

        // row 7
        RobotoText { label: qsTr("Intensity") }
        CustomSpinBox
        {
            Layout.fillWidth: true
            height: UISettings.listItemHeight
            from: 0
            to: 100
            suffix: "%"
            value: ambientIntensity * 100
            onValueChanged: View3D.ambientIntensity = value / 100
        }

        // row 8
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

        // row 9
        RobotoText { visible: fxPropsVisible; label: "X" }
        CustomSpinBox
        {
            visible: fxPropsVisible
            height: UISettings.listItemHeight
            Layout.fillWidth: true
            from: -10000
            to: 100000
            stepSize: 10
            suffix: "mm"
            value: fxPosition.x
            onValueChanged: updatePosition(value, fxPosition.y, fxPosition.z)
        }

        // row 10
        RobotoText { visible: fxPropsVisible; label: "Y" }
        CustomSpinBox
        {
            visible: fxPropsVisible
            height: UISettings.listItemHeight
            Layout.fillWidth: true
            from: -10000
            to: 100000
            stepSize: 10
            suffix: "mm"
            value: fxPosition.y
            onValueChanged: updatePosition(fxPosition.x, value, fxPosition.z)
        }

        // row 11
        RobotoText { visible: fxPropsVisible; label: "Z" }
        CustomSpinBox
        {
            visible: fxPropsVisible
            height: UISettings.listItemHeight
            Layout.fillWidth: true
            from: -10000
            to: 100000
            stepSize: 10
            suffix: "mm"
            value: fxPosition.z
            onValueChanged: updatePosition(fxPosition.x, fxPosition.y, value)
        }

        // row 12
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

        // row 13
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
            onValueChanged: updateRotation(value, fxRotation.y, fxRotation.z)
        }

        // row 14
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
            onValueChanged: updateRotation(fxRotation.x, value, fxRotation.z)
        }

        // row 15
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
            onValueChanged: updateRotation(fxRotation.x, fxRotation.y, value)
        }
    }
}
