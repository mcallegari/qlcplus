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
import "."

Rectangle
{
    id: rtRoot
    width: wrapText ? 100 : leftMargin + textBox.paintedWidth + rightMargin
    height: UISettings.iconSizeDefault

    color: "transparent"
    clip: true

    property string label: ""
    property color labelColor: UISettings.fgMain
    property real fontSize: UISettings.textSizeDefault
    property bool fontBold: false
    property bool wrapText: false
    property int textHAlign: Text.AlignLeft
    property int textVAlign: wrapText ? Text.AlignVCenter : Text.AlignTop
    property alias leftMargin: textBox.x
    property real rightMargin: 0

    Text
    {
        id: textBox
        width: parent.width
        height: wrapText ? parent.height : Text.paintedHeight
        anchors.verticalCenter: parent.verticalCenter
        text: label
        font.family: UISettings.robotoFontName
        font.pixelSize: fontSize ? fontSize : 12
        font.bold: fontBold
        color: rtRoot.enabled ? labelColor : Qt.darker(labelColor, 2.0)
        wrapMode: wrapText ? Text.Wrap : Text.NoWrap
        horizontalAlignment: textHAlign
        verticalAlignment: textVAlign
    }
}

