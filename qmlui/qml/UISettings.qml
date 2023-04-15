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

    /* Colors */
    property color bgMain:     "#211F22"
    property color bgStronger: "#211F22"
    property color bgStrong:   "#211F22"
    property color bgMedium:   "#211F22"
    property color bgControl:  "#211F22"
    property color bgLight:    "#2C292E"
    property color bgLighter:  "#2C292E"

    property color fgMain:     "#d6d6d6"
    property color fgMedium:   "#d6d6d6"
    property color fgLight:    "#d6d6d6"

    property color sectionHeader:    "#224458"
    property color sectionHeaderDiv: "#224458"
    property color highlight:        "#d6d6d6"
    property color highlightPressed: "#d6d6d6"
    property color hover:            "#B6B6B6"
    property color selection:        "#ffd42a"
    property color activeDropArea:   "#9DFF52"
    property color borderColorDark:  "#2C292E"

    property color toolbarStartMain: "#2C292E"
    property color toolbarStartSub:  "#2C292E"
    property color toolbarEnd:       "#2C292E"
    property color toolbarHoverStart:"#2C292E"
    property color toolbarHoverEnd:  "#2C292E"

    property color toolbarSelectionMain: "#78DCE8"
    property color toolbarSelectionSub:  "#ffd42a"

    /* Sizes */
    property int  textSizeDefault:  screenPixelDensity * 4.5
    property real iconSizeDefault:  screenPixelDensity * 10 // more or less the size of a finger
    property real iconSizeMedium:   screenPixelDensity * 8
    property real listItemHeight:   screenPixelDensity * 7
    property real mediumItemHeight: screenPixelDensity * 15
    property real bigItemHeight:    screenPixelDensity * 25
    property real scrollBarWidth:   screenPixelDensity * 6
    property real sidePanelWidth:   350

    // channel properties column widths
    property real chPropsModesWidth: bigItemHeight * 1.2
    property real chPropsFlagsWidth: bigItemHeight
    property real chPropsCanFadeWidth: bigItemHeight * 0.7
    property real chPropsPrecedenceWidth: bigItemHeight * 1.2
    property real chPropsModifierWidth: bigItemHeight
}
