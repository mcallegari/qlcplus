/*
  Q Light Controller Plus
  PluginDragItem.qml

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

import "GenericHelpers.js" as Helpers
import "."

Rectangle
{
    x: 3
    id: pluginDragItem
    width: parent.width
    height: UISettings.listItemHeight * 1.7
    color: "transparent"

    property int pluginUniverse
    property string pluginName
    property string pluginLine
    property string lineName

    Row
    {
        spacing: 3
        Rectangle
        {
            radius: 3
            height: pluginDragItem.height - 4
            width: height
            gradient:
                Gradient
                {
                    id: bgGradient
                    GradientStop { position: 0.75 ; color: UISettings.fgMain }
                    GradientStop { position: 1 ; color: UISettings.fgMedium }
                }
            border.width: 2
            border.color: "#777"
            x: 5
            y: 2

            Image
            {
                id: pIcon
                anchors.fill: parent
                anchors.margins: 3
                source: Helpers.pluginIconFromName(pluginName)
                sourceSize: Qt.size(width, height)
                fillMode: Image.Stretch
            }
        }

        RobotoText
        {
            height: pluginDragItem.height
            width: pluginDragItem.width - pIcon.width
            label: lineName
            wrapText: true
        }
    }
}
