/*
  Q Light Controller Plus
  CustomScrollBar.qml

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

import QtQuick 2.6
import QtQuick.Controls 2.1

import "."

ScrollBar
{
    id: control
    active: true
    visible: size == 1.0 ? false : true
    orientation: Qt.Vertical

    background:
        Rectangle
        {
            color: UISettings.bgMedium
            border.width: 1
            border.color: UISettings.bgMedium
        }

    contentItem:
        Rectangle
        {
            implicitWidth: UISettings.scrollBarWidth
            implicitHeight: UISettings.scrollBarWidth
            color: (control.pressed ? UISettings.highlight : UISettings.bgControl)

            Rectangle
            {
                anchors.centerIn: parent
                width: control.orientation == Qt.Vertical ? parent.width * 0.8 : 5
                height: control.orientation == Qt.Vertical ? 5 : parent.height * 0.8
                color: UISettings.bgLight
            }
        }
}
