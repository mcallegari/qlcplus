/*
  Q Light Controller Plus
  2DView.qml

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

Flickable {
    id: twoDView
    anchors.fill: parent
    z: 1
    boundsBehavior: Flickable.StopAtBounds

    property int gridWidth: 8
    property int gridHeight: 6

    contentWidth: twoDContents.width * twoDContents.zoomFactor
    contentHeight: twoDContents.height * twoDContents.zoomFactor

    Rectangle {
        id: twoDContents
        objectName: "twoDView"
        //transformOrigin: Item.TopLeft
        x: 0; y: 0; z: 0
        width: parent.width
        height: parent.height
        color: "black"
        scale: zoomFactor

        property real zoomFactor: 1.0

        Grid {
            id: twoDGrid
            anchors.horizontalCenter: parent.horizontalCenter
            columns: twoDView.gridWidth

            Repeater {
                model: twoDView.gridWidth * twoDView.gridHeight
                delegate:
                    Rectangle {
                        width: {
                            if (twoDView.width / twoDView.gridWidth < twoDView.height / twoDView.gridHeight)
                                twoDView.width / twoDView.gridWidth
                            else
                                twoDView.height / twoDView.gridHeight
                        }
                        height: width
                        color: "transparent"
                        border.width: 1
                        border.color: "#111"
                    }
            }
        }

        MouseArea {
            anchors.fill: parent
            onWheel: {
                if (wheel.modifiers & Qt.ControlModifier) {
                    if (wheel.angleDelta.y > 0)
                        twoDContents.zoomFactor += 0.1;
                    else if (twoDContents.zoomFactor - 0.1 >= 0.1)
                        twoDContents.zoomFactor -= 0.1;
                }
            }
        }
        DropArea {
            anchors.fill: parent

        }
    }
}
