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

    // ── Header ──────────────────────────────────────────────────────────────
    RowLayout
    {
        id: hdr
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right

        ColumnLayout
        {
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
                label: qsTr("Connect a MIDI, OSC, or DMX controller. This step is optional — you can skip it and assign controls later from the Virtual Console.")
                fontSize: UISettings.textSizeDefault * 0.9
                labelColor: "#888899"
                wrapText: true
            }
        }

        Item { Layout.fillWidth: true }

        // Skip hint
        Rectangle
        {
            height: UISettings.listItemHeight * 0.8
            width: skipText.implicitWidth + 20
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
    RowLayout
    {
        anchors.top: hdr.bottom
        anchors.topMargin: UISettings.listItemHeight * 0.5
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        spacing: UISettings.listItemHeight

        // ── Left: detected controllers ──────────────────────────────────────
        ColumnLayout
        {
            Layout.fillHeight: true
            width: parent.width * 0.38
            spacing: UISettings.listItemHeight * 0.4

            RobotoText
            {
                label: qsTr("Detected controllers")
                fontSize: UISettings.textSizeDefault * 1.0
                fontBold: true
                labelColor: "#AAAACC"
            }

            // Controller list from ioManager
            ListView
            {
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true
                spacing: 4
                model: ioManager ? ioManager.universeInputSources(0) : []

                delegate: Rectangle
                {
                    width: ListView.view.width
                    height: UISettings.listItemHeight * 1.1
                    radius: 8
                    color: "#0E0E20"

                    RowLayout
                    {
                        anchors.fill: parent
                        anchors.margins: 10
                        spacing: 8

                        Text { text: "🎛"; font.pixelSize: UISettings.textSizeDefault * 1.1 }

                        ColumnLayout
                        {
                            spacing: 2
                            RobotoText
                            {
                                label: modelData.pluginName || qsTr("Unknown")
                                fontBold: true
                                fontSize: UISettings.textSizeDefault
                                labelColor: "white"
                            }
                            RobotoText
                            {
                                label: modelData.lineName || ""
                                fontSize: UISettings.textSizeDefault * 0.82
                                labelColor: "#777799"
                            }
                        }
                    }
                }

                // Empty state
                RobotoText
                {
                    anchors.centerIn: parent
                    visible: parent.count === 0
                    label: qsTr("No controllers detected")
                    labelColor: "#444466"
                    fontSize: UISettings.textSizeDefault
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
                        label: qsTr("Open I/O panel to add a controller")
                        fontSize: UISettings.textSizeDefault * 0.9
                        labelColor: "#AAAACC"
                    }
                }
            }
        }

        // Vertical divider
        Rectangle
        {
            Layout.fillHeight: true
            width: 1
            color: "#2A2A44"
        }

        // ── Right: explanation / instructions ───────────────────────────────
        ColumnLayout
        {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: UISettings.listItemHeight * 0.6

            RobotoText
            {
                label: qsTr("How controller mapping works")
                fontSize: UISettings.textSizeDefault * 1.0
                fontBold: true
                labelColor: "#AAAACC"
            }

            Repeater
            {
                model: [
                    {
                        icon: "①",
                        title: qsTr("Generate the show"),
                        body: qsTr("Complete the wizard. The Virtual Console is built automatically with buttons, sliders, cue lists, and XY pads for each fixture group.")
                    },
                    {
                        icon: "②",
                        title: qsTr("Open the Virtual Console"),
                        body: qsTr("Switch to the Virtual Console view. Each widget has an 'External Controls' section in its properties panel.")
                    },
                    {
                        icon: "③",
                        title: qsTr("Learn mode"),
                        body: qsTr("Click 'Auto Detect' on any widget, then move a fader or press a button on your controller. QLC+ learns the mapping automatically.")
                    },
                    {
                        icon: "④",
                        title: qsTr("Profile library"),
                        body: qsTr("Many popular controllers (Behringer X-TOUCH, Akai APC, Novation Launch Control…) have pre-built profiles in the I/O panel.")
                    }
                ]

                Rectangle
                {
                    Layout.fillWidth: true
                    height: stepContent.implicitHeight + 20
                    radius: 8
                    color: "#0E0E20"

                    RowLayout
                    {
                        id: stepContent
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.margins: 12
                        spacing: 10

                        RobotoText
                        {
                            label: modelData.icon
                            fontSize: UISettings.textSizeDefault * 1.6
                            labelColor: "#E94560"
                            fontBold: true
                        }

                        ColumnLayout
                        {
                            Layout.fillWidth: true
                            spacing: 3

                            RobotoText
                            {
                                label: modelData.title
                                fontBold: true
                                fontSize: UISettings.textSizeDefault
                                labelColor: "white"
                            }
                            RobotoText
                            {
                                Layout.fillWidth: true
                                label: modelData.body
                                fontSize: UISettings.textSizeDefault * 0.85
                                labelColor: "#777799"
                                wrapText: true
                            }
                        }
                    }
                }
            }

            Item { Layout.fillHeight: true }
        }
    }
}
