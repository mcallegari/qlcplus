/*
  Q Light Controller Plus
  FixtureGroupRoleDelegate.qml

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
import "."

Rectangle
{
    id: delegate

    property var    groupData:  ({})
    property int    groupIndex: 0
    property var    roleNames:  []
    property var    roleColors: []
    property var    roleIcons:  []

    readonly property int    currentRole: groupData.role !== undefined ? groupData.role : 0
    readonly property string roleColor:   roleColors[currentRole] || "#888888"
    readonly property real   hpad: UISettings.listItemHeight * 0.4
    readonly property real   vpad: UISettings.listItemHeight * 0.3
    readonly property real   innerWidth: width - hpad * 2

    // Height is driven entirely by the Column child
    height: innerCol.implicitHeight + vpad * 2

    radius: 8
    color: "#141428"
    border.color: Qt.rgba(
        parseInt(roleColor.slice(1,3), 16) / 255,
        parseInt(roleColor.slice(3,5), 16) / 255,
        parseInt(roleColor.slice(5,7), 16) / 255,
        0.45)
    border.width: 1

    Behavior on border.color { ColorAnimation { duration: 200 } }

    // ── Inner Column — drives the height ─────────────────────────────────────
    Column
    {
        id: innerCol
        x: delegate.hpad
        y: delegate.vpad
        width: delegate.innerWidth
        spacing: delegate.vpad * 0.6

        // ── Top info row: accent · icon · name · count · caps ───────────────
        Row
        {
            id: topRow
            width: parent.width
            spacing: UISettings.listItemHeight * 0.3

            Rectangle
            {
                width: 4
                height: nameLabel.height
                radius: 2
                color: delegate.roleColor
                anchors.verticalCenter: parent.verticalCenter
                Behavior on color { ColorAnimation { duration: 200 } }
            }

            Text
            {
                text: delegate.roleIcons[delegate.currentRole] || "?"
                font.family: UISettings.robotoFontName
                font.pixelSize: UISettings.textSizeDefault * 1.1
                color: "white"
                anchors.verticalCenter: parent.verticalCenter
            }

            Text
            {
                id: nameLabel
                text: groupData.name || ""
                font.family: UISettings.robotoFontName
                font.pixelSize: UISettings.textSizeDefault
                font.bold: true
                color: "white"
                anchors.verticalCenter: parent.verticalCenter
            }

            Text
            {
                text: qsTr("(%1)").arg(groupData.fixtureCount || 0)
                font.family: UISettings.robotoFontName
                font.pixelSize: UISettings.textSizeDefault * 0.82
                color: "#666688"
                anchors.verticalCenter: parent.verticalCenter
            }

            // Capability tags
            Repeater
            {
                model: buildCapTags()

                Rectangle
                {
                    height: capLbl.implicitHeight + 6
                    width: capLbl.implicitWidth + 12
                    radius: height / 2
                    color: modelData.color
                    anchors.verticalCenter: parent.verticalCenter

                    Text
                    {
                        id: capLbl
                        anchors.centerIn: parent
                        text: modelData.label
                        font.family: UISettings.robotoFontName
                        font.pixelSize: UISettings.textSizeDefault * 0.75
                        color: "white"
                    }
                }
            }
        }

        // ── Role pills — Flow wraps to multiple rows when narrow ────────────
        Flow
        {
            id: pillsFlow
            width: parent.width
            spacing: 4

            Repeater
            {
                model: delegate.roleNames

                Rectangle
                {
                    id: pill
                    height: pillLabel.implicitHeight + 8
                    width: pillLabel.implicitWidth + 16
                    radius: height / 2
                    property bool isThis: delegate.currentRole === index
                    color: isThis ? delegate.roleColors[index] : "#222240"
                    border.color: isThis ? "transparent" : "#333355"
                    border.width: 1

                    Behavior on color { ColorAnimation { duration: 150 } }

                    Text
                    {
                        id: pillLabel
                        anchors.centerIn: parent
                        text: delegate.roleIcons[index] + " " + modelData
                        font.family: UISettings.robotoFontName
                        font.pixelSize: UISettings.textSizeDefault * 0.78
                        font.bold: pill.isThis
                        color: pill.isThis ? "white" : "#9999BB"
                    }

                    MouseArea
                    {
                        anchors.fill: parent
                        onClicked:
                        {
                            if (stageWizard)
                                stageWizard.setGroupRole(delegate.groupIndex, index)
                        }
                    }
                }
            }
        }
    }

    function buildCapTags()
    {
        var tags = []
        if (groupData.hasMovement) tags.push({ label: qsTr("Pan/Tilt"), color: "#3A3A6A" })
        if (groupData.hasRGB)      tags.push({ label: qsTr("RGB"),      color: "#3A2A6A" })
        if (groupData.hasGobo)     tags.push({ label: qsTr("Gobo"),     color: "#2A4A3A" })
        if (groupData.hasShutter)  tags.push({ label: qsTr("Shutter"),  color: "#4A3A2A" })
        if (groupData.hasDimmer)   tags.push({ label: qsTr("Dimmer"),   color: "#3A3A3A" })
        return tags
    }
}
