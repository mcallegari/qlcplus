/*
  Q Light Controller Plus
  VCCueListProperties.qml

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
    height: clPropsColumn.height

    property VCCueList widgetRef: null
    property QLCFunction chaser
    property int chaserID: widgetRef ? widgetRef.chaserID : -1
    property int gridItemsHeight: UISettings.listItemHeight

    onChaserIDChanged: chaser = functionManager.getFunction(chaserID)

    Column
    {
        id: clPropsColumn
        width: parent.width
        spacing: 5

        SectionBox
        {
            sectionLabel: qsTr("Attached Chaser")

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

                    tLabel: chaser ? chaser.name : ""
                    functionType: chaser ? chaser.type : -1

                    IconButton
                    {
                        anchors.top: parent.top
                        anchors.right: parent.right
                        faSource: FontAwesome.fa_remove
                        faColor: UISettings.bgControl
                        tooltip: qsTr("Detach the current chaser")
                        onClicked: widgetRef.chaserID = -1
                    }
                }

              } // GridLayout
        } // SectionBox

        SectionBox
        {
            sectionLabel: qsTr("Buttons behavior")

            sectionContents:
              GridLayout
              {
                  width: parent.width
                  columns: 2
                  columnSpacing: 5
                  rowSpacing: 3

                  // row 1
                  RobotoText
                  {
                      height: gridItemsHeight
                      Layout.fillWidth: true
                      label: qsTr("Play/Stop layout")
                  }

                  CustomComboBox
                  {
                      ListModel
                      {
                          id: layoutModel
                          ListElement { mLabel: qsTr("Play/Pause + Stop"); mValue: VCCueList.PlayPauseStop }
                          ListElement { mLabel: qsTr("Play/Stop + Pause"); mValue: VCCueList.PlayStopPause }
                      }

                      Layout.fillWidth: true
                      height: UISettings.listItemHeight
                      model: layoutModel
                      currentIndex: widgetRef ? widgetRef.playbackLayout : VCCueList.PlayPauseStop
                      onCurrentIndexChanged: if (widgetRef) widgetRef.playbackLayout = currentIndex
                  }

                  // row 2
                  RobotoText
                  {
                      height: gridItemsHeight * 2
                      Layout.fillWidth: true
                      wrapText: true
                      label: qsTr("Next/Previous\n(when chaser is not running)")
                  }

                  CustomComboBox
                  {
                      ListModel
                      {
                          id: nextPrevModel
                          ListElement { mLabel: qsTr("Run from first/last cue"); mValue: VCCueList.DefaultRunFirst }
                          ListElement { mLabel: qsTr("Run from next/previous cue"); mValue: VCCueList.RunNext }
                          ListElement { mLabel: qsTr("Select next/previous cue"); mValue: VCCueList.Select }
                          ListElement { mLabel: qsTr("Do nothing"); mValue: VCCueList.Nothing }
                      }

                      Layout.fillWidth: true
                      height: UISettings.listItemHeight
                      model: nextPrevModel
                      currentIndex: widgetRef ? widgetRef.nextPrevBehavior : VCCueList.DefaultRunFirst
                      onCurrentIndexChanged: if (widgetRef) widgetRef.nextPrevBehavior = currentIndex
                  }
              } // GridLayout
        } // SectionBox

        SectionBox
        {
            sectionLabel: qsTr("Side fader")

            sectionContents:
              GridLayout
              {
                  width: parent.width
                  columns: 7
                  columnSpacing: 5
                  rowSpacing: 3

                  // row 1
                  RobotoText
                  {
                      height: gridItemsHeight
                      Layout.fillWidth: true
                      label: qsTr("Mode")
                  }

                  CustomCheckBox
                  {
                      implicitWidth: UISettings.iconSizeMedium
                      implicitHeight: implicitWidth
                      checked: widgetRef ? widgetRef.sideFaderMode === VCCueList.None : false
                      onClicked: if (checked && widgetRef) widgetRef.sideFaderMode = VCCueList.None
                  }

                  RobotoText
                  {
                      height: gridItemsHeight
                      Layout.fillWidth: true
                      label: qsTr("None")
                  }

                  CustomCheckBox
                  {
                      implicitWidth: UISettings.iconSizeMedium
                      implicitHeight: implicitWidth
                      checked: widgetRef ? widgetRef.sideFaderMode === VCCueList.Crossfade : false
                      onClicked: if (checked && widgetRef) widgetRef.sideFaderMode = VCCueList.Crossfade
                  }

                  RobotoText
                  {
                      height: gridItemsHeight
                      Layout.fillWidth: true
                      label: qsTr("Crossfade")
                  }

                  CustomCheckBox
                  {
                      implicitWidth: UISettings.iconSizeMedium
                      implicitHeight: implicitWidth
                      checked: widgetRef ? widgetRef.sideFaderMode === VCCueList.Steps : false
                      onClicked: if (checked && widgetRef) widgetRef.sideFaderMode = VCCueList.Steps
                  }

                  RobotoText
                  {
                      height: gridItemsHeight
                      Layout.fillWidth: true
                      label: qsTr("Steps")
                  }
              }
        }
    } // Column
}
