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

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import org.qlcplus.classes 1.0
import "."

SidePanel
{
    id: leftSidePanel
    anchors.left: parent.left
    anchors.leftMargin: 0
    panelAlignment: Qt.AlignLeft

    onContentLoaded: (item, ID) =>
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
            z: 2
            spacing: 3

            IconButton
            {
                id: fxEditor
                visible: qlcplus.accessMask & App.AC_FixtureEditing
                width: iconSize
                height: iconSize
                imgSource: "qrc:/fixture.svg"
                tooltip: qsTr("Add Fixtures")
                ButtonGroup.group: fxManagerGroup
                autoExclusive: false
                onClicked:
                {
                    checked = !checked
                    if (checked === true)
                        loaderSource = "qrc:/FixtureBrowser.qml"
                    animatePanel(checked)
                }

                Image
                {
                    x: parent.width - width - 3
                    y: 3
                    width: parent.height / 3
                    height: width
                    source: "qrc:/add.svg"
                    sourceSize: Qt.size(width, height)
                }
            }

            IconButton
            {
                id: grpEditor
                width: iconSize
                height: iconSize
                imgSource: "qrc:/group.svg"
                tooltip: qsTr("Fixture Groups")
                ButtonGroup.group: fxManagerGroup
                autoExclusive: false
                onClicked:
                {
                    checked = !checked
                    if (checked === true)
                    {
                        loaderSource = "qrc:/FixtureGroupManager.qml"
                        fixtureManager.searchFilter = ""
                    }
                    animatePanel(checked)
                }
            }

            IconButton
            {
                id: paletteEditor
                width: iconSize
                height: iconSize
                imgSource: "qrc:/palette.svg"
                tooltip: qsTr("Palettes")
                ButtonGroup.group: fxManagerGroup
                autoExclusive: false
                onClicked:
                {
                    checked = !checked
                    if (checked === true)
                        loaderSource = "qrc:/PaletteManager.qml"
                    animatePanel(checked)
                }
            }

            IconButton
            {
                id: intToolButton
                objectName: "capIntensity"
                width: iconSize
                height: iconSize
                imgSource: "qrc:/intensity.svg"
                tooltip: qsTr("Intensity")
                counter: 0
                ButtonGroup.group: capabilitiesGroup

                onClicked:
                {
                    checked = !checked
                    if (checked)
                        intTool.setValue(contextManager.getCurrentValue(QLCChannel.Intensity, false))
                }
                onCounterChanged: if (counter == 0) intToolButton.checked = false

                IntensityTool
                {
                    id: intTool
                    parent: mainView
                    x: leftSidePanel.width
                    y: UISettings.bigItemHeight
                    visible: intToolButton.checked

                    onValueChanged:
                        function(value)
                        {
                            contextManager.setChannelValueByType(QLCChannel.Intensity, value, relativeValue)
                        }
                    onClose: intToolButton.checked = false
                }
            }

            IconButton
            {
                id: shutterToolButton
                objectName: "capShutter"
                width: iconSize
                height: iconSize
                imgSource: "qrc:/shutter.svg"
                tooltip: qsTr("Shutter")
                counter: 0
                ButtonGroup.group: capabilitiesGroup

                onClicked: checked = !checked
                onCounterChanged: if (counter == 0) shutterToolButton.checked = false

                PresetsTool
                {
                    id: cShutterTool
                    parent: mainView
                    x: leftSidePanel.width
                    y: UISettings.bigItemHeight
                    visible: shutterToolButton.checked
                    onVisibleChanged: if (visible) updatePresets(fixtureManager.shutterChannels)
                    onPresetSelected:
                        function(cap, fxID, chIdx, value)
                        {
                            fixtureManager.setPresetValue(fxID, chIdx, value)
                        }
                }
            }

            IconButton
            {
                id: posToolButton
                objectName: "capPosition"
                width: iconSize
                height: iconSize
                imgSource: "qrc:/position.svg"
                tooltip: qsTr("Position")
                counter: 0
                ButtonGroup.group: capabilitiesGroup

                onClicked: checked = !checked
                onCounterChanged: if (counter == 0) posToolButton.checked = false

                property alias panDegrees: posTool.panMaxDegrees
                property alias tiltDegrees: posTool.tiltMaxDegrees

                PositionTool
                {
                    id: posTool
                    parent: mainView
                    visible: posToolButton.checked
                    x: leftSidePanel.width
                    y: UISettings.bigItemHeight
                    onClose: posToolButton.checked = false
                }
            }

            IconButton
            {
                id: colorToolButton
                objectName: "capColor"
                width: iconSize
                height: iconSize
                imgSource: "qrc:/color.svg"
                tooltip: qsTr("Color")
                counter: 0
                ButtonGroup.group: capabilitiesGroup

                onClicked: checked = !checked
                onCounterChanged: if (counter == 0) colorToolButton.checked = false

                ColorTool
                {
                    id: colTool
                    parent: mainView
                    x: leftSidePanel.width
                    y: UISettings.bigItemHeight
                    visible: colorToolButton.checked
                    colorsMask: fixtureManager.colorsMask

                    onToolColorChanged:
                        function(r, g, b, w, a, uv)
                        {
                            contextManager.setColorValue(Qt.rgba(r, g, b, 1.0), Qt.rgba(w, a, uv, 1.0))
                        }
                    onClose: colorToolButton.checked = false
                }
            }

            IconButton
            {
                id: cWheelToolButton
                objectName: "capColorWheel"
                width: iconSize
                height: iconSize
                imgSource: "qrc:/colorwheel.svg"
                tooltip: qsTr("Color Wheel")
                counter: 0
                ButtonGroup.group: capabilitiesGroup

                onClicked: checked = !checked
                onCounterChanged: if (counter == 0) cWheelToolButton.checked = false

                PresetsTool
                {
                    id: cWheelTool
                    parent: mainView
                    x: leftSidePanel.width
                    y: UISettings.bigItemHeight
                    visible: cWheelToolButton.checked
                    onVisibleChanged: if (visible) updatePresets(fixtureManager.colorWheelChannels)
                    onPresetSelected:
                        function(cap, fxID, chIdx, value)
                        {
                            fixtureManager.setPresetValue(fxID, chIdx, value)
                        }
                }
            }

            IconButton
            {
                id: goboToolButton
                objectName: "capGobos"
                width: iconSize
                height: iconSize
                imgSource: "qrc:/gobo.svg"
                tooltip: qsTr("Gobos")
                counter: 0
                ButtonGroup.group: capabilitiesGroup

                onClicked: checked = !checked
                onCounterChanged: if (counter == 0) goboToolButton.checked = false

                PresetsTool
                {
                    id: gobosTool
                    parent: mainView
                    x: leftSidePanel.width
                    y: UISettings.bigItemHeight
                    visible: goboToolButton.checked
                    onVisibleChanged: if (visible) updatePresets(fixtureManager.goboChannels)
                    onPresetSelected:
                        function(cap, fxID, chIdx, value)
                        {
                            fixtureManager.setPresetValue(fxID, chIdx, value)
                        }
                }
            }

            IconButton
            {
                id: beamToolButton
                objectName: "capBeam"
                width: iconSize
                height: iconSize
                imgSource: "qrc:/beam.svg"
                tooltip: qsTr("Beam")
                counter: 0
                ButtonGroup.group: capabilitiesGroup

                onClicked: checked = !checked
                onCounterChanged: if (counter == 0) beamToolButton.checked = false

                function setZoomRange(min, max, inverted)
                {
                    beamTool.setZoomRange(min, max, inverted)
                }

                BeamTool
                {
                    id: beamTool
                    parent: mainView
                    x: leftSidePanel.width
                    y: UISettings.bigItemHeight
                    visible: beamToolButton.checked
                    onClose: beamToolButton.checked = false
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
                width: iconSize
                height: iconSize
                faSource: FontAwesome.fa_bolt
                tooltip: qsTr("Highlight")
                counter: contextManager.selectedFixturesCount
                onClicked: contextManager.highlightFixtureSelection()
            }

            IconButton
            {
                property bool pickingActive: contextManager ? contextManager.positionPicking : false

                onPickingActiveChanged: checked = pickingActive

                visible: fixtureAndFunctions.currentView === "3D"
                width: iconSize
                height: iconSize
                checkable: true
                checked: contextManager ? contextManager.positionPicking : false
                faSource: FontAwesome.fa_crosshairs
                tooltip: qsTr("Pick a 3D point") + " (CTRL+P)"
                onToggled: contextManager.positionPicking = checked
            }

            IconButton
            {
                width: iconSize
                height: iconSize
                imgSource: "qrc:/multiple.svg"
                tooltip: qsTr("Toggle multiple item selection")
                checkable: true
                checked: contextManager ? contextManager.multipleSelection : false
                onToggled: contextManager.multipleSelection = checked
            }

            IconButton
            {
                width: iconSize
                height: iconSize
                imgSource: "qrc:/selectall.svg"
                tooltip: qsTr("Select/Deselect all fixtures") + " (CTRL+A)"
                onClicked: contextManager.toggleFixturesSelection()
            }
        } // ColumnLayout
    } // Rectangle
}
