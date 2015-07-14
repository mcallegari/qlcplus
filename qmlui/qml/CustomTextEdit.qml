/*
  Q Light Controller Plus
  CustomTextEdit.qml

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

Rectangle
{
    id: customTextEditRect
    width: 200
    height: 30
    radius: 3
    color: "#333333"

    property alias inputText: textEdit2.text
    property int fontSize: 17

    border.color: "#222"

    TextInput
    {
        id: textEdit2
        color: "#ffffff"
        anchors.right: parent.right
        anchors.rightMargin: 4
        anchors.left: parent.left
        anchors.leftMargin: 4
        clip: false
        font.family: "RobotoCondensed"
        font.pixelSize: fontSize
        echoMode: TextInput.Normal
        anchors.verticalCenter: parent.verticalCenter
    }
}
