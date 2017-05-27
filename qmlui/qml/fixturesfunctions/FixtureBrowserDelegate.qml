/*
  Q Light Controller Plus
  FixtureBrowserDelegate.qml

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
import "FixtureDrag.js" as FxDragJS
import "."

Item
{
    id: fxDraggableItem
    width: parent.width - 30
    height: UISettings.listItemHeight

    property bool isManufacturer: false
    property string iconSource: ""
    property string manufacturer: ""
    property string textLabel
    property int channels: 1
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

    Image
    {
        id: entryIcon
        visible: iconSource ? true : false
        anchors.right: parent.right
        anchors.rightMargin: 5
        anchors.verticalCenter: parent.verticalCenter
        source: iconSource
        height: parent.height - 2
        width: height
    }

    RobotoText
    {
        id: textitem
        x: 2
        width: parent.width
        label: textLabel
        height: parent.height
        fontSize: UISettings.textSizeDefault
        //fontBold: true
    }

    Rectangle
    {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 5
        height: 1
        color: "#424246"
    }

    Image
    {
        id: rightArrow
        visible: isManufacturer
        anchors.right: parent.right
        anchors.rightMargin: 5
        anchors.verticalCenter: parent.verticalCenter
        source: "qrc:/arrow-right.svg"
        height: 26
        width: 20
    }

    MouseArea
    {
        id: fxMouseArea
        anchors.fill: parent
        hoverEnabled: true
        onClicked: fxDraggableItem.mouseEvent(App.Clicked, 0, 0, fxDraggableItem, mouse.modifiers)
        drag.target: FixtureDragItem { }
        drag.threshold: 30

        onPressed:
        {
            if (fxDraggableItem.isManufacturer == false)
            {
                fxDraggableItem.mouseEvent(App.Clicked, 0, 0, fxDraggableItem, mouse.modifiers);
                FxDragJS.initProperties();
            }
        }
        onPositionChanged:
            if(fxDraggableItem.isManufacturer == false && drag.active == true)
                FxDragJS.handleDrag(mouse);
        onReleased:
            if(fxDraggableItem.isManufacturer == false && drag.active == true)
                FxDragJS.endDrag(mouse);
    }
}
