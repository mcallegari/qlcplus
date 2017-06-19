/*
  Q Light Controller Plus
  VCSliderProperties.qml

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
    height: sPropsColumn.height

    property VCSlider widgetRef: null
    property Function func
    property int funcID: widgetRef ? widgetRef.playbackFunction : -1
    property int gridItemsHeight: UISettings.listItemHeight

    onFuncIDChanged: func = functionManager.getFunction(funcID)

    Column
    {
        id: sPropsColumn
        width: parent.width
        spacing: 5

        SectionBox
        {
            sectionLabel: qsTr("Display Style")

            sectionContents:
              GridLayout
              {
                width: parent.width
                columns: 4
                columnSpacing: 5
                rowSpacing: 4

                ButtonGroup { id: valueStyleGroup }
                ButtonGroup { id: invDisplayGroup }

                // row 1
                CustomCheckBox
                {
                    implicitWidth: UISettings.iconSizeMedium
                    implicitHeight: implicitWidth
                    ButtonGroup.group: valueStyleGroup
                    checked: widgetRef ? widgetRef.valueDisplayStyle === VCSlider.DMXValue : true
                    onClicked: if (checked && widgetRef) widgetRef.valueDisplayStyle = VCSlider.DMXValue
                }

                RobotoText
                {
                    height: gridItemsHeight
                    Layout.fillWidth: true
                    label: qsTr("DMX Value")
                }

                CustomCheckBox
                {
                    implicitWidth: UISettings.iconSizeMedium
                    implicitHeight: implicitWidth
                    ButtonGroup.group: valueStyleGroup
                    checked: widgetRef ? widgetRef.valueDisplayStyle === VCSlider.PercentageValue : false
                    onClicked: if (checked && widgetRef) widgetRef.valueDisplayStyle = VCSlider.PercentageValue
                }

                RobotoText
                {
                    height: gridItemsHeight
                    Layout.fillWidth: true
                    label: qsTr("Percentage")
                }

                // row 2
                CustomCheckBox
                {
                    implicitWidth: UISettings.iconSizeMedium
                    implicitHeight: implicitWidth
                    ButtonGroup.group: invDisplayGroup
                    checked: widgetRef ? !widgetRef.invertedAppearance : true
                    onClicked: if (checked && widgetRef) widgetRef.invertedAppearance = false
                }

                RobotoText
                {
                    height: gridItemsHeight
                    Layout.fillWidth: true
                    label: qsTr("Normal")
                }

                CustomCheckBox
                {
                    implicitWidth: UISettings.iconSizeMedium
                    implicitHeight: implicitWidth
                    ButtonGroup.group: invDisplayGroup
                    checked: widgetRef ? widgetRef.invertedAppearance : false
                    onClicked: if (checked && widgetRef) widgetRef.invertedAppearance = true
                }

                RobotoText
                {
                    height: gridItemsHeight
                    Layout.fillWidth: true
                    label: qsTr("Inverted")
                }
              }
        } // end of SectionBox

        SectionBox
        {
            sectionLabel: qsTr("Slider Mode")

            sectionContents:
              GridLayout
              {
                width: parent.width
                columns: 4
                columnSpacing: 5
                rowSpacing: 4

                ButtonGroup { id: sliderModeGroup }

                // row 1
                CustomCheckBox
                {
                    implicitWidth: UISettings.iconSizeMedium
                    implicitHeight: implicitWidth
                    ButtonGroup.group: sliderModeGroup
                    checked: widgetRef ? widgetRef.sliderMode === VCSlider.Level : true
                    onClicked: if (checked && widgetRef) widgetRef.sliderMode = VCSlider.Level
                }

                RobotoText
                {
                    height: gridItemsHeight
                    Layout.fillWidth: true
                    label: qsTr("Level")
                }

                CustomCheckBox
                {
                    implicitWidth: UISettings.iconSizeMedium
                    implicitHeight: implicitWidth
                    ButtonGroup.group: sliderModeGroup
                    checked: widgetRef ? widgetRef.sliderMode === VCSlider.Playback : false
                    onClicked: if (checked && widgetRef) widgetRef.sliderMode = VCSlider.Playback
                }

                RobotoText
                {
                    height: gridItemsHeight
                    Layout.fillWidth: true
                    label: qsTr("Playback")
                }

                // row 2
                CustomCheckBox
                {
                    implicitWidth: UISettings.iconSizeMedium
                    implicitHeight: implicitWidth
                    ButtonGroup.group: sliderModeGroup
                    checked: widgetRef ? widgetRef.sliderMode === VCSlider.Submaster : false
                    onClicked: if (checked && widgetRef) widgetRef.sliderMode = VCSlider.Submaster
                }

                RobotoText
                {
                    height: gridItemsHeight
                    Layout.fillWidth: true
                    label: qsTr("Submaster")
                }

                CustomCheckBox
                {
                    implicitWidth: UISettings.iconSizeMedium
                    implicitHeight: implicitWidth
                    ButtonGroup.group: sliderModeGroup
                    checked: widgetRef ? widgetRef.sliderMode === VCSlider.GrandMaster : false
                    onClicked: if (checked && widgetRef) widgetRef.sliderMode = VCSlider.GrandMaster
                }

                RobotoText
                {
                    height: gridItemsHeight
                    Layout.fillWidth: true
                    label: qsTr("Grand Master")
                }

                // row 3
                CustomCheckBox
                {
                    implicitWidth: UISettings.iconSizeMedium
                    implicitHeight: implicitWidth
                    ButtonGroup.group: sliderModeGroup
                    checked: widgetRef ? widgetRef.sliderMode === VCSlider.Attribute : false
                    onClicked: if (checked && widgetRef) widgetRef.sliderMode = VCSlider.Attribute
                }

                RobotoText
                {
                    height: gridItemsHeight
                    Layout.fillWidth: true
                    label: qsTr("Attribute")
                }
              }
        } // end of SectionBox

        SectionBox
        {
            visible: widgetRef ? widgetRef.sliderMode === VCSlider.Playback : false
            sectionLabel: qsTr("Playback Function")

            sectionContents:
              GridLayout
              {
                width: parent.width
                columns: 2
                columnSpacing: 5
                rowSpacing: 4

                // row 1
                IconTextEntry
                {
                    Layout.columnSpan: 2
                    Layout.fillWidth: true

                    tFontSize: UISettings.textSizeDefault

                    tLabel: func ? func.name : ""
                    functionType: func ? func.type : -1

                    IconButton
                    {
                        anchors.top: parent.top
                        anchors.right: parent.right
                        imgSource: "qrc:/reset.svg"
                        tooltip: qsTr("Detach the current function")
                        onClicked: widgetRef.playbackFunction = -1
                    }
                }

              } // GridLayout
        } // SectionBox Playback mode

        SectionBox
        {
            visible: widgetRef ? widgetRef.sliderMode === VCSlider.Level : false
            sectionLabel: qsTr("Level mode")

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
                      label: qsTr("Lower limit")
                  }
                  CustomSpinBox
                  {
                      Layout.fillWidth: true
                      from: 0
                      to: 255
                      value: widgetRef ? widgetRef.levelLowLimit : 0
                      onValueChanged: if (widgetRef) widgetRef.levelLowLimit = value
                  }
                  // row 2
                  RobotoText
                  {
                      height: gridItemsHeight
                      label: qsTr("Upper limit")
                  }
                  CustomSpinBox
                  {
                      Layout.fillWidth: true
                      from: 0
                      to: 255
                      value: widgetRef ? widgetRef.levelHighLimit : 0
                      onValueChanged: if (widgetRef) widgetRef.levelHighLimit = value
                  }
                  // row 3
                  RobotoText
                  {
                      height: gridItemsHeight
                      label: qsTr("Channels")
                  }
                  RobotoText
                  {
                      height: gridItemsHeight
                      Layout.fillWidth: true
                      label: ""

                      IconButton
                      {
                          id: serviceEntry
                          z: 2
                          anchors.right: parent.right
                          height: gridItemsHeight
                          width: height
                          faSource: FontAwesome.fa_bars
                          faColor: "white"
                          checkable: true
                          tooltip: qsTr("Add/Remove channels")
                          onCheckedChanged:
                          {
                              if (checked)
                              {
                                  vcRightPanel.width += mainView.width / 3
                                  sideLoader.width = mainView.width / 3
                                  sideLoader.modelProvider = widgetRef
                                  sideLoader.source = "qrc:/FixtureGroupManager.qml"
                              }
                              else
                              {
                                  vcRightPanel.width = vcRightPanel.width - sideLoader.width
                                  sideLoader.source = ""
                                  sideLoader.width = 0
                              }
                          }
                      }
                  }

                  CustomCheckBox
                  {
                      implicitWidth: UISettings.iconSizeMedium
                      implicitHeight: implicitWidth
                      autoExclusive: false
                      checked: widgetRef ? widgetRef.monitorEnabled : true
                      onClicked: if (widgetRef) widgetRef.monitorEnabled = checked
                  }
                  RobotoText
                  {
                      height: gridItemsHeight
                      label: qsTr("Monitor channel levels")
                  }
              }
        } // SectionBox Level mode

        SectionBox
        {
            visible: widgetRef ? widgetRef.sliderMode === VCSlider.GrandMaster : false
            sectionLabel: qsTr("Grand Master mode")

            sectionContents:
              GridLayout
              {
                  width: parent.width
                  columns: 4
                  columnSpacing: 5
                  rowSpacing: 4

                  ButtonGroup { id: gmValueModeGroup }
                  ButtonGroup { id: gmChannelModeGroup }

                  // row 1
                  CustomCheckBox
                  {
                      implicitWidth: UISettings.iconSizeMedium
                      implicitHeight: implicitWidth
                      ButtonGroup.group: gmValueModeGroup
                      checked: widgetRef ? widgetRef.grandMasterValueMode === GrandMaster.Reduce : true
                      onClicked: if (checked && widgetRef) widgetRef.grandMasterValueMode = GrandMaster.Reduce
                  }

                  RobotoText
                  {
                      height: gridItemsHeight
                      Layout.fillWidth: true
                      label: qsTr("Reduce values")
                  }

                  // row 2
                  CustomCheckBox
                  {
                      implicitWidth: UISettings.iconSizeMedium
                      implicitHeight: implicitWidth
                      ButtonGroup.group: gmValueModeGroup
                      checked: widgetRef ? widgetRef.grandMasterValueMode === GrandMaster.Limit : true
                      onClicked: if (checked && widgetRef) widgetRef.grandMasterValueMode = GrandMaster.Limit
                  }

                  RobotoText
                  {
                      height: gridItemsHeight
                      Layout.fillWidth: true
                      label: qsTr("Limit values")
                  }

                  // row 3
                  CustomCheckBox
                  {
                      implicitWidth: UISettings.iconSizeMedium
                      implicitHeight: implicitWidth
                      ButtonGroup.group: gmChannelModeGroup
                      checked: widgetRef ? widgetRef.grandMasterChannelMode === GrandMaster.Intensity : true
                      onClicked: if (checked && widgetRef) widgetRef.grandMasterChannelMode = GrandMaster.Intensity
                  }

                  RobotoText
                  {
                      height: gridItemsHeight
                      Layout.fillWidth: true
                      label: qsTr("Intensity channels")
                  }

                  // row 4
                  CustomCheckBox
                  {
                      implicitWidth: UISettings.iconSizeMedium
                      implicitHeight: implicitWidth
                      ButtonGroup.group: gmChannelModeGroup
                      checked: widgetRef ? widgetRef.grandMasterChannelMode === GrandMaster.AllChannels : true
                      onClicked: if (checked && widgetRef) widgetRef.grandMasterChannelMode = GrandMaster.AllChannels
                  }

                  RobotoText
                  {
                      height: gridItemsHeight
                      Layout.fillWidth: true
                      label: qsTr("All channels")
                  }
              }
        } // SectionBox Grand Master mode

    } // end of Column
}
