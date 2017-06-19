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

import QtQuick 2.6
import QtQuick.Layouts 1.0
import QtQuick.Dialogs 1.2
import QtQuick.Controls 2.1

import org.qlcplus.classes 1.0
import "."

Rectangle
{
    anchors.fill: parent
    color: "transparent"

    property int functionID: -1
    property var mediaInfo: videoEditor.mediaInfo

    signal requestView(int ID, string qmlSrc)

    function updateCustomGeometry()
    {
        videoEditor.customGeometry = Qt.rect(geomXSpin.value, geomYSpin.value, geomWSpin.value, geomHSpin.value)
    }

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
        rowSpacing: 5

        // row 1
        RobotoText
        {
            height: UISettings.iconSizeDefault
            //anchors.verticalCenter: parent.verticalCenter
            label: qsTr("File name")
        }
        RowLayout
        {
            Layout.fillWidth: true
            height: UISettings.iconSizeDefault

            RobotoText
            {
                Layout.fillWidth: true
                fontSize: UISettings.textSizeDefault * 0.8
                labelColor: UISettings.fgLight
                wrapText: true
                label: videoEditor.sourceFileName
            }
            IconButton
            {
                RobotoText { anchors.centerIn: parent; label: "..." }

                onClicked:
                {
                    var videoExtList = videoEditor.videoExtensions
                    var picExtList = videoEditor.pictureExtensions
                    var vexts = qsTr("Video files") + " ("
                    for (var i = 0; i < videoExtList.length; i++)
                        vexts += videoExtList[i] + " "
                    vexts += ")"
                    var pexts = qsTr("Picture files") + " ("
                    for (var i = 0; i < picExtList.length; i++)
                        pexts += picExtList[i] + " "
                    pexts += ")"

                    openVideoDialog.nameFilters = [ vexts, pexts, qsTr("All files (*)") ]
                    openVideoDialog.visible = true
                    openVideoDialog.open()
                }
            }

            IconButton
            {
                imgSource: "qrc:/global.svg"
                tooltip: qsTr("Set a URL")
                onClicked: getUrlDialog.visible = true

                CustomPopupDialog
                {
                    id: getUrlDialog
                    title: qsTr("Enter a URL")

                    contentItem:
                        CustomTextEdit
                        {
                            id: urlInputBox
                            implicitWidth: UISettings.bigItemHeight * 3
                            implicitHeight: UISettings.listItemHeight
                            inputText: "http://"
                            Component.onCompleted: selectAndFocus()
                        }

                    onAccepted: videoEditor.sourceFileName = urlInputBox.inputText
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
        RobotoText { label: qsTr("Playback mode"); height: UISettings.listItemHeight }
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
                checked: !videoEditor.looped
                onClicked: if (checked) videoEditor.looped = false
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
                checked: videoEditor.looped
                onClicked: if (checked) videoEditor.looped = true
            }
            RobotoText
            {
                height: UISettings.listItemHeight
                label: qsTr("Looped")
            }
        }

        // row 7
        RobotoText { label: qsTr("Output screen"); height: UISettings.listItemHeight }
        CustomComboBox
        {
            id: screenCombo
            height: UISettings.listItemHeight
            Layout.fillWidth: true
            model: videoEditor.screenList
            currentIndex: videoEditor.screenIndex
            onCurrentIndexChanged: videoEditor.screenIndex = currentIndex
        }

        // row 8
        RobotoText { label: qsTr("Output mode"); height: UISettings.listItemHeight }
        RowLayout
        {
            height: UISettings.listItemHeight

            ButtonGroup { id: outputModeGroup }

            CustomCheckBox
            {
                implicitWidth: UISettings.iconSizeMedium
                implicitHeight: implicitWidth
                ButtonGroup.group: outputModeGroup
                checked: !videoEditor.fullscreen
                onClicked: if (checked) videoEditor.fullscreen = false
            }
            RobotoText
            {
                height: UISettings.listItemHeight
                label: qsTr("Windowed")
            }

            CustomCheckBox
            {
                implicitWidth: UISettings.iconSizeMedium
                implicitHeight: implicitWidth
                ButtonGroup.group: outputModeGroup
                checked: videoEditor.fullscreen
                onClicked: if (checked) videoEditor.fullscreen = true
            }
            RobotoText
            {
                height: UISettings.listItemHeight
                label: qsTr("Fullscreen")
            }
        }

        // row 9
        RobotoText { label: qsTr("Geometry"); height: UISettings.listItemHeight }
        RowLayout
        {
            height: UISettings.listItemHeight

            ButtonGroup { id: geometryGroup }

            CustomCheckBox
            {
                implicitWidth: UISettings.iconSizeMedium
                implicitHeight: implicitWidth
                ButtonGroup.group: geometryGroup
                checked: videoEditor.customGeometry.width == 0 && videoEditor.customGeometry.height == 0
            }
            RobotoText
            {
                height: UISettings.listItemHeight
                label: qsTr("Original")
            }

            CustomCheckBox
            {
                id: custGeomCheck
                implicitWidth: UISettings.iconSizeMedium
                implicitHeight: implicitWidth
                ButtonGroup.group: geometryGroup
                checked: videoEditor.customGeometry.width != 0 && videoEditor.customGeometry.height != 0
                onClicked:
                {
                    if (checked)
                    {
                        if (!mediaInfo || !mediaInfo.Resolution)
                            return

                        videoEditor.customGeometry = Qt.rect(0, 0, mediaInfo.Resolution.width, mediaInfo.Resolution.height)
                    }
                }
            }
            RobotoText
            {
                height: UISettings.listItemHeight
                label: qsTr("Custom")
            }
        }

        // row 10
        RobotoText
        {
            visible: custGeomCheck.checked
            height: UISettings.listItemHeight
            label: qsTr("Position");
        }
        RowLayout
        {
            visible: custGeomCheck.checked
            height: UISettings.listItemHeight
            width: Layout.fillWidth

            RobotoText { label: "X" }
            CustomSpinBox
            {
                id: geomXSpin
                width: Layout.fillWidth
                from: 0
                to: 99999
                value: videoEditor.customGeometry.x
                onValueChanged: updateCustomGeometry()
            }
            RobotoText { label: "Y" }
            CustomSpinBox
            {
                id: geomYSpin
                width: Layout.fillWidth
                from: 0
                to: 99999
                value: videoEditor.customGeometry.y
                onValueChanged: updateCustomGeometry()
            }
        }

        // row 11
        RobotoText
        {
            visible: custGeomCheck.checked
            height: UISettings.listItemHeight
            label: qsTr("Size");
        }
        RowLayout
        {
            visible: custGeomCheck.checked
            height: UISettings.listItemHeight
            width: Layout.fillWidth

            RobotoText { label: qsTr("W") }
            CustomSpinBox
            {
                id: geomWSpin
                width: Layout.fillWidth
                from: 0
                to: 99999
                value: videoEditor.customGeometry.width
                onValueChanged: updateCustomGeometry()
            }
            RobotoText { label: qsTr("H") }
            CustomSpinBox
            {
                id: geomHSpin
                width: Layout.fillWidth
                from: 0
                to: 99999
                value: videoEditor.customGeometry.height
                onValueChanged: updateCustomGeometry()
            }
        }
    }
}
