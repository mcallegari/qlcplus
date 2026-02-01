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

import QtQuick
import QtQuick.Layouts
import QtQuick.Dialogs

import org.qlcplus.classes 1.0
import "."

Rectangle
{
    id: settingsRoot
    width: mainView.width / 5
    height: parent.height

    color: UISettings.bgStrong
    border.width: 1
    border.color: UISettings.bgStrong

    property vector3d envSize: contextManager ? contextManager.environmentSize : Qt.vector3d(0, 0, 0)
    property int selFixturesCount: contextManager ? contextManager.selectedFixturesCount : 0
    property bool fxPropsVisible: selFixturesCount ? true : false
    property vector3d fxRotation: selFixturesCount === 1 ? contextManager.fixturesRotation : lastRotation
    property vector3d lastRotation
    property int previousGridUnits: MonitorProperties.Meters

    Component.onCompleted:
    {
        if (View2D)
            previousGridUnits = View2D.gridUnits
    }

    onSelFixturesCountChanged:
    {
        if (selFixturesCount > 1)
            lastRotation = Qt.vector3d(0, 0, 0)
    }

    function updateRotation(degrees)
    {
        if (visible === false)
            return;

        var rot
        switch (View2D.pointOfView)
        {
            case MonitorProperties.LeftSideView:
            case MonitorProperties.RightSideView:
                rot = Qt.vector3d(degrees, fxRotation.y, fxRotation.z)
            break;
            case MonitorProperties.FrontView:
                rot = Qt.vector3d(fxRotation.x, fxRotation.y, degrees)
            break;
            default:
                rot = Qt.vector3d(fxRotation.x, degrees, fxRotation.z)
            break;
        }

        if (selFixturesCount == 1)
        {
            contextManager.fixturesRotation = Qt.vector3d(rot.x, rot.y, rot.z)
        }
        else
        {
            contextManager.fixturesRotation = Qt.vector3d(rot.x - lastRotation.x, rot.y - lastRotation.y, rot.z - lastRotation.z)
            lastRotation = Qt.vector3d(rot.x, rot.y, rot.z)
        }
    }

    ColorTool
    {
        id: gelColorTool
        parent: mainView
        x: rightPanel.x - settingsRoot.width - width
        y: UISettings.bigItemHeight
        visible: false

        onToolColorChanged:
            function(r, g, b, w, a, uv)
            {
                contextManager.setFixturesGelColor(Qt.rgba(r, g, b, 1.0))
            }
        onClose: visible = false
    }

    Column
    {
        id: settingsColumn
        x: 5
        width: parent.width - 10
        spacing: 2

        SectionBox
        {
            width: parent.width
            sectionLabel: qsTr("Environment")
            sectionContents:
                GridLayout
                {
                    width: settingsRoot.width
                    columns: 2
                    columnSpacing: 5
                    rowSpacing: 5

                    // row 1
                    RobotoText { height: UISettings.listItemHeight; label: qsTr("Width") }
                    CustomSpinBox
                    {
                        height: UISettings.listItemHeight
                        Layout.fillWidth: true
                        from: 1
                        to: 50
                        suffix: View2D.gridUnits === MonitorProperties.Meters ? "m" : "ft"
                        value: envSize.x
                        onValueModified:
                        {
                            if (settingsRoot.visible && contextManager)
                                contextManager.environmentSize = Qt.vector3d(value, envSize.y, envSize.z)
                        }
                    }

                    // row 2
                    RobotoText { height: UISettings.listItemHeight; label: qsTr("Height") }
                    CustomSpinBox
                    {
                        height: UISettings.listItemHeight
                        Layout.fillWidth: true
                        from: 1
                        to: 50
                        suffix: View2D.gridUnits === MonitorProperties.Meters ? "m" : "ft"
                        value: envSize.y
                        onValueModified:
                        {
                            if (settingsRoot.visible && contextManager)
                                contextManager.environmentSize = Qt.vector3d(envSize.x, value, envSize.z)
                        }
                    }

                    // row 3
                    RobotoText { height: UISettings.listItemHeight; label: qsTr("Depth") }
                    CustomSpinBox
                    {
                        height: UISettings.listItemHeight
                        Layout.fillWidth: true
                        from: 1
                        to: 100
                        suffix: View2D.gridUnits === MonitorProperties.Meters ? "m" : "ft"
                        value: envSize.z
                        onValueModified:
                        {
                            if (settingsRoot.visible && contextManager)
                                contextManager.environmentSize = Qt.vector3d(envSize.x, envSize.y, value)
                        }
                    }

                    // row 4
                    RobotoText { height: UISettings.listItemHeight; label: qsTr("Grid units") }
                    CustomComboBox
                    {
                        Layout.fillWidth: true
                        height: UISettings.listItemHeight
                        model: [
                            { mLabel: qsTr("Meters"), mValue: MonitorProperties.Meters },
                            { mLabel: qsTr("Feet"), mValue: MonitorProperties.Feet }
                        ]
                        currentIndex: View2D.gridUnits
                        onCurrentIndexChanged:
                        {
                            if (settingsRoot.visible === false || View2D === null || contextManager === null)
                            {
                                if (View2D)
                                    View2D.gridUnits = currentIndex
                                previousGridUnits = currentIndex
                                return
                            }

                            if (currentIndex !== previousGridUnits)
                            {
                                var factor = 1.0
                                if (previousGridUnits === MonitorProperties.Meters &&
                                    currentIndex === MonitorProperties.Feet)
                                {
                                    factor = 3.280839895
                                }
                                else if (previousGridUnits === MonitorProperties.Feet &&
                                         currentIndex === MonitorProperties.Meters)
                                {
                                    factor = 0.3048
                                }

                                if (factor !== 1.0)
                                {
                                    var newSize = Qt.vector3d(
                                                Math.round(envSize.x * factor),
                                                Math.round(envSize.y * factor),
                                                Math.round(envSize.z * factor))
                                    contextManager.environmentSize = newSize
                                }
                            }

                            View2D.gridUnits = currentIndex
                            previousGridUnits = currentIndex
                        }
                    }

                    // row 5
                    RobotoText { height: UISettings.listItemHeight; label: qsTr("Point of view") }
                    CustomComboBox
                    {
                        Layout.fillWidth: true
                        height: UISettings.listItemHeight
                        model: [
                            { mLabel: qsTr("Top view"), mValue: MonitorProperties.TopView },
                            { mLabel: qsTr("Front view"), mValue: MonitorProperties.FrontView },
                            { mLabel: qsTr("Right side view"), mValue: MonitorProperties.RightSideView },
                            { mLabel: qsTr("Left side view"), mValue: MonitorProperties.LeftSideView }
                        ]
                        currentIndex: View2D.pointOfView - 1
                        onCurrentIndexChanged:
                        {
                            if (settingsRoot.visible && View2D)
                                View2D.pointOfView = currentIndex + 1
                        }
                    }
                } // GridLayout
        } // SectionBox

        SectionBox
        {
            width: parent.width
            sectionLabel: qsTr("Custom Background")
            sectionContents:
                GridLayout
                {
                    IconButton
                    {
                        id: imgButton
                        width: UISettings.iconSizeMedium
                        height: width
                        faSource: FontAwesome.fa_image
                        faColor: "lightyellow"
                        tooltip: qsTr("Set a custom background")

                        onClicked: fileDialog.open()

                        FileDialog
                        {
                            id: fileDialog
                            visible: false
                            title: qsTr("Select an image")
                            nameFilters: [ "Image files (*.png *.bmp *.jpg *.jpeg *.gif *.svg)", "All files (*)" ]

                            onAccepted:
                            {
                                View2D.backgroundImage = fileDialog.selectedFile
                            }
                        }
                    }

                    CustomTextEdit
                    {
                        Layout.fillWidth: true
                        height: UISettings.iconSizeMedium
                        text: View2D.backgroundImage
                    }

                    IconButton
                    {
                        z: 2
                        width: UISettings.iconSizeMedium
                        height: width
                        faSource: FontAwesome.fa_xmark
                        tooltip: qsTr("Reset background")
                        onClicked: View2D.backgroundImage = ""
                    }
                }
        }

        SectionBox
        {
            visible: fxPropsVisible
            width: parent.width
            isExpanded: fxPropsVisible
            sectionLabel: qsTr("Selected fixtures")

            sectionContents:
                GridLayout
                {
                    width: parent.width
                    columns: 2
                    columnSpacing: 5
                    rowSpacing: 2

                    // row 1
                    RobotoText
                    {
                        visible: contextManager.selectedDimmersCount
                        height: UISettings.listItemHeight
                        label: qsTr("Gel color")
                    }
                    Rectangle
                    {
                        visible: contextManager.selectedDimmersCount
                        Layout.fillWidth: true
                        height: UISettings.listItemHeight
                        color: gelColorTool.currentRGB

                        MouseArea
                        {
                            anchors.fill: parent
                            onClicked: gelColorTool.visible = !gelColorTool.visible
                        }
                    }

                    // row 2
                    RobotoText
                    {
                        visible: contextManager.selectedDimmersCount
                        height: UISettings.listItemHeight
                        label: qsTr("Fixed zoom")
                    }

                    CustomSpinBox
                    {
                        visible: contextManager.selectedDimmersCount
                        Layout.fillWidth: true
                        height: UISettings.listItemHeight
                        from: 0
                        to: 180
                        suffix: "°"
                        onValueModified: contextManager.setFixedZoom(value)
                    }

                    // row 3
                    RobotoText
                    {
                        height: UISettings.listItemHeight
                        label: qsTr("Rotation")
                    }
                    CustomSpinBox
                    {
                        id: fxRotSpin
                        Layout.fillWidth: true
                        height: UISettings.listItemHeight
                        from: -359
                        to: 359
                        suffix: "°"
                        value:
                        {
                            switch (View2D.pointOfView)
                            {
                                case MonitorProperties.LeftSideView:
                                case MonitorProperties.RightSideView:
                                    return fxRotation.x

                                case MonitorProperties.FrontView:
                                    return fxRotation.z

                                default:
                                    return fxRotation.y
                            }
                        }
                        onValueModified: updateRotation(value)
                    }

                    // row 4
                    RobotoText
                    {
                        height: UISettings.listItemHeight;
                        label: qsTr("Alignment")
                    }

                    Row
                    {
                        Layout.fillWidth: true

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

                    // row 5
                    RobotoText
                    {
                        height: UISettings.listItemHeight;
                        label: qsTr("Distribution")
                    }

                    Row
                    {
                        Layout.fillWidth: true

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
        } // SectionBox
    } // Column
}
