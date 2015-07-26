/*
  Q Light Controller Plus
  InputOutputManager.qml

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

Rectangle
{
    id: ioMgrContainer
    anchors.fill: parent
    color: "transparent"

    property string contextName: "IOMGR"

    IOLeftPanel
    {
        id: leftPanel
        x: 0
        z: 5
        height: parent.height
    }

    IORightPanel
    {
        id: rightPanel
        x: parent.width - width
        z: 5
        height: parent.height
    }

    Flickable
    {
        width: parent.width - leftPanel.width - rightPanel.width
        height: parent.height
        x: leftPanel.width
        z: 4
        boundsBehavior: Flickable.StopAtBounds

        contentHeight: ioList.height

        Column
        {
            id: ioList
            width: parent.width
            AudioIOItem
            {
                onSelected:
                {
                    leftPanel.showPluginsButton = false
                    leftPanel.showAudioButton = true
                    rightPanel.showPluginsButton = false
                    rightPanel.showAudioButton = true
                }
            }
            Repeater
            {
                model: ioManager.universes
                delegate:
                    UniverseIOItem
                    {
                        universe: model.modelData

                        onSelected:
                        {
                            leftPanel.universeIndex = index
                            leftPanel.showPluginsButton = true
                            leftPanel.showAudioButton = false
                            rightPanel.universeIndex = index
                            rightPanel.showPluginsButton = true
                            rightPanel.showAudioButton = false
                        }
                    }
            }
        }
    }
}

