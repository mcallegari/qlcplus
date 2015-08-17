/*
  Q Light Controller Plus
  IORightPanel.qml

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

Rectangle
{
    id: rightSidePanel
    x: 0
    y: 0
    width: collapseWidth
    height: 500
    color: "#232323"
    z: 0

    property bool isOpen: false
    property int collapseWidth: 50
    property int expandedWidth: 400
    property string editorSource: ""
    property int universeIndex
    property bool showAudioButton: false
    property bool showPluginsButton: false

    onUniverseIndexChanged:
    {
        if (isOpen == true)
        {
            editorLoader.source = ""
            editorLoader.source = editorSource
        }
    }

    function animatePanel()
    {
        if (isOpen == false)
        {
            editorLoader.source = editorSource
            animateOpen.start()
            isOpen = true
        }
        else
        {
            animateClose.start()
            isOpen = false
            editorLoader.source = ""
        }
    }

    Rectangle
    {
        id: editorArea
        x: collapseWidth
        z: 5
        width: rightSidePanel.width - collapseWidth;
        height: parent.height
        color: "transparent"

        Loader
        {
            id: editorLoader
            anchors.fill: parent
            onLoaded:
            {
                item.universeIndex = universeIndex
                item.loadSources(false)
            }
        }
    }

    Rectangle
    {
        x: 3
        width: collapseWidth
        height: parent.height
        color: "#00000000"
        z: 2

        Column
        {
            anchors.fill: parent
            spacing: 3

            IconButton
            {
                id: audioOutputButton
                z: 2
                visible: showAudioButton
                width: collapseWidth - 4
                height: collapseWidth - 4
                imgSource: "qrc:/audiocard.svg"
                checkable: true
                tooltip: qsTr("Show the audio output sources")
                onToggled:
                {
                    editorSource = "qrc:///AudioCardsList.qml"
                    animatePanel();
                }
            }

            IconButton
            {
                id: uniOutputButton
                z: 2
                visible: showPluginsButton
                width: collapseWidth - 4
                height: collapseWidth - 4
                imgSource: "qrc:/inputoutput.svg"
                checkable: true
                tooltip: qsTr("Show the universe output sources")
                onToggled:
                {
                    editorSource = "qrc:///PluginsList.qml"
                    animatePanel();
                }
            }
        }
    }

    PropertyAnimation
    {
        id: animateOpen
        target: rightSidePanel
        properties: "width"
        to: expandedWidth
        duration: 200
    }

    PropertyAnimation
    {
        id: animateClose;
        target: rightSidePanel
        properties: "width"
        to: collapseWidth
        duration: 200
    }

    Rectangle
    {
        id: gradientBorder
        y: 0
        x: height
        height: collapseWidth
        color: "#141414"
        width: parent.height
        transformOrigin: Item.TopLeft
        rotation: 90
        gradient:
            Gradient
            {
                GradientStop { position: 0; color: "#141414" }
                GradientStop { position: 0.213; color: "#232323" }
                GradientStop { position: 0.79; color: "#232323" }
                GradientStop { position: 1; color: "#141414" }
            }

        MouseArea
        {
            id: rpClickArea
            anchors.fill: parent
            z: 1
            x: parent.width - width
            hoverEnabled: true
            cursorShape: pressed ? Qt.ClosedHandCursor : Qt.OpenHandCursor
            drag.target: rightSidePanel
            drag.axis: Drag.XAxis
            drag.minimumX: collapseWidth

            onPositionChanged:
            {
                if (drag.active == true)
                    rightSidePanel.width = rightSidePanel.parent.width - rightSidePanel.x
            }
            //onClicked: animatePanel()
        }
    }
}

