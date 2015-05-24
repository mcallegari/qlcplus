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

import "PluginUtils.js" as PluginUtils

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
            Rectangle {
                x: 3
                id: uniDelegate
                width: pluginsContainer.width
                height: 60
                color: "transparent"
                Row {
                    spacing: 3
                    Rectangle {
                        radius: 3
                        height: uniDelegate.height - 4
                        width: height
                        gradient: Gradient {
                            id: bgGradient
                            GradientStop { position: 0.75 ; color: "#FFF" }
                            GradientStop { position: 1 ; color: "#7F7F7F" }
                        }
                        border.width: 2
                        border.color: "#777"
                        x: 5
                        y: 2

                        Image {
                            anchors.fill: parent
                            anchors.margins: 3
                            source: PluginUtils.iconFromName(modelData.plugin)
                            sourceSize: Qt.size(width, height)
                            fillMode: Image.Stretch
                        }
                    }

                    RobotoText {
                        height: uniDelegate.height
                        width: uniDelegate.width
                        label: modelData.name
                    }
                }
                Rectangle {
                    width: uniDelegate.width
                    height: 1
                    y: uniDelegate.height - 1
                    color: "#555"
                }
            }
    }
}

