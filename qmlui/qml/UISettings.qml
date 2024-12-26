/*
  Q Light Controller Plus
  UISettings.qml

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

pragma Singleton

import QtQuick 2.0

QtObject
{
    property string robotoFontName: "Roboto Condensed"

    property real scalingFactor: 1.0

    /* Colors */
    property color bgStronger:      "#161616"
    property color bgStrong:        "#232323"
    property color bgMedium:        "#333"
    property color bgControl:       "#555"
    property color bgLight:         "#6F6F6F"
    property color bgLighter:       "#8F8F8F"
    property color bgFixtureOdd:    "#414b41"
    property color bgFixtureEven:   "#42444b"

    property color fgMain:          "white"
    property color fgMedium:        "#888"
    property color fgLight:         "#aaa"

    property color sectionHeader:     "#31456B"
    property color sectionHeaderDiv:  "#22304a"
    property color highlight:         "#0978FF"
    property color highlightPressed:  "#044089"
    property color hover:             "#B6B6B6"
    property color selection:         "yellow"
    property color activeDropArea:    "#9DFF52"
    property color borderColorDark:   "#111"

    property color toolbarStartMain:  "#222"
    property color toolbarStartSub:   "#333"
    property color toolbarEnd:        "#111"
    property color toolbarHoverStart: "#444"
    property color toolbarHoverEnd:   "#171717"

    property color toolbarSelectionMain: "#12B4FF"
    property color toolbarSelectionSub:  "yellow"

    /* Sizes */
    property int  textSizeDefault:  screenPixelDensity * scalingFactor * 4.5
    property real iconSizeDefault:  screenPixelDensity * scalingFactor * 10 // more or less the size of a finger
    property real iconSizeMedium:   screenPixelDensity * scalingFactor * 8
    property real listItemHeight:   screenPixelDensity * scalingFactor * 7
    property real mediumItemHeight: screenPixelDensity * scalingFactor * 15
    property real bigItemHeight:    screenPixelDensity * scalingFactor * 25
    property real scrollBarWidth:   screenPixelDensity * scalingFactor * 6
    property real sidePanelWidth:   screenPixelDensity * scalingFactor * 50

    /* Channel properties column widths */
    property real chPropsModesWidth: bigItemHeight * 1.2
    property real chPropsFlagsWidth: bigItemHeight
    property real chPropsCanFadeWidth: bigItemHeight * 0.7
    property real chPropsPrecedenceWidth: bigItemHeight * 1.2
    property real chPropsModifierWidth: bigItemHeight
}
