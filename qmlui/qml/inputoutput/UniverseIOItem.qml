/*
  Q Light Controller Plus
  UniverseIOItem.qml

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
    width: parent.width
    height: 140
    color: "transparent"
    border.width: 2
    border.color: "#666"

    property string universeName

    Rectangle {
        anchors.centerIn: parent
        width: 200
        height: 120
        radius: 5
        color: "#1C2255"
        border.width: 2
        border.color: "#444"

        RobotoText {
            height: parent.height
            width: parent.width
            label: universeName
            wrapText: true
        }
    }
}

