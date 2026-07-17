/*
  Q Light Controller Plus
  WidgetDragItem.qml

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
    x: 3
    id: widgetDragItem
    // when dragged, the item reduces to just the icon square so that it can
    // be centered on the mouse/finger position and dropped more intuitively
    width: reduced ? iconBox.width : parent.width
    height: reduced ? iconBox.height : UISettings.listItemHeight * 1.7
    color: "transparent"

    property string widgetName
    property string widgetType
    property string widgetIconName
    property bool reduced: Drag.active

    Row
    {
        spacing: 3
        Rectangle
        {
            id: iconBox
            radius: 3
            height: UISettings.listItemHeight * 1.7 - 4
            width: height
            gradient:
                Gradient
                {
                    id: bgGradient
                    GradientStop { position: 0.75 ; color: "#444" }
                    GradientStop { position: 1 ; color: "#222" }
                }
            border.width: 2
            border.color: UISettings.borderColorDark
            x: reduced ? 0 : 5
            y: reduced ? 0 : 2

            Image
            {
                id: wIcon
                anchors.fill: parent
                anchors.margins: 3
                source: widgetIconName ? "qrc:/" + widgetIconName + ".svg" : ""
                sourceSize: Qt.size(width, height)
                fillMode: Image.Stretch
            }
        }

        RobotoText
        {
            height: widgetDragItem.height
            width: widgetDragItem.width - iconBox.width
            label: widgetName
            wrapText: true
            visible: reduced ? false : true
        }
    }
}
