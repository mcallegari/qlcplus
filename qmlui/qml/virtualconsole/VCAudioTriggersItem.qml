/*
  Q Light Controller Plus
  VCAudioTriggersItem.qml

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

import org.qlcplus.classes 1.0
import "."

VCWidgetItem
{
    id: audioTriggerRoot
    property VCAudioTriggers audioTriggerObj: null

    property variant barValues: audioTriggerObj ? audioTriggerObj.audioLevels : null

    clip: true

    onAudioTriggerObjChanged:
    {
        setCommonProperties(audioTriggerObj)
    }

    GridLayout
    {
        id: itemsLayout
        anchors.fill: parent
        columns: 2
        rows: 2

        // bars area
        Rectangle
        {
            id: barsItem
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.rowSpan: 2

            color: "transparent"

            Row
            {
                id: bars
                anchors.fill: parent

                Repeater
                {
                    id: barsRep
                    model: audioTriggerObj.barsNumber

                    Rectangle
                    {
                        width: barsItem.width / audioTriggerObj.barsNumber
                        height: parent.height
                        //radius: 3
                        color: UISettings.bgStrong
                        border.width: 1
                        border.color: UISettings.bgLight

                        Rectangle
                        {
                            anchors.left: parent.left
                            anchors.right: parent.right
                            anchors.bottom: parent.bottom
                            // Keep a simple height binding
                            height: barValues ? parent.height * (Math.max(0, Math.min(255, barValues[index] || 0)) / 255.0) : 0
                            radius: 3
                            color: index == 0 ? "#00FF00" : UISettings.selection
                        }
                    }
                }
            }
        }

        // enable button
        IconButton
        {
            width: height
            height: UISettings.iconSizeMedium
            Layout.alignment: Qt.AlignHCenter
            radius: 0
            border.width: 0
            checkable: true
            tooltip: qsTr("Enable/Disable the audio capture")
            faSource: FontAwesome.fa_check
            faColor: "lime"
            imgMargins: 1
            checked: audioTriggerObj ? audioTriggerObj.captureEnabled : false
            onToggled: if (audioTriggerObj) audioTriggerObj.captureEnabled = checked
        }

        // the volume fader
        QLCPlusFader
        {
            enabled: !audioTriggerObj.isDisabled
            Layout.alignment: Qt.AlignHCenter
            Layout.fillHeight: true
            width: parent.width
            from: 0
            to: 100
            value: audioTriggerObj.volumeLevel
            onMoved: if (audioTriggerObj) audioTriggerObj.volumeLevel = valueAt(position)
        }
    }
}
