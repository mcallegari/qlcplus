/*
  Q Light Controller Plus
  PopupCreatePalette.qml

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
import QtQuick.Controls 2.2

import org.qlcplus.classes 1.0
import "."

CustomPopupDialog
{
    id: popupRoot
    title: qsTr("Create a new palette")

    property QLCPalette paletteObj

    function focusEditItem()
    {
        nameInputBox.selectAndFocus()
    }

    function typeToString(type)
    {
        switch (type)
        {
            case QLCPalette.Dimmer:    return qsTr("Dimmer");
            case QLCPalette.Color:     return qsTr("Color");
            case QLCPalette.Position:  return qsTr("Position");
            case QLCPalette.Shutter:   return qsTr("Shutter");
            case QLCPalette.Gobo:      return qsTr("Gobo");
            case QLCPalette.Undefined: return "";
        }
    }

    onAccepted:
    {
        paletteManager.createPalette(paletteObj, nameInputBox.inputText)
    }

    contentItem:
        GridLayout
        {
            columns: 2
            columnSpacing: 5


            // row 1
            RobotoText
            {
                height: UISettings.listItemHeight
                label: qsTr("Palette name")
            }

            CustomTextEdit
            {
                id: nameInputBox
                Layout.fillWidth: true
                inputText: qsTr("New Palette")
                onEnterPressed: popupRoot.accept()
            }

            // row 2
            RobotoText
            {
                height: UISettings.listItemHeight
                label: qsTr("Type")
            }

            RobotoText
            {
                height: UISettings.listItemHeight
                Layout.fillWidth: true
                label: typeToString(popupRoot.type)
            }

            // row 3
            RowLayout
            {
                Layout.columnSpan: 2

                CustomCheckBox
                {
                    id: createSceneCheck
                    implicitHeight: UISettings.listItemHeight
                    implicitWidth: implicitHeight
                    autoExclusive: false
                }

                RobotoText
                {
                    height: UISettings.listItemHeight
                    label: qsTr("Also create a Scene")
                }
            }

            // row 4
            RobotoText
            {
                height: UISettings.listItemHeight
                label: qsTr("Scene name")
                enabled: createSceneCheck.checked
            }

            CustomTextEdit
            {
                Layout.fillWidth: true
                inputText: qsTr("New Scene")
                enabled: createSceneCheck.checked
            }
        } // GridLayout
}
