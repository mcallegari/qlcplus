/*
  Q Light Controller Plus
  UniverseGridView.qml

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

import QtQuick 2.0
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.14

import "."

Flickable
{
    id: universeGridView
    anchors.fill: parent
    boundsBehavior: Flickable.StopAtBounds
    contentHeight: uniGrid.height + topbar.height + UISettings.bigItemHeight

    property alias contextItem: uniGrid
    property string contextName: "UNIGRID"
    property int uniStartAddr: contextManager.universeFilter * 512
    property var fixtureClipboard: null

    function hasSettings()
    {
        return false;
    }

    CustomPopupDialog
    {
        id: errorPopup
        standardButtons: Dialog.Ok
        title: qsTr("Error")
        message: qsTr("Unable to perform the operation.\nThere is either not enough space or the target universe is invalid")
        onAccepted: close()
    }

    RowLayout
    {
        id: topbar
        x: UISettings.iconSizeMedium
        height: UISettings.iconSizeDefault * 1.5
        width: parent.width - (UISettings.iconSizeMedium * 2)

        RobotoText
        {
            id: uniText
            height: UISettings.textSizeDefault * 2
            labelColor: UISettings.fgLight
            label: ioManager.universeName(contextManager.universeFilter)
            fontSize: UISettings.textSizeDefault * 1.5
            fontBold: true
        }

        // filler
        Rectangle
        {
            Layout.fillWidth: true
            height: parent.height
            color: "transparent"
        }

        IconButton
        {
            id: cutBtn
            imgSource: "qrc:/edit-cut.svg"
            tooltip: qsTr("Cut the selected items into clipboard")
            onClicked: fixtureClipboard = contextManager.selectedFixtureIDVariantList()
        }

        IconButton
        {
            id: pasteBtn
            enabled: fixtureClipboard && fixtureClipboard.length
            imgSource: "qrc:/edit-paste.svg"
            tooltip: qsTr("Paste items in the clipboard at the first available position")
            onClicked:
            {
                if (fixtureManager.pasteFromClipboard(fixtureClipboard) !== 0)
                    errorPopup.open()
            }
        }
    }

    GridEditor
    {
        id: uniGrid
        x: UISettings.iconSizeMedium
        anchors.top: topbar.bottom
        width: parent.width - (UISettings.iconSizeMedium * 3)
        height: 32 * gridSize.height

        showIndices: 512
        gridSize: Qt.size(24, 22)
        gridLabels: fixtureManager.fixtureNamesMap
        gridData: fixtureManager.fixturesMap

        Component.onCompleted: contextManager.enableContext("UNIGRID", true, uniGrid)
        Component.onDestruction: if (contextManager) contextManager.enableContext("UNIGRID", false, uniGrid)

        property int prevFixtureID: -1

        function getItemIcon(itemID, chNumber)
        {
            return fixtureManager.channelIcon(itemID, chNumber)
        }

        function getTooltip(xPos, yPos)
        {
            var uniAddress = (yPos * gridSize.width) + xPos
            return fixtureManager.getTooltip(uniAddress)
        }

        onPressed:
        {
            universeGridView.interactive = false
            var uniAddress = (yPos * gridSize.width) + xPos
            var multiSelection = (contextManager.multipleSelection || mods)

            if (selectionData && selectionData.indexOf(uniAddress) >= 0)
                return

            // no modifiers means exclusive selection:
            // start from an empty selection
            if (multiSelection === 0)
                contextManager.resetFixtureSelection()

            console.log("prevFixtureID: " + prevFixtureID + "currentItemID: " + currentItemID)
            if (prevFixtureID != currentItemID && multiSelection === 0)
                contextManager.setFixtureIDSelection(prevFixtureID, false)

            if (currentItemID != -1)
                contextManager.setFixtureIDSelection(currentItemID, true)

            console.log("Fixture pressed at address: " + uniAddress + ", itemID: " + currentItemID)
            setSelectionData(contextManager.selectedFixtureAddress())

            prevFixtureID = currentItemID
        }

        onReleased:
        {
            universeGridView.interactive = true

            if (currentItemID === -1 || validSelection === false || offset === 0)
                return

            var uniAddress = (yPos * gridSize.width) + xPos
            fixtureManager.moveFixture(currentItemID, selectionData[0] + offset)
        }

        onDragEntered:
        {
            var channels = dragEvent.source.channels
            console.log("Drag entered. Channels: " + channels)
            var uniAddress = (yPos * gridSize.width) + xPos
            var tmp = []
            for (var q = 0; q < dragEvent.source.quantity; q++)
            {
                for (var i = 0; i < channels; i++)
                    tmp.push(uniAddress + i)

                uniAddress += channels + dragEvent.source.gap
            }
            console.log("Selection data contains " + tmp.length + " entries")
            setSelectionData(tmp)
        }

        onDragPositionChanged:
        {
            var uniAddress = (yPos * gridSize.width) + xPos
            dragEvent.source.address = uniAddress
            var freeAddr = fixtureBrowser.availableChannel(contextManager.universeFilter, dragEvent.source.channels,
                                                           dragEvent.source.quantity,
                                                           dragEvent.source.gap, uniAddress)
            if (freeAddr === uniAddress)
                validSelection = true
            else
                validSelection = false
        }

        onPositionChanged:
        {
            var uniAddress = (yPos * gridSize.width) + xPos
            var freeAddr = fixtureBrowser.availableChannel(currentItemID, uniAddress)

            if (freeAddr === uniAddress)
                validSelection = true
            else
                validSelection = false
        }
    }
}
