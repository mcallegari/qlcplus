/*
  Q Light Controller Plus
  InputProfileEditor.qml

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
import QtQuick.Controls 2.13
import QtQml.Models 2.13

import org.qlcplus.classes 1.0
import "."

ColumnLayout
{
    id: peContainer

    Rectangle
    {
        implicitWidth: peContainer.width
        implicitHeight: infoBox.height
        z: 2
        color: UISettings.bgMedium

        GridLayout
        {
            id: infoBox
            width: peContainer.width
            columns: 2

            RobotoText
            {
                label: qsTr("Manufacturer")
            }
            CustomTextEdit
            {
                Layout.fillWidth: true
                text: peContainer.visible ? profileEditor.manufacturer : ""
                onTextChanged: profileEditor.manufacturer = text
            }
            RobotoText
            {
                label: qsTr("Model")
            }
            CustomTextEdit
            {
                Layout.fillWidth: true
                text: peContainer.visible ? profileEditor.model : ""
                onTextChanged: profileEditor.model = text
            }
            RobotoText
            {
                label: qsTr("Type")
            }
            CustomComboBox
            {
                ListModel
                {
                    id: profTypeModel
                    ListElement { mLabel: "MIDI"; mValue: 0 }
                    ListElement { mLabel: "OS2L"; mValue: 1 }
                    ListElement { mLabel: "OSC"; mValue: 2 }
                    ListElement { mLabel: "HID"; mValue: 3 }
                    ListElement { mLabel: "DMX"; mValue: 4 }
                    ListElement { mLabel: "ENTTEC"; mValue: 5 }
                }

                Layout.fillWidth: true
                model: profTypeModel
                currentIndex: peContainer.visible ? profileEditor.type : 0
                onValueChanged: profileEditor.type = currentValue
            }
        } // GridLayout
    } // Rectangle

    ListView
    {
        id: channelList
        implicitWidth: peContainer.width
        Layout.fillHeight: true
        boundsBehavior: Flickable.StopAtBounds
        z: 1

        model: peContainer.visible ? profileEditor.channels : null

        ScrollBar.vertical: CustomScrollBar { }

        property int selectedIndex: -1

        header:
            RowLayout
            {
                width: channelList.width
                height: UISettings.listItemHeight

                RobotoText
                {
                    width: UISettings.bigItemHeight
                    height: UISettings.listItemHeight
                    label: qsTr("Channel")
                    color: UISettings.sectionHeader
                }
                Rectangle { width: 1; height: UISettings.listItemHeight; color: UISettings.fgMedium }

                RobotoText
                {
                    Layout.fillWidth: true
                    height: UISettings.listItemHeight
                    label: qsTr("Name")
                    color: UISettings.sectionHeader
                }
                Rectangle { width: 1; height: UISettings.listItemHeight; color: UISettings.fgMedium }

                RobotoText
                {
                    width: UISettings.bigItemHeight * 1.5
                    height: UISettings.listItemHeight
                    label: qsTr("Type")
                    color: UISettings.sectionHeader
                }
            }

        delegate:
            Item
            {
                width: channelList.width
                height: UISettings.listItemHeight

                Rectangle
                {
                    anchors.fill: parent
                    color: UISettings.highlight
                    visible: channelList.selectedIndex === index
                }

                RowLayout
                {
                    anchors.fill: parent

                    RobotoText
                    {
                        width: UISettings.bigItemHeight
                        height: UISettings.listItemHeight
                        label: modelData.chNumber
                    }
                    Rectangle { width: 1; height: UISettings.listItemHeight; color: UISettings.fgMedium }

                    RobotoText
                    {
                        Layout.fillWidth: true
                        height: UISettings.listItemHeight
                        label: modelData.chName
                    }
                    Rectangle { width: 1; height: UISettings.listItemHeight; color: UISettings.fgMedium }

                    IconTextEntry
                    {
                        width: UISettings.bigItemHeight * 1.5
                        height: UISettings.listItemHeight
                        tLabel: modelData.chType
                        iSrc: modelData.chIconPath
                    }
                }

                MouseArea
                {
                    anchors.fill: parent
                    onClicked:
                    {
                        channelList.selectedIndex = index
                    }
                }
            }
    }
} // ColumnLayout
