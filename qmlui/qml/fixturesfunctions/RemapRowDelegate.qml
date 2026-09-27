/*
  Q Light Controller Plus
  RemapRowDelegate.qml

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
    id: rowRoot

    readonly property real rowH: UISettings.listItemHeight
    height: rowH
    clip: true   // needed so height:0 collapsed rows show nothing

    /** Data map: isHeader, fxId, name, address, chIdx, chIcon. */
    property var rowData: ({})

    /** Whether this delegate lives in the target (right) list. */
    property bool isTarget: false

    /** Whether this channel is the pending source for click-to-connect. */
    property bool isPending: false

    /** Whether the fixture is currently expanded (only meaningful for header rows). */
    property bool isExpanded: false

    /** Whether this fixture header is selected as pending remap source. */
    property bool isSelectedFixture: false

    /** Emitted when the user clicks the fixture name row (fixture-level selection). */
    signal headerClicked(var fxId)

    /** Emitted when the user clicks the chevron arrow (expand/collapse). */
    signal headerExpandToggled(var fxId)

    /** True when at least one connection exists for this fixture (header rows only). */
    property bool hasConnections: false

    /** True when this specific channel has an active connection (channel rows only). */
    property bool isChannelConnected: false

    signal channelClicked(var fxId, int chIdx)
    signal disconnectClicked(var fxId)
    signal disconnectChannelClicked(var fxId, int chIdx)
    signal removeClicked(var fxId)

    // ---- Universe separator row ----
    Rectangle
    {
        visible: rowData.isUniverse === true
        anchors.fill: parent
        color: "darkcyan"

        RobotoText
        {
            anchors.centerIn: parent
            label: rowData.name || ""
            fontSize: UISettings.textSizeDefault
            fontBold: true
            labelColor: UISettings.fgMain
        }
    }

    // ---- Fixture header row ----
    Rectangle
    {
        id: headerRow
        visible: rowData.isHeader === true
        anchors.fill: parent
        color: isSelectedFixture ? UISettings.highlight
                                 : (headerNameMouse.containsMouse ? UISettings.hover
                                                                  : UISettings.sectionHeaderDiv)

        // Background MouseArea declared FIRST so it is BELOW the RowLayout in z-order.
        // Child MouseAreas (chevron, IconButtons) intercept their own clicks and do not
        // propagate, so this area only fires when clicking the name/address text.
        MouseArea
        {
            id: headerNameMouse
            anchors.fill: parent
            hoverEnabled: true
            onClicked: rowRoot.headerClicked(rowData.fxId)
        }

        RowLayout
        {
            anchors.fill: parent
            anchors.leftMargin: 2
            anchors.rightMargin: 4
            spacing: 2

            // Chevron: click toggles expand/collapse
            Text
            {
                Layout.alignment: Qt.AlignVCenter
                color: UISettings.fgMedium
                font.family: UISettings.fontAwesomeFontName
                font.pixelSize: rowRoot.rowH
                text: isExpanded ? FontAwesome.fa_chevron_down : FontAwesome.fa_chevron_right

                MouseArea
                {
                    anchors.fill: parent
                    anchors.margins: -4   // slightly larger hit area
                    onClicked: rowRoot.headerExpandToggled(rowData.fxId)
                }
            }

            RobotoText
            {
                Layout.fillWidth: true
                implicitHeight: UISettings.listItemHeight
                label: rowData.name || ""
                fontSize: UISettings.textSizeDefault
                fontBold: true
                labelColor: isSelectedFixture ? UISettings.bgStrong : UISettings.fgMain
            }

            RobotoText
            {
                visible: (rowData.address || "").length > 0
                Layout.alignment: Qt.AlignVCenter
                implicitHeight: UISettings.listItemHeight
                label: rowData.address || ""
                fontSize: UISettings.textSizeDefault
                labelColor: isSelectedFixture ? UISettings.bgStrong : UISettings.fgMedium
            }

            // Target only: disconnect all connections for this fixture
            IconButton
            {
                visible: isTarget && hasConnections
                Layout.alignment: Qt.AlignVCenter
                width: height
                height: rowRoot.rowH - 4
                faSource: FontAwesome.fa_link_slash
                faColor: "yellow"
                tooltip: qsTr("Remove all connections for this fixture")
                onClicked: rowRoot.disconnectClicked(rowData.fxId)
            }

            // Target only: remove fixture
            IconButton
            {
                visible: isTarget
                Layout.alignment: Qt.AlignVCenter
                width: height
                height: rowRoot.rowH - 4
                faSource: FontAwesome.fa_trash_can
                faColor: "darkred"
                tooltip: qsTr("Remove this target fixture")
                onClicked: rowRoot.removeClicked(rowData.fxId)
            }
        }
    }

    // ---- Channel row ----
    Rectangle
    {
        visible: rowData.isHeader === false && rowData.isUniverse === false
        anchors.fill: parent
        color: isPending ? UISettings.highlight
                         : (chMouseArea.containsMouse ? UISettings.hover : "transparent")

        // Background MouseArea declared FIRST so it is BELOW the RowLayout in z-order.
        // The IconButton inside the RowLayout intercepts its own click and does not propagate.
        MouseArea
        {
            id: chMouseArea
            anchors.fill: parent
            hoverEnabled: true
            onClicked: rowRoot.channelClicked(rowData.fxId, rowData.chIdx)
        }

        RowLayout
        {
            anchors.fill: parent
            anchors.leftMargin: 20
            anchors.rightMargin: 4
            spacing: 4

            Image
            {
                visible: (rowData.chIcon || "").length > 0
                Layout.alignment: Qt.AlignVCenter
                width: height
                height: rowRoot.rowH - 6
                source: rowData.chIcon || ""
                sourceSize: Qt.size(width, height)
            }

            RobotoText
            {
                Layout.fillWidth: true
                label: rowData.name || ""
                implicitHeight: UISettings.listItemHeight
                fontSize: UISettings.textSizeDefault
                labelColor: isPending ? UISettings.bgStrong : UISettings.fgMain
            }

            // Target only: disconnect this single channel connection
            IconButton
            {
                visible: isTarget && isChannelConnected
                Layout.alignment: Qt.AlignVCenter
                width: height
                height: rowRoot.rowH - 4
                faSource: FontAwesome.fa_link_slash
                faColor: "yellow"
                tooltip: qsTr("Remove connection for this channel")
                onClicked: rowRoot.disconnectChannelClicked(rowData.fxId, rowData.chIdx)
            }
        }

        Rectangle
        {
            width: parent.width
            height: 1
            anchors.bottom: parent.bottom
            color: UISettings.fgMedium
        }
    }
}
