/*
  Q Light Controller Plus
  CustomSlider.qml

  Copyright (c) Massimo Callegari
  Copied from StackOverflow and customized

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

import QtQuick 2.9
import QtQuick.Controls 2.14
import "."

Slider
{
    id: control
    orientation: Qt.Horizontal
    from: 0
    to: 100

    background:
        Rectangle
        {
            x: control.leftPadding
            y: control.topPadding + control.availableHeight / 2 - height / 2
            implicitWidth: 200
            implicitHeight: UISettings.listItemHeight * 0.15
            width: control.availableWidth
            height: implicitHeight
            radius: height / 2
            color: UISettings.bgLight

            Rectangle
            {
                width: control.visualPosition * parent.width
                height: parent.height
                color: UISettings.highlight
                radius: height / 2
            }
        }

    handle:
        Rectangle
        {
            x: control.leftPadding + control.visualPosition * (control.availableWidth - width)
            y: control.topPadding + control.availableHeight / 2 - height / 2
            implicitWidth: UISettings.listItemHeight * 0.8
            implicitHeight: UISettings.listItemHeight * 0.8
            radius: implicitWidth / 5
        }

    Rectangle
    {
        anchors.fill: parent
        z: 3
        color: "black"
        opacity: 0.6
        visible: !parent.enabled
    }
}
