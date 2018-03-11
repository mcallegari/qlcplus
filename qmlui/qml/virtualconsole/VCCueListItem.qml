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
        anchors.fill: parent
        spacing: 2

        ChaserWidget
        {
            id: chWidget
            //isSequence: ceContainer.isSequence
            Layout.fillWidth: true
            height: cueListRoot.height - controlsRow.height
            model: cueListObj ? cueListObj.stepsList : null
            playbackIndex: cueListObj ? cueListObj.playbackIndex : -1
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
