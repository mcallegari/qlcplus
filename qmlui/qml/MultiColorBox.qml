/*
  Q Light Controller Plus
  VCSliderItem.qml

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
    width: UISettings.mediumItemHeight
    height: UISettings.listItemHeight
    color: rgbValue

    property color rgbValue: "black"
    property color wauvValue: "black"

    Rectangle
    {
        anchors.fill: parent
        radius: parent.radius
        color: "white"
        opacity: wauvValue.r
    }
    Rectangle
    {
        anchors.fill: parent
        radius: parent.radius
        color: "#FF7E00"
        opacity: wauvValue.g
    }
    Rectangle
    {
        anchors.fill: parent
        radius: parent.radius
        color: "#9400D3"
        opacity: wauvValue.b
    }
}
