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
import com.qlcplus.classes 1.0

import "FunctionDrag.js" as FuncDragJS
import "."

Rectangle
{
    id: funcDelegate
    width: 100
    height: 35

    color: "transparent"

    property Function cRef
    property string textLabel
    property bool isSelected: false

    signal toggled
    signal clicked(int ID, var qItem, int mouseMods)
    signal doubleClicked(int ID, int Type)
    signal destruction(int ID, var qItem)

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
        visible: isSelected
    }

    IconTextEntry
    {
        id: funcEntry
        width: parent.width
        height: parent.height
        tLabel: cRef ? cRef.name : textLabel
        functionType: cRef ? cRef.type : -1
    }
    Rectangle
    {
        width: parent.width
        height: 1
        y: parent.height - 1
        color: "#666"
    }

    MouseArea
    {
        id: funcMouseArea
        anchors.fill: parent

        drag.target:
            FunctionDragItem
            {
                funcID: cRef ? cRef.id : -1
                funcLabel: cRef ? cRef.name : textLabel
                funcIcon: funcEntry.iSrc
            }
        drag.threshold: 30

        onPressed: FuncDragJS.initProperties(cRef.id, textLabel, funcEntry.iSrc);

        onPositionChanged:
            if(drag.active == true)
                FuncDragJS.handleDrag(mouse);
        onReleased:
            if(drag.active == true)
                FuncDragJS.endDrag(mouse);

        onClicked:
        {
            // inform the upper layers of the click.
            // A ModelSelector will be in charge to actually select this item
            funcDelegate.clicked(cRef.id, funcDelegate, mouse.modifiers)
        }
        onDoubleClicked:
        {
            funcDelegate.doubleClicked(cRef.id, cRef.type)
        }
    }
}

