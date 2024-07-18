/*
  Q Light Controller Plus
  DMXView.qml

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
    id: dmxViewRoot
    anchors.fill: parent
    color: UISettings.bgMedium

    property alias contextItem: flowLayout
    property int viewMargin: 20

    function hasSettings()
    {
        return true
    }

    function showSettings(show)
    {
        dmxSettings.visible = show
    }

    ChannelToolLoader
    {
        id: channelToolLoader
        z: 2

        onValueChanged: fixtureManager.setChannelValue(fixtureID, channelIndex, value)
    }

    Flickable
    {
        id: fixtureDMXView
        anchors.fill: parent
        anchors.leftMargin: viewMargin
        anchors.topMargin: viewMargin

        contentHeight: flowLayout.height
        contentWidth: flowLayout.width
        //interactive: false

        boundsBehavior: Flickable.StopAtBounds

        property string contextName: "DMX"

        Flow
        {
            id: flowLayout
            objectName: "DMXFlowView"
            spacing: 5
            width: dmxViewRoot.width - viewMargin

            function loadTool(item, fixtureID, chIndex, value)
            {
                channelToolLoader.loadChannelTool(item, fixtureID, chIndex, value)
            }

            function itemWidthChanged(width)
            {
                if (fixtureDMXView.contentWidth < width)
                    fixtureDMXView.contentWidth = width + (viewMargin * 2)
            }

            Component.onCompleted: contextManager.enableContext("DMX", true, flowLayout)
            Component.onDestruction: if (contextManager) contextManager.enableContext("DMX", false, flowLayout)
        }

        ScrollBar.vertical: CustomScrollBar { }
        ScrollBar.horizontal : CustomScrollBar { orientation: Qt.Horizontal }
    }

    SettingsViewDMX
    {
        id: dmxSettings
        visible: false
        x: parent.width - width
        z: 5
    }
}
