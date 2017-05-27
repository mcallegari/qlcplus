/*
  Q Light Controller Plus
  VideoEditor.qml

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
import QtQuick.Controls 1.0

import org.qlcplus.classes 1.0
import "."

Rectangle
{
    anchors.fill: parent
    color: "transparent"

    property int functionID: -1
    property var mediaInfo: videoEditor.mediaInfo

    signal requestView(int ID, string qmlSrc)

    EditorTopBar
    {
        id: topBar
        text: videoEditor.functionName
        onTextChanged: videoEditor.functionName = text

        onBackClicked:
        {
            functionManager.setEditorFunction(-1, false)
            requestView(-1, "qrc:/FunctionManager.qml")
        }
    }

    FileDialog
    {
        id: openVideoDialog
        visible: false

        onAccepted:
        {
            var url = "" + openVideoDialog.fileUrl
            videoEditor.sourceFileName = url.replace("file://", "")
        }
    }

    GridLayout
    {
        id: aeGrid
        y: topBar.height
        width: parent.width

        columns: 2
        columnSpacing: 5
        rowSpacing: 10


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
                label: videoEditor.sourceFileName
            }
            IconButton
            {
                id: selFileBtn
                x: parent.width - width - 3
                RobotoText { anchors.centerIn: parent; label: "..." }

                onClicked:
                {
                    var extList = videoEditor.mimeTypes
                    var exts = qsTr("Video files") + " ("
                    for (var i = 0; i < extList.length; i++)
                        exts += extList[i] + " "
                    exts += ")"
                    openVideoDialog.nameFilters = [ exts, qsTr("All files (*)") ]
                    openVideoDialog.visible = true
                    openVideoDialog.open()
                }
            }
        }

        // row 2
        RobotoText { label: qsTr("Duration"); height: UISettings.listItemHeight }
        RobotoText
        {
            height: UISettings.listItemHeight
            Layout.fillWidth: true
            label: mediaInfo && mediaInfo.Duration ? mediaInfo.Duration : ""
            labelColor: UISettings.fgLight
        }

        // row 3
        RobotoText { label: qsTr("Resolution"); height: UISettings.listItemHeight }
        RobotoText
        {
            height: UISettings.listItemHeight
            Layout.fillWidth: true
            label: mediaInfo && mediaInfo.Resolution ? "" + mediaInfo.Resolution.width + "x" + mediaInfo.Resolution.height : ""
            labelColor: UISettings.fgLight
        }

        // row 4
        RobotoText { label: qsTr("Video Codec"); height: UISettings.listItemHeight }
        RobotoText
        {
            height: UISettings.listItemHeight
            Layout.fillWidth: true
            label: mediaInfo && mediaInfo.VideoCodec ? mediaInfo.VideoCodec : ""
            labelColor: UISettings.fgLight
        }

        // row 5
        RobotoText { label: qsTr("Audio Codec"); height: UISettings.listItemHeight }
        RobotoText
        {
            height: UISettings.listItemHeight
            Layout.fillWidth: true
            label: mediaInfo && mediaInfo.AudioCodec ? mediaInfo.AudioCodec : ""
            labelColor: UISettings.fgLight
        }

        // row 6
        RobotoText { label: qsTr("Output screen"); height: UISettings.listItemHeight }
        CustomComboBox
        {
            height: UISettings.listItemHeight
            Layout.fillWidth: true
            model: videoEditor.screenList
            currentIndex: videoEditor.screenIndex
            onCurrentIndexChanged: videoEditor.screenIndex = currentIndex
        }

        // row 7
        RobotoText { label: qsTr("Output mode"); height: UISettings.listItemHeight }
        GridLayout
        {
            height: UISettings.listItemHeight
            Layout.fillWidth: true

            ExclusiveGroup { id: outputModeGroup }

            CustomCheckBox
            {
                width: UISettings.iconSizeMedium
                height: width
                exclusiveGroup: outputModeGroup
                checked: !videoEditor.fullscreen
                onToggle: if (checked) videoEditor.fullscreen = false
            }
            RobotoText
            {
                height: UISettings.listItemHeight
                Layout.fillWidth: true
                label: qsTr("Windowed")
            }

            CustomCheckBox
            {
                width: UISettings.iconSizeMedium
                height: width
                exclusiveGroup: outputModeGroup
                checked: videoEditor.fullscreen
                onToggle: if (checked) videoEditor.fullscreen = true
            }
            RobotoText
            {
                height: UISettings.listItemHeight
                Layout.fillWidth: true
                label: qsTr("Fullscreen")
            }
        }

        // row 8
        RobotoText { label: qsTr("Playback mode"); height: UISettings.listItemHeight }
        GridLayout
        {
            height: UISettings.listItemHeight
            Layout.fillWidth: true

            ExclusiveGroup { id: playbackModeGroup }

            CustomCheckBox
            {
                width: UISettings.iconSizeMedium
                height: width
                exclusiveGroup: playbackModeGroup
                checked: !videoEditor.looped
                onToggle: if (checked) videoEditor.looped = false
            }
            RobotoText
            {
                height: UISettings.listItemHeight
                Layout.fillWidth: true
                label: qsTr("Single shot")
            }

            CustomCheckBox
            {
                width: UISettings.iconSizeMedium
                height: width
                exclusiveGroup: playbackModeGroup
                checked: videoEditor.looped
                onToggle: if (checked) videoEditor.looped = true
            }
            RobotoText
            {
                height: UISettings.listItemHeight
                Layout.fillWidth: true
                label: qsTr("Looped")
            }
        }
    }
}
