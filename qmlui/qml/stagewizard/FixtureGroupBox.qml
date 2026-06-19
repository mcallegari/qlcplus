/*
  Q Light Controller Plus
  FixtureGroupBox.qml

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
import "."

Rectangle
{
    id: box

    property var  groupData: ({})
    property bool dropHover: false

    readonly property int     groupIndex: groupData.index !== undefined ? groupData.index : -1
    readonly property string  groupName:  groupData.name || ""
    readonly property int     fixtureCount: groupData.fixtureCount || 0
    readonly property bool    selected:   groupData.selected === true

    readonly property real pad: UISettings.listItemHeight * 0.35
    readonly property real rowH: UISettings.listItemHeight * 1.1

    height: contentCol.height + pad * 2
    radius: 8
    color: dropHover ? "#13203A" : (selected ? "#161636" : "#101024")
    border.color: dropHover ? "#0978FF" : (selected ? "#0978FF" : "#2A2A44")
    border.width: dropHover || selected ? 2 : 1

    Behavior on border.color { ColorAnimation { duration: 120 } }
    Behavior on color        { ColorAnimation { duration: 120 } }

    Column
    {
        id: contentCol
        x: box.pad
        y: box.pad
        width: box.width - box.pad * 2
        spacing: box.pad * 0.7

        // ── Header: editable name · count · checkbox ────────────────────────
        Item
        {
            id: headerItem
            width: parent.width
            height: Math.max(nameField.height, checkBox.height)

            TextField
            {
                id: nameField
                anchors.left: parent.left
                anchors.right: checkBox.left
                anchors.rightMargin: 8
                anchors.verticalCenter: parent.verticalCenter
                text: box.groupName
                color: "white"
                font.family: UISettings.robotoFontName
                font.pixelSize: UISettings.textSizeDefault
                font.bold: true
                selectByMouse: true
                padding: 4
                background: Rectangle
                {
                    color: nameField.activeFocus ? "#0A0A18" : "transparent"
                    border.color: nameField.activeFocus ? "#0978FF" : "transparent"
                    border.width: 1
                    radius: 4
                }
                onEditingFinished:
                {
                    if (stageWizard && text.length > 0)
                        stageWizard.renameGroup(box.groupIndex, text)
                }
            }

            // Checkbox in the upper-right corner
            Rectangle
            {
                id: checkBox
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                width: UISettings.listItemHeight * 0.75
                height: width
                radius: 4
                color: box.selected ? "#0978FF" : "#1A1A2E"
                border.color: box.selected ? "#0978FF" : "#444466"
                border.width: 2

                Behavior on color { ColorAnimation { duration: 120 } }

                Text
                {
                    anchors.centerIn: parent
                    visible: box.selected
                    text: "✓"
                    font.family: UISettings.robotoFontName
                    font.pixelSize: parent.height * 0.7
                    font.bold: true
                    color: "white"
                }

                MouseArea
                {
                    anchors.fill: parent
                    onClicked:
                    {
                        if (stageWizard)
                            stageWizard.setGroupSelected(box.groupIndex, !box.selected)
                    }
                }
            }
        }

        // ── Member fixtures (full-width rows) ───────────────────────────────
        ListView
        {
            id: fixturesView
            width: parent.width
            visible: box.fixtures.length > 0
            interactive: false
            spacing: 3
            model: box.fixtures
            height: count * box.rowH + Math.max(count - 1, 0) * spacing

            delegate: Rectangle
            {
                width: fixturesView.width
                height: box.rowH
                radius: 4
                color: "#1A1A33"
                border.color: "#2E2E50"
                border.width: 1

                required property var modelData

                // Right-side cluster: address + remove button (intrinsic width)
                Row
                {
                    id: rightGroup
                    anchors.right: parent.right
                    anchors.rightMargin: 8
                    anchors.verticalCenter: parent.verticalCenter
                    spacing: 8

                    Text
                    {
                        anchors.verticalCenter: parent.verticalCenter
                        text: qsTr("U%1 · A%2").arg(modelData.universe).arg(modelData.address)
                        font.family: UISettings.robotoFontName
                        font.pixelSize: UISettings.textSizeDefault * 0.78
                        color: "#8888AA"
                    }

                    Text
                    {
                        id: removeFxBtn
                        anchors.verticalCenter: parent.verticalCenter
                        text: "✕"
                        font.family: UISettings.robotoFontName
                        font.pixelSize: UISettings.textSizeDefault * 0.85
                        color: removeFxMa.containsMouse ? "#E94560" : "#555577"

                        MouseArea
                        {
                            id: removeFxMa
                            anchors.fill: parent
                            anchors.margins: -4
                            hoverEnabled: true
                            onClicked:
                            {
                                if (stageWizard)
                                    stageWizard.removeFixtureFromGroup(box.groupIndex, modelData.id)
                            }
                        }
                    }
                }

                // Name fills the space left of the right cluster
                Text
                {
                    anchors.left: parent.left
                    anchors.leftMargin: 8
                    anchors.right: rightGroup.left
                    anchors.rightMargin: 8
                    anchors.verticalCenter: parent.verticalCenter
                    text: modelData.name
                    font.family: UISettings.robotoFontName
                    font.pixelSize: UISettings.textSizeDefault * 0.85
                    color: "white"
                    elide: Text.ElideRight
                }
            }
        }

        Text
        {
            id: emptyHint
            width: parent.width
            visible: box.fixtures.length === 0
            text: qsTr("Drag fixtures here")
            font.family: UISettings.robotoFontName
            font.pixelSize: UISettings.textSizeDefault * 0.78
            font.italic: true
            color: "#555577"
        }

        // ── Footer: remove group ────────────────────────────────────────────
        Item
        {
            id: footerItem
            width: parent.width
            height: removeText.implicitHeight

            Text
            {
                id: removeText
                anchors.right: parent.right
                text: qsTr("✕ Remove group")
                font.family: UISettings.robotoFontName
                font.pixelSize: UISettings.textSizeDefault * 0.78
                color: removeMa.containsMouse ? "#E94560" : "#666688"

                MouseArea
                {
                    id: removeMa
                    anchors.fill: parent
                    hoverEnabled: true
                    onClicked: { if (stageWizard) stageWizard.removeGroup(box.groupIndex) }
                }
            }
        }
    }

    // Member fixtures (refreshed whenever the model changes)
    property var fixtures: refreshFixtures()

    function refreshFixtures()
    {
        if (stageWizard && box.groupIndex >= 0)
            return stageWizard.groupFixtures(box.groupIndex)
        return []
    }

    Connections
    {
        target: stageWizard
        function onGroupsModelChanged() { box.fixtures = box.refreshFixtures() }
    }
}
