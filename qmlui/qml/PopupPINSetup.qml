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

import com.qlcplus.classes 1.0
import "."

Rectangle
{
    id: pinRoot

    color: "transparent"

    signal requestButtonStatus(int button, bool disable)
    signal requestPopupClose()

    property string currentPIN: ""
    property string newPIN: ""
    property string confirmPIN: ""

    /* This is raised to true when user confirm and something goes wrong */
    property bool errorStatus: false

    function buttonsEnableMask()
    {
        currentPinEdit.selectAndFocus()
        return ActionManager.Cancel
    }

    function acceptAction()
    {
        var result = virtualConsole.setPagePIN(virtualConsole.selectedPage, currentPIN, newPIN)

        if (result === true)
            pinRoot.requestPopupClose()
        else
            errorStatus = true
    }

    function rejectAction()
    {
        if (errorStatus)
            errorStatus = false
        else
            pinRoot.requestPopupClose()
    }

    function checkPIN()
    {
        var disableStatus = true
        console.log("newPin: " + newPIN + ",confirm: " + confirmPIN)
        if (newPIN.length === confirmPIN.length && newPIN === confirmPIN)
        {
            disableStatus = false
        }

        mismatchAlert.visible = disableStatus
        pinRoot.requestButtonStatus(ActionManager.OK, disableStatus)
    }

    RobotoText
    {
        anchors.fill: parent
        visible: errorStatus
        wrapText: true
        label: qsTr("The entered PINs are either invalid or incorrect")
    }

    GridLayout
    {
        id: pinGrid
        visible: !errorStatus
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
            nextTabItem: newPinEdit
            previousTabItem: confirmPinEdit

            onTextChanged:
            {
                currentPIN = text
                pinRoot.checkPIN()
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
            nextTabItem: confirmPinEdit
            previousTabItem: currentPinEdit

            onTextChanged:
            {
                newPIN = text
                pinRoot.checkPIN()
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
            nextTabItem: currentPinEdit
            previousTabItem: newPinEdit

            onTextChanged:
            {
                confirmPIN = text
                pinRoot.checkPIN()
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
