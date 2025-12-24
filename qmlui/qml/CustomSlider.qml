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

import QtQuick
import QtQuick.Controls.Basic

import "."

Slider
{
    id: control

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitHandleWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitHandleHeight + topPadding + bottomPadding)

    from: 0
    to: 100

    background:
        Rectangle
        {
            x: control.leftPadding + (control.horizontal ? 0 : (control.availableWidth - width) / 2)
            y: control.topPadding + (control.horizontal ? (control.availableHeight - height) / 2 : 0)
            implicitWidth: control.horizontal ? 200 : UISettings.listItemHeight * 0.15
            implicitHeight: control.horizontal ? UISettings.listItemHeight * 0.15 : 200
            width: control.horizontal ? control.availableWidth : implicitWidth
            height: control.horizontal ? implicitHeight : control.availableHeight
            radius: control.horizontal ? height / 2 : width / 2
            color: UISettings.bgLight

            Rectangle
            {
                y: control.horizontal ? 0 : control.visualPosition * parent.height
                width: control.horizontal ? control.position * parent.width : UISettings.listItemHeight * 0.15
                height: control.horizontal ? UISettings.listItemHeight * 0.15 : control.position * parent.height
                color: UISettings.highlight
                radius: control.horizontal ? height / 2 : width / 2
            }
        }

    handle:
        Rectangle
        {
            x: control.leftPadding + (control.horizontal ? control.visualPosition * (control.availableWidth - width) : (control.availableWidth - width) / 2)
            y: control.topPadding + (control.horizontal ? (control.availableHeight - height) / 2 : control.visualPosition * (control.availableHeight - height))
            implicitWidth: UISettings.listItemHeight * 0.8
            implicitHeight: UISettings.listItemHeight * 0.8
            radius: implicitWidth / 5
            color: UISettings.fgMain
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
