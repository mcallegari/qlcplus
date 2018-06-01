/*
  Q Light Controller Plus
  IconTextEntry.qml

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
import QtQuick.Layouts 1.1

import org.qlcplus.classes 1.0
import "."

Rectangle
{
    height: UISettings.iconSizeDefault
    color: "transparent"

    property string iSrc
    property string tLabel
    property color tLabelColor: UISettings.fgMain
    property int tFontSize: UISettings.textSizeDefault
    property int functionType: -1
    property string faSource: ""
    property color faColor: "#222"
    property int iconSize: height - 4

    onFunctionTypeChanged: iSrc = functionManager.functionIcon(functionType)

    RowLayout
    {
        anchors.fill: parent
        spacing: 4

        Image
        {
            visible: iSrc ? true : false
            source: iSrc
            height: iconSize
            width: iconSize
            sourceSize: Qt.size(iconSize, iconSize)
        }

        Text
        {
            id: faIcon
            visible: faSource ? true : false
            color: faColor
            font.family: "FontAwesome"
            font.pixelSize: iconSize
            text: faSource
        }

        RobotoText
        {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignVCenter
            height: parent.height
            label: tLabel
            labelColor: tLabelColor
            fontSize: tFontSize
        }
    }
}


