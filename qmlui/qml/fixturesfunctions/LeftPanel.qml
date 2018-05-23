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

import org.qlcplus.classes 1.0
import "."

SidePanel
{
    id: leftSidePanel
    anchors.left: parent.left
    anchors.leftMargin: 0
    panelAlignment: Qt.AlignLeft

    onContentLoaded:
    {
        item.width = Qt.binding(function() { return leftSidePanel.width - collapseWidth })
        item.height = Qt.binding(function() { return leftSidePanel.height })
    }

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
                visible: qlcplus.accessMask & App.AC_FixtureEditing
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
                    y: UISettings.bigItemHeight
                    visible: false
                }
            }

            IconButton
            {
                objectName: "capShutter"
                z: 2
                width: iconSize
                height: iconSize
                imgSource: "qrc:/shutter.svg"
                checkable: true
                tooltip: qsTr("Shutter")
                counter: 0
                ButtonGroup.group: capabilitiesGroup

                onCheckedChanged: cShutterTool.visible = !cShutterTool.visible
                onCounterChanged: if (counter == 0) cShutterTool.visible = false

                PresetsTool
                {
                    id: cShutterTool
                    parent: mainView
                    x: leftSidePanel.width
                    y: UISettings.bigItemHeight
                    visible: false
                    onVisibleChanged: if (visible) updatePresets(fixtureManager.shutterChannels)
                    onPresetSelected: fixtureManager.setPresetValue(fxID, chIdx, value)
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
                    y: UISettings.bigItemHeight
                    visible: false
                    panMaxDegrees: posToolButton.panDegrees
                    tiltMaxDegrees: posToolButton.tiltDegrees
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
                    y: UISettings.bigItemHeight
                    visible: false
                    colorsMask: fixtureManager.colorsMask

                    onColorChanged: fixtureManager.setColorValue(r * 255, g * 255, b * 255, w * 255, a * 255, uv * 255)
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
                    y: UISettings.bigItemHeight
                    visible: false
                    onVisibleChanged: if (visible) updatePresets(fixtureManager.colorWheelChannels)
                    onPresetSelected: fixtureManager.setPresetValue(fxID, chIdx, value)
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
                    y: UISettings.bigItemHeight
                    visible: false
                    onVisibleChanged: if (visible) updatePresets(fixtureManager.goboChannels)
                    onPresetSelected: fixtureManager.setPresetValue(fxID, chIdx, value)
                }
            }

            IconButton
            {
                id: beamToolButton
                objectName: "capBeam"
                z: 2
                width: iconSize
                height: iconSize
                imgSource: "qrc:/beam.svg"
                checkable: true
                tooltip: qsTr("Beam")
                counter: 0
                ButtonGroup.group: capabilitiesGroup
                onCheckedChanged: beamTool.visible = !beamTool.visible
                onCounterChanged: if (counter == 0) beamTool.visible = false

                property real minBeamDegrees: 15.0
                property real maxBeamDegrees: 30.0

                BeamTool
                {
                    id: beamTool
                    parent: mainView
                    x: leftSidePanel.width
                    y: UISettings.bigItemHeight
                    visible: false
                    minDegrees: beamToolButton.minBeamDegrees
                    maxDegrees: beamToolButton.maxBeamDegrees
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
                property bool pickingActive: contextManager ? contextManager.positionPicking : false

                onPickingActiveChanged: checked = pickingActive

                visible: fixtureAndFunctions.currentView === "3D"
                z: 2
                width: iconSize
                height: iconSize
                checkable: true
                faSource: FontAwesome.fa_crosshairs
                tooltip: qsTr("Pick a 3D point") + " (CTRL+P)"
                onToggled: contextManager.positionPicking = checked
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
