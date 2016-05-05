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
import QtQuick.Controls 1.2

import com.qlcplus.classes 1.0
import "."

Rectangle
{
    color: "transparent"
    height: bPropsColumn.height

    property VCButton widgetRef: null
    property Function func
    property int funcID: widgetRef ? widgetRef.functionID : -1
    property int gridItemsHeight: 38

    //onWidgetRefChanged: func = functionManager.getFunction(widgetRef.functionID)

    onFuncIDChanged: func = functionManager.getFunction(funcID)

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

                    fontSize: 14

                    tLabel: func ? func.name : ""
                    functionType: func ? func.type : -1

                    IconButton
                    {
                        id: fontButton
                        anchors.top: parent.top
                        anchors.right: parent.right
                        imgSource: "qrc:/reset.svg"
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
                rowSpacing: 4

                ExclusiveGroup { id: pressBehaviourGroup }

                // row 1
                RobotoText { fontSize: 14; label: qsTr("Toggle Function on/off") }

                CustomCheckBox
                {
                    width: gridItemsHeight
                    height: gridItemsHeight
                    exclusiveGroup: pressBehaviourGroup
                    checked: widgetRef ? widgetRef.actionType === VCButton.Toggle : false
                    onCheckedChanged: if (checked && widgetRef) widgetRef.actionType = VCButton.Toggle
                }

                // row 2
                RobotoText { fontSize: 14; label: qsTr("Flash Function (only for Scenes)") }

                CustomCheckBox
                {
                    width: gridItemsHeight
                    height: gridItemsHeight
                    exclusiveGroup: pressBehaviourGroup
                    checked: widgetRef ? widgetRef.actionType === VCButton.Flash : false
                    onCheckedChanged: if (checked && widgetRef) widgetRef.actionType = VCButton.Flash
                }

                // row 3
                RobotoText { fontSize: 14; label: qsTr("Toggle Blackout") }

                CustomCheckBox
                {
                    width: gridItemsHeight
                    height: gridItemsHeight
                    exclusiveGroup: pressBehaviourGroup
                    checked: widgetRef ? widgetRef.actionType === VCButton.Blackout : false
                    onCheckedChanged: if (checked && widgetRef) widgetRef.actionType = VCButton.Blackout
                }

                // row 4
                RobotoText { fontSize: 14; label: qsTr("Stop all Functions") }

                CustomCheckBox
                {
                    width: gridItemsHeight
                    height: gridItemsHeight
                    exclusiveGroup: pressBehaviourGroup
                    checked: widgetRef ? widgetRef.actionType === VCButton.StopAll : false
                    onCheckedChanged: if (checked && widgetRef) widgetRef.actionType = VCButton.StopAll
                }

              } // GridLayout
        } // SectionBox
    } // Column
}
