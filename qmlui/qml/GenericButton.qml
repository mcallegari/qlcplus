/*
  Q Light Controller Plus
  GenericButton.qml

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
    id: btnRoot
    width: 150
    height: 40
    color: gbMouseArea.containsMouse ? (gbMouseArea.pressed ? pressedColor : hoverColor) : bgColor
    border.width: 2
    border.color: UISettings.bgStrong

    property bool useFontawesome: false // false means Roboto, true means FontAwesome
    property int fontSize: 16 //UISettings.textSizeDefault
    property alias label: btnText.text
    property color bgColor: UISettings.bgLight
    property color hoverColor: UISettings.highlight
    property color pressedColor: UISettings.highlightPressed
    property bool repetition: false

    signal clicked

    Text
    {
        id: btnText
        anchors.centerIn: parent
        color: "white"
        font.family: useFontawesome ? "FontAwesome" : "Roboto Condensed"
        font.pointSize: fontSize
    }

    MouseArea
    {
        id: gbMouseArea
        anchors.fill: parent
        hoverEnabled: true
        onClicked: btnRoot.clicked()
        onPressAndHold:
        {
            if (repetition == true)
                repTimer.running = true
        }
        onReleased: repTimer.running = false
    }

    Timer
    {
        id: repTimer
        running: false
        interval: 100
        repeat: true
        onTriggered: btnRoot.clicked()
    }
}
