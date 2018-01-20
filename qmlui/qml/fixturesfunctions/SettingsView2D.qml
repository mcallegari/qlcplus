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

import org.qlcplus.classes 1.0
import "."

Rectangle
{
    id: settingsRoot
    width: mainView.width / 5
    height: parent.height

    color: UISettings.bgMedium
    border.width: 1
    border.color: "#222"

    property vector3d envSize: contextManager ? contextManager.environmentSize : Qt.vector3d(0, 0, 0)
    property int selFixturesCount: contextManager ? contextManager.selectedFixturesCount : 0
    property bool fxPropsVisible: selFixturesCount ? true : false
    property vector3d fxRotation: selFixturesCount === 1 ? contextManager.fixturesRotation : lastRotation
    property vector3d lastRotation

    onSelFixturesCountChanged:
    {
        if (selFixturesCount > 1)
            lastRotation = Qt.vector3d(0, 0, 0)
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
                label: qsTr("Environment")
            }
        }

        // row 2
        RobotoText { label: qsTr("Width") }
        CustomSpinBox
        {
            height: UISettings.listItemHeight
            Layout.fillWidth: true
            from: 1
            to: 50
            suffix: View2D.gridUnits === MonitorProperties.Meters ? "m" : "ft"
            value: envSize.x
            onValueChanged:
            {
                if (settingsRoot.visible)
                    contextManager.environmentSize = Qt.vector3d(value, envSize.y, envSize.z)
            }
        }

        // row 3
        RobotoText { label: qsTr("Height") }
        CustomSpinBox
        {
            height: UISettings.listItemHeight
            Layout.fillWidth: true
            from: 1
            to: 50
            suffix: View2D.gridUnits === MonitorProperties.Meters ? "m" : "ft"
            value: envSize.y
            onValueChanged:
            {
                if (settingsRoot.visible)
                    contextManager.environmentSize = Qt.vector3d(envSize.x, value, envSize.z)
            }
        }

        // row 4
        RobotoText { label: qsTr("Depth") }
        CustomSpinBox
        {
            height: UISettings.listItemHeight
            Layout.fillWidth: true
            from: 1
            to: 100
            suffix: View2D.gridUnits === MonitorProperties.Meters ? "m" : "ft"
            value: envSize.z
            onValueChanged:
            {
                if (settingsRoot.visible)
                    contextManager.environmentSize = Qt.vector3d(envSize.x, envSize.y, value)
            }
        }

        // row 5
        RobotoText { label: qsTr("Grid units") }
        CustomComboBox
        {
            ListModel
            {
                id: unitsModel
                ListElement { mLabel: qsTr("Meters"); mValue: MonitorProperties.Meters }
                ListElement { mLabel: qsTr("Feet"); mValue: MonitorProperties.Feet }
            }

            Layout.fillWidth: true
            height: UISettings.listItemHeight
            model: unitsModel
            currentIndex: View2D.gridUnits
            onCurrentIndexChanged:
            {
                if (settingsRoot.visible)
                    View2D.gridUnits = currentIndex
            }
        }

        // row 6
        RobotoText { label: qsTr("Point of view") }
        CustomComboBox
        {
            ListModel
            {
                id: povModel
                ListElement { mLabel: qsTr("Top view"); mValue: MonitorProperties.TopView }
                ListElement { mLabel: qsTr("Front view"); mValue: MonitorProperties.FrontView }
                ListElement { mLabel: qsTr("Right side view"); mValue: MonitorProperties.RightSideView }
                ListElement { mLabel: qsTr("Left side view"); mValue: MonitorProperties.LeftSideView }
            }

            Layout.fillWidth: true
            height: UISettings.listItemHeight
            model: povModel
            currentIndex: View2D.pointOfView - 1
            onCurrentIndexChanged:
            {
                if (settingsRoot.visible)
                    View2D.pointOfView = currentIndex + 1
            }
        }

        // row 7
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

        // row 8
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
            value: fxRotation.y
            onValueChanged: updateRotation(fxRotation.x, value, fxRotation.z)
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

        RobotoText { visible: fxPropsVisible; label: qsTr("Distribution") }

        Row
        {
            Layout.fillWidth: true
            visible: fxPropsVisible

            IconButton
            {
                id: distributeXBtn
                width: UISettings.iconSizeDefault
                height: width
                bgColor: UISettings.bgLighter
                imgSource: "qrc:/distribute-x.svg"
                tooltip: qsTr("Equally distribute horizontally the selected items")
                onClicked: contextManager.setFixturesDistribution(Qt.Horizontal)
            }
            IconButton
            {
                id: distributeYBtn
                width: UISettings.iconSizeDefault
                height: width
                bgColor: UISettings.bgLighter
                imgSource: "qrc:/distribute-y.svg"
                tooltip: qsTr("Equally distribute vertically the selected items")
                onClicked: contextManager.setFixturesDistribution(Qt.Vertical)
            }
        }
    } // GridLayout
}
