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
import QtQuick.Layouts 1.0

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

        ColumnLayout
        {
            anchors.horizontalCenter: parent.horizontalCenter
            height: parent.height
            width: iconSize
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

            IconButton
            {
                faSource: checked ? FontAwesome.fa_eye_slash : FontAwesome.fa_eye
                faColor: UISettings.fgMain
                bgColor: "green"
                checkedColor: "red"
                checkable: true
                checked: ioManager.blackout
                tooltip: qsTr("Enable/Disable blackout on all the output patches")
                onToggled: ioManager.blackout = checked
            }

            IconButton
            {
                id: outputConfigureButton
                z: 2
                visible: ioManager.outputCanConfigure
                width: iconSize
                height: iconSize
                imgSource: "qrc:/configure.svg"
                tooltip: qsTr("Open the plugin configuration")
                onClicked: ioManager.configurePlugin(false)
            }

            /* filler object */
            Rectangle
            {
                Layout.fillHeight: true
                width: iconSize
                color: "transparent"
            }

            IconButton
            {
                z: 2
                width: iconSize
                height: iconSize
                imgSource: "qrc:/add.svg"
                tooltip: qsTr("Add a new universe")
                onClicked: ioManager.addUniverse()
            }

            IconButton
            {
                z: 2
                visible: ioManager.selectedIndex === ioManager.universeNames.length - 1
                width: iconSize
                height: iconSize
                imgSource: "qrc:/remove.svg"
                tooltip: qsTr("Remove the selected universe")
                onClicked: ioManager.removeLastUniverse()
            }
        }
    }
}

