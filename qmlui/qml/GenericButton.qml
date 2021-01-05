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
    height: UISettings.iconSizeDefault
    color: gbMouseArea.containsMouse ? (gbMouseArea.pressed ? pressedColor : hoverColor) : bgColor
    border.width: 2
    border.color: UISettings.bgStrong

    property bool useFontawesome: false // false means Roboto, true means FontAwesome
    property int fontSize: UISettings.textSizeDefault
    property alias label: btnText.text
    property color bgColor: UISettings.bgControl
    property color fgColor: UISettings.fgMain
    property color hoverColor: UISettings.highlight
    property color pressedColor: UISettings.highlightPressed
    property bool repetition: false
    property bool autoHeight: false
    property int originalHeight

    signal clicked(int mouseButton)

    /* Record the original height to perform the "auto height" calculation later */
    Component.onCompleted: originalHeight = height

    onWidthChanged:
    {
        if (autoHeight === false)
            return
        /* temporarily reset the wrap mode to
         * measure a "linear" text painted width */
        btnText.wrapMode = Text.NoWrap
        var ratio = Math.ceil(btnText.paintedWidth / width)
        if (ratio < 5 && ratio > 1)
        {
            //console.log("Ratio changed to " + ratio + ", painted: " + btnText.paintedWidth + ", width: " + width)
            implicitHeight = ratio * originalHeight
            btnText.wrapMode = Text.Wrap
        }
        else if (ratio == 1)
        {
            implicitHeight = originalHeight
        }
    }

    /* Overlay rectangle to represent the disabled status */
    Rectangle
    {
        visible: !btnRoot.enabled
        anchors.fill: parent
        z: 1
        color: "black"
        opacity: 0.6
    }

    Text
    {
        id: btnText
        anchors.fill: parent
        color: fgColor
        font.family: useFontawesome ? "FontAwesome" : UISettings.robotoFontName
        font.pixelSize: fontSize
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }

    MouseArea
    {
        id: gbMouseArea
        enabled: btnRoot.enabled
        anchors.fill: parent
        hoverEnabled: true
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        onClicked: btnRoot.clicked(mouse.button)
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
        onTriggered: btnRoot.clicked(Qt.LeftButton)
    }
}
