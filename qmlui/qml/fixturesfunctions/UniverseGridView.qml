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

Flickable
{
    id: universeGridView
    anchors.fill: parent
    anchors.margins: 20
    boundsBehavior: Flickable.StopAtBounds

    contentHeight: uniGrid.height + uniText.height

    property int uniStartAddr: viewUniverseCombo.currentIndex * 512

    function hasSettings()
    {
        return false;
    }

    RobotoText
    {
        id: uniText
        height: 45
        labelColor: "#ccc"
        label: viewUniverseCombo.currentText
        fontSize: 30
        fontBold: true
    }

    GridEditor
    {
        id: uniGrid
        anchors.top: uniText.bottom
        width: parent.width

        showIndices: 512
        gridSize: Qt.size(24, 22)
        gridData: fixtureManager.fixturesMap

        onPressed:
        {
            universeGridView.interactive = false
            var uniAddress = (yPos * gridSize.width) + xPos
            console.log("Fixture pressed at address: " + uniAddress)
            setSelectionData(fixtureManager.fixtureSelection(uniAddress))
        }

        onReleased:
        {
            if (currentFixtureID === -1)
                return;
            var uniAddress = (yPos * gridSize.width) + xPos
            fixtureManager.moveFixture(currentFixtureID, uniAddress + offset)
            universeGridView.interactive = true
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
            var freeAddr = fixtureBrowser.availableChannel(currentFixtureID, uniAddress)

            if (freeAddr === uniAddress)
                validSelection = true
            else
                validSelection = false
        }
    }
}
