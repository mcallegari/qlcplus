/*
  Q Light Controller Plus
  ProfilesList.qml

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
import "."

Rectangle
{
    id: profilesContainer
    anchors.fill: parent
    color: "transparent"

    property int universeIndex: 0

    onUniverseIndexChanged:
    {
        profListView.model = ioManager.universeInputProfiles(universeIndex)
    }

    function loadSources(input)
    {
        profListView.model = ioManager.universeInputProfiles(universeIndex)
    }

    ListView
    {
        id: profListView
        anchors.fill: parent
        boundsBehavior: Flickable.StopAtBounds
        delegate:
            Item
            {
                id: root
                height: UISettings.listItemHeight * 2
                width: profilesContainer.width

                MouseArea
                {
                    id: delegateRoot
                    width: profilesContainer.width
                    height: parent.height

                    drag.target: profileItem
                    drag.threshold: 30

                    onPressed: profileItem.color = "#444"
                    onReleased:
                    {
                        profileItem.x = 3
                        profileItem.y = 0

                        if (profileItem.Drag.target !== null)
                        {
                            ioManager.setInputProfile(profileItem.pluginUniverse, profileItem.lineName)
                            profListView.model = ioManager.universeInputProfiles(universeIndex)
                        }
                        else
                        {
                            // return the dragged item to its original position
                            parent = root
                            profileItem.color = "transparent"
                        }
                    }

                    PluginDragItem
                    {
                        id: profileItem
                        x: 3

                        // this key must match the one in UniverseIOItem, to avoid dragging
                        // an input profile in the wrong place

                        pluginUniverse: modelData.universe
                        pluginName: modelData.plugin
                        lineName: modelData.name
                        pluginLine: modelData.line

                        Drag.active: delegateRoot.drag.active
                        Drag.source: delegateRoot
                        Drag.hotSpot.x: width / 2
                        Drag.hotSpot.y: height / 2
                        Drag.keys: [ "profile-" + universeIndex ]

                        // line divider
                        Rectangle
                        {
                            width: parent.width - 6
                            height: 1
                            y: parent.height - 1
                            color: "#555"
                        }
                    } // PluginDragItem
                } // MouseArea
            } // Item
    } // ListView
}
