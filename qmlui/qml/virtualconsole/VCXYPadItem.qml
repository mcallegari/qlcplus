/*
  Q Light Controller Plus
  VCXYPadItem.qml

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
import QtQuick.Layouts
import QtQuick.Controls

import org.qlcplus.classes 1.0
import "."

VCWidgetItem
{
    id: xyPadRoot
    property VCXYPad xyPadObj: null

    clip: true

    onXyPadObjChanged:
    {
        setCommonProperties(xyPadObj)
    }

    GridLayout
    {
        id: itemsLayout
        anchors.fill: parent
        rowSpacing: 0
        columnSpacing: 0
        columns: 3

        // row 1
        Rectangle
        {
            height: UISettings.listItemHeight
            width: height
            color: "transparent"
        }

        CustomRangeSlider
        {
            Layout.fillWidth: true
            topPadding: 0
            bottomPadding: 0
        }

        Rectangle
        {
            height: UISettings.listItemHeight
            width: height
            color: "transparent"
        }

        // row 2
        CustomRangeSlider
        {
            Layout.fillHeight: true
            rightPadding: 0
            orientation: Qt.Vertical
        }

        Rectangle
        {
            Layout.fillHeight: true
            Layout.fillWidth: true
            color: UISettings.bgStrong
        }

        CustomSlider
        {
            Layout.fillHeight: true
            orientation: Qt.Vertical
        }

        // row 3
        Rectangle
        {
            height: UISettings.listItemHeight
            width: height
            color: "transparent"
        }

        CustomSlider
        {
            Layout.fillWidth: true
        }

        Rectangle
        {
            height: UISettings.listItemHeight
            width: height
            color: "transparent"
        }
    }
}
