/*
  Q Light Controller Plus
  PopupInputChannelEditor.qml

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
    width: mainView.width / 3
    title: qsTr("Input Channel Editor")
    standardButtons: Dialog.Cancel | Dialog.Ok

    property QLCInputChannel editChannel: null
    property int currentChannelNumber
    property bool isMIDI: false
    property int midiChannelOffset: 4096

    function initialize(chNum, profType)
    {
        if (chNum === -1)
        {
            currentChannelNumber = 0
            channelSpin.value = 1
            editChannel = profileEditor.getEditChannel(-1)
        }
        else
        {
            currentChannelNumber = chNum
            channelSpin.value = chNum + 1
            editChannel = profileEditor.getEditChannel(chNum)
        }
        chTypeCombo.currValue = editChannel.type
        isMIDI = (profType === QLCInputProfile.MIDI) ? true : false
        updateMIDIInfo()
    }

    function updateMIDIInfo()
    {
        var chNum = channelSpin.value - 1
        var midiChannel = chNum / midiChannelOffset + 1
        var midiParam = 0
        var midiNote = "--"
        midiChannelSpin.value = parseInt(midiChannel)

        chNum = parseInt(chNum % midiChannelOffset)

        if (chNum < InputProfEditor.NoteOffset)
        {
            midiMessageCombo.currValue = InputProfEditor.ControlChange
            midiParam = chNum - InputProfEditor.ControlChangeOffset
        }
        else if (chNum < InputProfEditor.NoteAfterTouchOffset)
        {
            midiMessageCombo.currValue = InputProfEditor.NoteOnOff
            midiParam = chNum - InputProfEditor.NoteOffset
            var notes = [ "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"]
            var octave = parseInt(midiParam / 12) - 1
            var pitch = parseInt(midiParam % 12)
            midiNote = notes[pitch] + "" + octave
        }
        else if (chNum < InputProfEditor.ProgramChangeOffset)
        {
            midiMessageCombo.currValue = InputProfEditor.NoteAftertouch
            midiParam = chNum - InputProfEditor.NoteAfterTouchOffset
        }
        else if (chNum < InputProfEditor.ChannelAfterTouchOffset)
        {
            midiMessageCombo.currValue = InputProfEditor.ProgramChange
            midiParam = chNum - InputProfEditor.ProgramChangeOffset
        }
        else if (chNum < InputProfEditor.PitchWheelOffset)
            midiMessageCombo.currValue = InputProfEditor.ChannelAfterTouch
        else if (chNum < InputProfEditor.MBCPlaybackOffset)
            midiMessageCombo.currValue = InputProfEditor.PitchWheel
        else if (chNum < InputProfEditor.MBCBeatOffset)
            midiMessageCombo.currValue = InputProfEditor.MBCPlayback
        else if (chNum < InputProfEditor.MBCStopOffset)
            midiMessageCombo.currValue = InputProfEditor.MBCBeat
        else
            midiMessageCombo.currValue = InputProfEditor.MBCStop

        midiParamSpin.value = midiParam
        midiNoteLabel.label = midiNote
    }

    function updateChannel()
    {
        var midiChannel = midiChannelSpin.value
        var midiMessage = midiMessageCombo.currentValue
        var midiParam = midiParamSpin.value
        var chNum

        switch (midiMessage)
        {
            case InputProfEditor.ControlChange:
                chNum = (midiChannel - 1) * midiChannelOffset + InputProfEditor.ControlChangeOffset + midiParam
            break
            case InputProfEditor.NoteOnOff:
                chNum = (midiChannel - 1) * midiChannelOffset + InputProfEditor.NoteOffset + midiParam
            break
            case InputProfEditor.NoteAfterTouch:
                chNum = (midiChannel - 1) * midiChannelOffset + InputProfEditor.NoteAfterTouchOffset + midiParam
            break
            case InputProfEditor.ProgramChange:
                chNum = (midiChannel - 1) * midiChannelOffset + InputProfEditor.ProgramChangeOffset + midiParam
            break
            case InputProfEditor.ChannelAfterTouch:
                chNum = (midiChannel - 1) * midiChannelOffset + InputProfEditor.ChannelAfterTouchOffset
            break
            case InputProfEditor.PitchWheel:
                chNum = (midiChannel - 1) * midiChannelOffset + InputProfEditor.PitchWheelOffset
            break
            case InputProfEditor.MBCPlayback:
                chNum = (midiChannel - 1) * midiChannelOffset + InputProfEditor.MBCPlaybackOffset
            break
            case InputProfEditor.MBCBeat:
                chNum = (midiChannel - 1) * midiChannelOffset + InputProfEditor.MBCBeatOffset
            break
            case InputProfEditor.MBCStop:
                chNum = (midiChannel - 1) * midiChannelOffset + InputProfEditor.MBCStopOffset
            break
        }

        channelSpin.value = chNum + 1
    }

    contentItem:
        ColumnLayout
        {
            GroupBox
            {
                id: channelGroup
                title: qsTr("Input Channel")
                Layout.fillWidth: true
                font.family: UISettings.robotoFontName
                font.pixelSize: UISettings.textSizeDefault
                palette.windowText: UISettings.fgMain

                GridLayout
                {
                    columns: 3

                    RobotoText { height: UISettings.listItemHeight; label: qsTr("Number") }
                    RobotoText { height: UISettings.listItemHeight; label: qsTr("Name") }
                    RobotoText { height: UISettings.listItemHeight; label: qsTr("Type") }

                    CustomSpinBox
                    {
                        id: channelSpin
                        from: 1
                        to: 65535
                        onValueModified:
                        {
                            currentChannelNumber = value - 1
                            updateMIDIInfo()
                        }
                    }
                    CustomTextEdit
                    {
                        id: channelName
                        Layout.fillWidth: true
                        text: editChannel ? editChannel.name : ""
                        onTextEdited: editChannel.name = text
                    }
                    CustomComboBox
                    {
                        id: chTypeCombo
                        model: popupRoot.visible ? profileEditor.channelTypeModel : null
                        currValue: -1
                        onValueChanged: editChannel.type = currentValue
                    }
                }
            }

            GroupBox
            {
                id: midiGroup
                visible: isMIDI
                title: "MIDI"
                Layout.fillWidth: true
                font.family: UISettings.robotoFontName
                font.pixelSize: UISettings.textSizeDefault
                palette.windowText: UISettings.fgMain

                GridLayout
                {
                    columns: 4

                    RobotoText { height: UISettings.listItemHeight; label: qsTr("Channel") }
                    RobotoText { height: UISettings.listItemHeight; label: qsTr("Message") }
                    RobotoText { height: UISettings.listItemHeight; label: qsTr("Parameter") }
                    RobotoText { height: UISettings.listItemHeight; label: qsTr("Note") }

                    CustomSpinBox
                    {
                        id: midiChannelSpin
                        from: 1
                        to: 16
                        onValueModified: updateChannel()
                    }

                    CustomComboBox
                    {
                        id: midiMessageCombo
                        ListModel
                        {
                            id: midiTypeModel
                            ListElement { mLabel: "Control Change"; mValue: InputProfEditor.ControlChange }
                            ListElement { mLabel: "Note On/Off"; mValue: InputProfEditor.NoteOnOff }
                            ListElement { mLabel: "Note Aftertouch"; mValue: InputProfEditor.NoteAftertouch }
                            ListElement { mLabel: "Program Change"; mValue: InputProfEditor.ProgramChange }
                            ListElement { mLabel: "Channel Aftertouch"; mValue: InputProfEditor.ChannelAfterTouch }
                            ListElement { mLabel: "Pitch Wheel"; mValue: InputProfEditor.PitchWheel }
                            ListElement { mLabel: "Beat Clock: Start/Stop/Continue"; mValue: InputProfEditor.MBCPlayback }
                            ListElement { mLabel: "Beat Clock: Beat"; mValue: InputProfEditor.MBCBeat }
                        }

                        Layout.fillWidth: true
                        model: midiTypeModel
                        onValueChanged: updateChannel()
                    }

                    CustomSpinBox
                    {
                        id: midiParamSpin
                        from: 0
                        to: 127
                        onValueModified: updateChannel()
                    }

                    RobotoText
                    {
                        id: midiNoteLabel
                        height: UISettings.listItemHeight
                        Layout.fillWidth: true
                        label: "--"
                    }
                }
            }
        }
}
