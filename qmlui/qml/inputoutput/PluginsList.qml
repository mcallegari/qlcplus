/*
  Q Light Controller Plus
  AudioCardsList.qml

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
        //model: ioManager.audioInputSources()
        delegate:
            Rectangle {
                id: uniDelegate
                width: pluginsContainer.width
                height: 60
                color: "transparent"
                Row {
                    Rectangle {
                        radius: 3
                        height: uniDelegate.height - 4
                        width: height
                        x: 5
                        y: 2

                        Image {
                            anchors.fill: parent
                            source: {
                                switch(modelData.plugin)
                                {
                                    case "ArtNet": "qrc:/artnetplugin.svg"; break;
                                    case "DMX USB": "qrc:/dmxusbplugin.svg"; break;
                                    case "OLA": "qrc:/olaplugin.svg"; break;
                                    case "MIDI": "qrc:/midiplugin.svg"; break;
                                    case "OSC": "qrc:/oscplugin.svg"; break;
                                    case "E1.31": "qrc:/e131plugin.svg"; break;
                                    case "Loopback": "qrc:/loopbackplugin.svg"; break;
                                }
                            }
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

