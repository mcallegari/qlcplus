/*
  Q Light Controller Plus
  FixtureGroupEditor.qml

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

import "GenericHelpers.js" as Helpers
import "."

Flickable
{
    id: fixtureGroupEditor
    anchors.fill: parent
    anchors.margins: 20
    boundsBehavior: Flickable.StopAtBounds

    function hasSettings() { return false; }

    RobotoText
    {
        id: groupName
        height: UISettings.textSizeDefault * 2
        label: fixtureManager.fixtureGroupName
        labelColor: UISettings.fgLight
        fontSize: UISettings.textSizeDefault * 1.5
        fontBold: true
    }

    GridEditor
    {
        id: groupGrid
        anchors.top: groupName.bottom
        width: parent.width
        height: parent.height - groupName.height

        function getItemIcon(fixtureID, headNumber)
        {
            return fixtureManager.fixtureIcon(fixtureID)
        }

        gridSize: fixtureManager.fixtureGroupSize
        fillDirection: Qt.Horizontal | Qt.Vertical
        gridData: fixtureManager.fixtureGroupMap
        evenColor: "white"

        onPressed:
        {
            fixtureGroupEditor.interactive = false
            var headAddress = (yPos * gridSize.width) + xPos
            console.log("Head pressed at address: " + headAddress)
            //setSelectionData(fixtureManager.fixtureSelection(headAddress))
        }
    }
}
