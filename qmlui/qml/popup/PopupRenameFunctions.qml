/*
  Q Light Controller Plus
  PopupRenameFunctions.qml

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
    property string baseName
    property var functionIDs
    property int count: functionIDs ? functionIDs.length : 0

    onOpened: newNameEdit.selectAndFocus()

    contentItem:
        GridLayout
        {
            id: renameGrid
            //width: parent.width
            //height: UISettings.listItemHeight * rows
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
                inputText: baseName
            }

            // Row 2
            Row
            {
                Layout.fillWidth: true
                Layout.columnSpan: 4
                spacing: 5
                visible: count

                CustomCheckBox
                {
                    id: numCheckBox
                    autoExclusive: false
                }
                RobotoText
                {
                    label: qsTr("Enable numbering")
                }
            }

            // Row 3
            RobotoText
            {
                visible: count
                label: qsTr("Start number")
            }
            CustomSpinBox
            {
                id: startNumSpin
                visible: count
                Layout.fillWidth: true
                value: 1
            }

            RobotoText
            {
                visible: count
                label: qsTr("Digits")
            }
            CustomSpinBox
            {
                id: digitsSpin
                visible: count
                Layout.fillWidth: true
                from: 1
                to: 10
            }
        }

    onAccepted: functionManager.renameFunctions(functionIDs, newNameEdit.inputText,
                                                numCheckBox.checked, startNumSpin.value, digitsSpin.value)
}
