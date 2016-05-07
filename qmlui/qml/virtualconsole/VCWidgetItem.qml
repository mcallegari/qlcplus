/*
  Q Light Controller Plus
  VCWidgetItem.qml

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

Rectangle
{
    id: wRoot
    x: wObj ? wObj.geometry.x : 0
    y: wObj ? wObj.geometry.y : 0
    width: wObj ? wObj.geometry.width : 100
    height: wObj ? wObj.geometry.height : 100
    color: wObj ? wObj.backgroundColor : "darkgray"
    border.width: 1
    border.color: "#111"
    visible: wObj ? wObj.isVisible : true

    property VCWidget wObj: null
    property bool isSelected: false

    onIsSelectedChanged:
    {
        if (wObj)
            wObj.isEditing = isSelected
    }

    function setCommonProperties(obj)
    {
        if (obj === null)
            return;

        wObj = obj
    }

    // resize area
    Rectangle
    {
        id: resizeLayer
        anchors.fill: parent
        color: "transparent"
        border.width: isSelected ? 2 : 1
        border.color: isSelected ? "yellow" : "#111"
        // this must be above the widget root but
        // underneath the widget children (if any)
        z: isSelected ? 99 : 1
        visible: virtualConsole.editMode && wObj && wObj.allowResize

        // mouse area to select and move the widget
        MouseArea
        {
            anchors.fill: parent

            onPressed:
            {
                if (virtualConsole.editMode)
                {
                    isSelected = !isSelected
                    virtualConsole.setWidgetSelection(wObj.id, wRoot, isSelected)
                }

                drag.target = wRoot
                drag.minimumX = 0
                drag.minimumY = 0
                drag.threshold = 10
                drag.maximumX = wRoot.parent.width - wRoot.width
                drag.maximumY = wRoot.parent.height - wRoot.height
            }

            onReleased:
            {
                if (drag.target !== null)
                {
                    drag.target = null
                    wObj.geometry = Qt.rect(wRoot.x, wRoot.y, wRoot.width, wRoot.height)
                }
            }
        }


        // top-left corner
        Image
        {
            id: tlHandle
            rotation: 180
            source: "qrc:/arrow-corner.svg"
            sourceSize: Qt.size(32, 32)
            visible: isSelected && wObj && wObj.allowResize

            MouseArea
            {
                anchors.fill: parent
                cursorShape: pressed ? Qt.ClosedHandCursor : Qt.OpenHandCursor

                onPressed:
                {
                    drag.target = tlHandle
                    drag.threshold = 0
                }
                onPositionChanged:
                {
                    if (drag.target === null)
                        return;
                    wRoot.x += tlHandle.x
                    wRoot.width -= tlHandle.x
                    wRoot.height -= tlHandle.y
                    wRoot.y += tlHandle.y
                }
                onReleased:
                {
                    drag.target = null
                    wObj.geometry = Qt.rect(wRoot.x, wRoot.y, wRoot.width, wRoot.height)
                }
            }
        }
        // top-right corner
        Image
        {
            id: trHandle
            x: parent.width - 32
            rotation: 270
            source: "qrc:/arrow-corner.svg"
            sourceSize: Qt.size(32, 32)
            visible: isSelected && wObj && wObj.allowResize

            MouseArea
            {
                anchors.fill: parent
                cursorShape: pressed ? Qt.ClosedHandCursor : Qt.OpenHandCursor

                onPressed:
                {
                    drag.target = trHandle
                    drag.threshold = 0
                }
                onPositionChanged:
                {
                    if (drag.target === null)
                        return;
                    wRoot.width = trHandle.x + trHandle.width
                    wRoot.height -= trHandle.y
                    wRoot.y += trHandle.y
                }
                onReleased:
                {
                    drag.target = null
                    wObj.geometry = Qt.rect(wRoot.x, wRoot.y, wRoot.width, wRoot.height)
                }
            }
        }
        // bottom-right corner
        Image
        {
            id: brHandle
            x: parent.width - 32
            y: parent.height - 32
            source: "qrc:/arrow-corner.svg"
            sourceSize: Qt.size(32, 32)
            visible: isSelected && wObj && wObj.allowResize

            MouseArea
            {
                anchors.fill: parent
                cursorShape: pressed ? Qt.ClosedHandCursor : Qt.OpenHandCursor
                onPressed:
                {
                    drag.target = brHandle
                    drag.threshold = 0
                }
                onPositionChanged:
                {
                    if (drag.target === null)
                        return;
                    wRoot.width = brHandle.x + brHandle.width
                    wRoot.height = brHandle.y + brHandle.height
                }
                onReleased:
                {
                    drag.target = null
                    wObj.geometry = Qt.rect(wRoot.x, wRoot.y, wRoot.width, wRoot.height)
                }
            }
        }
        // bottom-left corner
        Image
        {
            id: blHandle
            rotation: 90
            y: parent.height - 32
            source: "qrc:/arrow-corner.svg"
            sourceSize: Qt.size(32, 32)
            visible: isSelected && wObj && wObj.allowResize

            MouseArea
            {
                anchors.fill: parent
                cursorShape: pressed ? Qt.ClosedHandCursor : Qt.OpenHandCursor

                onPressed:
                {
                    drag.target = blHandle
                    drag.threshold = 0
                }
                onPositionChanged:
                {
                    if (drag.target === null)
                        return;
                    wRoot.height = blHandle.y + blHandle.height
                    wRoot.x += blHandle.x
                    wRoot.width -= blHandle.x
                }
                onReleased:
                {
                    drag.target = null
                    wObj.geometry = Qt.rect(wRoot.x, wRoot.y, wRoot.width, wRoot.height)
                }
            }
        }
    }
}
