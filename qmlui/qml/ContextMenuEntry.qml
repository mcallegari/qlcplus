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

import QtQuick

import "."

Rectangle
{
    id: baseMenuEntry
    width: parent ? (parent.width > itemWidth) ? parent.width : itemWidth : 400
    implicitWidth: itemWidth
    height: iconHeight + 6

    property string imgSource: ""
    property string entryText: ""
    property string faSource: ""
    property color faColor: UISettings.fgMain
    property color bgColor: "transparent"
    property color hoverColor: UISettings.highlight
    property color pressColor: UISettings.highlightPressed

    property int iconHeight: UISettings.iconSizeDefault
    property int iconWidth: iconHeight
    property int itemWidth: entryRow.width + 20

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

    Row
    {
        id: entryRow
        x: 5
        spacing: 5

        Image
        {
            visible: imgSource ? true : false
            anchors.verticalCenter: parent.verticalCenter
            height: iconHeight
            width: iconWidth
            source: imgSource
            sourceSize: Qt.size(width, height)
        }

        Text
        {
            visible: faSource ? true : false
            width: iconHeight
            color: faColor
            anchors.verticalCenter: parent.verticalCenter
            horizontalAlignment: Text.AlignHCenter
            font.family: UISettings.fontAwesomeFontName
            font.pixelSize: baseMenuEntry.height * 0.80
            text: faSource
        }

        RobotoText
        {
            label: entryText
            height: baseMenuEntry.height
            fontSize: UISettings.textSizeDefault
            fontBold: true
        }
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
