/*
  Q Light Controller Plus
  VCAudioTriggerItem.qml

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

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

import org.qlcplus.classes 1.0

VCWidgetItem
{
    id: audioTriggerRoot
    property VCAudioTrigger audioTriggerObj: null

    clip: true

    onAudioTriggerObjChanged:
    {
        setCommonProperties(audioTriggerObj)
    }

    Row
    {
        anchors.fill: parent

        // value text box
        Text
        {
            width: parent.width
            height: parent.height
            color: "#bbb"
            lineHeight: 0.8
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            textFormat: Text.RichText
            wrapMode: Text.Wrap
            text: "VCAudioTrigger not implemented yet.<br />See <a href=\"https://docs.google.com/spreadsheets/d/1J1BK0pYCsLVBfLpDZ-GqpNgUzbgTwmhf9zOjg4J_MWg\">QML Status</a>"

            onLinkActivated: Qt.openUrlExternally(link)

            MouseArea
            {
                anchors.fill: parent
                acceptedButtons: Qt.NoButton
                cursorShape: parent.hoveredLink ? Qt.PointingHandCursor : Qt.ArrowCursor
            }
        }
    }
}
