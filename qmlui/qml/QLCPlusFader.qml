/*
  Q Light Controller Plus
  QLCPlusFader.qml

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
import QtQuick.Controls 1.0
import QtQuick.Controls.Styles 1.0

Slider
{
    id: slider
    width: 32
    height: 100
    orientation: Qt.Vertical
    minimumValue: 0
    maximumValue: 255
    stepSize: 1.0

    Gradient
    {
        id: handleGradient
        GradientStop { position: 0; color: "#ccc" }
        GradientStop { position: 0.45; color: "#555" }
        GradientStop { position: 0.50; color: "#000" }
        GradientStop { position: 0.55; color: "#555" }
        GradientStop { position: 1.0; color: "#888" }
    }

    Gradient
    {
        id: handleGradientHover
        GradientStop { position: 0; color: "#eee" }
        GradientStop { position: 0.45; color: "#999" }
        GradientStop { position: 0.50; color: "red" }
        GradientStop { position: 0.55; color: "#999" }
        GradientStop { position: 1.0; color: "#ccc" }
    }

    style: SliderStyle
        {
        //groove: Rectangle { color: "transparent" }
        handle:
            Rectangle
            {
                anchors.centerIn: parent
                color: "transparent"
                implicitWidth: 30
                implicitHeight: 32

                Rectangle
                {
                    anchors.fill: parent
                    rotation: 90
                    gradient: control.pressed ? handleGradientHover : handleGradient
                    border.color: "#5c5c5c"
                    border.width: 1
                    radius: 4
                }
    /*
                Image {
                    rotation: 90
                    source: "qrc:/faderhandle.svg"
                    sourceSize: Qt.size(parent.width, parent.height)
                }
    */
            }
        }
}
