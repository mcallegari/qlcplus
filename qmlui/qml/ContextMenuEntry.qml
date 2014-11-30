/*
  Q Light Controller Plus
  ContextMenuEntry.qml

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
    id: baseIconEntry
    width: (parent.width > imgSize + textBox.width + 5) ? parent.width: imgSize + textBox.width + 5
    height: imgSize + 4

    property int imgSize: 40
    property string imgSource: ""
    property string entryText: ""
    property color bgColor: "transparent"
    property color hoverColor: "#0978FF"
    property color pressedColor: "#054A9E"

    signal clicked

    color: "transparent"
    border.color: "#1D1D1D"
    border.width: 1

    Gradient {
        id: hoverGradient
        GradientStop { position: 0 ; color: "#444" }
        GradientStop { position: 1 ; color: "#171717" }
    }
    Gradient {
        id: pressGradient
        GradientStop { position: 0 ; color: "#6BA6FF" }
        GradientStop { position: 1 ; color: "#171717" }
    }

    Row {
        spacing: 2
        anchors.fill: parent

        BorderImage {
            id: btnIcon
            height: parent.height - 4
            width: height
            x: 5
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
        }
    }

    MouseArea {
        id: mouseArea1
        anchors.fill: parent
        hoverEnabled: true
        onEntered: { baseIconEntry.color = hoverColor }
        onExited: { baseIconEntry.color = bgColor }
        onPressed: { baseIconEntry.color = pressedColor }
        onReleased: {
                baseIconEntry.clicked();
        }
    }
}
