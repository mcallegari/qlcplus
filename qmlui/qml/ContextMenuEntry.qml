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

import "."

Rectangle
{
    id: baseMenuEntry
    width: parent ? (parent.width > itemWidth) ? parent.width : itemWidth : 400
    implicitWidth: itemWidth
    height: iconHeight + 4

    property int iconHeight: UISettings.iconSizeDefault
    property int iconWidth: iconHeight
    property string imgSource: ""
    property string entryText: ""
    property color bgColor: "transparent"
    property color hoverColor: UISettings.highlight
    property color pressColor: UISettings.highlightPressed
    property int itemWidth: btnIcon.width + (textBox ? textBox.width : 100) + 15

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
        height: imgSource ? iconHeight : 0
        width: imgSource ? iconWidth : 0
        x: 5
        y: 2
        source: imgSource
        sourceSize: Qt.size(width, height)
    }

    RobotoText
    {
        id: textBox
        x: btnIcon.x + btnIcon.width + 2
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
