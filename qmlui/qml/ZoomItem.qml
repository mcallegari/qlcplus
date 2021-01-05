/*
  Q Light Controller Plus
  ZoomItem.qml

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

import "."

Rectangle
{
    id: itemRoot
    implicitWidth: UISettings.iconSizeDefault * 2
    implicitHeight: UISettings.iconSizeDefault

    color: UISettings.bgLight

    border.color: "#1D1D1D"
    border.width: 2
    radius: 5
    clip: true

    property color fontColor: "white"

    signal zoomInClicked
    signal zoomOutClicked

    Rectangle
    {
        width: parent.width / 2
        height: parent.height
        color: zoMouseArea.pressed ? UISettings.highlight :
                                     (zoMouseArea.containsMouse ? UISettings.bgLighter : "transparent")
        radius: 5

        Text
        {
            x: 4
            y: 2
            color: fontColor
            font.family: "FontAwesome"
            font.pixelSize: parent.height - 6
            text: FontAwesome.fa_search_minus
        }
        MouseArea
        {
            id: zoMouseArea
            anchors.fill: parent
            hoverEnabled: true
            onClicked: itemRoot.zoomOutClicked()
        }
    }

    // vertical divider
    Rectangle
    {
        x: (parent.width / 2) - 2
        z: 2
        width: 4
        height: parent.height
        color: fontColor
    }

    Rectangle
    {
        x: parent.width / 2
        width: parent.width / 2
        height: parent.height
        color: ziMouseArea.pressed ? UISettings.highlight :
                                     (ziMouseArea.containsMouse ? UISettings.bgLighter : "transparent")
        radius: 5

        Text
        {
            x: parent.width - width - 4
            y: 2
            color: fontColor
            font.family: "FontAwesome"
            font.pixelSize: parent.height - 6
            text: FontAwesome.fa_search_plus
        }
        MouseArea
        {
            id: ziMouseArea
            anchors.fill: parent
            hoverEnabled: true
            onClicked: itemRoot.zoomInClicked()
        }
    }
}
