/*
  Q Light Controller Plus
  WizardStep1ShowType.qml

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
import "."

Item
{
    id: root

    // Show types must match StageWizard::ShowType enum order
    readonly property var showTypes: [
        {
            id: 0,
            name: qsTr("Club Night"),
            icon: "🎉",
            description: qsTr("Fast chasers, strobe hits, RGB chases, BPM-locked effects. Built for high energy dance floors."),
            stage: qsTr("Box / Club"),
            palette: ["#E94560", "#7C5C9F", "#0044FF"],
            tags: [qsTr("Fast"), qsTr("Strobe"), qsTr("RGB"), qsTr("BPM")]
        },
        {
            id: 1,
            name: qsTr("Concert / Live"),
            icon: "🎸",
            description: qsTr("Position presets, color washes, audience blinders, movement EFX. Designed for live performances."),
            stage: qsTr("Rock Stage"),
            palette: ["#FF4500", "#FFA500", "#FFD700"],
            tags: [qsTr("Wash"), qsTr("Position"), qsTr("Blinder"), qsTr("EFX")]
        },
        {
            id: 2,
            name: qsTr("Theatrical"),
            icon: "🎭",
            description: qsTr("Scene-based, slow fades, warm colors, gobo patterns, position presets. For theatre and dance."),
            stage: qsTr("Theatre"),
            palette: ["#FFD700", "#FFA07A", "#E8D5B7"],
            tags: [qsTr("Scenes"), qsTr("Slow Fade"), qsTr("Gobo"), qsTr("Warm")]
        },
        {
            id: 3,
            name: qsTr("Architectural"),
            icon: "🏛",
            description: qsTr("Gentle pixel chases, color blends, ambient loops. Ideal for permanent installations."),
            stage: qsTr("Open Space"),
            palette: ["#00CED1", "#40E0D0", "#7FFFD4"],
            tags: [qsTr("Pixel"), qsTr("Ambient"), qsTr("Soft"), qsTr("Loop")]
        },
        {
            id: 4,
            name: qsTr("Custom"),
            icon: "⚙",
            description: qsTr("Choose your own effects and settings in the next steps. Nothing is pre-selected."),
            stage: qsTr("Any"),
            palette: ["#888888", "#AAAAAA", "#CCCCCC"],
            tags: [qsTr("Manual"), qsTr("Flexible")]
        }
    ]

    ColumnLayout
    {
        anchors.fill: parent
        spacing: 0

        // ── Title ──────────────────────────────────────────────────────────
        RobotoText
        {
            Layout.alignment: Qt.AlignHCenter
            label: qsTr("What kind of show are you creating?")
            fontSize: UISettings.textSizeDefault * 1.5
            fontBold: true
            labelColor: "#DDDDEE"
        }

        RobotoText
        {
            Layout.alignment: Qt.AlignHCenter
            Layout.topMargin: 4
            label: qsTr("Your choice sets sensible defaults for stage layout and effects — you can customise everything in the next steps.")
            fontSize: UISettings.textSizeDefault
            labelColor: "#888899"
        }

        Item { Layout.preferredHeight: UISettings.listItemHeight * 0.8 }

        // ── Cards ──────────────────────────────────────────────────────────
        GridLayout
        {
            Layout.alignment: Qt.AlignHCenter
            Layout.fillWidth: true
            Layout.fillHeight: true
            columns: 3
            columnSpacing: UISettings.listItemHeight * 0.6
            rowSpacing:    UISettings.listItemHeight * 0.6

            Repeater
            {
                model: root.showTypes

                ShowTypeCard
                {
                    showData:    modelData
                    isSelected:  stageWizard && stageWizard.showType === modelData.id
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.preferredHeight: UISettings.bigItemHeight * 1.8

                    onClicked: if (stageWizard) stageWizard.showType = modelData.id
                }
            }
        }

        Item { Layout.preferredHeight: UISettings.listItemHeight * 0.4 }
    }
}
