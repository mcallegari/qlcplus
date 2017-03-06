/*
  Q Light Controller Plus
  FixtureChannelDelegate.qml

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

import com.qlcplus.classes 1.0
import "."

Rectangle
{
    id: chDelegate
    width: 100
    height: UISettings.listItemHeight

    color: "transparent"

    property string chIcon: ""
    property string textLabel: cRef ? cRef.name : ""
    property bool isSelected: false
    property Item dragItem

    signal mouseEvent(int type, int iID, int iType, var qItem, int mouseMods)

    Rectangle
    {
        anchors.fill: parent
        radius: 3
        color: UISettings.highlight
        visible: isSelected
    }

    Row
    {
        CustomCheckBox
        {
            id: chCheckBox
            height: UISettings.listItemHeight
            width: height
        }

        IconTextEntry
        {
            id: chEntry
            height: UISettings.listItemHeight
            width: chDelegate.width - chCheckBox.width
            tLabel: textLabel
            iSrc: chIcon

            MouseArea
            {
                anchors.fill: parent
                hoverEnabled: true

                onClicked: chDelegate.mouseEvent(App.Clicked, cRef.id, cRef.type, chDelegate, mouse.modifiers)
                onDoubleClicked: chDelegate.mouseEvent(App.DoubleClicked, cRef.id, cRef.type, chDelegate, -1)
            }
        }
    }

    Rectangle
    {
        width: parent.width
        height: 1
        y: parent.height - 1
        color: "#666"
    }
}
