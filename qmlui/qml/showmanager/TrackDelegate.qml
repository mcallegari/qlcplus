/*
  Q Light Controller Plus
  TrackDelegate.qml

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

import org.qlcplus.classes 1.0
import "."

Rectangle
{
    width: 100
    height: UISettings.mediumItemHeight
    clip: true

    color: isSelected ? UISettings.highlight : "#313F4A"

    property Track trackRef: null
    property int trackIndex
    property bool isSelected: false

    RobotoText
    {
        x: 2
        width: parent.width - 4
        height: parent.height
        label: trackRef ? trackRef.name : ""
        wrapText: true
    }

    Rectangle
    {
        width: parent.width
        height: 2
        y: parent.height - 2
        color: "#263039"
    }

    IconButton
    {
        id: soloButton
        x: parent.width - (width * 2) - 6
        y: 2
        z: 2
        width: parent.width / 6
        height: parent.height * 0.3
        bgColor: "#8191A0"
        checkedColor: "yellow"
        imgSource: ""
        checkable: true
        tooltip: qsTr("Solo this track")
        onToggled:
        {
        }

        RobotoText
        {
            anchors.centerIn: parent
            height: parent.height - 2
            label: "S"
            labelColor: "#3C4A55"
            fontSize: height - 2
            fontBold: true
        }
    }

    IconButton
    {
        id: muteButton
        x: parent.width - width - 2
        y: 2
        z: 2
        width: parent.width / 6
        height: parent.height * 0.3
        bgColor: "#8191A0"
        checkedColor: "red"
        imgSource: ""
        checkable: true
        tooltip: qsTr("Mute this track")
        onToggled:
        {
        }

        RobotoText
        {
            anchors.centerIn: parent
            height: parent.height - 2
            label: "M"
            labelColor: "#3C4A55"
            fontSize: height - 2
            fontBold: true
        }
    }

    MouseArea
    {
        anchors.fill: parent
        onClicked: showManager.selectedTrack = trackIndex
    }
}
