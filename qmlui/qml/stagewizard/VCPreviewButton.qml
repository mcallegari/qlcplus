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
    property string label: ""

    width: Math.max(labelText.implicitWidth + 12, 60)
    height: UISettings.listItemHeight * 0.8
    radius: 5
    color: "#1A1A3A"
    border.color: "#333366"

    RobotoText
    {
        id: labelText
        anchors.centerIn: parent
        label: parent.label
        fontSize: UISettings.textSizeDefault * 0.78
        labelColor: "#AAAACC"
    }
}
