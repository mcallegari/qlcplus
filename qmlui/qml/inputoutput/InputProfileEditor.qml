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

    function updateOptions(type)
    {
        var extraPress = false
        var movement = false
        var absRel = false
        var feedback = false

        switch (type)
        {
            case QLCInputChannel.Button:
                extraPress = true
                feedback = true
            break
            case QLCInputChannel.Slider:
            case QLCInputChannel.Knob:
                movement = true
                absRel = true
                sensitivitySpin.from = 10
                sensitivitySpin.to = 100
            break
            case QLCInputChannel.Encoder:
                movement = true
                sensitivitySpin.from = 1
                sensitivitySpin.to = 20
            break
            default:
            break
        }

        extraPressGroup.visible = extraPress
        movementGroup.visible = movement
        movementCombo.visible = absRel
        movementLabel.visible = absRel
        feedbackGroup.visible = feedback
    }

    function selectedChannel()
    {
        return channelList.selectedChannelNumber;
    }

    function addNewChannel()
    {
        channelList.selectedChannelNumber = -1
        inputChannelEditor.open()
        inputChannelEditor.initialize(-1, profileEditor.type)
    }

    function editSelectedChannel()
    {
        inputChannelEditor.open()
        inputChannelEditor.initialize(channelList.selectedChannelNumber - 1, profileEditor.type)
    }

    function removeSelectedChannel()
    {
        profileEditor.removeChannel(channelList.selectedChannelNumber - 1)
        channelList.selectedChannelNumber = -1
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

    PopupInputChannelEditor
    {
        id: inputChannelEditor

        onAccepted:
        {
            if (profileEditor.saveChannel(channelList.selectedChannelNumber - 1, currentChannelNumber) < 0)
            {
                messagePopup.title = qsTr("!! Warning !!")
                messagePopup.message = qsTr("Channel " + (currentChannelNumber + 1) + " already exists!")
                messagePopup.standardButtons = Dialog.Ok
                messagePopup.open()
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
                currValue: peContainer.visible ? profileEditor.type : 0
                onValueChanged: profileEditor.type = currentValue
            }

            GroupBox
            {
                title: qsTr("MIDI Global Settings")
                Layout.columnSpan: 2
                Layout.fillWidth: true
                visible: peContainer.visible ? profileEditor.type === QLCInputProfile.MIDI : false
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
                        checked: peContainer.visible ? profileEditor.midiNoteOff : false
                        onToggled: profileEditor.midiNoteOff = checked
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

        property int selectedChannelNumber: -1
        property QLCInputChannel selectedChannel: null

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

                property QLCInputChannel channel: modelData.cRef

                Rectangle
                {
                    anchors.fill: parent
                    color: UISettings.highlight
                    visible: channelList.selectedChannelNumber === modelData.chNumber
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
                        label: channel.name
                    }
                    Rectangle { width: 1; height: UISettings.listItemHeight; color: UISettings.fgMedium }

                    IconTextEntry
                    {
                        width: UISettings.bigItemHeight * 1.5
                        height: UISettings.listItemHeight
                        tLabel: channel.typeString
                        iSrc: channel.iconResource(channel.type, true)
                    }
                }

                MouseArea
                {
                    anchors.fill: parent
                    onClicked:
                    {
                        channelList.selectedChannelNumber = modelData.chNumber
                        channelList.selectedChannel = channel
                        updateOptions(channel.type)
                    }
                }
            }
    } // ListView

    GroupBox
    {
        id: extraPressGroup
        title: qsTr("Behaviour")
        Layout.fillWidth: true
        visible: false
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
                checked: channelList.selectedChannel ? channelList.selectedChannel.sendExtraPress : false
                onToggled: channelList.selectedChannel.sendExtraPress = checked
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
        id: movementGroup
        title: qsTr("Behaviour")
        Layout.fillWidth: true
        visible: false
        font.family: UISettings.robotoFontName
        font.pixelSize: UISettings.textSizeDefault
        palette.windowText: UISettings.fgMain

        RowLayout
        {
            width: parent.width

            RobotoText
            {
                id: movementLabel
                implicitHeight: UISettings.listItemHeight
                label: qsTr("Movement")
            }
            CustomComboBox
            {
                id: movementCombo
                ListModel
                {
                    id: moveTypeModel
                    ListElement { mLabel: "Absolute"; mValue: QLCInputChannel.Absolute }
                    ListElement { mLabel: "Relative"; mValue: QLCInputChannel.Relative }
                }

                implicitHeight: UISettings.listItemHeight
                model: moveTypeModel
                currentIndex: channelList.selectedChannel ? channelList.selectedChannel.movementType : QLCInputChannel.Absolute
                onValueChanged: channelList.selectedChannel.movementType = currentValue
            }
            RobotoText
            {
                implicitHeight: UISettings.listItemHeight
                label: qsTr("Sensitivity")
            }
            CustomSpinBox
            {
                id: sensitivitySpin
                implicitHeight: UISettings.listItemHeight
                from: 10
                to: 100
                value: channelList.selectedChannel ? channelList.selectedChannel.movementSensitivity : 20
                onValueModified: channelList.selectedChannel.movementSensitivity = value
            }
        }
    } // GroupBox

    GroupBox
    {
        id: feedbackGroup
        title: qsTr("Custom Feedback")
        Layout.fillWidth: true
        visible: false
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
                value: channelList.selectedChannel ? channelList.selectedChannel.lowerValue : 0
                onValueModified: channelList.selectedChannel.lowerValue = value
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
                value: channelList.selectedChannel ? channelList.selectedChannel.upperValue : 255
                onValueModified: channelList.selectedChannel.upperValue = value
            }
        }
    } // GroupBox

} // ColumnLayout
