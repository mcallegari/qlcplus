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

import org.qlcplus.classes 1.0
import "."

CustomPopupDialog
{
    width: mainView.width / 3

    property string currentPIN: ""
    property alias sessionValidate: validateCheck.checked

    onOpened:
    {
        currentPinEdit.text = ""
        currentPinEdit.selectAndFocus()
    }

    contentItem:
        GridLayout
        {
            id: pinGrid
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
                }

                RobotoText
                {
                    label: qsTr("Remember for this session")
                }
            }
        }
}

