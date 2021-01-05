/*
  Q Light Controller Plus
  VCCueListItem.qml

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

import org.qlcplus.classes 1.0

VCWidgetItem
{
    id: cueListRoot
    property VCCueList cueListObj: null
    property int buttonsLayout: cueListObj ? cueListObj.playbackLayout : VCCueList.PlayPauseStop
    property int playbackStatus: cueListObj ? cueListObj.playbackStatus : VCCueList.Stopped
    property int sideFaderMode: cueListObj ? cueListObj.sideFaderMode : VCCueList.None

    clip: true

    onCueListObjChanged:
    {
        setCommonProperties(cueListObj)
    }

    onPlaybackStatusChanged:
    {
        if (cueListObj.playbackLayout == VCCueList.PlayPauseStop)
        {
            if (playbackStatus == VCCueList.Playing)
                playbackBtn.bgColor = UISettings.highlight
            else if (playbackStatus == VCCueList.Paused)
                playbackBtn.bgColor = "red"
            else
                playbackBtn.bgColor = UISettings.bgLight

            stopBtn.bgColor = UISettings.bgLight
        }
        else
        {
            if (playbackStatus == VCCueList.Playing)
                playbackBtn.bgColor = UISettings.highlight
            else if (playbackStatus == VCCueList.Paused)
                stopBtn.bgColor = "red"
            else
            {
                playbackBtn.bgColor = UISettings.bgLight
                stopBtn.bgColor = UISettings.bgLight
            }
        }
    }

    ColumnLayout
    {
        id: sideFaderLayout
        visible: sideFaderMode === VCCueList.None ? false : true
        height: parent.height
        width: UISettings.iconSizeDefault * 1.2

        property string labelSuffix: sideFaderMode === VCCueList.Crossfade ? "%" : ""
        property int playbackIndex: cueListObj ? cueListObj.playbackIndex : -1
        property int nextStepIndex: cueListObj ? cueListObj.nextStepIndex : -1
        property bool primaryTop: cueListObj ? cueListObj.primaryTop : true

        onNextStepIndexChanged: updateLabels()
        onPrimaryTopChanged: updateLabels()

        function updateLabels()
        {
            if (playbackIndex == -1)
            {
                topLabelBox.color = "transparent"
                topLabel.label = ""
                bottomLabelBox.color = "transparent"
                bottomLabel.label = ""
            }
            else
            {
                if (sideFaderMode == VCCueList.Steps)
                {
                    bottomLabelBox.color = UISettings.highlight
                    bottomLabel.label = "#" + (playbackIndex + 1)
                }
                else
                {
                    topLabelBox.color = primaryTop ? UISettings.highlight : "orange"
                    topLabel.label = "#" + ((primaryTop ? playbackIndex : nextStepIndex) + 1)
                    bottomLabelBox.color = primaryTop ? "orange" : UISettings.highlight
                    bottomLabel.label = "#" + ((primaryTop ? nextStepIndex : playbackIndex) + 1)
                }
            }
        }

        RobotoText
        {
            height: UISettings.listItemHeight
            width: parent.width
            textHAlign: Text.AlignHCenter
            label: "" + sideFader.value + sideFaderLayout.labelSuffix
        }

        Rectangle
        {
            id: topLabelBox
            visible: sideFaderMode === VCCueList.Crossfade
            Layout.alignment: Qt.AlignHCenter
            height: UISettings.listItemHeight
            width: UISettings.iconSizeDefault
            color:  "transparent"
            border.width: 1
            border.color: UISettings.fgMain

            RobotoText
            {
                id: topLabel
                anchors.centerIn: parent
            }
        }

        QLCPlusFader
        {
            id: sideFader
            Layout.fillHeight: true
            Layout.alignment: Qt.AlignHCenter
            from: 0
            to: sideFaderMode === VCCueList.Crossfade ? 100 : 255
            value: cueListObj ? cueListObj.sideFaderLevel : 0
            onMoved: if (cueListObj) cueListObj.sideFaderLevel = value
        }

        Rectangle
        {
            id: bottomLabelBox
            height: UISettings.listItemHeight
            width: UISettings.iconSizeDefault
            Layout.alignment: Qt.AlignHCenter
            color: "transparent" // TODO
            border.width: 1
            border.color: UISettings.fgMain

            RobotoText
            {
                id: bottomLabel
                anchors.centerIn: parent
            }
        }

        RobotoText
        {
            visible: sideFaderMode === VCCueList.Crossfade
            height: UISettings.listItemHeight
            width: parent.width
            textHAlign: Text.AlignHCenter
            label: "" + (sideFader.to - sideFader.value) + sideFaderLayout.labelSuffix
        }
    }

    ColumnLayout
    {
        width: parent.width - (sideFaderLayout.visible ? sideFaderLayout.width : 0)
        height: parent.height
        x: sideFaderLayout.visible ? sideFaderLayout.width : 0
        spacing: 2

        ChaserWidget
        {
            id: chWidget
            //isSequence: ceContainer.isSequence
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: cueListObj ? cueListObj.stepsList : null
            playbackIndex: cueListObj ? cueListObj.playbackIndex : -1
            nextIndex: sideFaderMode === VCCueList.Crossfade ? sideFaderLayout.nextStepIndex : -1
            //tempoType: chaserEditor.tempoType
            //isRunning: chaserEditor.previewEnabled

            onIndexChanged: if (cueListObj) cueListObj.playbackIndex = index
            //onStepValueChanged: chaserEditor.setStepSpeed(index, value, type)
            onAddFunctions: if (cueListObj) cueListObj.addFunctions(list, index)

            onDragEntered: virtualConsole.setDropTarget(item, true)
            onDragExited: virtualConsole.setDropTarget(item, false)

            states: [
                State
                {
                    when: chWidget.containsDrag
                    PropertyChanges
                    {
                        target: cueListRoot
                        color: UISettings.activeDropArea
                    }
                }
            ]
        }

        Row
        {
            id: controlsRow
            height: UISettings.iconSizeMedium

            IconButton
            {
                id: playbackBtn
                width: cueListRoot.width / 4
                height: UISettings.iconSizeMedium
                imgSource: cueListRoot.playbackStatus === VCCueList.Paused ? "qrc:/pause.svg" : "qrc:/play.svg"
                tooltip: qsTr("Play/Pause")
                onClicked: if (cueListObj) cueListObj.playClicked()
            }
            IconButton
            {
                id: stopBtn
                width: cueListRoot.width / 4
                height: UISettings.iconSizeMedium
                imgSource: (cueListObj && cueListObj.playbackLayout == VCCueList.PlayStopPause) ? "qrc:/pause.svg" : "qrc:/stop.svg"
                tooltip: (cueListObj && cueListObj.playbackLayout == VCCueList.PlayStopPause) ? qsTr("Pause") : qsTr("Stop")
                onClicked: if (cueListObj) cueListObj.stopClicked()
            }
            IconButton
            {
                id: previousBtn
                width: cueListRoot.width / 4
                height: UISettings.iconSizeMedium
                imgSource: "qrc:/back.svg"
                tooltip: qsTr("Previous cue")
                onClicked: if (cueListObj) cueListObj.previousClicked()
            }
            IconButton
            {
                id: nextBtn
                width: cueListRoot.width / 4
                height: UISettings.iconSizeMedium
                imgSource: "qrc:/forward.svg"
                tooltip: qsTr("Next cue")
                onClicked: if (cueListObj) cueListObj.nextClicked()
            }
        }
    } // ColumnLayout
}
