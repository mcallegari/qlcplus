/*
  Q Light Controller Plus
  AudioEditor.qml

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
import QtQuick.Dialogs 1.1
import QtQuick.Controls 2.1

import org.qlcplus.classes 1.0
import "TimeUtils.js" as TimeUtils
import "."

Rectangle
{
    anchors.fill: parent
    color: "transparent"

    property int functionID: -1
    property var mediaInfo: audioEditor ? audioEditor.mediaInfo : null

    signal requestView(int ID, string qmlSrc)

    TimeEditTool
    {
        id: timeEditTool

        parent: mainView
        z: 99
        x: rightSidePanel.x - width
        visible: false

        onValueChanged:
        {
            if (speedType == QLCFunction.FadeIn)
                audioEditor.fadeInSpeed = val
            else if (speedType == QLCFunction.FadeOut)
                audioEditor.fadeOutSpeed = val
        }
    }

    EditorTopBar
    {
        id: topBar
        text: audioEditor.functionName
        onTextChanged: audioEditor.functionName = text

        onBackClicked:
        {
            var prevID = audioEditor.previousID
            functionManager.setEditorFunction(prevID, false, true)
            requestView(prevID, functionManager.getEditorResource(prevID))
        }
    }

    FileDialog
    {
        id: openAudioDialog
        visible: false

        onAccepted:
        {
            //console.log("You chose: " + openAudioDialog.fileUrl)
            audioEditor.sourceFileName = openAudioDialog.fileUrl
        }
    }

    onWidthChanged: aeGrid.width = width

    GridLayout
    {
        id: aeGrid
        columns: 2
        columnSpacing: 5
        rowSpacing: 10
        y: topBar.height

        // row 1
        RobotoText
        {
            height: selFileBtn.height
            //anchors.verticalCenter: parent.verticalCenter
            label: qsTr("File name")
        }
        Rectangle
        {
            Layout.fillWidth: true
            height: selFileBtn.height
            color: "transparent"

            RobotoText
            {
                width: selFileBtn.x - 5
                fontSize: UISettings.textSizeDefault * 0.8
                labelColor: UISettings.fgLight
                wrapText: true
                label: audioEditor.sourceFileName
            }
            IconButton
            {
                id: selFileBtn
                x: parent.width - width - 3
                RobotoText { anchors.centerIn: parent; label: "..." }

                onClicked:
                {
                    var extList = audioEditor.audioExtensions
                    var exts = qsTr("Audio files") + " ("
                    for (var i = 0; i < extList.length; i++)
                        exts += extList[i] + " "
                    exts += ")"
                    openAudioDialog.nameFilters = [ exts, qsTr("All files") + " (*)" ]
                    openAudioDialog.visible = true
                    openAudioDialog.open()
                }
            }
        }

        // row 2
        RobotoText
        {
            label: qsTr("Duration")
            height: UISettings.listItemHeight
        }
        RobotoText
        {
            height: UISettings.listItemHeight
            Layout.fillWidth: true
            label: mediaInfo ? mediaInfo.duration : ""
            labelColor: UISettings.fgLight
        }

        // row 3
        RobotoText
        {
            label: qsTr("Channels")
            height: UISettings.listItemHeight
        }
        RobotoText
        {
            height: UISettings.listItemHeight
            Layout.fillWidth: true
            label: mediaInfo ? mediaInfo.channels : ""
            labelColor: UISettings.fgLight
        }

        // row 4
        RobotoText
        {
            label: qsTr("Sample Rate")
            height: UISettings.listItemHeight
        }
        RobotoText
        {
            height: UISettings.listItemHeight
            Layout.fillWidth: true
            label: mediaInfo ? mediaInfo.sampleRate : ""
            labelColor: UISettings.fgLight
        }

        // row 5
        RobotoText
        {
            label: qsTr("Bitrate")
            height: UISettings.listItemHeight
        }
        RobotoText
        {
            height: UISettings.listItemHeight
            Layout.fillWidth: true
            label: mediaInfo ? mediaInfo.bitrate : ""
            labelColor: UISettings.fgLight
        }

        // row 6
        RobotoText
        {
            label: qsTr("Playback mode")
            height: UISettings.listItemHeight
        }
        RowLayout
        {
            height: UISettings.listItemHeight
            //Layout.fillWidth: true

            ButtonGroup { id: playbackModeGroup }

            CustomCheckBox
            {
                implicitWidth: UISettings.iconSizeMedium
                implicitHeight: implicitWidth
                ButtonGroup.group: playbackModeGroup
                checked: !audioEditor.looped
                onClicked: if (checked) audioEditor.looped = false
            }
            RobotoText
            {
                height: UISettings.listItemHeight
                label: qsTr("Single shot")
            }

            CustomCheckBox
            {
                implicitWidth: UISettings.iconSizeMedium
                implicitHeight: implicitWidth
                ButtonGroup.group: playbackModeGroup
                checked: audioEditor.looped
                onClicked: if (checked) audioEditor.looped = true
            }
            RobotoText
            {
                height: UISettings.listItemHeight
                label: qsTr("Looped")
            }
        }

        // row 7
        RobotoText
        {
            label: qsTr("Output device")
            height: UISettings.listItemHeight
        }
        CustomComboBox
        {
            height: UISettings.listItemHeight
            Layout.fillWidth: true
            model: ioManager.audioOutputSources
            currentIndex: audioEditor.cardLineIndex
            onCurrentIndexChanged: audioEditor.cardLineIndex = currentIndex
        }

        // row 8
        RobotoText
        {
            label: qsTr("Volume")
            height: UISettings.listItemHeight
        }
        CustomSpinBox
        {
            height: UISettings.listItemHeight
            Layout.fillWidth: true
            from: 0
            to: 100
            value: audioEditor.volume
            suffix: "%"
            onValueChanged: audioEditor.volume = value
       }

        // row 9
        RobotoText
        {
            id: fiLabel
            label: qsTr("Fade in")
            height: UISettings.listItemHeight
        }

        Rectangle
        {
            Layout.fillWidth: true
            height: UISettings.listItemHeight
            color: UISettings.bgMedium

            RobotoText
            {
                anchors.fill: parent
                label: TimeUtils.timeToQlcString(audioEditor.fadeInSpeed, QLCFunction.Time)

                MouseArea
                {
                    anchors.fill: parent
                    onDoubleClicked:
                    {
                        timeEditTool.show(-1, this.mapToItem(mainView, 0, 0).y,
                                          fiLabel.label, parent.label, QLCFunction.FadeIn)
                    }
                }
            }
        }

        // row 10
        RobotoText
        {
            id: foLabel
            height: UISettings.listItemHeight
            label: qsTr("Fade out")
        }

        Rectangle
        {
            Layout.fillWidth: true
            height: UISettings.listItemHeight
            color: UISettings.bgMedium

            RobotoText
            {
                anchors.fill: parent
                label: TimeUtils.timeToQlcString(audioEditor.fadeOutSpeed, QLCFunction.Time)

                MouseArea
                {
                    anchors.fill: parent
                    onDoubleClicked:
                    {
                        timeEditTool.show(-1, this.mapToItem(mainView, 0, 0).y,
                                          foLabel.label, parent.label, QLCFunction.FadeOut)
                    }
                }
            }
        }
    }
}
