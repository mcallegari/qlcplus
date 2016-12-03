/*
  Q Light Controller Plus
  IORightPanel.qml

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

SidePanel
{
    id: rightSidePanel

    property int universeIndex
    property bool showAudioButton: false
    property bool showPluginsButton: false

    onUniverseIndexChanged:
    {
        if (isOpen == true)
        {
            var tmpSource = loaderSource
            loaderSource = ""
            loaderSource = tmpSource
        }
    }

    onContentLoaded:
    {
        item.universeIndex = universeIndex
        item.loadSources(false)
    }

    Rectangle
    {
        width: collapseWidth
        height: parent.height
        color: "transparent"
        z: 2

        Column
        {
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 3

            IconButton
            {
                id: audioOutputButton
                z: 2
                visible: showAudioButton
                width: iconSize
                height: iconSize
                imgSource: "qrc:/audiocard.svg"
                checkable: true
                tooltip: qsTr("Show the audio output sources")
                onToggled:
                {
                    if (checked == true)
                        loaderSource = "qrc:/AudioCardsList.qml"
                    animatePanel(checked)
                }
            }

            IconButton
            {
                id: uniOutputButton
                z: 2
                visible: showPluginsButton
                width: iconSize
                height: iconSize
                imgSource: "qrc:/inputoutput.svg"
                checkable: true
                tooltip: qsTr("Show the universe output sources")
                onToggled:
                {
                    if (checked == true)
                        loaderSource = "qrc:/PluginsList.qml"
                    animatePanel(checked)
                }
            }
        }
    }
}

