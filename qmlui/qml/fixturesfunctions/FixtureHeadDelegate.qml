/*
  Q Light Controller Plus
  FixtureHeadDelegate.qml

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

import org.qlcplus.classes 1.0
import "."

Rectangle
{
    id: headDelegate
    width: 100
    height: UISettings.listItemHeight

    color: "transparent"

    property string textLabel
    property int itemType: App.HeadDragItem
    property bool isSelected: false
    property bool isCheckable: false
    property bool isChecked: false
    property int itemID
    property int headIndex
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
            visible: isCheckable
            implicitWidth: UISettings.listItemHeight
            implicitHeight: implicitWidth
            checked: isChecked
            onCheckedChanged: headDelegate.mouseEvent(App.Checked, headIndex, checked, headDelegate, 0)
        }

        IconTextEntry
        {
            id: chEntry
            height: UISettings.listItemHeight
            width: headDelegate.width - (chCheckBox.visible ? chCheckBox.width : 0)
            tLabel: textLabel
            faSource: FontAwesome.fa_certificate
            faColor: UISettings.fgMain

            MouseArea
            {
                anchors.fill: parent

                property bool dragActive: drag.active

                onDragActiveChanged:
                {
                    //console.log("Drag changed on function: " + cRef.id)
                    headDelegate.mouseEvent(dragActive ? App.DragStarted : App.DragFinished, headIndex, -1, headDelegate, 0)
                }

                drag.target: dragItem

                onPressed: headDelegate.mouseEvent(App.Pressed, headIndex, -1, headDelegate, mouse.modifiers)
                onClicked: headDelegate.mouseEvent(App.Clicked, headIndex, -1, headDelegate, mouse.modifiers)
                onDoubleClicked: headDelegate.mouseEvent(App.DoubleClicked, headIndex, -1, headDelegate, -1)
            }
        }
    }

    Rectangle
    {
        width: parent.width
        height: 1
        y: parent.height - 1
        color: UISettings.bgLight
    }

}
