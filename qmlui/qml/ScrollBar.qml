/*
  Q Light Controller Plus
  ScrollBar.qml

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

Item {
    id: scrollbar
    width: (handleSize + 2 * (backScrollbar.border.width +1))
    visible: (flickable.visibleArea.heightRatio < 1.0)
    anchors {
        top: flickable.top
        right: flickable.right
        bottom: flickable.bottom
        margins: 1
    }

    property Flickable flickable               : null
    property int       handleSize              : 20

    function scrollDown () {
        flickable.contentY = Math.min (flickable.contentY + (flickable.height / 4), flickable.contentHeight - flickable.height);
    }
    function scrollUp () {
        flickable.contentY = Math.max (flickable.contentY - (flickable.height / 4), 0);
    }

   Binding {
        target: handle
        property: "y"
        value: (flickable.contentY * clicker.drag.maximumY / (flickable.contentHeight - flickable.height))
        when: (!clicker.drag.active)
    }
    Binding {
        target: flickable
        property: "contentY"
        value: (handle.y * (flickable.contentHeight - flickable.height) / clicker.drag.maximumY)
        when: (clicker.drag.active || clicker.pressed)
    }
    Rectangle {
        id: backScrollbar
        radius: 2
        antialiasing: true
        color: "#444"
        border.width: 1
        border.color: "#333"
        anchors.fill: parent

        MouseArea {
            anchors.fill: parent
            onClicked: { }
        }
    }
    MouseArea {
        id: btnUp
        height: width
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
            margins: (backScrollbar.border.width +1)
        }
        onClicked: { scrollUp (); }

        Rectangle {
            anchors.fill: parent
            anchors.centerIn: parent
            color: (btnUp.pressed ? "#0978FF" : "#333")

            Image {
                anchors.centerIn: parent
                source: "qrc:/arrow-down.svg"
                rotation: 180
                sourceSize: Qt.size(parent.width - 2, parent.height)
            }
        }
    }
    MouseArea {
        id: btnDown
        height: width
        anchors {
            left: parent.left
            right: parent.right
            bottom: parent.bottom
            margins: (backScrollbar.border.width +1)
        }
        onClicked: { scrollDown (); }

        Rectangle {
            anchors.fill: parent
            anchors.centerIn: parent
            color: (btnDown.pressed ? "#0978FF" : "#333")

            Image {
                anchors.centerIn: parent
                source: "qrc:/arrow-down.svg"
                sourceSize: Qt.size(parent.width - 2, parent.height)
            }
        }
    }
    Item {
        id: groove
        clip: true
        anchors {
            fill: parent
            topMargin: (backScrollbar.border.width +1 + btnUp.height +1)
            leftMargin: (backScrollbar.border.width +1)
            rightMargin: (backScrollbar.border.width +1)
            bottomMargin: (backScrollbar.border.width +1 + btnDown.height +1)
        }

        MouseArea {
            id: clicker
            drag {
                target: handle
                minimumY: 0;
                maximumY: (groove.height - handle.height)
                axis: Drag.YAxis
            }
            anchors { fill: parent; }
            onClicked: { flickable.contentY = (mouse.y / groove.height * (flickable.contentHeight - flickable.height)); }
        }
        Item {
            id: handle
            height: Math.max (20, (flickable.visibleArea.heightRatio * groove.height))
            anchors {
                left: parent.left
                right: parent.right
            }

            Rectangle {
                id: backHandle
                radius: 2
                color: (clicker.pressed ? "white" : "black")
                opacity: (flickable.moving ? 0.65 : 0.35)
                anchors { fill: parent; }

                Behavior on opacity { NumberAnimation { duration: 150; } }
            }
        }
    }
} 
