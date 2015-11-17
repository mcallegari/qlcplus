/*
  Q Light Controller Plus
  TimeEditTool.qml

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

import QtQuick 2.0
import "."

Column
{
    property color bgCol: "#2D3492"
    property int btnFontSize: 12

    // top row: close, increase values
    Row
    {
        Rectangle
        {
            width: 35
            height: 35
            color: bgCol
            border.width: 1
            border.color: "#333"

            Text
            {
                id: faIcon
                anchors.centerIn: parent
                color: "white"
                font.family: "FontAwesome"
                font.pointSize: btnFontSize
                text: FontAwesome.fa_times
            }
        }

        Rectangle
        {
            width: 35
            height: 35
            color: bgCol
            border.width: 1
            border.color: "#333"

            RobotoText
            {
                anchors.centerIn: parent
                label: "+H"
                fontSize: btnFontSize
            }
        }
        Rectangle
        {
            width: 35
            height: 35
            color: bgCol
            border.width: 1
            border.color: "#333"

            RobotoText
            {
                anchors.centerIn: parent
                label: "+M"
                fontSize: btnFontSize
            }
        }

        Rectangle
        {
            width: 40
            height: 35
            color: bgCol
            border.width: 1
            border.color: "#333"

            RobotoText
            {
                anchors.centerIn: parent
                label: "+S"
                fontSize: btnFontSize
            }
        }

        Rectangle
        {
            width: 40
            height: 35
            color: bgCol
            border.width: 1
            border.color: "#333"

            RobotoText
            {
                anchors.centerIn: parent
                label: "+ms"
                fontSize: btnFontSize
            }
        }
    }

    // middle row: tap, values
    Row
    {
        Rectangle
        {
            width: 35
            height: 35
            color: bgCol
            border.width: 1
            border.color: "#333"

            RobotoText
            {
                anchors.centerIn: parent
                label: "Tap"
                fontSize: btnFontSize
            }
        }
        Rectangle
        {
            width: 35
            height: 35
            color: "#444"
            border.width: 1
            border.color: "#333"

            CustomTextEdit
            {
                anchors.fill: parent
                inputText: "0h"
                fontSize: btnFontSize
            }
        }
        Rectangle
        {
            width: 35
            height: 35
            color: "#444"
            border.width: 1
            border.color: "#333"

            CustomTextEdit
            {
                anchors.fill: parent
                inputText: "0m"
                fontSize: btnFontSize
            }
        }
        Rectangle
        {
            width: 40
            height: 35
            color: "#444"
            border.width: 1
            border.color: "#333"

            CustomTextEdit
            {
                anchors.fill: parent
                inputText: "0s"
                fontSize: btnFontSize
            }
        }
        Rectangle
        {
            width: 40
            height: 35
            color: "#444"
            border.width: 1
            border.color: "#333"

            CustomTextEdit
            {
                anchors.fill: parent
                inputText: ".00"
                fontSize: btnFontSize
            }
        }

    }

    // bottom row: infinite, decrease values
    Row
    {
        Rectangle
        {
            width: 35
            height: 35
            color: bgCol
            border.width: 1
            border.color: "#333"

            RobotoText
            {
                anchors.centerIn: parent
                label: "∞"
                fontSize: btnFontSize
            }
        }

        Rectangle
        {
            width: 35
            height: 35
            color: bgCol
            border.width: 1
            border.color: "#333"

            RobotoText
            {
                anchors.centerIn: parent
                label: "-H"
                fontSize: btnFontSize
            }
        }
        Rectangle
        {
            width: 35
            height: 35
            color: bgCol
            border.width: 1
            border.color: "#333"

            RobotoText
            {
                anchors.centerIn: parent
                label: "-M"
                fontSize: btnFontSize
            }
        }

        Rectangle
        {
            width: 40
            height: 35
            color: bgCol
            border.width: 1
            border.color: "#333"

            RobotoText
            {
                anchors.centerIn: parent
                label: "-S"
                fontSize: btnFontSize
            }
        }

        Rectangle
        {
            width: 40
            height: 35
            color: bgCol
            border.width: 1
            border.color: "#333"

            RobotoText
            {
                anchors.centerIn: parent
                label: "-ms"
                fontSize: btnFontSize
            }
        }
    }
}
