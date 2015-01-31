/*
  Q Light Controller Plus
  FunctionDelegate.qml

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
    id: funcDelegate
    width: 100
    height: 35

    color: "transparent"

    property string textLabel
    property int functionID
    property int functionType

    signal clicked

    IconTextEntry {
        width: parent.width
        height: parent.height
        iSrc: {
            switch (functionType)
            {
                case 1: "qrc:/scene.svg"; break;
                case 2: "qrc:/chaser.svg"; break;
                case 4: "qrc:/efx.svg"; break;
                case 8: "qrc:/collection.svg"; break;
                case 16: "qrc:/script.svg"; break;
                case 32: "qrc:/rgbmatrix.svg"; break;
                case 64: "qrc:/show.svg"; break;
            }
        }

        tLabel: textLabel
    }
    MouseArea {
        anchors.fill: parent
        onClicked: funcDelegate.clicked()
    }
}

