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

import QtQuick
import "."

Rectangle
{
    id: pluginsContainer
    anchors.fill: parent
    color: "transparent"

    property int universeIndex: 0
    property bool isInput: false
    readonly property int sourcesCount: uniListView.count

    function refreshSources()
    {
        uniListView.model = isInput ?
                    ioManager.universeInputSources(universeIndex) :
                    ioManager.universeOutputSources(universeIndex)
    }

    function activateSource(pluginName, available, canConfigure)
    {
        if (available === false && canConfigure === true)
            ioManager.configurePluginByName(pluginName)
    }

    function loadSources(input)
    {
        isInput = input
        refreshSources()
    }

    Connections
    {
        target: ioManager
        function onIoSourcesChanged() { pluginsContainer.refreshSources() }
    }

    ListView
    {
        id: uniListView
        anchors.fill: parent
        boundsBehavior: Flickable.StopAtBounds
        delegate:
            Item
            {
                id: root
                height: UISettings.listItemHeight * 1.7
                width: pluginsContainer.width

                MouseArea
                {
                    id: delegateRoot
                    width: pluginsContainer.width
                    height: parent.height

                    drag.target: pluginItem.sourceAvailable ? pluginItem : null
                    drag.threshold: height / 2

                    onClicked:
                        pluginsContainer.activateSource(pluginItem.pluginName,
                                                        pluginItem.sourceAvailable,
                                                        pluginItem.canConfigure)

                    onReleased:
                    {
                        if (pluginItem.sourceAvailable === true &&
                            pluginItem.Drag.target !== null)
                        {
                            pluginItem.Drag.drop()
                            if (pluginsContainer.isInput === false)
                            {
                                uniListView.model = ioManager.universeOutputSources(universeIndex)
                            }
                            else
                            {
                                ioManager.addInputPatch(pluginItem.pluginUniverse, pluginItem.pluginName,
                                                        pluginItem.pluginLine)
                                uniListView.model = ioManager.universeInputSources(universeIndex)
                            }
                        }
                        else
                        {
                            // return the dragged item to its original position
                            parent = root
                        }
                        pluginItem.x = 3
                        pluginItem.y = 0
                    }

                    PluginDragItem
                    {
                        id: pluginItem
                        objectName: "pluginSource_" + index
                        x: 3
                        color: delegateRoot.pressed ? UISettings.highlightPressed : "transparent"

                        // this key must match the one in UniverseIOItem, to avoid dragging
                        // an input plugin on output and vice-versa
                        property string dragKey: isInput ? "input-" + universeIndex : "output-" + universeIndex

                        pluginUniverse: modelData.universe
                        pluginName: modelData.plugin
                        lineName: modelData.name
                        pluginLine: modelData.line
                        sourceAvailable: modelData.available !== false
                        canConfigure: modelData.canConfigure === true
                        statusText: modelData.status || ""

                        Drag.active: sourceAvailable && delegateRoot.drag.active
                        Drag.source: pluginItem
                        //Drag.hotSpot.x: width / 2
                        //Drag.hotSpot.y: height / 2
                        Drag.keys: [ dragKey ]

                        // line divider
                        Rectangle
                        {
                            width: parent.width - 6
                            height: 1
                            y: parent.height - 1
                            color: UISettings.bgLight
                        }
                    } // PluginDragItem
                } // MouseArea
            } // Item
    } // ListView
}
