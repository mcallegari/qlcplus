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

import "FixtureDrag.js" as FxDragJS

Item
{
    id: fxDraggableItem
    width: parent.width - 30
    height: 32

    //property alias text: textitem.text
    property bool isManufacturer: false
    property string iconSource: ""
    property string manufacturer: ""
    property int channels: 1
    signal clicked

    Rectangle
    {
        anchors.fill: parent
        color: "#11ffffff"
        visible: fxMouseArea.pressed
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
        label: modelData
        height: parent.height
        fontSize: 12
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
        onClicked: fxDraggableItem.clicked()
        //onEntered: rightArrow.visible = true
        //onExited: rightArrow.visible = false
        drag.target: FixtureDragItem { }
        drag.threshold: 30

        //onPressed: if(drag.active) FxDragJS.startDrag(mouse);
        onPressed:
        {
            if (fxDraggableItem.isManufacturer == false)
            {
                fxDraggableItem.clicked();
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
