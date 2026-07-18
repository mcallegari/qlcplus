/*
  Q Light Controller Plus
  WizardStep6Summary.qml

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

    readonly property var sectionIcons: ({
        "Stage":           "🏛",
        "Functions":       "🧩",
        "Virtual Console": "🎛"
    })

    // ── Header ──────────────────────────────────────────────────────────────
    Column
    {
        id: hdr
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        spacing: 4

        Text
        {
            text: qsTr("Ready to generate")
            font.family: UISettings.robotoFontName
            font.pixelSize: UISettings.textSizeDefault * 1.5
            font.bold: true
            color: "#DDDDEE"
        }
        Text
        {
            width: parent.width
            text: qsTr("Review what will be created and press Generate. The wizard will build your show in a single step — fully undoable.")
            font.family: UISettings.robotoFontName
            font.pixelSize: UISettings.textSizeDefault * 0.95
            color: "#888899"
            wrapMode: Text.WordWrap
        }
    }

    // ── Two-column layout ────────────────────────────────────────────────────
    RowLayout
    {
        anchors.top: hdr.bottom
        anchors.topMargin: UISettings.listItemHeight * 0.6
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        spacing: UISettings.listItemHeight

        // ── Left: summary cards ──────────────────────────────────────────────
        ColumnLayout
        {
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.preferredWidth: 1   // equal split with the right column
            Layout.alignment: Qt.AlignTop
            spacing: UISettings.listItemHeight * 0.35

            Text
            {
                text: qsTr("What will be created")
                font.family: UISettings.robotoFontName
                font.bold: true
                font.pixelSize: UISettings.textSizeDefault
                color: "#AAAACC"
            }

            Repeater
            {
                model: stageWizard ? stageWizard.summaryModel : []

                Rectangle
                {
                    Layout.fillWidth: true
                    Layout.preferredHeight: cardRow.implicitHeight + UISettings.listItemHeight * 0.7
                    radius: 10
                    color: "#0E0E20"
                    border.color: "#2A2A44"

                    RowLayout
                    {
                        id: cardRow
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.leftMargin: 14
                        anchors.rightMargin: 14
                        spacing: 12

                        Text
                        {
                            Layout.alignment: Qt.AlignTop
                            text: root.sectionIcons[modelData.section] || "•"
                            font.pixelSize: UISettings.textSizeDefault * 1.5
                        }

                        ColumnLayout
                        {
                            Layout.fillWidth: true
                            spacing: 3

                            Text
                            {
                                Layout.fillWidth: true
                                text: modelData.section
                                font.family: UISettings.robotoFontName
                                font.bold: true
                                font.pixelSize: UISettings.textSizeDefault
                                color: "white"
                            }
                            Text
                            {
                                Layout.fillWidth: true
                                text: modelData.detail
                                font.family: UISettings.robotoFontName
                                font.pixelSize: UISettings.textSizeDefault * 0.88
                                color: "#8888AA"
                                wrapMode: Text.WordWrap
                            }
                        }
                    }
                }
            }

            // Effects selected
            Rectangle
            {
                Layout.fillWidth: true
                Layout.preferredHeight: effectsCol.implicitHeight + UISettings.listItemHeight * 0.7
                radius: 10
                color: "#0E0E20"
                border.color: "#2A2A44"

                ColumnLayout
                {
                    id: effectsCol
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: parent.top
                    anchors.leftMargin: 14
                    anchors.rightMargin: 14
                    anchors.topMargin: UISettings.listItemHeight * 0.35
                    spacing: 6

                    Text
                    {
                        font.family: UISettings.robotoFontName
                        font.bold: true
                        font.pixelSize: UISettings.textSizeDefault
                        color: "white"
                        text: qsTr("⚡  Selected effects")
                    }

                    Flow
                    {
                        Layout.fillWidth: true
                        spacing: 5

                        Repeater
                        {
                            model: stageWizard ? stageWizard.effectsModel : []

                            Rectangle
                            {
                                visible: modelData.enabled
                                height: efText.implicitHeight + 6
                                width: efText.implicitWidth + 14
                                radius: height / 2
                                color: "#1A1A3A"
                                border.color: "#E94560"

                                Text
                                {
                                    id: efText
                                    anchors.centerIn: parent
                                    text: modelData.name
                                    font.family: UISettings.robotoFontName
                                    font.pixelSize: UISettings.textSizeDefault * 0.78
                                    color: "#DDAACC"
                                }
                            }
                        }
                    }
                }
            }

            Item { Layout.fillHeight: true }
        }

        // Vertical divider
        Rectangle
        {
            Layout.fillHeight: true
            Layout.preferredWidth: 1
            color: "#2A2A44"
        }

        // ── Right: VC layout preview ─────────────────────────────────────────
        ColumnLayout
        {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.preferredWidth: 1   // equal split with the left column
            spacing: UISettings.listItemHeight * 0.4

            Text
            {
                text: qsTr("Virtual Console layout preview")
                font.family: UISettings.robotoFontName
                font.bold: true
                font.pixelSize: UISettings.textSizeDefault
                color: "#AAAACC"
            }

            // Schematic mockup of the actual VC layout: ONE multipage frame with
            // an "All Groups" page (index 0) plus one page per selected group.
            Rectangle
            {
                id: vcMock
                Layout.fillWidth: true
                Layout.fillHeight: true
                radius: 10
                color: "#080810"
                border.color: "#2A2A44"
                clip: true

                // Selected groups (page 1..N). Page 0 is the "All Groups" master.
                readonly property var groups: stageWizard ? stageWizard.fixtureRoleModel : []
                // Currently previewed page: 0 = All Groups, 1..N = a group.
                property int previewPage: 0
                readonly property var pageData: previewPage === 0
                    ? ({ name: qsTr("All Groups"),
                         hasRGB: true, hasDimmer: true,
                         hasMovement: groupsAnyMovement(), all: true })
                    : (groups[previewPage - 1] || ({}))

                function groupsAnyMovement()
                {
                    for (let i = 0; i < groups.length; ++i)
                        if (groups[i].hasMovement) return true
                    return false
                }

                Column
                {
                    anchors.fill: parent
                    anchors.margins: 10
                    spacing: 8

                    // ── Frame caption ────────────────────────────────────────
                    Text
                    {
                        text: qsTr("🎛  Show Wizard  ·  multipage frame")
                        font.family: UISettings.robotoFontName
                        font.pixelSize: UISettings.textSizeDefault * 0.78
                        font.bold: true
                        color: "#7777AA"
                    }

                    // ── Page-switch tabs (flash buttons): All Groups + each grp ─
                    Flow
                    {
                        width: parent.width
                        spacing: 6

                        VCPreviewButton
                        {
                            label: qsTr("All Groups")
                            highlighted: vcMock.previewPage === 0
                            MouseArea { anchors.fill: parent; onClicked: vcMock.previewPage = 0 }
                        }

                        Repeater
                        {
                            model: vcMock.groups
                            VCPreviewButton
                            {
                                label: modelData.name
                                highlighted: vcMock.previewPage === (index + 1)
                                MouseArea { anchors.fill: parent; onClicked: vcMock.previewPage = index + 1 }
                            }
                        }
                    }

                    // Separator
                    Rectangle { width: parent.width; height: 1; color: "#1A1A30" }

                    // ── Current page body ────────────────────────────────────
                    Column
                    {
                        width: parent.width
                        spacing: 8

                        Text
                        {
                            text: (vcMock.pageData.all ? "◉ " : "▸ ") +
                                  (vcMock.pageData.name || "")
                            font.family: UISettings.robotoFontName
                            font.pixelSize: UISettings.textSizeDefault * 0.8
                            font.bold: true
                            color: vcMock.pageData.all ? "#E9C046" : "#AAAACC"
                        }

                        // Intensity slider + XY pad
                        Row
                        {
                            spacing: 8
                            visible: vcMock.pageData.hasDimmer || vcMock.pageData.hasRGB

                            // Intensity slider (Level mode)
                            Rectangle
                            {
                                width: 16; height: 64
                                radius: 6; color: "#0A0A1A"
                                border.color: "#222244"
                                Rectangle
                                {
                                    anchors.bottom: parent.bottom
                                    anchors.horizontalCenter: parent.horizontalCenter
                                    width: 10; height: parent.height * 0.7
                                    radius: 4; color: "#E94560"
                                }
                            }

                            // XY pad (movers only)
                            Rectangle
                            {
                                visible: vcMock.pageData.hasMovement
                                width: 64; height: 64
                                radius: 4; color: "#0A0A1A"
                                border.color: "#222244"
                                Rectangle
                                {
                                    width: 10; height: 10
                                    radius: 5; color: "#E94560"
                                    x: 27; y: 27
                                }
                            }

                            // Color buttons (background = scene colour)
                            Grid
                            {
                                visible: vcMock.pageData.hasRGB
                                columns: 4
                                spacing: 4
                                Repeater
                                {
                                    model: ["#FF0000","#00FF00","#0000FF","#00FFFF",
                                            "#FF00FF","#FFFF00","#FFFFFF"]
                                    Rectangle
                                    {
                                        width: 14; height: 14
                                        radius: 3; color: modelData
                                        border.color: "#222244"
                                    }
                                }
                            }
                        }

                        // Effect / blinder buttons
                        Flow
                        {
                            width: parent.width
                            spacing: 4
                            Repeater
                            {
                                model: vcMock.pageData.hasMovement ? 5 : 2
                                Rectangle
                                {
                                    width: 30; height: 14
                                    radius: 3; color: "#1A1A2A"
                                    border.color: "#333355"
                                }
                            }
                        }
                    }

                    // Separator
                    Rectangle { width: parent.width; height: 1; color: "#1A1A30" }

                    // ── Shared show-cue row (present on every page) ───────────
                    Flow
                    {
                        width: parent.width
                        spacing: 6
                        Repeater
                        {
                            model: [qsTr("Blackout"), qsTr("Show Open"), qsTr("Big Moment"),
                                    qsTr("Show Close"), qsTr("Ambient"), qsTr("Blinder")]
                            VCPreviewButton { label: modelData }
                        }
                    }
                }
            }

            // Undoable note
            Rectangle
            {
                Layout.fillWidth: true
                Layout.preferredHeight: undoRow.implicitHeight + UISettings.listItemHeight * 0.5
                radius: 8
                color: "#0A1A0A"
                border.color: "#1A4A1A"

                RowLayout
                {
                    id: undoRow
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.leftMargin: 10
                    anchors.rightMargin: 10
                    spacing: 6
                    Text
                    {
                        Layout.alignment: Qt.AlignTop
                        text: "↩"
                        font.pixelSize: UISettings.textSizeDefault * 1.1
                        color: "#66AA66"
                    }
                    Text
                    {
                        Layout.fillWidth: true
                        text: qsTr("Everything the wizard creates is fully undoable with Ctrl+Z.")
                        font.family: UISettings.robotoFontName
                        font.pixelSize: UISettings.textSizeDefault * 0.85
                        color: "#66AA66"
                        wrapMode: Text.WordWrap
                    }
                }
            }
        }
    }
}
