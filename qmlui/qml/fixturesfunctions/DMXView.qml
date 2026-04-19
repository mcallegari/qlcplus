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

import QtQuick
import QtQuick.Controls

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

    function showFixtureBrowser(show)
    {
        if (show)
        {
            fixtureAndFunctions.leftPanel.resetSelection()
            fixtureAndFunctions.leftPanel.loaderSource = "qrc:/FixtureSubstitutionPanel.qml"
            fixtureAndFunctions.leftPanel.animatePanel(true)
        }
        else
        {
            fixtureAndFunctions.leftPanel.animatePanel(false)
        }
    }

    Connections
    {
        target: fixtureAndFunctions.leftPanel
        function onContentLoaded(item, ID)
        {
            if (fixtureAndFunctions.leftPanel.loaderSource === "qrc:/FixtureSubstitutionPanel.qml")
            {
                item.closed.connect(function() {
                    fixtureAndFunctions.leftPanel.animatePanel(false)
                })
            }
        }
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
        contentWidth: Math.max(flowLayout.width, flowLayout.childrenRect.width + (viewMargin * 2))
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

            function closeTool()
            {
                channelToolLoader.visible = false
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
