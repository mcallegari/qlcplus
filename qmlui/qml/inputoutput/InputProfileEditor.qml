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

    property bool isEditing: false

    function showWarning()
    {
        messagePopup.title = qsTr("!! Warning !!")
        messagePopup.message = qsTr("You are trying to edit a bundled input profile.<br>" +
                                    "If you modify and save it, a new file will be stored in<br><i>" +
                                    ioManager.profileUserFolder + "</i><br>and will override the bundled file.")
        messagePopup.standardButtons = Dialog.Ok
        messagePopup.open()
    }

    function showWizard()
    {
        messagePopup.title = qsTr("Channel wizard activated")
        messagePopup.message = qsTr("You have enabled the input channel wizard. After " +
                                    "clicking OK, wiggle your mapped input profile's " +
                                    "controls. They should appear into the list. " +
                                    "Click the wizard button again to stop channel " +
                                    "auto-detection.<br><br>Note that the wizard cannot " +
                                    "tell the difference between a knob and a slider " +
                                    "so you will have to do the change manually.")
        messagePopup.standardButtons = Dialog.Ok
        messagePopup.open()
    }

    function showSaveFirst()
    {
        messagePopup.title = qsTr("Unsaved changes")
        messagePopup.message = qsTr("Do you wish to save the current profile first?\nChanges will be lost if you don't save them.")
        messagePopup.standardButtons = Dialog.Yes | Dialog.No | Dialog.Cancel
        messagePopup.open()
    }

    CustomPopupDialog
    {
        id: messagePopup
        standardButtons: Dialog.Ok
        title: qsTr("!! Warning !!")

        onClicked:
        {
            if (role === Dialog.Yes)
            {
                ioManager.saveInputProfile()
            }
            else if (role === Dialog.No)
            {
                ioManager.finishInputProfile()
                isEditing = false
                close()
            }
            else if (role === Dialog.Ok || role === Dialog.Cancel)
            {
                close()
            }
        }
    }

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
                implicitHeight: UISettings.listItemHeight
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
                implicitHeight: UISettings.listItemHeight
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
                implicitHeight: UISettings.listItemHeight
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

            GroupBox
            {
                title: qsTr("MIDI Global Settings")
                Layout.columnSpan: 2
                Layout.fillWidth: true
                visible: profileEditor.type === 0
                font.family: UISettings.robotoFontName
                font.pixelSize: UISettings.textSizeDefault
                palette.windowText: UISettings.fgMain

                RowLayout
                {
                    width: parent.width

                    CustomCheckBox
                    {
                        implicitHeight: UISettings.listItemHeight
                        implicitWidth: implicitHeight
                        onToggled: { }
                    }
                    RobotoText
                    {
                        Layout.fillWidth: true
                        wrapText: true
                        label: qsTr("When MIDI notes are used, send a Note Off when value is 0")
                    }
                }
            } // GroupBox
        } // GridLayout
    } // Rectangle

    ListView
    {
        id: channelList
        implicitWidth: peContainer.width
        Layout.fillHeight: true
        boundsBehavior: Flickable.StopAtBounds
        clip: true
        z: 1

        model: peContainer.visible ? profileEditor.channels : null

        ScrollBar.vertical: CustomScrollBar { }

        property int selectedIndex: -1
        property int selectedChannel: -1
        property string selectedType: ""

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
                        channelList.selectedChannel = modelData.chNumber
                        channelList.selectedType = modelData.chType
                    }
                }
            }
    } // ListView

    GroupBox
    {
        title: qsTr("Behaviour")
        Layout.fillWidth: true
        visible: channelList.selectedIndex >= 0 && channelList.selectedType == "Button"
        font.family: UISettings.robotoFontName
        font.pixelSize: UISettings.textSizeDefault
        palette.windowText: UISettings.fgMain

        RowLayout
        {
            width: parent.width

            CustomCheckBox
            {
                implicitHeight: UISettings.listItemHeight
                implicitWidth: implicitHeight
                onToggled: { }
            }
            RobotoText
            {
                Layout.fillWidth: true
                wrapText: true
                label: qsTr("Generate an extra Press/Release when toggled")
            }
        }
    } // GroupBox

    GroupBox
    {
        title: qsTr("Behaviour")
        Layout.fillWidth: true
        visible: channelList.selectedIndex >= 0 && channelList.selectedType == "Slider"
        font.family: UISettings.robotoFontName
        font.pixelSize: UISettings.textSizeDefault
        palette.windowText: UISettings.fgMain

        RowLayout
        {
            width: parent.width

            RobotoText
            {
                implicitHeight: UISettings.listItemHeight
                label: qsTr("Movement")
            }
            CustomComboBox
            {
                ListModel
                {
                    id: moveTypeModel
                    ListElement { mLabel: "Absolute"; mValue: 0 }
                    ListElement { mLabel: "Relative"; mValue: 1 }
                }

                implicitHeight: UISettings.listItemHeight
                model: moveTypeModel
            }
            RobotoText
            {
                implicitHeight: UISettings.listItemHeight
                label: qsTr("Sensitivity")
            }
            CustomSpinBox
            {
                implicitHeight: UISettings.listItemHeight
                from: 10
                to: 100
            }
        }
    } // GroupBox

    GroupBox
    {
        title: qsTr("Custom Feedback")
        Layout.fillWidth: true
        visible: channelList.selectedIndex >= 0 && channelList.selectedType == "Button"
        font.family: UISettings.robotoFontName
        font.pixelSize: UISettings.textSizeDefault
        palette.windowText: UISettings.fgMain

        RowLayout
        {
            width: parent.width

            RobotoText
            {
                implicitHeight: UISettings.listItemHeight
                label: qsTr("Lower value")
            }
            CustomSpinBox
            {
                implicitHeight: UISettings.listItemHeight
                from: 0
                to: 255
            }
            RobotoText
            {
                implicitHeight: UISettings.listItemHeight
                label: qsTr("Upper value")
            }
            CustomSpinBox
            {
                implicitHeight: UISettings.listItemHeight
                from: 0
                to: 255
            }
        }
    } // GroupBox

} // ColumnLayout
