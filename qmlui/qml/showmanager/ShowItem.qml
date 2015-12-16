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
    z: 1

    property ShowFunction sfRef: null
    property Function funcRef: null
    property int startTime: sfRef ? sfRef.startTime : -1
    property int duration: sfRef ? sfRef.duration : -1
    property int trackIndex: -1
    property real timeScale: showManager.timeScale
    property bool isSelected: false

    onStartTimeChanged: x = TimeUtils.timeToSize(startTime, timeScale)
    onDurationChanged: width = TimeUtils.timeToSize(duration, timeScale)

    onTrackIndexChanged:
    {
        itemRoot.y = trackIndex * 80
    }

    MouseArea
    {
        id: sfMouseArea
        anchors.fill: parent

        drag.threshold: 30

        Rectangle
        {
            id: showItemBody
            width: itemRoot.width
            height: itemRoot.height
            color: sfRef ? sfRef.color : UISettings.bgLight
            border.width: isSelected ? 2 : 1
            border.color: isSelected ? UISettings.selection : "white"

            Drag.active: sfMouseArea.drag.active
            Drag.keys: [ "function" ]

            RobotoText
            {
                x: 3
                width: parent.width - 6
                label: funcRef ? funcRef.name : ""
                fontSize: 9
                wrapText: true
            }
        }

        onPressed:
        {
            showManager.enableFlicking(false)
            drag.target = showItemBody
            itemRoot.z = 2
        }
        onReleased:
        {
            if (drag.target !== null)
            {
                console.log("Show item drag finished: " + showItemBody.x + " " + showItemBody.y);
                drag.target = null
                var newTrackIdx = parseInt((itemRoot.y + showItemBody.y) / height)
                var res = showManager.checkAndMoveItem(sfRef, trackIndex, newTrackIdx,
                                                       TimeUtils.posToMs(itemRoot.x + showItemBody.x, timeScale))

                if (res === true)
                    trackIndex = newTrackIdx

                showItemBody.x = 0
                showItemBody.y = 0
            }
            itemRoot.z = 1
            showManager.enableFlicking(true)
        }

        onClicked: itemRoot.isSelected = !itemRoot.isSelected
    }
}
