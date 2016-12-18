/*
  Q Light Controller Plus
  PopupPINRequest.qml

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

    /* This is raised to true when user confirm and something goes wrong */
    property bool errorStatus: false

    function buttonsEnableMask()
    {
        currentPinEdit.selectAndFocus()
        return ActionManager.OK | ActionManager.Cancel
    }

    function acceptAction()
    {
        /* At this stage, the ActionManager has the VC page that is requested to be displayed */
        var vcPage = actionManager.actionData()

        var result = virtualConsole.validatePagePIN(vcPage[0], currentPIN, validateCheck.checked)

        if (result === true)
        {
            actionManager.acceptAction()
            pinRoot.requestPopupClose()
        }
        else
            errorStatus = true
    }

    function rejectAction()
    {
        if (errorStatus)
            errorStatus = false
        else
        {
            actionManager.rejectAction()
            pinRoot.requestPopupClose()
        }
    }

    RobotoText
    {
        anchors.fill: parent
        visible: errorStatus
        wrapText: true
        label: qsTr("Invalid PIN entered")
    }

    GridLayout
    {
        id: pinGrid
        visible: !errorStatus
        width: parent.width
        height: UISettings.iconSizeDefault * rows
        columns: 2
        rows: 2
        rowSpacing: 5
        columnSpacing: 5

        // Row 1
        RobotoText
        {
            label: qsTr("Page PIN")
        }

        CustomTextEdit
        {
            id: currentPinEdit
            Layout.fillWidth: true
            echoMode: TextInput.Password
            inputMethodHints: Qt.ImhFormattedNumbersOnly // accept 0-9 digits only
            maximumLength: 4

            onTextChanged: currentPIN = text
        }

        // Row 2
        Row
        {
            Layout.columnSpan: 2
            Layout.fillWidth: true
            spacing: 5

            CustomCheckBox
            {
                id: validateCheck
                width: UISettings.iconSizeDefault
                height: width
            }

            RobotoText
            {
                label: qsTr("Remember for this session")
            }
        }
    }
}
