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
import QtQuick.Controls 2.1

import "."

Button
{
    id: control
    visible: counter ? true : false
    height: UISettings.iconSizeDefault
    width: UISettings.iconSizeDefault
    hoverEnabled: true
    padding: 0
    topPadding: 0
    bottomPadding: 0
    leftPadding: 0
    rightPadding: 0

    property int counter: 1
    property color bgColor: UISettings.bgLight
    property color hoverColor: UISettings.hover
    property color pressColor: UISettings.highlightPressed
    property color checkedColor: UISettings.highlight

    property alias border: contentBody.border
    property alias radius: contentBody.radius
    property string imgSource: ""
    property int imgMargins: 6
    property string faSource: ""
    property color faColor: UISettings.bgStrong

    property string tooltip: ""

    onCounterChanged:
    {
        if (counter == 0 && checkable && checked)
        {
            control.toggle()
            control.toggled()
        }
    }

    Rectangle
    {
        anchors.fill: parent
        color: "black"
        opacity: 0.6
        visible: !parent.enabled
    }

    ToolTip
    {
        visible: tooltip && hovered
        text: tooltip
        delay: 1000
        timeout: 5000
        background:
            Rectangle
            {
                color: UISettings.bgMedium
                border.width: 1
                border.color: UISettings.bgLight
            }
        contentItem:
            Text
            {
              text: tooltip
              color: "white"
          }
    }

    contentItem:
        Rectangle
        {
            color: "transparent"
            Image
            {
                id: btnIcon
                visible: imgSource ? true : false
                anchors.centerIn: parent
                width: Math.min(control.width - imgMargins, control.height - imgMargins)
                height: width
                source: imgSource
                sourceSize: Qt.size(width, height)
            }

            Text
            {
                id: faIcon
                visible: faSource ? true : false
                anchors.centerIn: parent
                color: faColor
                font.family: "FontAwesome"
                font.pixelSize: control.height - imgMargins - 2
                text: faSource
            }
        }

    background:
        Rectangle
        {
            id: contentBody
            color: bgColor
            radius: 5
            border.color: "#1D1D1D"
            border.width: 2

            states: [
                State
                {
                    when: checked
                    PropertyChanges
                    {
                        target: contentBody
                        color: checkedColor
                    }
                },
                State
                {
                    when: ctrlMouseArea.pressed
                    PropertyChanges
                    {
                        target: contentBody
                        color: pressColor
                    }
                },
                State
                {
                    when: hovered
                    PropertyChanges
                    {
                        target: contentBody
                        color: hoverColor
                    }
                }
            ]

            MouseArea
            {
                id: ctrlMouseArea
                anchors.fill: parent
                onClicked:
                {
                    if (checkable)
                    {
                        control.toggle()
                        control.toggled()
                    }
                    else
                        control.clicked()
                }
            }
        }

}

