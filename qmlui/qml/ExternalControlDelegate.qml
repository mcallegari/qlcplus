/*
  Q Light Controller Plus
  ExternalControlDelegate.qml

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

import com.qlcplus.classes 1.0
import "."

Column
{
    width: parent.width

    property var dObjRef
    property int controlID
    property string uniName
    property string chName

    GridLayout
    {
        width: parent.width
        columns: 3
        columnSpacing: 3
        rowSpacing: 3

        // row 1
        RobotoText
        {
            height: UISettings.listItemHeight
            label: qsTr("Type")
        }
        CustomComboBox
        {
            Layout.fillWidth: true
            Layout.columnSpan: 2
            height: UISettings.listItemHeight
            model: dObjRef ? dObjRef.externalControlsList : null
            currentValue: controlID
        }

        // row 2
        RobotoText
        {
            height: UISettings.listItemHeight
            label: qsTr("Universe")
        }
        RobotoText
        {
            Layout.fillWidth: true
            height: UISettings.listItemHeight
            color: UISettings.bgLight
            label: uniName
        }
        IconButton
        {
            width: UISettings.iconSizeMedium
            height: width
            checkable: true
            imgSource: "qrc:/inputoutput.svg"
            tooltip: qsTr("Activate auto detection")
        }

        // row 3
        RobotoText
        {
            height: UISettings.listItemHeight
            label: qsTr("Channel")
        }
        RobotoText
        {
            Layout.fillWidth: true
            height: UISettings.listItemHeight
            color: UISettings.bgLight
            label: chName
        }
        IconButton
        {
            width: UISettings.iconSizeMedium
            height: width
            imgSource: "qrc:/remove.svg"
            tooltip: qsTr("Remove this input source")
        }

    } // end of GridLayout

    // items divider
    Rectangle
    {
        width: parent.width
        height: 1
        color: UISettings.fgMedium
    }
}
