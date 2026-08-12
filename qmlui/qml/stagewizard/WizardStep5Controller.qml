/*
  Q Light Controller Plus
  WizardStep5Controller.qml

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

    // Re-scan whenever this step becomes visible, so a controller patched in
    // the I/O panel while the wizard is open shows up on return.
    onVisibleChanged: if (visible && stageWizard) stageWizard.refreshControllers()

    // ── Header ──────────────────────────────────────────────────────────────
    RowLayout
    {
        id: hdr
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right

        ColumnLayout
        {
            Layout.fillWidth: true   // take the remaining width so the long
                                     // description wraps instead of pushing the
                                     // "Optional" pill off-screen
            spacing: 4
            RobotoText
            {
                label: qsTr("External controller mapping")
                fontSize: UISettings.textSizeDefault * 1.3
                fontBold: true
                labelColor: "#DDDDEE"
            }
            RobotoText
            {
                Layout.fillWidth: true
                label: qsTr("Pick a patched MIDI, OSC or DMX controller and the wizard will bind it to the Virtual Console it generates. This step is optional — you can skip it and assign controls later.")
                fontSize: UISettings.textSizeDefault * 0.9
                labelColor: "#888899"
                wrapText: true
            }
        }

        // Skip hint
        Rectangle
        {
            Layout.alignment: Qt.AlignVCenter
            height: UISettings.listItemHeight * 0.8
            // RobotoText reports its size via width (not implicitWidth).
            width: skipText.width + 20
            radius: height / 2
            color: "#1A2A1A"
            border.color: "#2A4A2A"

            RobotoText
            {
                id: skipText
                anchors.centerIn: parent
                label: qsTr("✓  Optional step — safe to skip")
                fontSize: UISettings.textSizeDefault * 0.85
                labelColor: "#66AA66"
            }
        }
    }

    // ── Content ──────────────────────────────────────────────────────────────
    // Two equal columns: both fillWidth with the same preferredWidth, so the
    // RowLayout splits the space 50/50 regardless of what either side contains.
    RowLayout
    {
        id: content
        anchors.top: hdr.bottom
        anchors.topMargin: UISettings.listItemHeight * 0.5
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        spacing: UISettings.listItemHeight * 0.75

        // ── Left: patched controllers ───────────────────────────────────────
        ColumnLayout
        {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.preferredWidth: 1   // equal weight with the right column
            spacing: UISettings.listItemHeight * 0.4

            RobotoText
            {
                label: qsTr("Connected controllers")
                fontSize: UISettings.textSizeDefault * 1.0
                fontBold: true
                labelColor: "#AAAACC"
            }

            // Universes that carry an input patch (the Loopback universes the
            // wizard uses for page switching are filtered out in C++).
            ListView
            {
                id: ctrlList
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true
                spacing: 4
                model: stageWizard ? stageWizard.controllersModel : []

                delegate: Rectangle
                {
                    id: ctrlDelegate
                    width: ctrlList.width
                    // Height follows the row's own implicit height. The row is
                    // anchored left/right/top only — anchoring it to fill the
                    // delegate would make its height depend on the delegate's,
                    // which depends on the row's: a binding loop that collapses
                    // the list.
                    height: ctrlRow.implicitHeight + 18
                    radius: 8
                    color: selected ? "#161636" : "#0E0E20"
                    border.width: selected ? 2 : 1
                    border.color: selected ? "#E94560" : "#22224A"

                    property bool selected: stageWizard &&
                                            stageWizard.controllerUniverse === modelData.universe

                    MouseArea
                    {
                        anchors.fill: parent
                        onClicked:
                        {
                            // Clicking the selected entry deselects it, so the
                            // user can opt out without leaving the step.
                            stageWizard.controllerUniverse =
                                ctrlDelegate.selected ? -1 : modelData.universe
                        }
                    }

                    RowLayout
                    {
                        id: ctrlRow
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.top: parent.top
                        anchors.margins: 9
                        spacing: 8

                        Text
                        {
                            text: "🎛"
                            font.pixelSize: UISettings.textSizeDefault * 1.3
                            Layout.alignment: Qt.AlignTop
                        }

                        ColumnLayout
                        {
                            id: ctrlCol
                            Layout.fillWidth: true
                            spacing: 3

                            RobotoText
                            {
                                label: modelData.lineName || modelData.plugin
                                fontBold: true
                                fontSize: UISettings.textSizeDefault
                                labelColor: "white"
                            }
                            RobotoText
                            {
                                label: qsTr("%1 · Universe %2")
                                       .arg(modelData.plugin)
                                       .arg(modelData.universe + 1)
                                fontSize: UISettings.textSizeDefault * 0.82
                                labelColor: "#777799"
                            }

                            // Capability pills: profile, channel counts, feedback
                            Flow
                            {
                                Layout.fillWidth: true
                                spacing: 4

                                Repeater
                                {
                                    model:
                                    {
                                        var pills = []
                                        if (modelData.hasProfile)
                                            pills.push({ t: modelData.profile, c: "#2A4A2A", f: "#77BB77" })
                                        else
                                            pills.push({ t: qsTr("No input profile"), c: "#4A3A1A", f: "#CCAA55" })

                                        pills.push({ t: qsTr("%1 buttons").arg(modelData.buttons),
                                                     c: "#1A2A4A", f: "#7799CC" })
                                        pills.push({ t: qsTr("%1 faders").arg(modelData.faders),
                                                     c: "#1A2A4A", f: "#7799CC" })
                                        if (modelData.hasColorTable)
                                            pills.push({ t: qsTr("colour LEDs"), c: "#3A1A3A", f: "#CC77CC" })
                                        if (modelData.feedback)
                                            pills.push({ t: qsTr("feedback on"), c: "#2A4A2A", f: "#77BB77" })
                                        return pills
                                    }

                                    Rectangle
                                    {
                                        height: UISettings.listItemHeight * 0.62
                                        width: pillText.width + 12
                                        radius: 4
                                        color: modelData.c

                                        RobotoText
                                        {
                                            id: pillText
                                            anchors.centerIn: parent
                                            label: modelData.t
                                            fontSize: UISettings.textSizeDefault * 0.75
                                            labelColor: modelData.f
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                // Empty state
                ColumnLayout
                {
                    anchors.centerIn: parent
                    width: parent.width * 0.8
                    visible: ctrlList.count === 0
                    spacing: 6

                    RobotoText
                    {
                        Layout.alignment: Qt.AlignHCenter
                        label: qsTr("No controller patched")
                        labelColor: "#666688"
                        fontSize: UISettings.textSizeDefault
                        fontBold: true
                    }
                    RobotoText
                    {
                        Layout.fillWidth: true
                        label: qsTr("Patch your controller's input line to a universe in the I/O panel, then come back here.")
                        labelColor: "#444466"
                        fontSize: UISettings.textSizeDefault * 0.85
                        wrapText: true
                        textHAlign: Text.AlignHCenter
                    }
                }

                ScrollBar.vertical: CustomScrollBar {}
            }

            // Open I/O panel button
            Rectangle
            {
                Layout.fillWidth: true
                height: UISettings.listItemHeight
                radius: 6
                color: ioHover ? "#222255" : "#1A1A44"
                border.color: "#333366"
                property bool ioHover: false

                MouseArea
                {
                    anchors.fill: parent
                    hoverEnabled: true
                    onEntered: parent.ioHover = true
                    onExited:  parent.ioHover = false
                    onClicked:
                    {
                        // Navigate to I/O panel to patch controller
                        if (contextManager)
                            contextManager.switchToContext("IOMGR")
                    }
                }

                RowLayout
                {
                    anchors.centerIn: parent
                    spacing: 6
                    Text { text: "⚙"; font.pixelSize: UISettings.textSizeDefault * 1.1 }
                    RobotoText
                    {
                        label: qsTr("Open I/O panel to patch a controller")
                        fontSize: UISettings.textSizeDefault * 0.9
                        labelColor: "#AAAACC"
                    }
                }
            }
        }

        // Vertical divider. A plain width in a RowLayout is advisory, so the
        // layout is told the exact width instead.
        Rectangle
        {
            Layout.fillHeight: true
            Layout.preferredWidth: 1
            color: "#2A2A44"
        }

        // ── Right: mapping options ──────────────────────────────────────────
        ColumnLayout
        {
            id: optionsCol
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.preferredWidth: 1   // equal weight with the left column
            Layout.alignment: Qt.AlignTop
            spacing: UISettings.listItemHeight * 0.5

            // Everything on this side needs a selected controller to mean
            // anything, so it's dimmed and inert until one is picked.
            property bool hasCtrl: stageWizard && stageWizard.controllerUniverse >= 0

            RobotoText
            {
                label: qsTr("Mapping options")
                fontSize: UISettings.textSizeDefault * 1.0
                fontBold: true
                labelColor: "#AAAACC"
            }

            WizardOptionRow
            {
                id: mapRow
                Layout.fillWidth: true
                rowEnabled: optionsCol.hasCtrl
                checked: stageWizard ? stageWizard.mapController : true
                title: qsTr("Auto-map Virtual Console controls")
                body: qsTr("Bind the generated buttons, faders and XY pads to the controller's channels. Buttons on the controller drive buttons in the VC; faders and encoders drive the intensity sliders and pan/tilt.")
                onToggled: function(value) { stageWizard.mapController = value }
            }

            WizardOptionRow
            {
                Layout.fillWidth: true
                rowEnabled: optionsCol.hasCtrl && mapRow.checked
                checked: stageWizard ? stageWizard.sendFeedback : true
                title: qsTr("Send feedback to the controller")
                body: qsTr("Patch the controller's output line so QLC+ lights its LEDs and moves its motorised faders to match the Virtual Console state.")
                onToggled: function(value) { stageWizard.sendFeedback = value }
            }

            WizardOptionRow
            {
                Layout.fillWidth: true
                rowEnabled: optionsCol.hasCtrl && mapRow.checked
                checked: stageWizard ? stageWizard.mapColors : true
                title: qsTr("Match LED colours to button colours")
                body: qsTr("For controllers whose input profile has a colour table, each colour button lights its pad in the nearest matching colour. Ignored on controllers without colour LEDs.")
                onToggled: function(value) { stageWizard.mapColors = value }
            }

            // Live estimate of what the mapping will consume.
            Rectangle
            {
                id: previewBox
                Layout.fillWidth: true
                // In a ColumnLayout a plain height is only advisory — the
                // layout sizes children from their implicit height.
                implicitHeight: previewCol.implicitHeight + 20
                radius: 8
                color: "#12121F"
                border.color: "#2A2A44"
                visible: optionsCol.hasCtrl && mapRow.checked

                // controllerMappingPreview() is a plain invokable, so it has no
                // binding dependencies — refresh it explicitly whenever the
                // selection or the toggles change.
                property string previewText: ""

                function refresh()
                {
                    previewText = (stageWizard && stageWizard.controllerUniverse >= 0)
                                  ? stageWizard.controllerMappingPreview() : ""
                }

                Component.onCompleted: refresh()
                onVisibleChanged: if (visible) refresh()

                Connections
                {
                    target: stageWizard
                    function onControllerChanged() { previewBox.refresh() }
                }

                ColumnLayout
                {
                    id: previewCol
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: parent.top
                    anchors.topMargin: 10
                    anchors.leftMargin: 12
                    anchors.rightMargin: 12
                    spacing: 3

                    RobotoText
                    {
                        label: qsTr("Estimated usage")
                        fontBold: true
                        fontSize: UISettings.textSizeDefault * 0.9
                        labelColor: "#AAAACC"
                    }
                    RobotoText
                    {
                        Layout.fillWidth: true
                        label: previewBox.previewText
                        fontSize: UISettings.textSizeDefault * 0.85
                        labelColor: "#888899"
                        wrapText: true
                    }
                }
            }

            // Hint shown until a controller is picked.
            RobotoText
            {
                Layout.fillWidth: true
                visible: !optionsCol.hasCtrl
                label: qsTr("Select a controller on the left to enable mapping, or continue without one — you can always use 'Auto Detect' on any Virtual Console widget later.")
                fontSize: UISettings.textSizeDefault * 0.85
                labelColor: "#666688"
                wrapText: true
            }

            Item { Layout.fillHeight: true }
        }
    }
}
