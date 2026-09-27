/*
  Q Light Controller Plus
  PopupAudioConfiguration.qml

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

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import "."

CustomPopupDialog
{
    id: popupRoot
    width: mainView.width / 3
    title: isInput ? qsTr("Audio input configuration") : qsTr("Audio output configuration")
    standardButtons: Dialog.Ok

    /** true: configure the audio input, false: configure the audio output */
    property bool isInput: true

    onVisibleChanged:
    {
        // make sure the signal level monitoring is stopped when the popup is closed
        if (visible == false)
            ioManager.enableAudioInputPreview(false)
    }

    contentItem:
        GridLayout
        {
            columns: 2
            rowSpacing: 5
            columnSpacing: 5

            /* *********************** Audio input ************************ */

            // Row 1 - Sample rate
            RobotoText
            {
                visible: popupRoot.isInput
                height: UISettings.listItemHeight
                label: qsTr("Sample rate")
            }

            CustomComboBox
            {
                id: sampleRateCombo
                visible: popupRoot.isInput
                Layout.fillWidth: true
                model: [
                    { mLabel: "8000 Hz", mValue: 8000 },
                    { mLabel: "11025 Hz", mValue: 11025 },
                    { mLabel: "22050 Hz", mValue: 22050 },
                    { mLabel: "32000 Hz", mValue: 32000 },
                    { mLabel: "44100 Hz", mValue: 44100 },
                    { mLabel: "48000 Hz", mValue: 48000 }
                ]
                currValue: ioManager.audioInputSampleRate
                onValueChanged: ioManager.audioInputSampleRate = value
            }

            // Row 2 - Channels
            RobotoText
            {
                visible: popupRoot.isInput
                height: UISettings.listItemHeight
                label: qsTr("Channels")
            }

            CustomComboBox
            {
                id: channelsCombo
                visible: popupRoot.isInput
                Layout.fillWidth: true
                model: [
                    { mLabel: qsTr("Mono"), mValue: 1 },
                    { mLabel: qsTr("Stereo"), mValue: 2 }
                ]
                currValue: ioManager.audioInputChannels
                onValueChanged: ioManager.audioInputChannels = value
            }

            // Row 3 - Signal level check
            RobotoText
            {
                visible: popupRoot.isInput
                height: UISettings.listItemHeight
                label: qsTr("Signal level")
            }

            RowLayout
            {
                visible: popupRoot.isInput
                Layout.fillWidth: true
                spacing: 5

                IconButton
                {
                    id: previewButton
                    width: UISettings.listItemHeight
                    height: width
                    checkable: true
                    faSource: checked ? FontAwesome.fa_stop : FontAwesome.fa_play
                    faColor: UISettings.fgMain
                    tooltip: qsTr("Start/Stop the audio input signal level check")
                    onToggled: ioManager.enableAudioInputPreview(checked)
                }

                // Level bar
                Rectangle
                {
                    Layout.fillWidth: true
                    height: UISettings.listItemHeight
                    radius: 3
                    color: UISettings.bgLight
                    border.width: 1
                    border.color: UISettings.bgStrong
                    clip: true

                    Rectangle
                    {
                        height: parent.height - 4
                        anchors.verticalCenter: parent.verticalCenter
                        x: 2
                        radius: 2
                        // signal power is in the 0 - 0x7FFF range
                        width: (parent.width - 4) * Math.min(1.0, ioManager.audioInputLevel / 32767)
                        gradient: Gradient
                        {
                            orientation: Gradient.Horizontal
                            GradientStop { position: 0.0; color: "green" }
                            GradientStop { position: 0.7; color: "yellow" }
                            GradientStop { position: 1.0; color: "red" }
                        }
                    }
                }
            }

            /* *********************** Audio output *********************** */

            // Output buffer length
            RobotoText
            {
                visible: !popupRoot.isInput
                height: UISettings.listItemHeight
                label: qsTr("Buffer size")
            }

            CustomSpinBox
            {
                id: bufferSpin
                visible: !popupRoot.isInput
                Layout.fillWidth: true
                from: 10
                to: 1000
                stepSize: 10
                suffix: " ms"
                value: ioManager.audioOutputBuffer
                onValueModified: ioManager.audioOutputBuffer = value
            }
        }
}
