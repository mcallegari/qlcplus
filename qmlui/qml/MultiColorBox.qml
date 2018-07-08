/*
  Q Light Controller Plus
  MultiColorBox.qml

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

Rectangle
{
    id: boxRoot
    width: UISettings.mediumItemHeight
    height: UISettings.listItemHeight
    color: primary

    property bool biColor: false

    // Primary color: always active. Represents the current RGB value
    property color primary: "black"

    // Secondary color:
    // biColor = false -> represents the current WAUV color
    // biColor = true -> represents the secondary color of a color wheel
    property color secondary: "black"

    Rectangle
    {
        visible: biColor ? false : true
        anchors.fill: parent
        radius: parent.radius
        color: "white"
        opacity: secondary.r
    }
    Rectangle
    {
        visible: biColor ? false : true
        anchors.fill: parent
        radius: parent.radius
        color: "#FF7E00"
        opacity: secondary.g
    }
    Rectangle
    {
        visible: biColor ? false : true
        anchors.fill: parent
        radius: parent.radius
        color: "#9400D3"
        opacity: secondary.b
    }

    Rectangle
    {
        visible: biColor ? true : false
        width: parent.width / 2
        height: parent.height
        x: parent.width / 2
        clip: true
        color: "transparent"

        Rectangle
        {
            width: boxRoot.width
            height: parent.height
            x: -width / 2
            radius: boxRoot.radius
            color: secondary
        }
    }

}
