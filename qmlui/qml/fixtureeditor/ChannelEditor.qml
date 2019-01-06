/*
  Q Light Controller Plus
  ChannelEditor.qml

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

import QtQuick 2.2
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.4

import org.qlcplus.classes 1.0
import "."

GridLayout
{
    columns: 4

    property EditorRef editor: null
    property ChannelEdit channel: null
    property string itemName

    onItemNameChanged: channel = editor.requestChannelEditor(itemName)

    // row 1
    RobotoText { label: qsTr("Name") }
    CustomTextEdit
    {
        Layout.fillWidth: true
        Layout.columnSpan: 3
        inputText: channel ? channel.name : ""
        onTextChanged: if (channel) channel.name = text
    }

    // row 2
    RobotoText { label: qsTr("Preset") }
    CustomComboBox
    {
        Layout.fillWidth: true
        Layout.columnSpan: 3

        model: channel ? channel.channelPresetList : null
        currentIndex: channel ? channel.preset : 0
        onCurrentIndexChanged: if (channel) channel.preset = currentIndex
    }

    // row 3
    RobotoText { label: qsTr("Type") }
    CustomComboBox
    {
        Layout.fillWidth: true
        enabled: channel ? (channel.preset ? false : true) : false
        model: channel ? channel.channelTypeList : null
        currentValue: channel ? channel.group : 0
    }

    RobotoText { label: qsTr("Role") }
    RowLayout
    {
        Layout.fillWidth: true

        ButtonGroup { id: roleGroup }

        CustomCheckBox
        {
            implicitHeight: UISettings.listItemHeight
            implicitWidth: implicitHeight
            ButtonGroup.group: roleGroup
            enabled: channel ? (channel.preset ? false : true) : false
            checked: channel ? !channel.controlByte : false
            onToggled: if (channel) channel.controlByte = 0
        }
        RobotoText { label: qsTr("Coarse (MSB)") }
        CustomCheckBox
        {
            implicitHeight: UISettings.listItemHeight
            implicitWidth: implicitHeight
            ButtonGroup.group: roleGroup
            enabled: channel ? (channel.preset ? false : true) : false
            checked: channel ? channel.controlByte : false
            onToggled: if (channel) channel.controlByte = 1
        }
        RobotoText { label: qsTr("Fine (LSB)") }
    }

    // row 4
    RobotoText { label: qsTr("Default value") }
    CustomSpinBox
    {
        Layout.columnSpan: 3
        from: 0
        to: 255
        stepSize: 1
        value: channel ? channel.defaultValue : 0
        onValueChanged: if (channel) channel.defaultValue = value
    }

    Rectangle
    {
        Layout.fillHeight: true
        color: "transparent"

        ListView
        {
            id: capsList
            anchors.fill: parent
            model: channel ? channel.capabilities : null
            delegate:
                RowLayout
                {
                    width: capsList.width
                    height: UISettings.listItemHeight

                    RobotoText
                    {
                        id: minValBox
                        width: UISettings.bigItemHeight
                        label: modelData.iMin
                    }
                    Rectangle { width: 1; height: UISettings.listItemHeight }

                    RobotoText
                    {
                        id: maxValBox
                        width: UISettings.bigItemHeight
                        label: modelData.iMax
                    }
                    Rectangle { width: 1; height: UISettings.listItemHeight }

                    RobotoText
                    {
                        id: capDescription
                        //Layout.fillWidth: true
                        label: modelData.sDesc
                    }
                }
        }
    }
}
