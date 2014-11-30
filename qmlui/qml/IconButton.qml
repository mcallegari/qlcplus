/*
  Q Light Controller Plus
  IconButton.qml

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
import QtQuick.Controls.Styles 1.1
import QtQuick.Controls.Private 1.0

Rectangle {
    id: baseIconButton
    width: 38
    height: 38

    property color bgColor: "#6F6F6F"
    property color hoverColor: "#0978FF"
    property color pressColor: "#054A9E"
    property color checkedColor: "#0978FF"

    property bool isCheckable: false
    property bool isChecked: false
    property string imgSource: ""

    property string tooltip: ""

    signal clicked
    signal checked

    color: bgColor
    radius: 5
    border.color: "#1D1D1D"
    border.width: 2

    BorderImage {
        id: btnIcon
        anchors.fill: parent
        anchors.margins: 4
        source: imgSource
    }

    MouseArea {
        id: mouseArea1
        anchors.fill: parent
        hoverEnabled: true
        onEntered: { if (isCheckable == false) baseIconButton.color = hoverColor }
        onExited: { if (isCheckable == false) baseIconButton.color = bgColor; Tooltip.hideText() }
        onPressed: { if (isCheckable == false) baseIconButton.color = pressColor }
        onReleased: {
            if (isCheckable == false)
            {
                baseIconButton.color = hoverColor
                baseIconButton.clicked();
            }
            else
            {
                if (isChecked == false) {
                    baseIconButton.color = checkedColor;
                    isChecked = true
                }
                else {
                    baseIconButton.color = bgColor;
                    isChecked = false
                }
                baseIconButton.checked();
            }
        }

        onCanceled: Tooltip.hideText()

        Timer {
           interval: 1000
           running: mouseArea1.containsMouse && tooltip.length
           onTriggered: Tooltip.showText(mouseArea1, Qt.point(mouseArea1.mouseX, mouseArea1.mouseY), tooltip)
        }

    }
}
