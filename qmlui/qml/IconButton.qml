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

import "."

Rectangle
{
    id: baseIconButton
    width: UISettings.iconSizeDefault
    height: UISettings.iconSizeDefault
    visible: counter ? true : false

    property color bgColor: UISettings.bgLight
    property color hoverColor: UISettings.hover
    property color pressColor: UISettings.highlightPressed
    property color checkedColor: UISettings.highlight

    property bool checkable: false
    property bool checked: false
    property int counter: 1

    property ExclusiveGroup exclusiveGroup: null

    property string imgSource: ""
    property int imgMargins: 4
    property string faSource: ""
    property color faColor: "#222"

    property string tooltip: ""

    signal clicked
    signal toggled

    color: bgColor
    radius: 5
    border.color: "#1D1D1D"
    border.width: 2

    onExclusiveGroupChanged:
    {
        if (exclusiveGroup)
            exclusiveGroup.bindCheckable(baseIconButton)
    }
    onCounterChanged:
    {
        if (counter == 0)
            checked = false
    }

    states: [
        State
        {
            when: checked
            PropertyChanges
            {
                target: baseIconButton
                color: checkedColor
            }
        },
        State
        {
            when: mouseArea1.pressed
            PropertyChanges
            {
                target: baseIconButton
                color: pressColor
            }
        },
        State
        {
            when: mouseArea1.containsMouse
            PropertyChanges
            {
                target: baseIconButton
                color: hoverColor
            }
        }
    ]

    Image
    {
        id: btnIcon
        visible: imgSource ? true : false
        anchors.centerIn: parent
        width: Math.min(parent.width - imgMargins, parent.height - imgMargins)
        height: width
        anchors.margins: imgMargins
        source: imgSource
        sourceSize: Qt.size(width, height)
    }

    Text
    {
        id: faIcon
        anchors.centerIn: parent
        visible: faSource ? true : false
        color: faColor
        font.family: "FontAwesome"
        font.pixelSize: parent.height - 4
        text: faSource
    }

    MouseArea
    {
        id: mouseArea1
        anchors.fill: parent
        hoverEnabled: true
        onExited: { Tooltip.hideText() }
        onReleased:
        {
            if (checkable == true)
            {
                checked = !checked
                baseIconButton.toggled(checked)
            }
            else
                baseIconButton.clicked()
        }

        onCanceled: Tooltip.hideText()

        Timer
        {
           interval: 1000
           running: mouseArea1.containsMouse && tooltip.length
           onTriggered: Tooltip.showText(mouseArea1, Qt.point(mouseArea1.mouseX, mouseArea1.mouseY), tooltip)
        }
    }
}
