/*
  Q Light Controller Plus
  FunctionDelegate.qml

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
    id: funcDelegate
    width: 100
    height: UISettings.listItemHeight

    color: "transparent"

    property QLCFunction cRef
    property string textLabel
    property string itemIcon: ""
    property int itemType: App.FunctionDragItem
    property bool isSelected: false
    property bool isCheckable: false
    property bool isChecked: false
    property Item dragItem

    onCRefChanged:
    {
        if (cRef == null)
            return

        itemIcon = functionManager.functionIcon(cRef.type)
        if (cRef.type == QLCFunction.SceneType)
            fdDropArea.keys = [ "dumpValues", "function" ]
    }

    signal toggled
    signal destruction(int ID, var qItem)
    signal mouseEvent(int type, int iID, int iType, var qItem, int mouseMods)

    Component.onDestruction:
    {
        if (cRef)
            funcDelegate.destruction(cRef.id, funcDelegate)
    }

    Rectangle
    {
        anchors.fill: parent
        radius: 3
        color: UISettings.highlight
        visible: isSelected || fdDropArea.containsDrag
    }

    Row
    {
        anchors.fill: parent

        CustomCheckBox
        {
            id: chCheckBox
            visible: isCheckable
            implicitWidth: UISettings.listItemHeight
            implicitHeight: implicitWidth
            checked: isChecked
            onCheckedChanged: funcDelegate.mouseEvent(App.Checked, cRef.id, checked, funcDelegate, 0)
        }

        IconTextEntry
        {
            id: funcEntry
            width: funcDelegate.width - (chCheckBox.visible ? chCheckBox.width : 0)
            height: funcDelegate.height
            tLabel: cRef ? cRef.name : textLabel
            functionType: cRef ? cRef.type : -1

            MouseArea
            {
                anchors.fill: parent

                property bool dragActive: drag.active

                onDragActiveChanged:
                {
                    //console.log("Drag changed on function: " + cRef.id)
                    funcDelegate.mouseEvent(dragActive ? App.DragStarted : App.DragFinished, cRef.id, cRef.type, funcDelegate, 0)
                }

                drag.target: dragItem

                onPressed: funcDelegate.mouseEvent(App.Pressed, cRef.id, cRef.type, funcDelegate, mouse.modifiers)
                onClicked: funcDelegate.mouseEvent(App.Clicked, cRef.id, cRef.type, funcDelegate, mouse.modifiers)
                onDoubleClicked: funcDelegate.mouseEvent(App.DoubleClicked, cRef.id, cRef.type, funcDelegate, mouse.modifiers)
            }

            DropArea
            {
                id: fdDropArea
                anchors.fill: parent
                keys: [ "function" ]

                onDropped: drag.source.itemDropped(cRef.id, cRef.name)
            }
        }
    }

    Image
    {
        anchors.right: parent.right
        source: "qrc:/autostart.svg"
        height: UISettings.listItemHeight
        width: height
        sourceSize: Qt.size(width, height)
        visible: cRef && cRef.id === functionManager.startupFunctionID ? true : false
    }

    Rectangle
    {
        width: parent.width
        height: 1
        y: parent.height - 1
        color: UISettings.bgLight
    }
}

