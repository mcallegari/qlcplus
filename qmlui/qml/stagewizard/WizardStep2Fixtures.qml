/*
  Q Light Controller Plus
  WizardStep2Fixtures.qml

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
import org.qlcplus.classes 1.0
import "."

Item
{
    id: root

    readonly property var roleNames: [
        qsTr("Key Light"), qsTr("Fill Light"), qsTr("Back Light"),
        qsTr("Side Light"), qsTr("Effect"), qsTr("Strip / Bar"),
        qsTr("Blinder"), qsTr("Hazer"), qsTr("Floor")
    ]
    readonly property var roleColors: [
        "#0978FF", "#6CA0DC", "#7C5C9F", "#4CBBAA",
        "#E94560", "#5B9BD5", "#DDDDDD", "#AAAAAA", "#88CC66"
    ]
    readonly property var roleIcons: [
        "💡", "🔦", "🔙", "📐", "✨", "▬", "💥", "💨", "⬆"
    ]

    /* Called by FixtureDrag.js (WIZARD context) on drop. (sx, sy) are scene
       coordinates. Hit-tests the group boxes; if one is hit, patches the
       fixture and assigns it to that box. Returns true if handled. */
    function dropFixtureAt(sx, sy, manuf, model, mode, name,
                           uniIdx, address, channels, quantity, gap)
    {
        var p = groupBoxList.mapFromItem(null, sx, sy)
        if (p.x < 0 || p.y < 0 || p.x > groupBoxList.width || p.y > groupBoxList.height)
            return false

        var item = groupBoxList.itemAt(p.x + groupBoxList.contentX,
                                       p.y + groupBoxList.contentY)
        if (!item || item.groupIndex === undefined || item.groupIndex < 0 || !stageWizard)
            return false

        stageWizard.setPendingDropGroup(item.groupIndex)
        fixtureManager.addFixture(manuf, model, mode, name, uniIdx,
                                  address, channels, quantity, gap, 0, 0)
        stageWizard.setPendingDropGroup(-1)
        return true
    }

    RowLayout
    {
        anchors.fill: parent
        spacing: UISettings.listItemHeight * 0.4

        // ── COL 1: Fixture browser ─────────────────────────────────────────
        Rectangle
        {
            Layout.fillHeight: true
            Layout.preferredWidth: parent.width * 0.28
            color: UISettings.bgStrong
            radius: 6
            clip: true

            ColumnLayout
            {
                anchors.fill: parent
                spacing: 0

                Rectangle
                {
                    Layout.fillWidth: true
                    height: UISettings.listItemHeight * 1.1
                    color: "#1A1A3A"
                    radius: 6

                    Text
                    {
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.left: parent.left
                        anchors.leftMargin: 10
                        text: qsTr("Fixture Browser")
                        font.family: UISettings.robotoFontName
                        font.pixelSize: UISettings.textSizeDefault
                        font.bold: true
                        color: "#AAAACC"
                    }
                }

                Text
                {
                    Layout.fillWidth: true
                    Layout.margins: 6
                    text: qsTr("Drag a fixture onto a group box on the right to patch and assign it.")
                    font.family: UISettings.robotoFontName
                    font.pixelSize: UISettings.textSizeDefault * 0.78
                    color: "#777799"
                    wrapMode: Text.WordWrap
                }

                Loader
                {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    source: "qrc:/FixtureBrowser.qml"
                }
            }
        }

        // ── COL 2: Group boxes (drag/drop targets) ─────────────────────────
        Rectangle
        {
            Layout.fillHeight: true
            Layout.preferredWidth: parent.width * 0.36
            color: UISettings.bgStrong
            radius: 6
            clip: true

            ColumnLayout
            {
                anchors.fill: parent
                spacing: 0

                Rectangle
                {
                    Layout.fillWidth: true
                    height: UISettings.listItemHeight * 1.1
                    color: "#1A1A3A"
                    radius: 6

                    RowLayout
                    {
                        anchors.fill: parent
                        anchors.leftMargin: 10
                        anchors.rightMargin: 0

                        Text
                        {
                            Layout.fillWidth: true
                            text: qsTr("Fixture Groups")
                            font.family: UISettings.robotoFontName
                            font.pixelSize: UISettings.textSizeDefault
                            font.bold: true
                            color: "#AAAACC"
                            verticalAlignment: Text.AlignVCenter
                        }

                        GenericButton
                        {
                            label: qsTr("+ Add group")
                            height: UISettings.listItemHeight
                            width: UISettings.bigItemHeight * 1.5
                            bgColor: "#0550AA"
                            hoverColor: "#0978FF"
                            fgColor: "white"
                            onClicked: { if (stageWizard) stageWizard.addGroup() }
                        }
                    }
                }

                ListView
                {
                    id: groupBoxList
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    spacing: UISettings.listItemHeight * 0.25
                    topMargin: UISettings.listItemHeight * 0.25
                    bottomMargin: UISettings.listItemHeight * 0.25
                    leftMargin: UISettings.listItemHeight * 0.25
                    // Reserve room on the right only while the scrollbar is shown
                    readonly property bool scrollShown: contentHeight > height
                    rightMargin: UISettings.listItemHeight * 0.25
                                 + (scrollShown ? UISettings.scrollBarWidth : 0)
                    boundsBehavior: Flickable.StopAtBounds
                    model: stageWizard ? stageWizard.groupsModel : []
                    ScrollBar.vertical: CustomScrollBar {}

                    delegate: FixtureGroupBox
                    {
                        width: groupBoxList.width - groupBoxList.leftMargin - groupBoxList.rightMargin
                        groupData: modelData
                    }

                    // Empty state
                    Item
                    {
                        anchors.fill: parent
                        visible: groupBoxList.count === 0

                        Column
                        {
                            anchors.centerIn: parent
                            spacing: UISettings.listItemHeight * 0.4
                            width: parent.width * 0.8

                            Text
                            {
                                anchors.horizontalCenter: parent.horizontalCenter
                                text: "📦"
                                font.pixelSize: UISettings.bigItemHeight * 0.4
                                color: "#333355"
                            }
                            Text
                            {
                                width: parent.width
                                text: qsTr("Add a group, then drag fixtures from the browser into it.")
                                font.family: UISettings.robotoFontName
                                font.pixelSize: UISettings.textSizeDefault
                                color: "#555577"
                                horizontalAlignment: Text.AlignHCenter
                                wrapMode: Text.WordWrap
                            }
                        }
                    }
                }
            }
        }

        // ── COL 3: Detected capabilities & roles (selected boxes) ──────────
        Rectangle
        {
            Layout.fillHeight: true
            Layout.fillWidth: true
            color: UISettings.bgStrong
            radius: 6
            clip: true

            ColumnLayout
            {
                anchors.fill: parent
                spacing: 0

                Rectangle
                {
                    Layout.fillWidth: true
                    height: UISettings.listItemHeight * 1.1
                    color: "#1A1A3A"
                    radius: 6

                    Text
                    {
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.left: parent.left
                        anchors.leftMargin: 10
                        text: qsTr("Detected capabilities & roles")
                        font.family: UISettings.robotoFontName
                        font.pixelSize: UISettings.textSizeDefault
                        font.bold: true
                        color: "#AAAACC"
                    }
                }

                Item
                {
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    readonly property bool hasRoles: stageWizard &&
                                                     stageWizard.fixtureRoleModel &&
                                                     stageWizard.fixtureRoleModel.length > 0

                    ListView
                    {
                        id: roleList
                        anchors.fill: parent
                        visible: parent.hasRoles
                        clip: true
                        spacing: UISettings.listItemHeight * 0.2
                        topMargin: UISettings.listItemHeight * 0.25
                        model: stageWizard ? stageWizard.fixtureRoleModel : []
                        ScrollBar.vertical: CustomScrollBar {}

                        delegate: FixtureGroupRoleDelegate
                        {
                            width: roleList.width
                            groupData: modelData
                            groupIndex: modelData.index !== undefined ? modelData.index : index
                            roleNames: root.roleNames
                            roleColors: root.roleColors
                            roleIcons: root.roleIcons
                        }
                    }

                    Column
                    {
                        anchors.centerIn: parent
                        visible: !parent.hasRoles
                        spacing: UISettings.listItemHeight * 0.4
                        width: parent.width * 0.8

                        Text
                        {
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: "☑"
                            font.pixelSize: UISettings.bigItemHeight * 0.4
                            color: "#333355"
                        }

                        Text
                        {
                            width: parent.width
                            text: qsTr("Tick a group box to detect its capabilities here.")
                            font.family: UISettings.robotoFontName
                            font.pixelSize: UISettings.textSizeDefault
                            color: "#555577"
                            horizontalAlignment: Text.AlignHCenter
                            wrapMode: Text.WordWrap
                        }
                    }
                }
            }
        }
    }
}
