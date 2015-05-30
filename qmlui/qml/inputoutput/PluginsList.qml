/*
  Q Light Controller Plus
  PluginsList.qml

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

Rectangle {
    id: pluginsContainer
    anchors.fill: parent
    color: "transparent"

    property int universeIndex: 0

    function loadSources(input)
    {
        if (input === true)
            uniListView.model = ioManager.universeInputSources(universeIndex)
        else
            uniListView.model = ioManager.universeOutputSources(universeIndex)
    }

    ListView {
        id: uniListView
        anchors.fill: parent
        boundsBehavior: Flickable.StopAtBounds
        delegate:
            PluginDragItem {
                x: 3
                width: pluginsContainer.width

                pluginName: modelData.plugin
                lineName: modelData.name
                pluginLine: modelData.line

                Rectangle {
                    width: parent.width
                    height: 1
                    y: parent.height - 1
                    color: "#555"
                }
            }
    }
}

