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
import QtQuick.Controls 1.2

import com.qlcplus.classes 1.0
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

                ExclusiveGroup { id: valueStyleGroup }
                ExclusiveGroup { id: invDisplayGroup }

                // row 1
                CustomCheckBox
                {
                    width: UISettings.iconSizeMedium
                    height: width
                    exclusiveGroup: valueStyleGroup
                    checked: widgetRef ? widgetRef.valueDisplayStyle === VCSlider.DMXValue : true
                    onCheckedChanged: if (checked && widgetRef) widgetRef.valueDisplayStyle = VCSlider.DMXValue
                }

                RobotoText
                {
                    height: gridItemsHeight
                    Layout.fillWidth: true
                    label: qsTr("DMX Value")
                }

                CustomCheckBox
                {
                    width: UISettings.iconSizeMedium
                    height: width
                    exclusiveGroup: valueStyleGroup
                    checked: widgetRef ? widgetRef.valueDisplayStyle === VCSlider.PercentageValue : false
                    onCheckedChanged: if (checked && widgetRef) widgetRef.valueDisplayStyle = VCSlider.PercentageValue
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
                    width: UISettings.iconSizeMedium
                    height: width
                    exclusiveGroup: invDisplayGroup
                    checked: widgetRef ? !widgetRef.invertedAppearance : true
                    onCheckedChanged: if (checked && widgetRef) widgetRef.invertedAppearance = false
                }

                RobotoText
                {
                    height: gridItemsHeight
                    Layout.fillWidth: true
                    label: qsTr("Normal")
                }

                CustomCheckBox
                {
                    width: UISettings.iconSizeMedium
                    height: width
                    exclusiveGroup: invDisplayGroup
                    checked: widgetRef ? widgetRef.invertedAppearance : false
                    onCheckedChanged: if (checked && widgetRef) widgetRef.invertedAppearance = true
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
                columns: 2
                columnSpacing: 5
                rowSpacing: 4

                ExclusiveGroup { id: sliderModeGroup }

                // row 1
                CustomCheckBox
                {
                    width: UISettings.iconSizeMedium
                    height: width
                    exclusiveGroup: sliderModeGroup
                    checked: widgetRef ? widgetRef.sliderMode === VCSlider.Level : true
                    onCheckedChanged: if (checked && widgetRef) widgetRef.sliderMode = VCSlider.Level
                }

                RobotoText
                {
                    height: gridItemsHeight
                    Layout.fillWidth: true
                    label: qsTr("Level")
                }

                // row 2
                CustomCheckBox
                {
                    width: UISettings.iconSizeMedium
                    height: width
                    exclusiveGroup: sliderModeGroup
                    checked: widgetRef ? widgetRef.sliderMode === VCSlider.Playback : false
                    onCheckedChanged: if (checked && widgetRef) widgetRef.sliderMode = VCSlider.Playback
                }

                RobotoText
                {
                    height: gridItemsHeight
                    Layout.fillWidth: true
                    label: qsTr("Playback")
                }

                // row 3
                CustomCheckBox
                {
                    width: UISettings.iconSizeMedium
                    height: width
                    exclusiveGroup: sliderModeGroup
                    checked: widgetRef ? widgetRef.sliderMode === VCSlider.Submaster : false
                    onCheckedChanged: if (checked && widgetRef) widgetRef.sliderMode = VCSlider.Submaster
                }

                RobotoText
                {
                    height: gridItemsHeight
                    Layout.fillWidth: true
                    label: qsTr("Submaster")
                }

                // row 4
                CustomCheckBox
                {
                    width: UISettings.iconSizeMedium
                    height: width
                    exclusiveGroup: sliderModeGroup
                    checked: widgetRef ? widgetRef.sliderMode === VCSlider.GrandMaster : false
                    onCheckedChanged: if (checked && widgetRef) widgetRef.sliderMode = VCSlider.GrandMaster
                }

                RobotoText
                {
                    height: gridItemsHeight
                    Layout.fillWidth: true
                    label: qsTr("Grand Master")
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
        } // SectionBox
    } // end of Column
}
