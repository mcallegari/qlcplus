/*
  Q Light Controller Plus
  VCClockProperties.qml

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
import QtQuick.Controls 2.1

import org.qlcplus.classes 1.0
import "."

Rectangle
{
    color: "transparent"
    height: cPropsColumn.height

    property VCClock widgetRef: null

    property int gridItemsHeight: UISettings.listItemHeight

    Column
    {
        id: cPropsColumn
        width: parent.width
        spacing: 5

        SectionBox
        {
            id: clockTypeProps
            sectionLabel: qsTr("Clock type")

            sectionContents:
              GridLayout
              {
                width: parent.width
                columns: 2
                columnSpacing: 5
                rowSpacing: 4

                ButtonGroup { id: clockTypeGroup }

                // row 1
                RobotoText
                {
                    height: gridItemsHeight
                    Layout.fillWidth: true
                    label: qsTr("Clock")
                }

                CustomCheckBox
                {
                    implicitWidth: UISettings.iconSizeMedium
                    implicitHeight: implicitWidth
                    ButtonGroup.group: clockTypeGroup
                    checked: widgetRef ? widgetRef.clockType === VCClock.Clock : false
                    onClicked: if (checked && widgetRef) widgetRef.clockType = VCClock.Clock
                }

                // row 2
                RobotoText
                {
                    height: gridItemsHeight
                    Layout.fillWidth: true
                    label: qsTr("Stopwatch")
                }

                CustomCheckBox
                {
                    implicitWidth: UISettings.iconSizeMedium
                    implicitHeight: implicitWidth
                    ButtonGroup.group: clockTypeGroup
                    checked: widgetRef ? widgetRef.clockType === VCClock.Stopwatch : false
                    onClicked: if (checked && widgetRef) widgetRef.clockType = VCClock.Stopwatch
                }

                // row 3
                RobotoText
                {
                    height: gridItemsHeight
                    Layout.fillWidth: true
                    label: qsTr("Countdown")
                }

                CustomCheckBox
                {
                    id: cdownCheck
                    implicitWidth: UISettings.iconSizeMedium
                    implicitHeight: implicitWidth
                    ButtonGroup.group: clockTypeGroup
                    checked: widgetRef ? widgetRef.clockType === VCClock.Countdown : false
                    onClicked: if (checked && widgetRef) widgetRef.clockType = VCClock.Countdown
                }

                DayTimeTool
                {
                    visible: cdownCheck.checked
                    Layout.columnSpan: 2
                    timeValue: widgetRef ? widgetRef.targetTime / 1000 : 0

                    onTimeValueChanged: widgetRef.targetTime = timeValue * 1000
                }
              } // GridLayout
        } // SectionBox

        SectionBox
        {
            id: scheduleProps
            sectionLabel: qsTr("Schedule")

            sectionContents:
                Column
                {
                    width: parent.width

                    Rectangle
                    {
                        color: UISettings.bgMedium
                        width: parent.width
                        height: UISettings.listItemHeight

                        IconButton
                        {
                            id: addSchedule
                            anchors.top: parent.top
                            anchors.right: parent.right

                            width: height
                            height: parent.height
                            imgSource: "qrc:/add.svg"
                            checkable: true
                            tooltip: qsTr("Add a function schedule")
                            onCheckedChanged:
                            {
                                if (checked)
                                {
                                    if (!sideLoader.visible)
                                        rightSidePanel.width += UISettings.sidePanelWidth
                                    sideLoader.visible = true
                                    sideLoader.source = "qrc:/FunctionManager.qml"
                                }
                                else
                                {
                                    rightSidePanel.width -= sideLoader.width
                                    sideLoader.source = ""
                                    sideLoader.visible = false
                                }
                            }
                        }
                    }

                    ListView
                    {
                        id: schListView
                        width: parent.width
                        height: model ? model.length * 180 : 0
                        boundsBehavior: Flickable.StopAtBounds

                        model: widgetRef ? widgetRef.scheduleList : null
                        delegate:
                            Rectangle
                            {
                                width: parent.width
                                height: 180
                                color: "transparent"

                                property VCClockSchedule schedule: modelData
                                property QLCFunction func

                                GridLayout
                                {
                                    anchors.fill: parent
                                    columns: 2
                                    columnSpacing: 3
                                    rowSpacing: 3

                                    // row 1
                                    IconTextEntry
                                    {
                                        Layout.columnSpan: 2
                                        Layout.fillWidth: true
                                        height: gridItemsHeight

                                        Component.onCompleted:
                                        {
                                            func = functionManager.getFunction(schedule.functionID)
                                            tLabel = func.name
                                            functionType = func.type
                                        }

                                        IconButton
                                        {
                                            id: fontButton
                                            height: gridItemsHeight
                                            width: height
                                            anchors.top: parent.top
                                            anchors.right: parent.right
                                            imgSource: "qrc:/cancel.svg"
                                            tooltip: qsTr("Remove this schedule")
                                            onClicked:
                                            {
                                                widgetRef.removeSchedule(index)
                                            }
                                        }
                                    }

                                    // row 2
                                    RobotoText
                                    {
                                        height: gridItemsHeight
                                        label: qsTr("Start time")
                                    }
                                    DayTimeTool
                                    {
                                        height: gridItemsHeight
                                        timeValue: schedule ? schedule.startTime : 0
                                        onTimeValueChanged: if (schedule) schedule.startTime = timeValue
                                    }

                                    // row 3
                                    RobotoText
                                    {
                                        height: gridItemsHeight
                                        label: qsTr("Stop time")
                                    }
                                    Rectangle
                                    {
                                        height: UISettings.listItemHeight
                                        Layout.fillWidth: true
                                        color: "transparent"
                                        Row
                                        {
                                            spacing: 5
                                            DayTimeTool
                                            {
                                                enabled: stEnableCheck.checked
                                                timeValue: schedule ? schedule.stopTime : 0

                                                onTimeValueChanged: if (schedule) schedule.stopTime = timeValue

                                                Rectangle
                                                {
                                                    anchors.fill: parent
                                                    color: "black"
                                                    opacity: 0.7
                                                    visible: !stEnableCheck.checked
                                                }
                                            }
                                            CustomCheckBox
                                            {
                                                implicitWidth: UISettings.iconSizeMedium
                                                implicitHeight: implicitWidth
                                                id: stEnableCheck
                                                tooltip: qsTr("Enable the stop time")
                                                checked: schedule ? schedule.stopTime !== -1 : false
                                            }
                                        }
                                    }

                                    Row
                                    {
                                        Layout.columnSpan: 2
                                        spacing: 1

                                        RobotoText { height: UISettings.listItemHeight; label: qsTr("M", "As in Monday") }
                                        CustomCheckBox
                                        {
                                            implicitWidth: UISettings.iconSizeMedium
                                            implicitHeight: implicitWidth
                                            checked: schedule ? schedule.weekFlags & 0x01 : false
                                            onCheckedChanged:
                                            {
                                                if(checked) schedule.weekFlags = schedule.weekFlags | 0x01
                                                else schedule.weekFlags &= ~0x01
                                            }
                                        }

                                        RobotoText { height: UISettings.listItemHeight; label: qsTr("T", "As in Tuesday") }
                                        CustomCheckBox
                                        {
                                            implicitWidth: UISettings.iconSizeMedium
                                            implicitHeight: implicitWidth
                                            checked: schedule ? schedule.weekFlags & 0x02 : false
                                            onCheckedChanged:
                                            {
                                                if(checked) schedule.weekFlags |= 0x02
                                                else schedule.weekFlags &= ~0x02
                                            }
                                        }

                                        RobotoText { height: UISettings.listItemHeight; label: qsTr("W", "As in Wednesday") }
                                        CustomCheckBox
                                        {
                                            implicitWidth: UISettings.iconSizeMedium
                                            implicitHeight: implicitWidth
                                            checked: schedule ? schedule.weekFlags & 0x04 : false
                                            onCheckedChanged:
                                            {
                                                if(checked) schedule.weekFlags |= 0x04
                                                else schedule.weekFlags &= ~0x04
                                            }
                                        }

                                        RobotoText { height: UISettings.listItemHeight; label: qsTr("T", "As in Thursday") }
                                        CustomCheckBox
                                        {
                                            implicitWidth: UISettings.iconSizeMedium
                                            implicitHeight: implicitWidth
                                            checked: schedule ? schedule.weekFlags & 0x08 : false
                                            onCheckedChanged:
                                            {
                                                if(checked) schedule.weekFlags |= 0x08
                                                else schedule.weekFlags &= ~0x08
                                            }
                                        }

                                        RobotoText { height: UISettings.listItemHeight; label: qsTr("F", "As in Friday") }
                                        CustomCheckBox
                                        {
                                            implicitWidth: UISettings.iconSizeMedium
                                            implicitHeight: implicitWidth
                                            checked: schedule ? schedule.weekFlags & 0x10 : false
                                            onCheckedChanged:
                                            {
                                                if(checked) schedule.weekFlags |= 0x10
                                                else schedule.weekFlags &= ~0x10
                                            }
                                        }

                                        RobotoText { height: UISettings.listItemHeight; label: qsTr("S", "As in Saturday") }
                                        CustomCheckBox
                                        {
                                            implicitWidth: UISettings.iconSizeMedium
                                            implicitHeight: implicitWidth
                                            checked: schedule ? schedule.weekFlags & 0x20 : false
                                            onCheckedChanged:
                                            {
                                                if(checked) schedule.weekFlags |= 0x20
                                                else schedule.weekFlags &= ~0x20
                                            }
                                        }

                                        RobotoText { height: UISettings.listItemHeight; label: qsTr("S", "As in Sunday") }
                                        CustomCheckBox
                                        {
                                            implicitWidth: UISettings.iconSizeMedium
                                            implicitHeight: implicitWidth
                                            checked: schedule ? schedule.weekFlags & 0x40 : false
                                            onCheckedChanged:
                                            {
                                                if(checked) schedule.weekFlags |= 0x40
                                                else schedule.weekFlags &= ~0x40
                                            }
                                        }

                                        IconButton
                                        {
                                            width: UISettings.iconSizeMedium
                                            height: width
                                            imgSource: "qrc:/loop.svg"
                                            checkable: true
                                            checked: schedule ? schedule.weekFlags & 0x80 : false
                                            tooltip: qsTr("Repeat weekly")
                                            onCheckedChanged:
                                            {
                                                if(checked) schedule.weekFlags |= 0x80
                                                else schedule.weekFlags &= ~0x80
                                            }
                                        }
                                    }

                                } // end of GridLayout

                                // items divider
                                Rectangle
                                {
                                    width: parent.width
                                    height: 1
                                    y: parent.height - 1
                                    color: UISettings.fgMedium
                                }
                            }
                    } // end of ListView

                    Rectangle
                    {
                        id: newScheduleBox
                        height: UISettings.bigItemHeight * 0.6
                        width: parent.width
                        color: "transparent"
                        radius: 10

                        RobotoText
                        {
                            id: ntText
                            visible: false
                            anchors.centerIn: parent
                            label: qsTr("Add a new schedule")
                        }

                        DropArea
                        {
                            id: newScheduleDrop
                            anchors.fill: parent

                            keys: [ "function" ]

                            states: [
                                State
                                {
                                    when: newScheduleDrop.containsDrag
                                    PropertyChanges
                                    {
                                        target: newScheduleBox
                                        color: "#3F00FF00"
                                    }
                                    PropertyChanges
                                    {
                                        target: ntText
                                        visible: true
                                    }
                                }
                            ]

                            onDropped:
                            {
                                console.log("Function item dropped here. x: " + drag.x + " y: " + drag.y)

                                if (drag.source.hasOwnProperty("fromFunctionManager"))
                                    widgetRef.addSchedules(drag.source.itemsList)
                            }
                        }
                    }
                } // end of Column
        } // end of SectionBox
    } // Column
}
