/*
  Q Light Controller Plus
  ShowItem.qml

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

import com.qlcplus.classes 1.0
import "TimeUtils.js" as TimeUtils
import "."

Item
{
    id: itemRoot
    height: 80

    property ShowFunction sfRef: null
    property Function funcRef: null
    property int trackIndex: -1
    property real timeScale: showManager.timeScale
    property bool isSelected: false

    onSfRefChanged:
    {
        showItemBody.color = sfRef.color
        x = TimeUtils.timeToSize(sfRef.startTime, timeScale)
        width = TimeUtils.timeToSize(sfRef.duration, timeScale)

        console.log("Item x: " + x + " width: " + width)
    }

    onTrackIndexChanged:
    {
        itemRoot.y = trackIndex * 80
    }

    MouseArea
    {
        anchors.fill: parent

        drag.target: showItemBody
        drag.threshold: 30

        Rectangle
        {
            id: showItemBody
            anchors.fill: parent
            color: UISettings.bgLight
            border.width: isSelected ? 2 : 1
            border.color: isSelected ? "yellow" : "white"

            RobotoText
            {
                x: 3
                width: parent.width - 6
                label: funcRef ? funcRef.name : ""
                fontSize: 9
                wrapText: true
            }
        }
    }
}
