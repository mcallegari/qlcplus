/*
  Q Light Controller Plus
  CustomScrollBar.qml

  Copyright (c) Massimo Callegari
  Copied from StackOverflow and customized

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
import "."

Item
{
    id: scrollbar
    width: orientation === Qt.Vertical ? (handleSize + 2 * (backScrollbar.border.width +1)) : undefined
    height: orientation === Qt.Vertical ? undefined : (handleSize + 2 * (backScrollbar.border.width +1))
    visible: orientation === Qt.Vertical ? (flickable.visibleArea.heightRatio < 1.0) :
                                           (flickable.visibleArea.widthRatio < 1.0)
    anchors
    {
        top: orientation === Qt.Vertical ? flickable.top : undefined
        bottom: flickable.bottom
        left: orientation === Qt.Horizontal ? flickable.left : undefined
        right: flickable.right
        bottomMargin: doubleBars === true ? handleSize : 0
    }

    property Flickable flickable  : null
    property int       handleSize : UISettings.scrollBarWidth
    property int      orientation : Qt.Vertical
    property bool      doubleBars : false

    function scrollDown()
    {
        flickable.contentY = Math.min(flickable.contentY + (flickable.height / 4),
                                      flickable.contentHeight - flickable.height);
    }
    function scrollUp()
    {
        flickable.contentY = Math.max(flickable.contentY - (flickable.height / 4), 0);
    }
    function scrollLeft()
    {
        flickable.contentX = Math.min(flickable.contentX + (flickable.width / 4),
                                      flickable.contentWidth - flickable.width);
    }
    function scrollRight()
    {
        flickable.contentX = Math.max(flickable.contentX - (flickable.width / 4), 0);
    }

    Binding
    {
        target: handle
        property: orientation === Qt.Vertical ? "y" : "x"
        value: orientation === Qt.Vertical ?
                   (flickable.contentY * clicker.drag.maximumY / (flickable.contentHeight - flickable.height)) :
                   (flickable.contentX * clicker.drag.maximumX / (flickable.contentWidth - flickable.width))
        when: (!clicker.drag.active)
    }

    Binding
    {
        target: flickable
        property: orientation === Qt.Vertical ? "contentY" : "contentX"
        value: orientation === Qt.Vertical ?
                   (handle.y * (flickable.contentHeight - flickable.height) / clicker.drag.maximumY) :
                   (handle.x * (flickable.contentWidth - flickable.width) / clicker.drag.maximumX)
        when: (clicker.drag.active || clicker.pressed)
    }

    Rectangle
    {
        id: backScrollbar
        radius: 2
        antialiasing: true
        color: "#444"
        border.width: 1
        border.color: "#333"
        anchors.fill: parent

        MouseArea
        {
            anchors.fill: parent
            onClicked: { }
        }
    }
    MouseArea
    {
        id: btnUp

        anchors
        {
            top: parent.top
            left: parent.left
            right: orientation === Qt.Vertical ? parent.right : undefined
            bottom: orientation === Qt.Vertical ? undefined : parent.bottom
            margins: (backScrollbar.border.width +1)
        }
        onClicked: { orientation === Qt.Vertical ? scrollUp() : scrollRight() }

        Component.onCompleted:
        {
            if (orientation === Qt.Vertical)
                height = width
            else
                width = height
        }

        Rectangle
        {
            anchors.fill: parent
            anchors.centerIn: parent
            color: (btnUp.pressed ? UISettings.highlight : UISettings.bgMedium)

            Image
            {
                anchors.centerIn: parent
                source: "qrc:/arrow-down.svg"
                rotation: orientation === Qt.Vertical ? 180 : 90
                sourceSize: Qt.size(parent.width - 2, parent.height)
            }
        }
    }
    MouseArea
    {
        id: btnDown

        anchors
        {
            left: orientation === Qt.Vertical ? parent.left : undefined
            right: parent.right
            bottom: parent.bottom
            top: orientation === Qt.Vertical ? undefined : parent.top
            margins: (backScrollbar.border.width +1)
        }
        onClicked: { orientation === Qt.Vertical ? scrollDown() : scrollLeft() }

        Component.onCompleted:
        {
            if (orientation === Qt.Vertical)
                height = width
            else
                width = height
        }

        Rectangle
        {
            anchors.fill: parent
            anchors.centerIn: parent
            color: (btnDown.pressed ? UISettings.highlight : UISettings.bgMedium)

            Image
            {
                anchors.centerIn: parent
                source: "qrc:/arrow-down.svg"
                rotation: orientation === Qt.Vertical ? 0 : 270
                sourceSize: Qt.size(parent.width - 2, parent.height)
            }
        }
    }
    Item
    {
        id: groove
        clip: true
        anchors
        {
            fill: parent
            topMargin: orientation === Qt.Vertical ? (backScrollbar.border.width + 1 + btnUp.height + 1) :
                                                     backScrollbar.border.width + 1
            leftMargin: orientation === Qt.Vertical ? (backScrollbar.border.width + 1) :
                                                      (backScrollbar.border.width + 1 + btnDown.width + 1)
            rightMargin: orientation === Qt.Vertical ? (backScrollbar.border.width + 1) :
                                                       (backScrollbar.border.width + 1 + btnUp.width + 1)
            bottomMargin: orientation === Qt.Vertical ? (backScrollbar.border.width + 1 + btnDown.height + 1) :
                                                        backScrollbar.border.width + 1
        }

        MouseArea
        {
            id: clicker
            anchors.fill: parent

            drag
            {
                target: handle
                minimumY: 0
                maximumY: orientation === Qt.Vertical ? (groove.height - handle.height) : 0
                minimumX: 0
                maximumX: orientation === Qt.Vertical ? 0 : (groove.width - handle.width)
                axis: orientation === Qt.Vertical ? Drag.YAxis : Drag.XAxis
            }

            onClicked:
            {
                if (orientation === Qt.Vertical)
                    flickable.contentY = (mouse.y / groove.height * (flickable.contentHeight - flickable.height))
                else
                    flickable.contentX = (mouse.x / groove.width * (flickable.contentWidth - flickable.width))
            }
        }
        Item
        {
            id: handle

            anchors
            {
                left: orientation === Qt.Vertical ? parent.left : undefined
                right: orientation === Qt.Vertical ? parent.right : undefined
                top: orientation === Qt.Vertical ? undefined : parent.top
                bottom: orientation === Qt.Vertical ? undefined : parent.bottom
            }

            Component.onCompleted:
            {
                if (orientation === Qt.Vertical)
                    height = Qt.binding(function() { return Math.max(UISettings.iconSizeMedium, (flickable.visibleArea.heightRatio * groove.height)) })
                else
                    width = Qt.binding(function() { return Math.max(UISettings.iconSizeMedium, (flickable.visibleArea.widthRatio * groove.width)) })
            }

            Rectangle
            {
                id: backHandle
                anchors.fill: parent
                radius: 2
                color: (clicker.pressed ? "white" : "black")
                opacity: (flickable.moving ? 0.65 : 0.35)

                Behavior on opacity { NumberAnimation { duration: 150 } }
            }
        }
    }
} 
