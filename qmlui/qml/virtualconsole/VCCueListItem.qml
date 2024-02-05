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
import QtQuick.Controls 2.14

import org.qlcplus.classes 1.0

VCWidgetItem
{
    id: cueListRoot
    property VCCueList cueListObj: null
    property int contentWidth: width - (sideFaderLayout.visible ? sideFaderLayout.width : 0)
    property int buttonsLayout: cueListObj ? cueListObj.playbackLayout : VCCueList.PlayPauseStop
    property int playbackStatus: cueListObj ? cueListObj.playbackStatus : VCCueList.Stopped
    property int sideFaderMode: cueListObj ? cueListObj.sideFaderMode : VCCueList.None

    property int progressStatus: VCCueList.ProgressIdle
    property real progressValue: 0
    property string progressText: ""

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
                playbackBtn.bgColor = "darkorange"
            else if (playbackStatus == VCCueList.Paused)
                playbackBtn.bgColor = "green"
            else
                playbackBtn.bgColor = UISettings.bgLight

            if (playbackStatus == VCCueList.Stopped)
                stopBtn.bgColor = UISettings.bgLight
            else
                stopBtn.bgColor = "red"
        }
        else
        {
            if (playbackStatus == VCCueList.Stopped)
                playbackBtn.bgColor = UISettings.bgLight
            else
                playbackBtn.bgColor = "red"

            if (playbackStatus == VCCueList.Paused)
                stopBtn.bgColor = "darkorange"
            else
                stopBtn.bgColor = UISettings.bgLight
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
        width: contentWidth
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
            onNoteTextChanged: if (cueListObj) cueListObj.setStepNote(index, text)
            onAddFunctions: if (cueListObj) cueListObj.addFunctions(list, index)
            onEnterPressed: if (cueListObj) cueListObj.playCurrentStep()

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

        ProgressBar
        {
            id: progressBar
            value: progressValue
            //padding: 2

            background: Rectangle {
                implicitWidth: contentWidth
                implicitHeight: UISettings.iconSizeMedium / 2
                color: UISettings.bgControl
                radius: 3
            }

            contentItem: Item {
                implicitWidth: contentWidth
                implicitHeight: UISettings.iconSizeMedium / 2

                Rectangle {
                    width: progressBar.visualPosition * parent.width
                    height: parent.height
                    radius: 2
                    color: progressStatus == VCCueList.ProgressIdle ? "transparent" :
                           progressStatus == VCCueList.ProgressFadeIn ? "#477f07" : "#0f76c5";
                }

                RobotoText
                {
                    anchors.centerIn: parent
                    fontSize: UISettings.textSizeDefault * 0.6
                    label: progressText
                }
            }
        }

        Row
        {
            id: controlsRow
            height: UISettings.iconSizeMedium

            IconButton
            {
                id: playbackBtn
                width: contentWidth / 4
                height: UISettings.iconSizeMedium
                enabled: visible && !cueListObj.isDisabled
                imgSource: (cueListObj && cueListObj.playbackLayout == VCCueList.PlayPauseStop) ?
                               (cueListRoot.playbackStatus === VCCueList.Stopped ||
                                cueListRoot.playbackStatus === VCCueList.Paused ? "qrc:/play.svg" : "qrc:/pause.svg") :
                               (cueListRoot.playbackStatus === VCCueList.Stopped ? "qrc:/play.svg" : "qrc:/stop.svg")
                tooltip: (cueListObj && cueListObj.playbackLayout == VCCueList.PlayPauseStop) ? qsTr("Play/Pause") : qsTr("Play/Stop")
                onClicked: if (cueListObj) cueListObj.playClicked()
            }
            IconButton
            {
                id: stopBtn
                width: contentWidth / 4
                height: UISettings.iconSizeMedium
                enabled: visible && !cueListObj.isDisabled
                imgSource: (cueListObj && cueListObj.playbackLayout == VCCueList.PlayStopPause) ? "qrc:/pause.svg" : "qrc:/stop.svg"
                tooltip: (cueListObj && cueListObj.playbackLayout == VCCueList.PlayStopPause) ? qsTr("Pause") : qsTr("Stop")
                onClicked: if (cueListObj) cueListObj.stopClicked()
            }
            IconButton
            {
                id: previousBtn
                width: contentWidth / 4
                height: UISettings.iconSizeMedium
                enabled: visible && !cueListObj.isDisabled
                imgSource: "qrc:/back.svg"
                tooltip: qsTr("Previous cue")
                onClicked: if (cueListObj) cueListObj.previousClicked()
            }
            IconButton
            {
                id: nextBtn
                width: contentWidth / 4
                height: UISettings.iconSizeMedium
                enabled: visible && !cueListObj.isDisabled
                imgSource: "qrc:/forward.svg"
                tooltip: qsTr("Next cue")
                onClicked: if (cueListObj) cueListObj.nextClicked()
            }
        }
    } // ColumnLayout
}
