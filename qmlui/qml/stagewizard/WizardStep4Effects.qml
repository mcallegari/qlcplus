/*
  Q Light Controller Plus
  WizardStep4Effects.qml

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
import QtQuick.Controls.Basic
import QtQuick.Layouts
import "."

Item
{
    id: root

    readonly property var familyIcons: ({
        "Color":     "🎨",
        "Intensity": "⚡",
        "Movement":  "🌀",
        "Matrix":    "▦",
        "Show Cues": "🎬"
    })

    readonly property var familyColors: ({
        "Color":     "#3A2A6A",
        "Intensity": "#4A2A2A",
        "Movement":  "#2A3A6A",
        "Matrix":    "#2A4A3A",
        "Show Cues": "#3A3A2A"
    })

    // Group effects model by family
    function groupedEffects()
    {
        if (!stageWizard) return []
        var model = stageWizard.effectsModel
        if (!model) return []
        var families = {}
        var order    = []
        for (var i = 0; i < model.length; ++i)
        {
            var e = model[i]
            if (!families[e.family])
            {
                families[e.family] = []
                order.push(e.family)
            }
            families[e.family].push(e)
        }
        var result = []
        for (var fi = 0; fi < order.length; ++fi)
            result.push({ family: order[fi], effects: families[order[fi]] })
        return result
    }

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
                label: qsTr("Effects & functions to generate")
                fontSize: UISettings.textSizeDefault * 1.3
                fontBold: true
                labelColor: "#DDDDEE"
            }
            RobotoText
            {
                label: qsTr("Pre-selected based on your show type. Greyed items need fixtures with the matching capability.")
                fontSize: UISettings.textSizeDefault * 0.9
                labelColor: "#888899"
                wrapText: true
            }
        }

        Item { Layout.fillWidth: true }

        // Quick summary count
        Rectangle
        {
            height: UISettings.listItemHeight * 0.9
            width: countText.implicitWidth + 20
            radius: height / 2
            color: "#E94560"

            RobotoText
            {
                id: countText
                anchors.centerIn: parent
                label: qsTr("%1 selected").arg(selectedCount())
                fontSize: UISettings.textSizeDefault * 0.9
                fontBold: true
                labelColor: "white"
            }

            function selectedCount()
            {
                if (!stageWizard) return 0
                var m = stageWizard.effectsModel
                if (!m) return 0
                var n = 0
                for (var i = 0; i < m.length; ++i)
                    if (m[i].enabled) n++
                return n
            }

            Connections
            {
                target: stageWizard
                function onEffectsModelChanged() { countText.parent.color = countText.parent.color }
            }
        }
    }

    // ── Grouped effect families ──────────────────────────────────────────────
    Flickable
    {
        anchors.top: hdr.bottom
        anchors.topMargin: UISettings.listItemHeight * 0.4
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        clip: true
        contentHeight: effectsColumn.implicitHeight
        boundsBehavior: Flickable.StopAtBounds
        ScrollBar.vertical: CustomScrollBar {}

        Column
        {
            id: effectsColumn
            width: parent.width
            spacing: UISettings.listItemHeight * 0.6

            Repeater
            {
                model: groupedEffects()

                Column
                {
                    width: parent.width
                    spacing: 0

                    // Family header
                    Rectangle
                    {
                        width: parent.width
                        height: UISettings.listItemHeight * 1.1
                        color: root.familyColors[modelData.family] || "#2A2A2A"
                        radius: 8

                        RowLayout
                        {
                            anchors.fill: parent
                            anchors.margins: 12
                            spacing: 8

                            Text
                            {
                                Layout.alignment: Qt.AlignVCenter
                                text: root.familyIcons[modelData.family] || "•"
                                font.family: UISettings.robotoFontName
                                font.pixelSize: UISettings.textSizeDefault * 1.3
                            }
                            Text
                            {
                                Layout.alignment: Qt.AlignVCenter
                                text: modelData.family
                                font.family: UISettings.robotoFontName
                                font.pixelSize: UISettings.textSizeDefault * 1.1
                                font.bold: true
                                color: "white"
                            }
                            Item { Layout.fillWidth: true }

                            // Select all / none for this family
                            Rectangle
                            {
                                Layout.alignment: Qt.AlignVCenter
                                height: UISettings.listItemHeight * 0.65
                                width: allText.implicitWidth + 14
                                radius: height / 2
                                color: "#00000033"
                                property bool famHover: false
                                MouseArea
                                {
                                    anchors.fill: parent
                                    hoverEnabled: true
                                    onEntered:  parent.famHover = true
                                    onExited:   parent.famHover = false
                                    onClicked:
                                    {
                                        var effects = modelData.effects
                                        var anyOff = effects.some(function(e) { return !e.enabled && e.available })
                                        for (var i = 0; i < effects.length; ++i)
                                            if (effects[i].available)
                                                stageWizard.setEffectEnabled(effects[i].flag, anyOff)
                                    }
                                }
                                Text
                                {
                                    id: allText
                                    anchors.centerIn: parent
                                    text: qsTr("All / None")
                                    font.family: UISettings.robotoFontName
                                    font.pixelSize: UISettings.textSizeDefault * 0.78
                                    color: parent.famHover ? "white" : "#AAAACC"
                                }
                            }
                        }
                    }

                    // Effect rows
                    Column
                    {
                        width: parent.width
                        spacing: 2
                        topPadding: 2

                        Repeater
                        {
                            model: modelData.effects

                            EffectToggleRow
                            {
                                width: parent.width
                                effectData: modelData
                            }
                        }
                    }
                }
            }
        }
    }
}
