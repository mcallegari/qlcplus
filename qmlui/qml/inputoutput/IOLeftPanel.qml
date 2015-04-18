/*
  Q Light Controller Plus
  IOLeftPanel.qml

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

Rectangle {
    id: leftSidePanel
    x: 0
    y: 0
    anchors.left: parent.left;
    anchors.leftMargin: 0
    width: collapseWidth
    height: parent.height
    color: "#232323"
    z: 0

    property bool isOpen: false
    property int collapseWidth: 50
    property int expandedWidth: 400
    property string editorSource: ""
    property int universeIndex
    property bool showAudioButton: false
    property bool showPluginsButton: false

    function animatePanel() {
        if (leftSidePanel.isOpen == false)
        {
            editorLoader.source = editorSource;
            animateOpen.start();
            leftSidePanel.isOpen = true;
        }
        else
        {
            animateClose.start();
            leftSidePanel.isOpen = false;
        }
    }

    Rectangle {
        id: editorArea
        width: leftSidePanel.width - collapseWidth;
        height: parent.height
        color: "transparent"

        Loader {
            id: editorLoader
            anchors.fill: parent
            onLoaded: {
                item.loadSources(true)
            }
        }
    }

    Rectangle {
        id: sideBar
        x: parent.width - collapseWidth
        width: collapseWidth
        height: parent.height
        color: "#00000000"
        z: 2

        Column {
            anchors.fill: parent
            anchors.leftMargin: 1
            spacing: 3

            IconButton {
                id: audioInputButton
                z: 2
                visible: showAudioButton
                width: collapseWidth - 4
                height: collapseWidth - 4
                imgSource: "qrc:/audiocard.svg"
                checkable: true
                tooltip: qsTr("Show the audio input sources")
                onToggled: {
                    editorSource = "qrc:///AudioCardsList.qml"
                    animatePanel();
                }
            }

            IconButton {
                id: uniInputButton
                z: 2
                visible: showPluginsButton
                width: collapseWidth - 4
                height: collapseWidth - 4
                imgSource: "qrc:/inputoutput.svg"
                checkable: true
                tooltip: qsTr("Show the universe input sources")
                onToggled: {
                    editorSource = "qrc:///PluginsList.qml"
                    animatePanel();
                }
            }
        }
    }

    PropertyAnimation {
        id: animateOpen;
        target: leftSidePanel;
        properties: "width";
        to: expandedWidth;
        duration: 200
    }

    PropertyAnimation {
        id: animateClose;
        target: leftSidePanel;
        properties: "width";
        to: collapseWidth;
        duration: 200

        onRunningChanged: {
            if (!animateClose.running)
                editorLoader.source = "";
        }
    }

    Rectangle {
        id: gradientBorder
        y: width
        x: parent.width - height
        height: collapseWidth
        color: "#141414"
        width: parent.height
        transformOrigin: Item.TopLeft
        rotation: 270
        gradient: Gradient {

            GradientStop {
                position: 0
                color: "#141414"
            }

            GradientStop {
                position: 0.213
                color: "#232323"
            }

            GradientStop {
                position: 0.79
                color: "#232323"
            }

            GradientStop {
                position: 1
                color: "#141414"
            }
        }

        MouseArea {
            id: lpClickArea
            anchors.fill: parent
            z: 1
            x: parent.width - width
            hoverEnabled: true
            cursorShape: Qt.OpenHandCursor
            drag.target: leftSidePanel
            drag.axis: Drag.XAxis
            drag.minimumX: collapseWidth

            onPositionChanged: {
                if (drag.active == true) {
                    var obj = mapToItem(null, mouseX, mouseY);
                    leftSidePanel.width = obj.x + (collapseWidth / 2);
                    //console.log("mouseX:", mouseX, "mapToItem().x:", obj.x);
                }
            }

            //onClicked: animatePanel("")
        }
    }
}
