/*
  Q Light Controller Plus
  IOLeftPanel.qml

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
import QtQuick.Controls 2.1

import "."

SidePanel
{
    id: leftSidePanel
    anchors.left: parent.left
    anchors.leftMargin: 0

    panelAlignment: Qt.AlignLeft

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
        item.loadSources(true)
    }

    Rectangle
    {
        id: sideBar
        x: parent.width - collapseWidth
        width: collapseWidth
        height: parent.height
        color: "transparent"
        z: 2

        Column
        {
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.leftMargin: 1
            spacing: 3

            ButtonGroup { id: ioInputGroup }

            IconButton
            {
                id: audioInputButton
                z: 2
                visible: showAudioButton
                width: iconSize
                height: iconSize
                imgSource: "qrc:/audiocard.svg"
                checkable: true
                ButtonGroup.group: ioInputGroup
                tooltip: qsTr("Show the audio input sources")
                onToggled:
                {
                    if (checked == true)
                        loaderSource = "qrc:/AudioCardsList.qml"
                    animatePanel(checked)
                }
            }

            IconButton
            {
                id: uniInputButton
                z: 2
                visible: showPluginsButton
                width: iconSize
                height: iconSize
                imgSource: "qrc:/inputoutput.svg"
                checkable: true
                ButtonGroup.group: ioInputGroup
                tooltip: qsTr("Show the universe input sources")
                onToggled:
                {
                    if (checked == true)
                        loaderSource = "qrc:/PluginsList.qml"
                    animatePanel(checked)
                }
            }

            IconButton
            {
                id: uniProfilesButton
                z: 2
                visible: showPluginsButton
                width: iconSize
                height: iconSize
                imgSource: ""
                checkable: true
                ButtonGroup.group: ioInputGroup
                tooltip: qsTr("Show the universe input profiles")
                onToggled:
                {
                    if (checked == true)
                        loaderSource = "qrc:/ProfilesList.qml"
                    animatePanel(checked)
                }

                RobotoText
                {
                    anchors.centerIn: parent
                    label: "P"
                    fontSize: UISettings.textSizeDefault * 1.1
                    fontBold: true
                }
            }

            IconButton
            {
                id: inputConfigureButton
                z: 2
                visible: ioManager.inputCanConfigure
                width: iconSize
                height: iconSize
                imgSource: "qrc:/configure.svg"
                tooltip: qsTr("Open the plugin configuration")
                onClicked: ioManager.configurePlugin(true)
            }
        }
    }
}
