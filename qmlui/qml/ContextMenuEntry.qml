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

import "."

Rectangle
{
    id: baseMenuEntry
    width: parent ? (parent.width > itemWidth) ? parent.width : itemWidth : 400
    height: imgSize + 4

    property int imgSize: UISettings.iconSizeDefault
    property string imgSource: ""
    property string entryText: ""
    property color bgColor: "transparent"
    property color hoverColor: UISettings.highlight
    property color pressColor: UISettings.highlightPressed
    property int itemWidth: imgSize + (textBox ? textBox.width : 100) + 15

    signal clicked
    signal entered
    signal exited

    color: bgColor
    border.color: UISettings.bgLight
    border.width: 1

    states: [
        State
        {
            when: entryMouseArea.pressed
            PropertyChanges
            {
                target: baseMenuEntry
                color: pressColor
            }
        },
        State
        {
            when: entryMouseArea.containsMouse
            PropertyChanges
            {
                target: baseMenuEntry
                color: hoverColor
            }
        }
    ]

    Image
    {
        id: btnIcon
        height: imgSource ? imgSize : 0
        width: height
        x: 5
        y: 2
        source: imgSource
        sourceSize: Qt.size(width, height)
    }

    RobotoText
    {
        id: textBox
        x: btnIcon.width + 7
        y: 0
        label: entryText
        height: baseMenuEntry.height
        fontSize: UISettings.textSizeDefault
        fontBold: true
    }

    MouseArea
    {
        id: entryMouseArea
        anchors.fill: parent
        hoverEnabled: baseMenuEntry.visible
        onEntered: baseMenuEntry.entered()
        onExited: baseMenuEntry.exited()
        onReleased: baseMenuEntry.clicked()
    }

    Rectangle
    {
        anchors.fill: parent
        color: "black"
        opacity: 0.6
        visible: !parent.enabled
    }
}
