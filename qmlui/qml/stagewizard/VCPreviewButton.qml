/*
  Q Light Controller Plus
  VCPreviewButton.qml  – tiny schematic button for the VC layout preview

  Copyright (c) Massimo Callegari

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0.txt
*/

import QtQuick
import "."

Rectangle
{
    id: previewButton
    property string label: ""
    property bool highlighted: false
    readonly property real hPad: 10

    // Size to the text: the label's painted width plus horizontal padding.
    implicitWidth: labelText.implicitWidth + hPad * 2
    width: implicitWidth
    height: UISettings.listItemHeight * 0.8
    radius: 5
    color: highlighted ? "#0550AA" : "#1A1A3A"
    border.color: highlighted ? "#0978FF" : "#333366"

    Text
    {
        id: labelText
        anchors.centerIn: parent
        text: previewButton.label
        font.family: UISettings.robotoFontName
        font.pixelSize: UISettings.textSizeDefault * 0.78
        color: previewButton.highlighted ? "white" : "#AAAACC"
    }
}
