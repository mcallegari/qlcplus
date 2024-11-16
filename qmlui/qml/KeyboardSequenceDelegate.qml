/*
  Q Light Controller Plus
  KeyboardSequenceDelegate.qml

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

Column
{
    width: parent.width

    property var widgetObjRef: null
    property int controlID
    property alias inputModel: controlsCombo.model
    property string sequence
    property bool invalid: false

    GridLayout
    {
        width: parent.width
        columns: 4
        columnSpacing: 3

        // row 1
        RobotoText
        {
            height: UISettings.listItemHeight
            label: qsTr("Control")
        }
        CustomComboBox
        {
            id: controlsCombo
            Layout.fillWidth: true
            Layout.columnSpan: 3
            height: UISettings.listItemHeight
            currValue: controlID
            onValueChanged:
            {
                if (widgetObjRef && value != controlID)
                {
                    console.log("Key control changed " + value)
                    controlID = value
                    virtualConsole.updateKeySequenceControlID(widgetObjRef, controlID, sequence)
                }
            }
        }

        // row 2
        RobotoText
        {
            height: UISettings.listItemHeight
            label: qsTr("Combination")
        }
        RobotoText
        {
            id: seqTextBox
            Layout.fillWidth: true
            height: UISettings.listItemHeight
            color: UISettings.bgLight
            label: sequence

            SequentialAnimation on color
            {
                PropertyAnimation { to: "red"; duration: 1000 }
                PropertyAnimation { to: UISettings.bgLight; duration: 1000 }
                running: invalid
                loops: Animation.Infinite
            }
        }
        IconButton
        {
            width: UISettings.iconSizeMedium
            height: width
            checkable: true
            imgSource: "qrc:/keybinding.svg"
            tooltip: qsTr("Activate auto detection")

            onToggled:
            {
                if (checked == true)
                {
                    if (invalid === false &&
                        virtualConsole.enableKeyAutoDetection(widgetObjRef, controlID, sequence) === true)
                        invalid = true
                    else
                        checked = false
                }
                else
                {
                    virtualConsole.disableAutoDetection()
                    invalid = false
                    seqTextBox.color = UISettings.bgLight
                }
            }
        }
        IconButton
        {
            width: UISettings.iconSizeMedium
            height: width
            imgSource: "qrc:/remove.svg"
            tooltip: qsTr("Remove this keyboard combination")

            onClicked: virtualConsole.deleteKeySequence(widgetObjRef, controlID, sequence)
        }
    }
}
