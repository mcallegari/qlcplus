/*
  Q Light Controller Plus
  FixtureDMXItem.qml

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

import QtQuick 2.2
import com.qlcplus.classes 1.0

Rectangle {
    property Fixture fixtureObj;

    width: channelsRow.width
    height: fxColumn.height
    color: "#777"
    border.width: 1
    border.color: "#aaa"
    radius: 3

    Column {
        id: fxColumn
        Rectangle {
            color: "#111"
            width: parent.width
            height: 20
            //radius: 3
            clip: true

            RobotoText {
                anchors.verticalCenter: parent.verticalCenter
                anchors.leftMargin: 2
                label: fixtureObj.name
                fontSize: 15
            }
        }
        Row {
            id: channelsRow
            Repeater {
                model: fixtureObj.channels
                delegate:
                    Rectangle {
                        color: "transparent"
                        border.width: 1
                        border.color: "#222"
                        width: 30
                        height: 50

                        Image {
                            x: 1
                            y: 1
                            width: 28
                            height: 28
                            source: if (fixtureObj) fixtureManager.channelIcon(fixtureObj.id, index)
                        }
                    }
            }
        }
    }
}
