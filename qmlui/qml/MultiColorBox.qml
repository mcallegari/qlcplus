/*
  Q Light Controller Plus
  MultiColorBox.qml

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

import QtQuick

import "."

Rectangle
{
    id: boxRoot
    width: UISettings.mediumItemHeight
    height: UISettings.listItemHeight

    property bool biColor: false

    // Primary color: always active. Represents the current RGB value
    property color primary: "black"

    // Secondary color:
    // biColor = false -> represents the current WAUV color
    // biColor = true -> represents the secondary color of a color wheel
    property color secondary: "black"

    // Computed color for additive mixing (when biColor is false)
    property color mixedColor: {
        if (biColor)
            return primary

        var r = primary.r
        var g = primary.g
        var b = primary.b

        // Add White (brightens all channels equally)
        if (secondary.r > 0) {
            r = Math.min(1.0, r + secondary.r)
            g = Math.min(1.0, g + secondary.r)
            b = Math.min(1.0, b + secondary.r)
        }

        // Add Amber (R=1.0, G=0.49, B=0.0)
        if (secondary.g > 0) {
            r = Math.min(1.0, r + secondary.g * 1.0)
            g = Math.min(1.0, g + secondary.g * 0.49)
            b = Math.min(1.0, b + secondary.g * 0.0)
        }

        // Add UV (R=0.58, G=0.0, B=0.83)
        if (secondary.b > 0) {
            r = Math.min(1.0, r + secondary.b * 0.58)
            g = Math.min(1.0, g + secondary.b * 0.0)
            b = Math.min(1.0, b + secondary.b * 0.83)
        }

        // Add Lime (R=0.68, G=1.0, B=0.18) - secondary doesn't have this channel,
        // so this is for future use if ColorTool is extended
        // Add Indigo (R=0.29, G=0.0, B=0.51) - secondary doesn't have this channel,
        // so this is for future use if ColorTool is extended

        return Qt.rgba(r, g, b, 1.0)
    }

    color: mixedColor

    Rectangle
    {
        visible: biColor ? true : false
        width: parent.width / 2
        height: parent.height
        x: parent.width / 2
        clip: true
        color: "transparent"

        Rectangle
        {
            width: boxRoot.width
            height: parent.height
            x: -width / 2
            radius: boxRoot.radius
            color: secondary
        }
    }
}
