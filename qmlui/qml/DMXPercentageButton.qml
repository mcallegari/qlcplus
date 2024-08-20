/*
  Q Light Controller Plus
  DMXPercentageButton.qml

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
    id: buttonRoot
    width: UISettings.iconSizeDefault * 1.1
    height: UISettings.listItemHeight
    border.width: 2
    border.color: "white"
    radius: 5
    color: UISettings.sectionHeader

    property bool dmxMode: true
    signal clicked

    RobotoText
    {
        id: text
        height: parent.height
        anchors.horizontalCenter: parent.horizontalCenter
        label: dmxMode ? "DMX" : "%"
        fontSize: UISettings.textSizeDefault
        fontBold: true
    }

    MouseArea
    {
        anchors.fill: parent
        onClicked: buttonRoot.clicked()
    }
}
