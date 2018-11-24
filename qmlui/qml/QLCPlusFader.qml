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
import QtQuick.Controls 2.2

import "."

Slider
{
    id: slider
    width: 32
    height: 100
    orientation: Qt.Vertical
    from: 0
    to: 255
    stepSize: 1.0
    wheelEnabled: true

    property Gradient handleGradient: defaultGradient
    property Gradient handleGradientHover: defaultGradientHover
    property color trackColor: defaultTrackColor

    property color defaultTrackColor: "#38b0ff"
    property Gradient defaultGradient:
        Gradient
        {
            GradientStop { position: 0; color: "#ccc" }
            GradientStop { position: 0.45; color: "#555" }
            GradientStop { position: 0.50; color: "#000" }
            GradientStop { position: 0.55; color: "#555" }
            GradientStop { position: 1.0; color: "#888" }
        }

    property Gradient defaultGradientHover:
        Gradient
        {
            GradientStop { position: 0; color: "#eee" }
            GradientStop { position: 0.45; color: "#999" }
            GradientStop { position: 0.50; color: "red" }
            GradientStop { position: 0.55; color: "#999" }
            GradientStop { position: 1.0; color: "#ccc" }
        }

    background:
        Rectangle
        {
            y: slider.leftPadding
            x: slider.topPadding + slider.availableWidth / 2 - width / 2
            //implicitWidth: 5
            implicitHeight: slider.height
            width: 5
            height: slider.availableHeight
            radius: 2
            color: trackColor

            Rectangle
            {
                width: parent.width
                height: slider.visualPosition * parent.height
                color: "#bdbebf"
                radius: 2
            }
        }

    handle:
        Rectangle
        {
            y: slider.leftPadding + slider.visualPosition * (slider.availableHeight - height)
            x: slider.topPadding + slider.availableWidth / 2 - width / 2
            implicitHeight: Math.min(slider.width, UISettings.iconSizeDefault * 0.75)
            implicitWidth: Math.min(UISettings.iconSizeDefault, slider.width)
            gradient: pressed ? handleGradientHover : handleGradient
            border.color: "#5c5c5c"
            border.width: 1
            radius: 4
        }
}
