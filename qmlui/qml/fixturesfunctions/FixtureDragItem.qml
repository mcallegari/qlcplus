/*
  Q Light Controller Plus
  FixtureDragItem.qml

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
    property string manufacturer
    property string model
    property string mode

    property int universe: 0
    property int address: 0
    property int channels: 1
    property int quantity: 1
    property int gap: 0

    width: 80
    height: 80
    z: 10
    border.width: 1
    border.color: "black"
    opacity: 0.7
    color: UISettings.bgMedium

    RobotoText
    {
        anchors.fill: parent
        anchors.margins: 1
        label: manufacturer + " - " + model
        labelColor: UISettings.fgMain
        fontSize: 10
        wrapText: true
        textAlign: Text.AlignHCenter
    }

    Drag.active: fxMouseArea.drag.active
}
