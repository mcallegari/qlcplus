/*
  Q Light Controller Plus
  RobotoText.qml

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

Rectangle {
    width: wrapText ? 100 : textBox.width
    height: 40

    color: "transparent"

    property string label
    property color labelColor: "white"
    property int fontSize: 16
    property bool fontBold: false
    property bool wrapText: false

    Text {
        id: textBox
        //x: 5
        width: wrapText ? parent.width : Text.paintedWidth
        anchors.verticalCenter: parent.verticalCenter
        text: label
        font.family: "RobotoCondensed"
        font.pointSize: fontSize
        font.bold: fontBold
        color: labelColor
        wrapMode: wrapText ? Text.Wrap : Text.NoWrap
    }
}

