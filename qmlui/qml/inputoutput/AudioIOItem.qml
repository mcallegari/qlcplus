/*
  Q Light Controller Plus
  AudioIOItem.qml

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
    id: audioItem
    width: parent.width
    height: UISettings.bigItemHeight
    color: isSelected ? "#2D444E" : "transparent"
    border.width: 2
    border.color: isSelected ? UISettings.selection : "transparent"

    property int wireBoxWidth: (audioItem.width - audioBox.width) / 8 // one quarter of a audioItem side
    property bool isSelected: false
    signal selected

    AudioDeviceItem
    {
        x: 14
        anchors.verticalCenter: audioItem.verticalCenter
        width: inWireBox.width * 3
        audioDevice: ioManager.audioInputDevice
    }

    // Input device wire box
    PatchWireBox
    {
        id: inWireBox
        x: audioBox.x - width + 6
        width: wireBoxWidth
        height: audioItem.height
        z: 10

        patchesNumber: 1
    }

    // representation of the central Audio block
    Rectangle
    {
        id: audioBox
        anchors.centerIn: parent
        width: UISettings.bigItemHeight * 1.2
        height: UISettings.bigItemHeight * 0.8
        radius: 5
        //color: "#1C2255"
        gradient: Gradient
        {
            id: bgGradient
            GradientStop { position: 0 ; color: "#1A551B" }
            GradientStop { position: 1 ; color: "#3C832B" }
        }
        border.width: 2
        border.color: "#111"

        RobotoText
        {
            height: parent.height
            width: parent.width
            label: qsTr("Global Audio")
            wrapText: true
            textHAlign: Text.AlignHCenter
            fontSize: UISettings.textSizeDefault
        }
    }

    // Output device wire box
    PatchWireBox
    {
        id: outWireBox
        x: audioBox.x + audioBox.width - 6
        width: wireBoxWidth
        height: audioItem.height
        z: 10

        patchesNumber: 1
    }

    AudioDeviceItem
    {
        x: outWireBox.x + outWireBox.width - 8
        anchors.verticalCenter: audioItem.verticalCenter
        width: outWireBox.width * 3
        audioDevice: ioManager.audioOutputDevice
    }

    // Global mouse area to select this Audio item
    MouseArea
    {
        anchors.fill: parent
        onClicked:
        {
            if (isSelected == false)
            {
                isSelected = true
                ioManager.setSelectedItem(audioItem, 0)
                audioItem.selected(0);
            }
        }
    }

    // divider
    Rectangle
    {
        width: parent.width
        height: 2
        y: parent.height - 2
        color: isSelected ? UISettings.selection : "#666"
    }
}

