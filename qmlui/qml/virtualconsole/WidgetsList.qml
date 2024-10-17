/*
  Q Light Controller Plus
  WidgetsList.qml

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
import QtQuick.Controls 2.1

import "."

Rectangle
{
    id: widgetsContainer
    anchors.fill: parent
    color: "transparent"

    ListModel
    {
        id: wModel
        ListElement { name: qsTr("Frame"); type: "Frame"; icon: "frame" }
        ListElement { name: qsTr("Solo Frame"); type: "Solo frame"; icon: "soloframe" }
        ListElement { name: qsTr("Button"); type: "Button"; icon: "button" }
        ListElement { name: qsTr("Button Matrix"); type: "buttonmatrix"; icon: "buttonmatrix" }
        ListElement { name: qsTr("Slider"); type: "Slider"; icon: "slider" }
        ListElement { name: qsTr("Slider Matrix"); type: "slidermatrix"; icon: "sliders" }
        ListElement { name: qsTr("Knob"); type: "Knob"; icon: "knob" }
        ListElement { name: qsTr("Cue List"); type: "Cue list"; icon: "cuelist" }
        ListElement { name: qsTr("Speed"); type: "Speed"; icon: "speed" }
        ListElement { name: qsTr("XY Pad"); type: "XYPad"; icon: "xypad" }
        ListElement { name: qsTr("Animation"); type: "Animation"; icon: "animation" }
        ListElement { name: qsTr("Label"); type: "Label"; icon: "label" }
        ListElement { name: qsTr("Audio Triggers"); type: "Audio Triggers"; icon: "audiotriggers" }
        ListElement { name: qsTr("Clock"); type: "Clock"; icon: "clock" }
    }

    ListView
    {
        id: widgetListView
        anchors.fill: parent
        boundsBehavior: Flickable.StopAtBounds
        model: wModel
        delegate:
            Item
            {
                id: root
                height: UISettings.listItemHeight * 1.7
                width: widgetsContainer.width

                MouseArea
                {
                    id: delegateRoot
                    width: widgetsContainer.width
                    height: parent.height

                    drag.target: widgetItem
                    drag.threshold: height / 2

                    onReleased:
                    {
                        if (widgetItem.Drag.target !== null)
                        {
                            // emit a drop event, for the active DropArea
                            widgetItem.Drag.drop()
                        }
                        else
                        {
                            // return the dragged item to its original position
                            parent = root
                        }
                        widgetItem.x = 3
                        widgetItem.y = 0
                    }
                    WidgetDragItem
                    {
                        id: widgetItem
                        x: 3

                        widgetName: name
                        widgetType: type
                        widgetIconName: icon

                        Drag.active: delegateRoot.drag.active
                        Drag.source: widgetItem
                        Drag.hotSpot.x: height / 2
                        Drag.hotSpot.y: height / 2
                        Drag.keys: [ "vcwidget" ]

                        // line divider
                        Rectangle
                        {
                            width: parent.width - 6
                            height: 1
                            y: parent.height - 1
                            color: UISettings.bgLight
                            visible: widgetItem.reduced ? false : true
                        }
                    } // WidgetDragItem
                } // MouseArea
            } // Item
        ScrollBar.vertical: CustomScrollBar { }
    } // ListView
}

