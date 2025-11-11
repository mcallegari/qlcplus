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

    CustomPopupDialog
    {
        id: thresholdsPopup
        width: mainView.width / 3

        property alias tMin: minThresholdSpin.value
        property alias tMax: maxThresholdSpin.value

        onOpened: maxThresholdSpin.focus = true
        onAccepted: widgetRef.setBarThresholds(tMin, tMax)

        contentItem:
            GridLayout
            {
                width: parent.width
                height: UISettings.iconSizeDefault * rows
                columns: 2
                columnSpacing: 5

                // Row 1
                RobotoText
                {
                    label: qsTr("Activation threshold")
                }

                CustomSpinBox
                {
                    id: maxThresholdSpin
                    Layout.fillWidth: true
                    suffix: "%"
                    from: 5
                    to: 95
                }

                // Row 2
                RobotoText
                {
                    label: qsTr("Deactivation threshold")
                }

                CustomSpinBox
                {
                    id: minThresholdSpin
                    Layout.fillWidth: true
                    suffix: "%"
                    from: 5
                    to: 95
                }
            }
    }

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
                        clip: true
                        //implicitWidth: audioTriggerPropsColumn.width
                        implicitHeight: count * gridItemsHeight
                        boundsBehavior: Flickable.StopAtBounds
                        headerPositioning: ListView.OverlayHeader
                        model: widgetRef ? widgetRef.barsInfo : null

                        property Item currentChecked: null
                        property int currentType: VCAudioTriggers.None

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
                                    label: qsTr("Information")
                                    color: UISettings.sectionHeader
                                }
                            }

                        delegate:
                            Row
                            {
                                width: barsList.width
                                height: modelData.type === VCAudioTriggers.FunctionBar ? gridItemsHeight * 2 : gridItemsHeight
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

                                    onActivated:
                                    {
                                        if (widgetRef)
                                        {
                                            widgetRef.selectBarForEditing(modelData.index)
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
                                        widgetRef.selectBarForEditing(modelData.index)
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
                                            barsList.currentChecked = this
                                            barsList.currentType = modelData.type
                                        }
                                        else
                                        {
                                            rightSidePanel.width -= sideLoader.width
                                            sideLoader.source = ""
                                            sideLoader.visible = false
                                            barsList.currentType = VCAudioTriggers.None
                                        }
                                    }
                                }

                                RobotoText
                                {
                                    visible: modelData.type !== VCAudioTriggers.None
                                    width: UISettings.bigItemHeight * 2
                                    height: gridItemsHeight
                                    clip: false
                                    color: thresholdsMa.containsMouse ? UISettings.bgLight : "transparent"
                                    label: modelData.type === VCAudioTriggers.DMXBar ?
                                               modelData.intVal + " " + qsTr("Channels") :
                                               qsTr("Thresholds:") + " " + modelData.minThreshold + "% - " + modelData.maxThreshold + "%"

                                    MouseArea
                                    {
                                        id: thresholdsMa
                                        enabled: modelData.type === VCAudioTriggers.FunctionBar ||
                                                 modelData.type === VCAudioTriggers.VCWidgetBar
                                        width: parent.width
                                        height: gridItemsHeight
                                        hoverEnabled: true
                                        onClicked:
                                        {
                                            widgetRef.selectBarForEditing(modelData.index)
                                            thresholdsPopup.tMin = Math.round(modelData.minThreshold)
                                            thresholdsPopup.tMax = Math.round(modelData.maxThreshold)
                                            thresholdsPopup.open()
                                        }
                                    }

                                    IconTextEntry
                                    {
                                        visible: modelData.type === VCAudioTriggers.FunctionBar
                                        y: gridItemsHeight
                                        height: gridItemsHeight
                                        width: parent.width

                                        property QLCFunction func: functionManager.getFunction(modelData.intVal)
                                        tLabel: func ? func.name : ""
                                        functionType: func ? func.type : -1
                                    }
                                }
                            }

                        Rectangle
                        {
                            id: addFunctionBox
                            visible: barsList.currentType === VCAudioTriggers.FunctionBar
                            anchors.fill: barsList
                            color: addFunctionDrop.containsDrag ? UISettings.activeDropArea : UISettings.bgMedium
                            opacity: 0.9
                            radius: 10

                            RobotoText
                            {
                                id: afText
                                anchors.centerIn: parent
                                label: qsTr("Drop a Function here")
                                labelColor: addFunctionDrop.containsDrag ? UISettings.bgStronger : UISettings.fgMain
                                fontBold: addFunctionDrop.containsDrag ? true : false
                            }

                            DropArea
                            {
                                id: addFunctionDrop
                                anchors.fill: parent

                                keys: [ "function" ]

                                onDropped:
                                {
                                    console.log("Function item dropped here. x: " + drag.x + " y: " + drag.y)

                                    if (drag.source.hasOwnProperty("fromFunctionManager"))
                                    {
                                        barsList.currentChecked.checked = false
                                        widgetRef.setBarFunction(drag.source.itemsList[0])
                                    }
                                }
                            }
                        }
                    } // ListView
                } // GridLayout
        } // SectionBox
    } // Column
}
