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
import QtQuick.Controls 2.1

import "."

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
        id: mainFlickable
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
                id: audioItem

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
                        universe: modelData.classRef

                        onSelected:
                        {
                            leftPanel.universeIndex = index
                            leftPanel.showPluginsButton = true
                            leftPanel.showAudioButton = false
                            rightPanel.universeIndex = index
                            rightPanel.showPluginsButton = true
                            rightPanel.showAudioButton = false

                            audioItem.isSelected = false
                        }
                        onPatchDragging:
                        {
                            removePatchBox.visible = status
                        }
                    }
            }
        }
        ScrollBar.vertical: CustomScrollBar { }
    }

    /* Bottom container to drag a patch and delete it */
    Rectangle
    {
        id: removePatchBox
        x: (ioMgrContainer.width / 2) - (width / 2)
        y: ioMgrContainer.height - (height / 2)
        z: 10
        width: UISettings.bigItemHeight * 2
        height: UISettings.bigItemHeight
        visible: false

        radius: height / 2
        color: "#7FFF0000"

        Text
        {
            id: faIcon
            anchors.horizontalCenter: parent.horizontalCenter
            y: 15
            color: "#aaa"
            font.family: "FontAwesome"
            font.pixelSize: UISettings.textSizeDefault * 2.5
            text: FontAwesome.fa_trash_o
        }

        DropArea
        {
            anchors.fill: parent
            id: delPatchDrop
            keys: [ "removePatch" ]

            states: [
                State
                {
                    when: delPatchDrop.containsDrag
                    PropertyChanges
                    {
                        target: removePatchBox
                        color: "#7FFF8000"
                    }
                }
            ]
        }
    }
}

