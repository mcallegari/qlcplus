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
import org.qlcplus.classes 1.0

import "."

Rectangle
{
    id: toolRoot
    width: UISettings.bigItemHeight * 3
    height: UISettings.bigItemHeight * 3
    color: UISettings.bgMedium
    border.color: UISettings.bgLight
    border.width: 2
    clip: true

    property bool closeOnSelect: false
    property alias presetModel: prList.model
    property int selectedFixture: -1
    property int selectedChannel: -1
    property bool showPalette: false
    property int currentValue: 0 // as DMX value
    property int rangeLowLimit: 0
    property int rangeHighLimit: 255

    signal presetSelected(QLCCapability cap, int fxID, int chIdx, int value)
    signal valueChanged(int value)

    function updatePresets(presetModel)
    {
        if (visible === true)
        {
            selectedFixture = -1
            prList.model = null // force reload
            prList.model = presetModel
        }
    }

    MouseArea
    {
        anchors.fill: parent
        onWheel: { return false }
    }

    // toolbar area containing the available preset channels
    Rectangle
    {
        id: presetToolBar
        width: parent.width
        height: UISettings.iconSizeDefault
        z: 10
        clip: true
        gradient: Gradient
        {
            GradientStop { position: 0; color: UISettings.toolbarStartSub }
            GradientStop { position: 1; color: UISettings.toolbarEnd }
        }

        ListView
        {
            id: prList
            anchors.fill: parent
            orientation: ListView.Horizontal
            boundsBehavior: Flickable.StopAtBounds

            delegate:
                Rectangle
                {
                    id: delegateRoot
                    width: UISettings.bigItemHeight * 1.3
                    height: presetToolBar.height
                    color: prMouseArea.pressed ? UISettings.bgLight : UISettings.bgMedium
                    border.width: 1
                    border.color: UISettings.bgLight

                    property int fxID: modelData.fixtureID
                    property int chIdx: modelData.channelIdx

                    Component.onCompleted:
                    {
                        if (selectedFixture === -1)
                        {
                            selectedFixture = fxID
                            selectedChannel = chIdx
                            capRepeater.model = fixtureManager.presetCapabilities(selectedFixture, selectedChannel)
                            prFlickable.contentY = 0
                        }
                    }

                    RobotoText
                    {
                        x: 2
                        width: parent.width - 4
                        height: parent.height
                        label: modelData.name
                        fontSize: UISettings.textSizeDefault * 0.70
                        wrapText: true
                    }
                    MouseArea
                    {
                        id: prMouseArea
                        anchors.fill: parent
                        hoverEnabled: true

                        onClicked:
                        {
                            selectedFixture = delegateRoot.fxID
                            selectedChannel = delegateRoot.chIdx
                            capRepeater.model = fixtureManager.presetCapabilities(selectedFixture, selectedChannel)
                            prFlickable.contentY = 0
                        }
                    }
            }
        }
    }

    // flickable layout containing the actual preset capabilities
    Flickable
    {
        id: prFlickable
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
                    visible: (capability.min <= toolRoot.rangeHighLimit || capability.max <= toolRoot.rangeLowLimit)
                    onValueChanged:
                    {
                        var val = Math.min(Math.max(value, rangeLowLimit), rangeHighLimit)
                        toolRoot.currentValue = val
                        toolRoot.presetSelected(capability, selectedFixture, selectedChannel, val)
                        toolRoot.valueChanged(val)
                        if (closeOnSelect)
                            toolRoot.visible = false
                    }
                }
            }
        }
    }
}
