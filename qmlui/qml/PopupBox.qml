/*
  Q Light Controller Plus
  PopupBox.qml

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

import com.qlcplus.classes 1.0

import "."

Rectangle
{
    id: popupScreen
    objectName: "popupBox"
    z: 99

    color: Qt.rgba(0, 0, 0, 0.5)

    visible: false

    property string message
    property int buttonsMask

    onButtonsMaskChanged:
    {
        okButton.visible = false
        cancelButton.visible = false

        if (buttonsMask & ActionManager.OK)
            okButton.visible = true
        if (buttonsMask & ActionManager.Cancel)
            cancelButton.visible = true
    }

    MouseArea
    {
        anchors.fill: parent
        hoverEnabled: true
        preventStealing: true
    }

    Rectangle
    {
        id: popupBox
        width: Math.min(500, (popupScreen.width / 4) * 3)
        height: Math.min(300, (popupScreen.height / 4) * 2)
        anchors.centerIn: parent

        color: "#222"
        border.color: "#555"
        border.width: 2

        RobotoText
        {
            id: popupTextBox
            x: 10
            y: 10
            width: parent.width - 20
            height: parent.height - 65
            label: message
            wrapText: true
            textAlign: Text.AlignHCenter
        }

        GenericButton
        {
            id: okButton
            y: popupBox.height - 45
            x: parent.width - width - 20
            width: parent.width / 3
            height: 35
            label: qsTr("OK")
            onClicked:
            {
                actionManager.acceptAction()
                popupScreen.visible = false
            }
        }

        GenericButton
        {
            y: popupBox.height - 45
            x: 20
            id: cancelButton
            width: parent.width / 3
            height: 35
            label: qsTr("Cancel")
            onClicked:
            {
                actionManager.rejectAction()
                popupScreen.visible = false
            }
        }
    }
}

