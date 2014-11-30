/*
  Q Light Controller Plus
  MenuBarEntry.qml

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

import QtQuick 2.2
import QtQuick.Controls 1.2

Rectangle {
    id: menuEntry
    width: parent.height + textBox.width + 5
    height: parent.height

    property color checkedColor: "#12B4FF"

    property bool checkable: false
    property string imgSource: ""
    property string entryText: ""
    property bool checked: false
    property Gradient bgGradient: defBgGradient
    property Gradient selGradient: defSelectionGradient
    property ExclusiveGroup exclusiveGroup: null

    onExclusiveGroupChanged: {
        if (exclusiveGroup)
            exclusiveGroup.bindCheckable(menuEntry)
    }
    onCheckedChanged: {
        if (checked == true) {
            selRect.color = checkedColor;
            menuEntry.gradient = selGradient
        }
        else {
            selRect.color = "transparent";
            menuEntry.gradient = bgGradient
        }
    }

    signal clicked
    signal toggled

    gradient: bgGradient
    //border.color: "black" //"#111"
    //border.width: 1

    Gradient {
        id: defBgGradient
        GradientStop { position: 0 ; color: "#171717" }
        GradientStop { position: 1 ; color: "#000" }
    }
    Gradient {
        id: defSelectionGradient
        GradientStop { position: 0 ; color: "#444" }
        GradientStop { position: 1 ; color: "#171717" }
    }

    Rectangle {
        anchors.fill: parent
        color: "#33ffffff"
        visible: mouseArea1.pressed
    }

    Row {
        spacing: 2
        anchors.fill: parent
        anchors.leftMargin: 3

        BorderImage {
            id: btnIcon
            height: parent.height - 4
            width: height
            x: 2
            y: 2
            source: imgSource
        }

        Rectangle {
            y: 0
            width: textBox.width
            height: parent.height
            color: "transparent"

            Text {
                id: textBox
                y: 0
                text: entryText
                height: parent.height
                width: Text.paintedWidth
                color: "white"
                font.pixelSize: 12
                font.bold: true
                verticalAlignment: Text.AlignVCenter
            }

            Rectangle {
                id: selRect
                radius: 2
                color: "transparent"
                height: 5
                width: textBox.width
                y: parent.height - height - 1
            }
        }
    }

    MouseArea {
        id: mouseArea1
        anchors.fill: parent
        hoverEnabled: true
        onEntered: { if (checked == false) menuEntry.gradient = selGradient }
        onExited: { if (checked == false) menuEntry.gradient = bgGradient }
        onReleased: {
            if (checkable == true)
            {
                if (checked == false)
                    checked = true
                menuEntry.toggled(checked);
            }
            else
                menuEntry.clicked();
        }
    }
}
