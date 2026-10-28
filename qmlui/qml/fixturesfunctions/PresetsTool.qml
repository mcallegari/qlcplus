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

import QtQuick

import org.qlcplus.classes 1.0
import "."

Rectangle
{
    id: toolRoot
    width: UISettings.bigItemHeight * 3
    height: presetsArea.height + (showPalette ? paletteBox.height : 0)
    color: UISettings.bgStrong
    border.color: UISettings.bgLight
    border.width: 2
    clip: true

    property bool closeOnSelect: false
    property var dragTarget: null
    property alias presetModel: prList.model
    property int selectedFixture: -1
    property int selectedChannel: -1
    property bool showPalette: false
    property int paletteType: QLCPalette.Undefined
    property int currentValue: -1 // as DMX value; -1 means unavailable
    property int currentPreset: QLCCapability.Custom
    property int rangeLowLimit: 0
    property int rangeHighLimit: 255

    signal presetSelected(QLCCapability cap, int fxID, int chIdx, int value)
    signal valueChanged(int value)

    function selectPresetChannel(channelData, valueOverride)
    {
        if (channelData === undefined || channelData === null)
        {
            selectedFixture = -1
            selectedChannel = -1
            currentValue = -1
            capRepeater.model = null
            return
        }

        selectedFixture = channelData.fixtureID
        selectedChannel = channelData.channelIdx
        currentValue = typeof valueOverride !== "undefined" ? valueOverride :
                       (typeof channelData.currentValue !== "undefined" ? channelData.currentValue : -1)
        capRepeater.model = null
        capRepeater.model = fixtureManager.presetCapabilities(selectedFixture, selectedChannel)
        prFlickable.contentY = 0
    }

    function updatePresets(newModel, valueOverride)
    {
        var previousFixture = selectedFixture
        var previousChannel = selectedChannel
        var selectedIndex = -1

        prList.model = null // force reload
        prList.model = newModel

        for (var i = 0; i < newModel.length; ++i)
        {
            if (newModel[i].fixtureID === previousFixture &&
                    newModel[i].channelIdx === previousChannel)
            {
                selectedIndex = i
                break
            }
        }

        if (selectedIndex < 0 && newModel.length > 0)
            selectedIndex = 0

        selectPresetChannel(selectedIndex >= 0 ? newModel[selectedIndex] : null,
                            valueOverride)
    }

    MouseArea
    {
        anchors.fill: parent
        onWheel: { return false }
    }

    Item
    {
        id: presetsArea
        width: parent.width
        height: UISettings.bigItemHeight * 3

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
                        property bool isCurrentPreset: toolRoot.selectedFixture === fxID &&
                                                        toolRoot.selectedChannel === chIdx
                        color: isCurrentPreset ? UISettings.highlight :
                                (prMouseArea.pressed ? UISettings.bgLight : UISettings.bgMedium)
                        border.width: 1
                        border.color: isCurrentPreset ? UISettings.highlight : UISettings.bgLight

                        property int fxID: modelData.fixtureID
                        property int chIdx: modelData.channelIdx

                        RobotoText
                        {
                            x: 3
                            width: parent.width - 6
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
                            drag.target: toolRoot.dragTarget ? toolRoot.dragTarget : toolRoot
                            drag.axis: Drag.XAndYAxis

                            onClicked:
                            {
                                toolRoot.selectPresetChannel(modelData)
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
                        currentValue: toolRoot.currentValue
                        visible: (capability.min <= toolRoot.rangeHighLimit || capability.max <= toolRoot.rangeLowLimit)
                        onValueChanged: function(value)
                        {
                            var val = Math.min(Math.max(value, rangeLowLimit), rangeHighLimit)
                            toolRoot.currentValue = val
                            toolRoot.currentPreset = capability.preset
                            toolRoot.presetSelected(capability, selectedFixture, selectedChannel, val)
                            toolRoot.valueChanged(val)

                            if (toolRoot.showPalette)
                            {
                                let pct = Math.round(val * 100 / 255)
                                paletteBox.updateValues(capability.preset, pct)
                            }

                            if (closeOnSelect)
                                toolRoot.visible = false
                        }
                    }
                }
            }
        }
    }

    PaletteFanningBox
    {
        id: paletteBox
        visible: toolRoot.showPalette
        y: presetsArea.height
        width: parent.width
        paletteType: toolRoot.paletteType
    }
}
