/*
  Q Light Controller Plus
  VCSpeedDialProperties.qml

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

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

import org.qlcplus.classes 1.0

Rectangle
{
    id: propsRoot
    color: "transparent"
    height: speedDialPropsColumn.height

    property VCSpeedDial widgetRef: null

    property int gridItemsHeight: UISettings.listItemHeight

    Column
    {
        id: speedDialPropsColumn
        width: parent.width
        spacing: 5

        SectionBox
        {
            id: speedDialProp
            sectionLabel: qsTr("Speed Dial Properties")

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
                    Layout.fillWidth: true
                    label: "Not implemented."
                }
              } // GridLayout
        } // SectionBox
    } // Column
}
