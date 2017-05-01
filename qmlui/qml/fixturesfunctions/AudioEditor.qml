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

import com.qlcplus.classes 1.0
import "."

Rectangle
{
    id: peContainer
    anchors.fill: parent
    color: "transparent"

    property int functionID: -1
    property var mediaInfo: audioEditor.mediaInfo

    signal requestView(int ID, string qmlSrc)

    EditorTopBar
    {
        id: topBar
        text: audioEditor.functionName
        onTextChanged: audioEditor.functionName = text

        onBackClicked:
        {
            functionManager.setEditorFunction(-1, false)
            requestView(-1, "qrc:/FunctionManager.qml")
        }
    }

    FileDialog
    {
        id: openAudioDialog
        visible: false

        onAccepted:
        {
            //console.log("You chose: " + openAudioDialog.fileUrl)
            var url = "" + openAudioDialog.fileUrl
            audioEditor.sourceFileName = url.replace("file://", "")
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
                    var extList = audioEditor.mimeTypes
                    var exts = qsTr("Audio files") + " ("
                    for (var i = 0; i < extList.length; i++)
                        exts += extList[i] + " "
                    exts += ")"
                    openAudioDialog.nameFilters = [ exts, qsTr("All files (*)") ]
                    openAudioDialog.visible = true
                    openAudioDialog.open()
                }
            }
        }

        // row 2
        RobotoText { label: qsTr("Duration"); height: UISettings.listItemHeight }
        RobotoText
        {
            height: UISettings.listItemHeight
            Layout.fillWidth: true
            label: mediaInfo ? mediaInfo.duration : ""
            labelColor: UISettings.fgLight
        }

        // row 3
        RobotoText { label: qsTr("Channels"); height: UISettings.listItemHeight }
        RobotoText
        {
            height: UISettings.listItemHeight
            Layout.fillWidth: true
            label: mediaInfo ? mediaInfo.channels : ""
            labelColor: UISettings.fgLight
        }

        // row 4
        RobotoText { label: qsTr("Sample Rate"); height: UISettings.listItemHeight }
        RobotoText
        {
            height: UISettings.listItemHeight
            Layout.fillWidth: true
            label: mediaInfo ? mediaInfo.sampleRate : ""
            labelColor: UISettings.fgLight
        }

        // row 5
        RobotoText { label: qsTr("Bitrate"); height: UISettings.listItemHeight }
        RobotoText
        {
            height: UISettings.listItemHeight
            Layout.fillWidth: true
            label: mediaInfo ? mediaInfo.bitrate : ""
            labelColor: UISettings.fgLight
        }


    }
}
