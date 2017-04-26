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

    /* A message string used for simple popup information */
    property string message
    /* A QML resource to be loaded for complex operations */
    property string resource
    /* The mask of buttons to be displayed in the popup */
    property int buttonsMask

    function closePopup()
    {
        popupScreen.message = ""
        popupScreen.resource = ""
        popupScreen.visible = false
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
            textHAlign: Text.AlignHCenter
        }

        Loader
        {
            id: popupLoader
            x: 10
            y: 10
            width: parent.width - 20
            height: parent.height - 65
            source: resource

            onLoaded:
            {
                var btnMask = popupLoader.item.buttonsEnableMask()
                okButton.disabled = !(btnMask & ActionManager.OK)
                cancelButton.disabled = !(btnMask & ActionManager.Cancel)
            }

            Connections
            {
                ignoreUnknownSignals: true
                target: popupLoader.item
                onRequestButtonStatus:
                {
                    if (button === ActionManager.OK)
                        okButton.disabled = disable
                    else if (button === ActionManager.Cancel)
                        cancelButton.disabled = disable
                }
            }
            Connections
            {
                ignoreUnknownSignals: true
                target: popupLoader.item
                onRequestPopupClose: popupScreen.closePopup()
            }
        }

        GenericButton
        {
            id: okButton
            visible: popupScreen.buttonsMask & ActionManager.OK
            y: popupBox.height - height - 5
            x: parent.width - width - 20
            width: parent.width / 3
            label: qsTr("OK")
            onClicked:
            {
                if (popupScreen.resource)
                {
                    popupLoader.item.acceptAction()
                }
                else
                {
                    actionManager.acceptAction()
                    popupScreen.closePopup()
                }
            }
        }

        GenericButton
        {
            id: cancelButton
            visible: popupScreen.buttonsMask & ActionManager.Cancel
            y: popupBox.height - height - 5
            x: 20
            width: parent.width / 3
            label: qsTr("Cancel")
            onClicked:
            {
                if (popupScreen.resource)
                {
                    popupLoader.item.rejectAction()
                }
                else
                {
                    actionManager.rejectAction()
                    popupScreen.closePopup()
                }
            }
        }
    }
}

