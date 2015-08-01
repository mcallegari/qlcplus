/*
  Q Light Controller Plus
  PresetsTool.qml

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

import QtQuick 2.3
import com.qlcplus.classes 1.0

Rectangle
{
    id: toolRoot
    width: 360
    height: 350
    color: "#333"
    border.color: "#666"
    border.width: 2
    clip: true

    property bool goboPresets: false // false for color wheel, true for gobos
    property int selectedIndex: -1

    onVisibleChanged:
    {
        if (visible === true)
        {
            selectedIndex = -1
            prList.model = null // force reload
            prList.model = goboPresets ? fixtureManager.goboChannels : fixtureManager.colorWheelChannels
            capRepeater.model = null // force reload
            capRepeater.model = fixtureManager.presetCapabilities(selectedIndex)
        }
    }

    // toolbar area containing the available preset channels
    Rectangle
    {
        id: presetToolBar
        width: parent.width
        height: 50
        z: 10
        clip: true
        gradient: Gradient
        {
            id: ffMenuGradient
            GradientStop { position: 0 ; color: "#222" }
            GradientStop { position: 1 ; color: "#111" }
        }

        ListView
        {
            id: prList
            anchors.fill: parent
            orientation: ListView.Horizontal
            boundsBehavior: Flickable.StopAtBounds
            //model: goboPresets ? fixtureManager.goboChannels : fixtureManager.colorWheelChannels
            delegate:
                Rectangle
                {
                    id: delRoot
                    width: 150
                    height: presetToolBar.height
                    color: "#333"
                    border.width: 1
                    border.color: "#666"

                    property int presetIdx: modelData.presetIndex

                    Component.onCompleted:
                    {
                        if (selectedIndex === -1)
                            selectedIndex = modelData.presetIndex
                    }

                    RobotoText
                    {
                        x: 1
                        width: parent.width - 2
                        height: parent.height
                        label: modelData.name
                        fontSize: 10
                        wrapText: true
                    }
                    MouseArea
                    {
                        anchors.fill: parent
                        hoverEnabled: true
                        onEntered: delRoot.color = "#555"
                        onExited: delRoot.color = "#333"
                        onClicked:
                        {
                            selectedIndex = presetIdx
                            capRepeater.model = fixtureManager.presetCapabilities(selectedIndex)
                        }
                    }
            }
        }
    }

    // flickable layout containing the actual preset capabilities
    Flickable
    {
        width: parent.width
        height: parent.height - presetToolBar.height
        y: presetToolBar.height
        boundsBehavior: Flickable.StopAtBounds
        contentWidth: width
        contentHeight: flowView.height

        Flow
        {
            id: flowView
            width: parent.width
            Repeater
            {
                id: capRepeater
                delegate: PresetCapabilityItem
                {
                    capability: modelData
                    capIndex: index + 1
                    onValueChanged:
                    {
                        fixtureManager.setPresetValue(selectedIndex, value)
                    }
                }
            }
        }
    }
}
