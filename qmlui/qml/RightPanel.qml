/*
  Q Light Controller Plus
  RightPanel.qml

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
    id: rightSidePanel
    property bool isOpen: false
    property int collapseWidth: 50
    property int expandedWidth: 450
    x: 0
    y: 0
    width: collapseWidth
    height: 500
    color: "#232323"
    z: 0

    function animatePanel() {
        if (rightSidePanel.isOpen == false)
        {
            animateOpen.start();
            rightSidePanel.isOpen = true;
        }
        else
        {
            animateClose.start();
            rightSidePanel.isOpen = false;
        }
    }

    Rectangle {
        x: 3
        width: collapseWidth
        height: parent.height
        color: "#00000000"
        z: 2

        Column {
            anchors.fill: parent
            spacing: 3

            IconButton {
                id: funcEditor
                z: 2
                width: collapseWidth - 4
                height: collapseWidth - 4
                imgSource: "qrc:/functions.svg"
                isCheckable: true
                onChecked: {
                    animatePanel();
                }
            }
        }
    }

    PropertyAnimation {
        id: animateOpen;
        target: rightSidePanel;
        properties: "width";
        to: expandedWidth;
        duration: 200
    }

    PropertyAnimation {
        id: animateClose;
        target: rightSidePanel;
        properties: "width";
        to: collapseWidth;
        duration: 200
    }

    MouseArea {
        id: clickArea
        width: collapseWidth
        height: parent.height
        z: 1
        x: 0
        hoverEnabled: true
        cursorShape: Qt.OpenHandCursor
        onClicked: animatePanel("")
    }

    Rectangle {
        id: gradientBorder
        y: 0
        x: height
        height: collapseWidth
        color: "#141414"
        width: parent.height
        transformOrigin: Item.TopLeft
        rotation: 90
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
    }
}

