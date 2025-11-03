/*
  Q Light Controller Plus
  VCAudioTriggersProperties.qml

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

Rectangle
{
    id: propsRoot
    color: "transparent"
    height: audioTriggerPropsColumn.height

    property VCAudioTriggers widgetRef: null

    property int gridItemsHeight: UISettings.listItemHeight

    Column
    {
        id: audioTriggerPropsColumn
        width: parent.width
        spacing: 5

        SectionBox
        {
            id: audioTriggerProp
            sectionLabel: qsTr("Spectrum Bars")

            sectionContents:
                GridLayout
                {
                    width: parent.width
                    columns: 2
                    columnSpacing: 5
                    rowSpacing: 4

                    // row 1
                    RobotoText
                    {
                        height: gridItemsHeight
                        label: qsTr("Number of bars")
                    }

                    CustomSpinBox
                    {
                        Layout.fillWidth: true
                        height: gridItemsHeight
                        value: widgetRef ? widgetRef.barsNumber - 1 : 0
                        onValueModified: if (widgetRef) widgetRef.barsNumber = value + 1
                    }

                    // row 2
                    ListView
                    {
                        id: barsList
                        Layout.columnSpan: 2
                        Layout.fillWidth: true
                        //implicitWidth: audioTriggerPropsColumn.width
                        implicitHeight: count * gridItemsHeight
                        boundsBehavior: Flickable.StopAtBounds
                        model: widgetRef ? widgetRef.barsInfo : null

                        header:
                            RowLayout
                            {
                                z: 2
                                width: barsList.width
                                height: gridItemsHeight

                                RobotoText
                                {
                                    width: UISettings.bigItemHeight * 1.8
                                    height: gridItemsHeight
                                    label: qsTr("Name")
                                    color: UISettings.sectionHeader
                                }
                                Rectangle { width: 1; height: gridItemsHeight }

                                RobotoText
                                {
                                    width: UISettings.bigItemHeight + gridItemsHeight + 10
                                    height: gridItemsHeight
                                    label: qsTr("Type")
                                    color: UISettings.sectionHeader
                                }

                                Rectangle { width: 1; height: gridItemsHeight }

                                RobotoText
                                {
                                    Layout.fillWidth: true
                                    height: gridItemsHeight
                                    label: qsTr("Thresholds")
                                    color: UISettings.sectionHeader
                                }
                            }

                        delegate:
                            Row
                            {
                                width: barsList.width
                                height: gridItemsHeight
                                spacing: 10

                                RobotoText
                                {
                                    width: UISettings.bigItemHeight * 1.8
                                    height: gridItemsHeight
                                    label: modelData.bLabel
                                }
                                CustomComboBox
                                {
                                    width: UISettings.bigItemHeight
                                    height: gridItemsHeight
                                    model: [
                                        { mLabel: qsTr("None"), faIcon: FontAwesome.fa_ban },
                                        { mLabel: qsTr("DMX"), faIcon: FontAwesome.fa_sliders },
                                        { mLabel: qsTr("Function"), faIcon: FontAwesome.fa_cubes },
                                        { mLabel: qsTr("Widget"), faIcon: FontAwesome.fa_table_list }
                                    ]
                                    currentIndex: modelData.type
                                    onCurrentIndexChanged:
                                    {
                                        if (widgetRef)
                                        {
                                            widgetRef.selectBarForEditing(index)
                                            widgetRef.setBarType(currentIndex)
                                        }
                                    }
                                }
                                Rectangle
                                {
                                    visible: modelData.type === VCAudioTriggers.None
                                    width: height
                                    height: gridItemsHeight
                                    color: "transparent"
                                }
                                IconButton
                                {
                                    visible: modelData.type !== VCAudioTriggers.None
                                    width: height
                                    height: gridItemsHeight
                                    faSource: FontAwesome.fa_pen_to_square
                                    checkable: true
                                    onCheckedChanged:
                                    {
                                        widgetRef.selectBarForEditing(index)
                                        if (checked)
                                        {
                                            if (!sideLoader.visible)
                                                rightSidePanel.width += UISettings.sidePanelWidth
                                            sideLoader.visible = true
                                            sideLoader.modelProvider = widgetRef
                                            if (modelData.type === VCAudioTriggers.DMXBar)
                                                sideLoader.source = "qrc:/FixtureGroupManager.qml"
                                            else if (modelData.type === VCAudioTriggers.FunctionBar)
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

                                RobotoText
                                {
                                    visible: modelData.type === VCAudioTriggers.FunctionBar ||
                                             modelData.type === VCAudioTriggers.VCWidgetBar
                                    width: UISettings.bigItemHeight * 2
                                    height: gridItemsHeight
                                    label: modelData.minThreshold + "% - " + modelData.maxThreshold + "%"
                                }
                            }
                    } // ListView
                } // GridLayout
        } // SectionBox
    } // Column
}
