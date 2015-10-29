/*
  Q Light Controller Plus
  UniverseGridView.qml

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
import "FixtureUtils.js" as FxUtils

Flickable {
    id: universeGridView
    anchors.fill: parent
    anchors.margins: 20

    //contentWidth: destGrid.width
    //contentHeight: destGrid.height
    contentHeight: destGrid.height + uniText.height

    property int uniStartAddr: viewUniverseCombo.currentIndex * 512
    property int cellWidth: width / destGrid.columns

    function hasSettings()
    {
        return false;
    }

    RobotoText {
        id: uniText
        height: 45
        labelColor: "#ccc"
        label: viewUniverseCombo.currentText
        fontSize: 30
        fontBold: true
    }

    Grid {
        id: destGrid

        //anchors.fill: parent
        anchors.top: uniText.bottom
        width: parent.width
        //opacity: 0.5
        columns: 24

        Repeater {
            model: 512
            delegate: //DropTile { colorKey: "red" }
                DropArea {
                    id: dragTarget

                    //property string colorKey
                    property alias dropProxy: dragTarget

                    width: cellWidth
                    height: width

                    Rectangle {
                        id: dropRectangle

                        anchors.fill: parent
                        border.width: 1
                        border.color: "black"
                        color: FxUtils.getColorForAddress(index)

                        Text {
                            anchors.fill: parent
                            anchors.margins: 2
                            text: index + 1
                            font.pixelSize: height / 3
                        }

                        states: [
                            State {
                                when: dragTarget.containsDrag
                                PropertyChanges {
                                    target: dropRectangle
                                    color: "green"
                                }
                            }
                        ]
                    }
                }
        }
    }
}
