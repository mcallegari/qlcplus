/*
  Q Light Controller Plus
  WidgetDelegate.qml

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
    id: delegateRoot
    width: 100
    height: UISettings.listItemHeight

    color: "transparent"

    property VCWidget cRef
    property string textLabel
    property string itemIcon: ""
    property int itemType: App.WidgetDragItem
    property bool isSelected: false
    property Item dragItem

    signal mouseEvent(int type, int iID, int iType, var qItem, int mouseMods)

    onCRefChanged:
    {
        if (cRef == null)
            return

        itemIcon = virtualConsole.widgetIcon(cRef.type)
        console.log("Item icon: " + itemIcon)
    }

    Rectangle
    {
        anchors.fill: parent
        radius: 3
        color: UISettings.highlight
        visible: isSelected
    }

    IconTextEntry
    {
        id: wEntry
        width: parent.width
        height: parent.height
        tLabel: cRef ? cRef.caption : textLabel
        iSrc: itemIcon
    }
    Rectangle
    {
        width: parent.width
        height: 1
        y: parent.height - 1
        color: UISettings.bgLight
    }

    MouseArea
    {
        anchors.fill: parent

        property bool dragActive: drag.active

        onDragActiveChanged:
        {
            //console.log("Drag changed on function: " + cRef.id)
            delegateRoot.mouseEvent(dragActive ? App.DragStarted : App.DragFinished, cRef.id, cRef.type, delegateRoot, 0)
        }

        drag.target: dragItem

        onPressed: delegateRoot.mouseEvent(App.Pressed, cRef.id, cRef.type, delegateRoot, mouse.modifiers)
        onClicked: delegateRoot.mouseEvent(App.Clicked, cRef.id, cRef.type, delegateRoot, mouse.modifiers)
        onDoubleClicked: delegateRoot.mouseEvent(App.DoubleClicked, cRef.id, cRef.type, delegateRoot, mouse.modifiers)
    }
}
