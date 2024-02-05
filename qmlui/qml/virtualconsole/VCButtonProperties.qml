/*
  Q Light Controller Plus
  VCButtonProperties.qml

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
import "TimeUtils.js" as TimeUtils
import "."

Rectangle
{
    color: "transparent"
    height: bPropsColumn.height

    property VCButton widgetRef: null
    property QLCFunction func
    property int funcID: widgetRef ? widgetRef.functionID : -1
    property int gridItemsHeight: UISettings.listItemHeight

    //onWidgetRefChanged: func = functionManager.getFunction(widgetRef.functionID)

    onFuncIDChanged: func = functionManager.getFunction(funcID)

    TimeEditTool
    {
        id: timeEditTool

        parent: mainView
        z: 99
        x: rightSidePanel.x - width
        visible: false

        onValueChanged:
        {
            if (speedType == QLCFunction.FadeOut)
                widgetRef.stopAllFadeOutTime = val
        }
    }

    Column
    {
        id: bPropsColumn
        width: parent.width
        spacing: 5

        SectionBox
        {
            id: btnFuncProps
            sectionLabel: qsTr("Attached Function")

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
                    id: funcBox
                    Layout.columnSpan: 2
                    Layout.fillWidth: true

                    tFontSize: UISettings.textSizeDefault

                    tLabel: func ? func.name : ""
                    functionType: func ? func.type : -1

                    IconButton
                    {
                        anchors.top: parent.top
                        anchors.right: parent.right
                        faSource: FontAwesome.fa_remove
                        faColor: UISettings.bgControl
                        tooltip: qsTr("Detach the current function")
                        onClicked: widgetRef.functionID = -1
                    }
                }

              } // GridLayout
        } // SectionBox

        SectionBox
        {
            id: btnPressProps
            sectionLabel: qsTr("Pressure behaviour")

            sectionContents:
              GridLayout
              {
                width: parent.width
                columns: 2
                columnSpacing: 5
                rowSpacing: 3

                ButtonGroup { id: pressBehaviourGroup }

                // row 1
                RobotoText
                {
                    height: gridItemsHeight
                    Layout.fillWidth: true
                    label: qsTr("Toggle Function on/off")
                }

                CustomCheckBox
                {
                    implicitWidth: UISettings.iconSizeMedium
                    implicitHeight: implicitWidth
                    ButtonGroup.group: pressBehaviourGroup
                    checked: widgetRef ? widgetRef.actionType === VCButton.Toggle : false
                    onClicked: if (checked && widgetRef) widgetRef.actionType = VCButton.Toggle
                }

                // row 2
                RobotoText
                {
                    height: gridItemsHeight
                    Layout.fillWidth: true
                    label: qsTr("Flash Function (only for Scenes)")
                }

                CustomCheckBox
                {
                    implicitWidth: UISettings.iconSizeMedium
                    implicitHeight: implicitWidth
                    ButtonGroup.group: pressBehaviourGroup
                    checked: widgetRef ? widgetRef.actionType === VCButton.Flash : false
                    onClicked: if (checked && widgetRef) widgetRef.actionType = VCButton.Flash
                }

                // row 3
                RobotoText
                {
                    height: gridItemsHeight
                    Layout.fillWidth: true
                    label: qsTr("Toggle Blackout")
                }

                CustomCheckBox
                {
                    implicitWidth: UISettings.iconSizeMedium
                    implicitHeight: implicitWidth
                    ButtonGroup.group: pressBehaviourGroup
                    checked: widgetRef ? widgetRef.actionType === VCButton.Blackout : false
                    onClicked: if (checked && widgetRef) widgetRef.actionType = VCButton.Blackout
                }

                // row 4
                RobotoText
                {
                    height: gridItemsHeight
                    Layout.fillWidth: true
                    label: qsTr("Stop all Functions")
                }

                CustomCheckBox
                {
                    implicitWidth: UISettings.iconSizeMedium
                    implicitHeight: implicitWidth
                    ButtonGroup.group: pressBehaviourGroup
                    checked: widgetRef ? widgetRef.actionType === VCButton.StopAll : false
                    onClicked: if (checked && widgetRef) widgetRef.actionType = VCButton.StopAll
                }

              } // GridLayout
        } // SectionBox

        SectionBox
        {
            id: startupIntensityProps
            visible: widgetRef ? (widgetRef.actionType === VCButton.Toggle || widgetRef.actionType === VCButton.Flash) : false
            sectionLabel: qsTr("Adjust Function intensity")

            sectionContents:
              RowLayout
              {
                  width: parent.width
                  spacing: 10

                  CustomCheckBox
                  {
                      id: startupIntCheck
                      implicitWidth: UISettings.iconSizeMedium
                      implicitHeight: implicitWidth
                      checked: widgetRef ? widgetRef.startupIntensityEnabled : false
                      onClicked: if (widgetRef) widgetRef.startupIntensityEnabled = checked
                  }

                  CustomSlider
                  {
                      Layout.fillWidth: true
                      enabled: startupIntCheck.checked
                      value: widgetRef ? widgetRef.startupIntensity * 100 : 100
                      onPositionChanged: if (widgetRef) widgetRef.startupIntensity = value / 100
                  }

                  CustomSpinBox
                  {
                      id: wSpin
                      enabled: startupIntCheck.checked
                      width: UISettings.bigItemHeight * 0.7
                      height: UISettings.listItemHeight
                      from: 0
                      to: 100
                      suffix: "%"
                      value: widgetRef ? widgetRef.startupIntensity * 100 : 100
                      onValueChanged: if (widgetRef) widgetRef.startupIntensity = value / 100
                  }
              }
        }

        SectionBox
        {
            id: stopAllProps
            visible: widgetRef ? widgetRef.actionType === VCButton.StopAll : false
            sectionLabel: qsTr("Stop all Functions")

            sectionContents:
              RowLayout
              {
                  width: parent.width
                  spacing: 10

                  RobotoText
                  {
                      id: foLabel
                      height: UISettings.listItemHeight
                      label: qsTr("Fade out")
                  }

                  Rectangle
                  {
                      Layout.fillWidth: true
                      height: UISettings.listItemHeight
                      color: UISettings.bgMedium

                      RobotoText
                      {
                          anchors.fill: parent
                          label: widgetRef ? TimeUtils.timeToQlcString(widgetRef.stopAllFadeOutTime, QLCFunction.Time) : ""

                          MouseArea
                          {
                              anchors.fill: parent
                              onDoubleClicked:
                              {
                                  timeEditTool.show(-1, this.mapToItem(mainView, 0, 0).y,
                                                    foLabel.label, parent.label, QLCFunction.FadeOut)
                              }
                          }
                      }
                  }
              }

        }

        SectionBox
        {
            id: flashProperties
            visible: widgetRef ? widgetRef.actionType === VCButton.Flash : false
            sectionLabel: qsTr("Flash properties")

            sectionContents:
              RowLayout
              {
                  width: parent.width
                  spacing: 10

                  RobotoText
                  {
                      id: flashOverrideLabel
                      height: UISettings.listItemHeight
                      label: qsTr("Override priority")
                  }

                  CustomCheckBox
                  {
                      id: flashOverrideCheckBox
                      implicitWidth: UISettings.iconSizeMedium
                      implicitHeight: implicitWidth
                      checked: widgetRef.flashOverrides
                      onClicked: widgetRef.flashOverrides = checked
                  }

                  RobotoText
                  {
                      id: flashForceLTPLabel
                      height: UISettings.listItemHeight
                      label: qsTr("Force LTP")
                  }

                  CustomCheckBox
                  {
                      id: flashForceLTPCheckBox
                      implicitWidth: UISettings.iconSizeMedium
                      implicitHeight: implicitWidth
                      checked: widgetRef.flashForceLTP
                      onClicked: widgetRef.flashForceLTP = checked
                  }
              }

        }
    } // Column
}
