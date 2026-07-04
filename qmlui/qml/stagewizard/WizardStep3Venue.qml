/*
  Q Light Controller Plus
  WizardStep3Venue.qml

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
import QtQuick.Controls.Basic
import "."

Item
{
    id: root

    // Must match MonitorProperties::StageType enum order
    readonly property var stageTypes: [
        {
            id: 0,
            name: qsTr("Open Space"),
            icon: "⬜",
            description: qsTr("Plain floor with no scenic elements. Good for temporary rigs and general-purpose events."),
            bestFor: [qsTr("Architectural"), qsTr("Custom")]
        },
        {
            id: 1,
            name: qsTr("Box / Club"),
            icon: "🏠",
            description: qsTr("Four walls and a ceiling. Truss along the perimeter. Best for club nights and small venues."),
            bestFor: [qsTr("Club Night")]
        },
        {
            id: 2,
            name: qsTr("Rock Stage"),
            icon: "🎸",
            description: qsTr("Raised stage with front truss and vertical columns. Standard for concerts and live shows."),
            bestFor: [qsTr("Concert / Live")]
        },
        {
            id: 3,
            name: qsTr("Theatre"),
            icon: "🎭",
            description: qsTr("Proscenium arch, front-of-house bars, side booms. Classic theatrical rig."),
            bestFor: [qsTr("Theatrical")]
        }
    ]

    // ── Left: Stage type selection ──────────────────────────────────────────
    ColumnLayout
    {
        id: leftPanel
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        width: parent.width * 0.42
        spacing: UISettings.listItemHeight * 0.4

        RobotoText
        {
            label: qsTr("Venue type")
            fontSize: UISettings.textSizeDefault * 1.3
            fontBold: true
            labelColor: "#DDDDEE"
        }
        RobotoText
        {
            Layout.fillWidth: true
            label: qsTr("Fixtures are automatically positioned following industry rigging conventions.")
            fontSize: UISettings.textSizeDefault * 0.9
            labelColor: "#888899"
            wrapText: true
        }

        Repeater
        {
            model: root.stageTypes

            Rectangle
            {
                id: stageCard
                Layout.fillWidth: true
                Layout.preferredHeight: cardRow.implicitHeight + UISettings.listItemHeight * 0.8
                radius: 10

                readonly property bool isSelected: stageWizard && stageWizard.stageType === modelData.id
                property bool hovered: false

                color: isSelected ? "#1E1040" : (hovered ? "#1A1A3A" : "#111128")
                border.color: isSelected ? "#E94560" : (hovered ? "#555577" : "#2A2A44")
                border.width: isSelected ? 2 : 1

                Behavior on color        { ColorAnimation { duration: 150 } }
                Behavior on border.color { ColorAnimation { duration: 150 } }

                MouseArea
                {
                    anchors.fill: parent
                    hoverEnabled: true
                    onEntered:  stageCard.hovered = true
                    onExited:   stageCard.hovered = false
                    onClicked:
                    {
                        if (stageWizard)
                            stageWizard.stageType = modelData.id
                    }
                }

                RowLayout
                {
                    id: cardRow
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.margins: UISettings.listItemHeight * 0.4
                    spacing: UISettings.listItemHeight * 0.4

                    Text
                    {
                        text: modelData.icon
                        font.pixelSize: UISettings.textSizeDefault * 1.8
                        Layout.alignment: Qt.AlignVCenter
                    }

                    ColumnLayout
                    {
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignVCenter
                        spacing: 4

                        RowLayout
                        {
                            Layout.fillWidth: true
                            spacing: 8

                            Text
                            {
                                text: modelData.name
                                font.family: UISettings.robotoFontName
                                font.pixelSize: UISettings.textSizeDefault * 1.05
                                font.bold: true
                                color: stageCard.isSelected ? "#E94560" : "white"
                            }

                            // "Best for" tags
                            Repeater
                            {
                                model: modelData.bestFor
                                Rectangle
                                {
                                    height: bfText.implicitHeight + 6
                                    width: bfText.implicitWidth + 12
                                    radius: height / 2
                                    color: "#2A1030"

                                    Text
                                    {
                                        id: bfText
                                        anchors.centerIn: parent
                                        text: modelData
                                        font.family: UISettings.robotoFontName
                                        font.pixelSize: UISettings.textSizeDefault * 0.72
                                        color: "#E94560"
                                    }
                                }
                            }

                            Item { Layout.fillWidth: true }
                        }

                        Text
                        {
                            Layout.fillWidth: true
                            text: modelData.description
                            font.family: UISettings.robotoFontName
                            font.pixelSize: UISettings.textSizeDefault * 0.82
                            color: "#777799"
                            wrapMode: Text.WordWrap
                        }
                    }

                    Rectangle
                    {
                        Layout.alignment: Qt.AlignVCenter
                        width: UISettings.listItemHeight * 0.6
                        height: width
                        radius: width / 2
                        color: "#E94560"
                        visible: stageCard.isSelected

                        Text
                        {
                            anchors.centerIn: parent
                            text: "✓"
                            font.family: UISettings.robotoFontName
                            font.pixelSize: UISettings.textSizeDefault * 0.85
                            font.bold: true
                            color: "white"
                        }
                    }
                }
            }
        }

        // ── Environment size (auto-suggested, editable) ──────────────────────
        RobotoText
        {
            Layout.topMargin: UISettings.listItemHeight * 0.3
            label: qsTr("Stage size (metres)")
            fontSize: UISettings.textSizeDefault * 1.0
            fontBold: true
            labelColor: "#AAAACC"
        }

        RowLayout
        {
            Layout.fillWidth: true
            spacing: UISettings.listItemHeight * 0.4

            // One labelled numeric field per dimension
            Repeater
            {
                model: [
                    { key: "W", tip: qsTr("Width") },
                    { key: "H", tip: qsTr("Height") },
                    { key: "D", tip: qsTr("Depth") }
                ]

                RowLayout
                {
                    spacing: 4

                    RobotoText
                    {
                        label: modelData.key
                        fontSize: UISettings.textSizeDefault
                        fontBold: true
                        labelColor: "#888899"
                    }

                    TextField
                    {
                        id: dimField
                        Layout.preferredWidth: UISettings.bigItemHeight
                        height: UISettings.listItemHeight
                        color: "white"
                        font.family: UISettings.robotoFontName
                        font.pixelSize: UISettings.textSizeDefault
                        horizontalAlignment: TextInput.AlignHCenter
                        inputMethodHints: Qt.ImhDigitsOnly
                        validator: IntValidator { bottom: 2; top: 100 }
                        selectByMouse: true

                        text: !stageWizard ? "5"
                              : (modelData.key === "W" ? Math.round(stageWizard.envWidth)
                              :  modelData.key === "H" ? Math.round(stageWizard.envHeight)
                              :                          Math.round(stageWizard.envDepth))

                        background: Rectangle
                        {
                            radius: 4
                            color: "#0E0E20"
                            border.color: dimField.activeFocus ? "#0978FF" : "#2A2A44"
                            border.width: 1
                        }

                        onEditingFinished:
                        {
                            if (!stageWizard) return
                            var v = parseFloat(text.replace(",", "."))
                            if (isNaN(v)) return
                            if (modelData.key === "W")      stageWizard.envWidth = v
                            else if (modelData.key === "H") stageWizard.envHeight = v
                            else                            stageWizard.envDepth = v
                        }
                    }
                }
            }
        }

        RobotoText
        {
            Layout.fillWidth: true
            label: qsTr("Suggested from your fixture count. Adjust if your real stage differs.")
            fontSize: UISettings.textSizeDefault * 0.8
            labelColor: "#666688"
            wrapText: true
        }

        // Push cards to the top
        Item { Layout.fillHeight: true }
    }

    // ── Divider ─────────────────────────────────────────────────────────────
    Rectangle
    {
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.left: leftPanel.right
        anchors.leftMargin: UISettings.listItemHeight * 0.5
        width: 1
        color: "#2A2A44"
    }

    // ── Right: Placement summary ─────────────────────────────────────────────
    ColumnLayout
    {
        anchors.top: parent.top
        anchors.left: leftPanel.right
        anchors.leftMargin: UISettings.listItemHeight
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        spacing: UISettings.listItemHeight * 0.5

        RobotoText
        {
            label: qsTr("Automatic fixture placement")
            fontSize: UISettings.textSizeDefault * 1.3
            fontBold: true
            labelColor: "#DDDDEE"
        }

        RobotoText
        {
            Layout.fillWidth: true
            label: qsTr("Based on the roles you assigned, fixtures will be positioned as follows. No manual placement needed.")
            fontSize: UISettings.textSizeDefault * 0.9
            labelColor: "#888899"
            wrapText: true
        }

        // Placement rule rows
        ListView
        {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            spacing: 6
            model: stageWizard ? stageWizard.fixtureRoleModel : []

            delegate: Rectangle
            {
                width: ListView.view.width
                height: UISettings.listItemHeight * 1.1
                radius: 8
                color: "#0E0E20"

                readonly property var rolePlacement: [
                    qsTr("Front truss, high — aimed at stage centre ~45°"),
                    qsTr("Mid-height, forward position — supplemental wash"),
                    qsTr("Rear truss, high — backlighting from behind"),
                    qsTr("Wing booms, alternating left/right — side fill"),
                    qsTr("Top truss, centre — aerial mid-air beams"),
                    qsTr("Full-width batten across top front — colour wash"),
                    qsTr("Front edge, high — horizontal, aimed at audience"),
                    qsTr("Centre, mid-height — atmospheric haze"),
                    qsTr("Floor level, front — aimed straight up")
                ]

                RowLayout
                {
                    anchors.fill: parent
                    anchors.margins: 10
                    spacing: 10

                    RobotoText
                    {
                        Layout.preferredWidth: parent.width * 0.28
                        implicitHeight: UISettings.listItemHeight
                        Layout.alignment: Qt.AlignVCenter
                        label: modelData.name
                        fontBold: true
                        labelColor: "white"
                    }

                    RobotoText
                    {
                        Layout.alignment: Qt.AlignVCenter
                        implicitHeight: UISettings.listItemHeight
                        label: "→"
                        labelColor: "#555577"
                    }

                    RobotoText
                    {
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignVCenter
                        implicitHeight: UISettings.listItemHeight
                        label: rolePlacement[modelData.role] || ""
                        fontSize: UISettings.textSizeDefault * 0.85
                        labelColor: "#8888AA"
                        wrapText: true
                    }

                    RobotoText
                    {
                        Layout.alignment: Qt.AlignVCenter
                        implicitHeight: UISettings.listItemHeight
                        label: modelData.fixtureCount + "x"
                        fontSize: UISettings.textSizeDefault * 0.85
                        labelColor: "#555566"
                    }
                }
            }

            ScrollBar.vertical: CustomScrollBar {}
        }
    }
}
