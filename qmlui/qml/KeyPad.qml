/*
  Q Light Controller Plus
  KeyPad.qml

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

import "."

Rectangle
{
    id: keyPadRoot
    width: UISettings.bigItemHeight * 2.5
    height: keyPadGrid.height
    color: "transparent"

    property bool showDMXcontrol: true
    property bool showTapButton: false
    property double tapTimeValue: 0
    property alias commandString: commandBox.inputText

    onVisibleChanged: if (visible) commandBox.selectAndFocus()

    signal executeCommand(string cmd)
    signal escapePressed()
    signal tapTimeChanged(int time)

    Timer
    {
        id: tapTimer
        repeat: true
        running: false
        interval: 500

        onTriggered:
        {
            if (tapButton.border.color == UISettings.bgMedium)
                tapButton.border.color = "#00FF00"
            else
                tapButton.border.color = UISettings.bgMedium
        }
    }

    GridLayout
    {
        id: keyPadGrid
        width: parent.width
        height: UISettings.iconSizeDefault * rows
        columns: showDMXcontrol ? 4 : 3
        rows: 6
        rowSpacing: 3
        columnSpacing: 3

        // row 1
        CustomTextEdit
        {
            id: commandBox
            property int span: showTapButton ? keyPadGrid.columns - 1 : keyPadGrid.columns
            Layout.columnSpan: span
            Layout.fillWidth: true
            height: UISettings.iconSizeDefault
            color: UISettings.bgLight
            onEnterPressed: keyPadRoot.executeCommand(keyPadRoot.commandString)
            onEscapePressed: keyPadRoot.escapePressed()
        }

        GenericButton
        {
            id: tapButton
            visible: showTapButton
            Layout.fillWidth: true
            label: qsTr("Tap")

            onClicked:
            {
                /* right click resets the current TAP time */
                if (mouseButton === Qt.RightButton)
                {
                    tapTimer.stop()
                    tapButton.border.color = UISettings.bgMedium
                    tapTimeValue = 0
                }
                else
                {
                    var currTime = new Date().getTime()
                    if (tapTimeValue != 0)
                    {
                        keyPadRoot.tapTimeChanged(currTime - tapTimeValue)
                        tapTimer.interval = currTime - tapTimeValue
                        tapTimer.restart()
                    }
                    tapTimeValue = currTime
                }
            }
        }

        // row 2
        GenericButton
        {
            Layout.fillWidth: true
            label: "7"
            onClicked: commandBox.appendText(label)
        }
        GenericButton
        {
            Layout.fillWidth: true
            label: "8"
            onClicked: commandBox.appendText(label)
        }
        GenericButton
        {
            Layout.fillWidth: true
            label: "9"
            onClicked: commandBox.appendText(label)
        }
        GenericButton
        {
            visible: showDMXcontrol
            Layout.fillWidth: true
            label: "AT"
            onClicked: commandBox.appendText(" AT")
        }

        // row 3
        GenericButton
        {
            Layout.fillWidth: true
            label: "4"
            onClicked: commandBox.appendText(label)
        }
        GenericButton
        {
            Layout.fillWidth: true
            label: "5"
            onClicked: commandBox.appendText(label)
        }
        GenericButton
        {
            Layout.fillWidth: true
            label: "6"
            onClicked: commandBox.appendText(label)
        }
        GenericButton
        {
            visible: showDMXcontrol
            Layout.fillWidth: true
            label: "THRU"
            onClicked: commandBox.appendText(" THRU")
        }

        // row 4
        GenericButton
        {
            Layout.fillWidth: true
            label: "1"
            onClicked: commandBox.appendText(label)
        }
        GenericButton
        {
            Layout.fillWidth: true
            label: "2"
            onClicked: commandBox.appendText(label)
        }
        GenericButton
        {
            Layout.fillWidth: true
            label: "3"
            onClicked: commandBox.appendText(label)
        }
        GenericButton
        {
            visible: showDMXcontrol
            Layout.fillWidth: true
            label: "FULL"
            onClicked: commandBox.appendText(" FULL")
        }

        // row 5
        GenericButton
        {
            Layout.fillWidth: true
            label: "-"
            repetition: true
            onClicked:
            {
                if (showDMXcontrol == false)
                    commandBox.inputText = parseInt(commandBox.inputText) - 1
            }
        }
        GenericButton
        {
            Layout.fillWidth: true
            label: "0"
            onClicked: commandBox.appendText(label)
        }
        GenericButton
        {
            Layout.fillWidth: true
            label: "+"
            repetition: true
            onClicked:
            {
                if (showDMXcontrol == false)
                    commandBox.inputText = parseInt(commandBox.inputText) + 1
            }
        }
        GenericButton
        {
            visible: showDMXcontrol
            Layout.fillWidth: true
            label: "ZERO"
        }

        // row 6
        GenericButton
        {
            Layout.columnSpan: 2
            Layout.fillWidth: true
            label: "ENTER"
            bgColor: "#43B008"
            hoverColor: "#61FF0C"
            pressedColor: "#225B04"
            onClicked: keyPadRoot.executeCommand(keyPadRoot.commandString)
        }
        GenericButton
        {
            Layout.fillWidth: true
            label: "CLR"
            onClicked: keyPadRoot.commandString = ""
        }
        GenericButton
        {
            visible: showDMXcontrol
            Layout.fillWidth: true
            label: "BY"
        }
    }
}
