/*
  Q Light Controller Plus
  ChannelToolLoader.qml

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
import QtQuick.Controls 2.5

import org.qlcplus.classes 1.0

Popup
{
    id: popupRoot
    padding: 0

    property int fixtureId: -1
    property int channelType: -1
    property int channelIndex: -1
    property int channelValue: 0
    property int yPos: 0

    signal valueChanged(int fixtureID, int channelIndex, int value)

    function loadChannelTool(cItem, fxId, chIdx, val)
    {
        channelType = fixtureManager.channelType(fxId, chIdx)
        if (channelType === QLCChannel.NoGroup || channelType === QLCChannel.Nothing)
            return

        var map = cItem.mapToItem(parent, cItem.x, cItem.y)
        toolLoader.source = ""
        x = map.x
        yPos = map.y
        fixtureId = fxId
        channelIndex = chIdx
        channelValue = val

        switch (channelType)
        {
            case QLCChannel.Intensity:
                toolLoader.source = "qrc:/IntensityTool.qml"
            break
            case QLCChannel.Pan:
            case QLCChannel.Tilt:
                toolLoader.source = "qrc:/SingleAxisTool.qml"
            break

            case QLCChannel.Colour:
            case QLCChannel.Gobo:
            case QLCChannel.Speed:
            case QLCChannel.Shutter:
            case QLCChannel.Prism:
            case QLCChannel.Beam:
            case QLCChannel.Effect:
            case QLCChannel.Maintenance:
                toolLoader.source = "qrc:/PresetsTool.qml"
            break
            default:
                toolLoader.source = "qrc:/ColorToolPrimary.qml"
            break
        }
    }

    Loader
    {
        id: toolLoader

        onLoaded:
        {
            popupRoot.y = popupRoot.yPos - height
            popupRoot.width = width
            popupRoot.height = height

            item.showPalette = false
            //item.closeOnSelect = true

            popupRoot.open()

            if (channelType >= 0xFF)
            {
                item.currentValue = popupRoot.channelValue
                item.targetColor = popupRoot.channelType
            }
            else if (channelType == QLCChannel.Intensity)
            {
                item.currentValue = popupRoot.channelValue
            }
            else if (channelType == QLCChannel.Pan ||
                     channelType == QLCChannel.Tilt)
            {
                item.currentValue = popupRoot.channelValue
                item.maxDegrees = fixtureManager.channelDegrees(popupRoot.fixtureId, popupRoot.channelIndex)
            }
            else
            {
                item.updatePresets(fixtureManager.presetChannel(popupRoot.fixtureId, popupRoot.channelIndex))
            }
        }

        Connections
        {
            ignoreUnknownSignals: true
            target: toolLoader.item
            function onValueChanged()
            {
                popupRoot.valueChanged(popupRoot.fixtureId, popupRoot.channelIndex, target.currentValue)
            }
        }
    }
}
