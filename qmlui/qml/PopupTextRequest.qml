/*
  Q Light Controller Plus
  PopupTextRequest.qml

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
    id: compRoot

    color: "transparent"

    signal requestButtonStatus(int button, bool disable)
    signal requestPopupClose()

    function buttonsEnableMask()
    {
        return ActionManager.OK | ActionManager.Cancel
    }

    function acceptAction()
    {
        var data = actionManager.actionData()
        // remove the first original string
        data.shift()

        functionManager.renameFunctions(data, newNameEdit.inputText, startNumSpin.value, digitsSpin.value)

        actionManager.acceptAction()
        compRoot.requestPopupClose()
    }

    function rejectAction()
    {
        actionManager.rejectAction()
        compRoot.requestPopupClose()
    }

    GridLayout
    {
        id: renameGrid
        width: parent.width
        height: UISettings.listItemHeight * rows
        columns: 4
        rows: 3
        rowSpacing: 5
        columnSpacing: 5

        // Row 1
        RobotoText
        {
            label: qsTr("New name")
        }

        CustomTextEdit
        {
            id: newNameEdit
            Layout.fillWidth: true
            Layout.columnSpan: 3
        }

        // Row 2
        Row
        {
            Layout.fillWidth: true
            Layout.columnSpan: 4

            CustomCheckBox
            {
                id: numCheckBox
            }
            RobotoText
            {
                label: qsTr("Enable numbering")
            }

            Component.onCompleted:
            {
                var data = actionManager.actionData()
                newNameEdit.inputText = data[0]
                newNameEdit.selectAndFocus()
                if (data.length - 1 == 1)
                    visible = false
            }
        }

        // Row 3
        RobotoText
        {
            visible: numCheckBox.checked
            label: qsTr("Start number")
        }
        CustomSpinBox
        {
            id: startNumSpin
            visible: numCheckBox.checked
            Layout.fillWidth: true
            value: 1
        }

        RobotoText
        {
            visible: numCheckBox.checked
            label: qsTr("Digits")
        }
        CustomSpinBox
        {
            id: digitsSpin
            visible: numCheckBox.checked
            Layout.fillWidth: true
            from: 1
            to: 10
        }
    }
}
