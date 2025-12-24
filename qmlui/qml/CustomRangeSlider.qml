/*
  Q Light Controller Plus
  CustomRangeSlider.qml

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

RangeSlider
{
    id: control

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            first.implicitHandleWidth + leftPadding + rightPadding,
                            second.implicitHandleWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             first.implicitHandleHeight + topPadding + bottomPadding,
                             second.implicitHandleHeight + topPadding + bottomPadding)

    padding: 2

    property color bgColor: UISettings.highlight

    first.handle:
        Rectangle
        {
            x: control.leftPadding + (control.horizontal ? control.first.visualPosition * (control.availableWidth - width) : (control.availableWidth - width) / 2)
            y: control.topPadding + (control.horizontal ? (control.availableHeight - height) / 2 : control.first.visualPosition * (control.availableHeight - height))
            implicitWidth: UISettings.listItemHeight * 0.8
            implicitHeight: UISettings.listItemHeight * 0.8
            radius: implicitWidth / 5
            color: UISettings.fgMain
        }

    second.handle:
        Rectangle
        {
            x: control.leftPadding + (control.horizontal ? control.second.visualPosition * (control.availableWidth - width) : (control.availableWidth - width) / 2)
            y: control.topPadding + (control.horizontal ? (control.availableHeight - height) / 2 : control.second.visualPosition * (control.availableHeight - height))
            implicitWidth: UISettings.listItemHeight * 0.8
            implicitHeight: UISettings.listItemHeight * 0.8
            radius: implicitWidth / 5
            color: UISettings.fgMain
        }

    background:
        Rectangle
        {
            x: control.leftPadding + (control.horizontal ? 0 : (control.availableWidth - width) / 2)
            y: control.topPadding + (control.horizontal ? (control.availableHeight - height) / 2 : 0)
            implicitWidth: control.horizontal ? 200 : UISettings.listItemHeight * 0.15
            implicitHeight: control.horizontal ? UISettings.listItemHeight * 0.15 : 200
            width: control.horizontal ? control.availableWidth : implicitWidth
            height: control.horizontal ? implicitHeight : control.availableHeight
            radius: 3
            color: control.palette.midlight
            scale: control.horizontal && control.mirrored ? -1 : 1

            Rectangle
            {
                x: control.horizontal ? control.first.position * parent.width + 3 : 0
                y: control.horizontal ? 0 : control.second.visualPosition * parent.height + 3
                width: control.horizontal ? control.second.position * parent.width - control.first.position * parent.width - UISettings.listItemHeight * 0.15 : UISettings.listItemHeight * 0.15
                height: control.horizontal ? UISettings.listItemHeight * 0.15 : control.second.position * parent.height - control.first.position * parent.height - UISettings.listItemHeight * 0.15

                color: control.bgColor
            }
        }
}
