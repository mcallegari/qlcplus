/*
  Q Light Controller Plus
  SceneFixtureConsole.qml

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
import QtQuick.Layouts 1.1

import org.qlcplus.classes 1.0
import "."

Rectangle
{
    id: sfcContainer
    anchors.fill: parent
    color: "transparent"
    objectName: "sceneFixtureConsole"

    Component.onCompleted: sceneEditor.sceneConsoleLoaded(true)
    Component.onDestruction: sceneEditor.sceneConsoleLoaded(false)

    property bool isSceneEditor: true
    property bool multipleSelection: false

    function scrollToItem(fxIdx)
    {
        console.log("[scrollToItem] fxIdx: " + fxIdx)
        fixtureList.positionViewAtIndex(fxIdx, ListView.Beginning)
        fixtureList.currentIndex = fxIdx
    }

    ChannelToolLoader
    {
        id: channelToolLoader
        z: 2

        onValueChanged: functionManager.setChannelValue(fixtureID, channelIndex, value)
    }

    ListView
    {
        id: fixtureList
        anchors.fill: parent
        orientation: ListView.Horizontal
        model: sceneEditor.fixtureList
        boundsBehavior: Flickable.StopAtBounds
        highlightFollowsCurrentItem: false
        currentIndex: -1
        z: 1

        delegate:
            Rectangle
            {
                height: parent.height
                width: fxConsole.width + 4
                color: UISettings.bgMedium

                Component.onCompleted: sceneEditor.registerFixtureConsole(index, fxConsole)
                Component.onDestruction: sceneEditor.unRegisterFixtureConsole(index)

                FixtureConsole
                {
                    id: fxConsole
                    x: 2
                    fixtureObj: model.cRef
                    isSelected: model.isSelected
                    height: parent.height
                    color: index % 2 ? UISettings.bgFixtureEven : UISettings.bgFixtureOdd
                    showEnablers: true
                    sceneConsole: true
                    multipleSelection: sfcContainer.multipleSelection

                    onRequestTool: channelToolLoader.loadChannelTool(item, fixtureID, chIndex, value)
                }
                // Fixture divider
                Rectangle
                {
                    anchors.fill: parent
                    width: 2
                    color: "transparent"
                    radius: 3
                    border.width: 2
                    border.color: fixtureList.currentIndex == index ? UISettings.selection : "transparent"
                }
            }
    }
}
