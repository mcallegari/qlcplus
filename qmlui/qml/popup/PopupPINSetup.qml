/*
  Q Light Controller Plus
  PopupPINSetup.qml

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

CustomPopupDialog
{
    id: pinDialogRoot
    width: mainView.width / 3

    property string currentPIN: ""
    property string newPIN: ""
    property string confirmPIN: ""

    onOpened:
    {
        setButtonStatus(0, false)
        currentPinEdit.selectAndFocus()
    }

    function checkPIN()
    {
        var enableStatus = false
        console.log("newPin: " + newPIN + ",confirm: " + confirmPIN)
        if (newPIN.length === confirmPIN.length && newPIN === confirmPIN)
        {
            enableStatus = true
        }

        mismatchAlert.visible = !enableStatus
        setButtonStatus(0, enableStatus)
    }

    contentItem:
        GridLayout
        {
            id: pinGrid
            width: parent.width
            height: UISettings.listItemHeight * rows
            columns: 2
            rows: mismatchAlert.visible ? 4 : 3
            rowSpacing: 5
            columnSpacing: 5

            // Row 1
            RobotoText
            {
                height: UISettings.listItemHeight
                label: qsTr("Current PIN")
            }

            CustomTextEdit
            {
                id: currentPinEdit
                Layout.fillWidth: true
                echoMode: TextInput.Password
                inputMethodHints: Qt.ImhFormattedNumbersOnly // accept 0-9 digits only
                maximumLength: 4
                KeyNavigation.tab: newPinEdit
                KeyNavigation.backtab: confirmPinEdit

                onTextChanged:
                {
                    currentPIN = text
                    pinDialogRoot.checkPIN()
                }
            }

            // Row 2
            RobotoText
            {
                height: UISettings.listItemHeight
                label: qsTr("New PIN")
            }

            CustomTextEdit
            {
                id: newPinEdit
                Layout.fillWidth: true
                echoMode: TextInput.Password
                inputMethodHints: Qt.ImhFormattedNumbersOnly // accept 0-9 digits only
                maximumLength: 4
                KeyNavigation.tab: confirmPinEdit
                KeyNavigation.backtab: currentPinEdit

                onTextChanged:
                {
                    newPIN = text
                    pinDialogRoot.checkPIN()
                }
            }

            // Row 3
            RobotoText
            {
                height: UISettings.listItemHeight
                label: qsTr("Confirm PIN")
            }

            CustomTextEdit
            {
                id: confirmPinEdit
                Layout.fillWidth: true
                echoMode: TextInput.Password
                inputMethodHints: Qt.ImhFormattedNumbersOnly // accept 0-9 digits only
                maximumLength: 4
                KeyNavigation.tab: currentPinEdit
                KeyNavigation.backtab: newPinEdit

                onTextChanged:
                {
                    confirmPIN = text
                    pinDialogRoot.checkPIN()
                }
            }

            // Row 4
            RobotoText
            {
                id: mismatchAlert
                visible: false
                Layout.columnSpan: 2
                Layout.fillWidth: true
                height: UISettings.listItemHeight
                labelColor: "red"
                label: qsTr("New PIN mismatch")
            }
        }
}

