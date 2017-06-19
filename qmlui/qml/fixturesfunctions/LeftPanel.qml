/*
  Q Light Controller Plus
  LeftPanel.qml

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
import QtQuick.Layouts 1.0
import QtQuick.Controls 2.2

import "."

SidePanel
{
    id: leftSidePanel
    anchors.left: parent.left
    anchors.leftMargin: 0
    panelAlignment: Qt.AlignLeft

    Rectangle
    {
        id: sideBar
        x: parent.width - collapseWidth
        width: collapseWidth
        height: parent.height
        color: "transparent"
        z: 2

        ButtonGroup { id: fxManagerGroup }
        ButtonGroup { id: capabilitiesGroup }

        ColumnLayout
        {
            anchors.horizontalCenter: parent.horizontalCenter
            height: parent.height
            width: iconSize
            spacing: 3

            IconButton
            {
                id: fxEditor
                z: 2
                width: iconSize
                height: iconSize
                imgSource: "qrc:/fixture.svg"
                checkable: true
                tooltip: qsTr("Add Fixtures")
                ButtonGroup.group: fxManagerGroup
                autoExclusive: false
                onToggled:
                {
                    if (checked == true)
                        loaderSource = "qrc:/FixtureBrowser.qml"
                    animatePanel(checked)
                }
            }

            IconButton
            {
                id: grpEditor
                z: 2
                width: iconSize
                height: iconSize
                imgSource: "qrc:/group.svg"
                checkable: true
                tooltip: qsTr("Fixture Groups")
                ButtonGroup.group: fxManagerGroup
                autoExclusive: false
                onToggled:
                {
                    if (checked == true)
                        loaderSource = "qrc:/FixtureGroupManager.qml"
                    animatePanel(checked)
                }
            }

            IconButton
            {
                objectName: "capIntensity"
                z: 2
                width: iconSize
                height: iconSize
                imgSource: "qrc:/intensity.svg"
                checkable: true
                tooltip: qsTr("Intensity")
                counter: 0
                ButtonGroup.group: capabilitiesGroup
                onCheckedChanged: intTool.visible = !intTool.visible
                onCounterChanged: if (counter == 0) intTool.visible = false

                IntensityTool
                {
                    id: intTool
                    parent: mainView
                    x: leftSidePanel.width
                    y: mainToolbar.height + 40
                    visible: false
                }
            }

            IconButton
            {
                objectName: "capColor"
                z: 2
                width: iconSize
                height: iconSize
                imgSource: "qrc:/color.svg"
                checkable: true
                tooltip: qsTr("Color")
                counter: 0
                ButtonGroup.group: capabilitiesGroup
                onCheckedChanged: colTool.visible = !colTool.visible
                onCounterChanged: if (counter == 0) colTool.visible = false

                ColorTool
                {
                    id: colTool
                    parent: mainView
                    x: leftSidePanel.width
                    y: mainToolbar.height + 40
                    visible: false
                    colorsMask: fixtureManager.colorsMask

                    onColorChanged: fixtureManager.setColorValue(r * 255, g * 255, b * 255, w, a, uv)
                }
            }

            IconButton
            {
                id: posToolButton
                objectName: "capPosition"
                z: 2
                width: iconSize
                height: iconSize
                imgSource: "qrc:/position.svg"
                checkable: true
                tooltip: qsTr("Position")
                counter: 0
                ButtonGroup.group: capabilitiesGroup
                onCheckedChanged: posTool.visible = !posTool.visible
                onCounterChanged: if (counter == 0) posTool.visible = false

                property int panDegrees: 360
                property int tiltDegrees: 270

                PositionTool
                {
                    id: posTool
                    parent: mainView
                    x: leftSidePanel.width
                    y: mainToolbar.height + 40
                    visible: false
                    panMaxDegrees: posToolButton.panDegrees
                    tiltMaxDegrees: posToolButton.tiltDegrees
                }
            }

            IconButton
            {
                objectName: "capColorWheel"
                z: 2
                width: iconSize
                height: iconSize
                imgSource: "qrc:/colorwheel.svg"
                checkable: true
                tooltip: qsTr("Color Wheel")
                counter: 0
                ButtonGroup.group: capabilitiesGroup

                onCheckedChanged: cWheelTool.visible = !cWheelTool.visible
                onCounterChanged: if (counter == 0) cWheelTool.visible = false

                PresetsTool
                {
                    id: cWheelTool
                    parent: mainView
                    x: leftSidePanel.width
                    y: mainToolbar.height + 40
                    visible: false
                }
            }

            IconButton
            {
                objectName: "capGobos"
                z: 2
                width: iconSize
                height: iconSize
                imgSource: "qrc:/gobo.svg"
                checkable: true
                tooltip: qsTr("Gobos")
                counter: 0
                ButtonGroup.group: capabilitiesGroup

                onCheckedChanged: gobosTool.visible = !gobosTool.visible
                onCounterChanged: if (counter == 0) gobosTool.visible = false

                PresetsTool
                {
                    id: gobosTool
                    parent: mainView
                    x: leftSidePanel.width
                    y: mainToolbar.height + 40
                    visible: false
                    goboPresets: true
                }
            }

            /* filler object */
            Rectangle
            {
                Layout.fillHeight: true
                width: iconSize
                color: "transparent"
            }

            IconButton
            {
                z: 2
                width: iconSize
                height: iconSize
                imgSource: "qrc:/selectall.svg"
                tooltip: qsTr("Select/Deselect all fixtures") + " (CTRL+A)"
                onClicked: contextManager.toggleFixturesSelection()
            }
        }
    }
}
