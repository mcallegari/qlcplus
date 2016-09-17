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
import QtQuick.Controls 1.2
import QtQuick.Controls.Private 1.0

import com.qlcplus.classes 1.0
import "TimeUtils.js" as TimeUtils
import "."

Item
{
    id: itemRoot
    height: UISettings.mediumItemHeight
    y: trackIndex >= 0 ? iTrackHeight * trackIndex : 0
    z: 1

    property ShowFunction sfRef: null
    property Function funcRef: null
    property int startTime: sfRef ? sfRef.startTime : -1
    property int duration: sfRef ? sfRef.duration : -1
    property int trackIndex: -1
    property real timeScale: showManager.timeScale
    property bool isSelected: false
    property color globalColor: showManager.itemsColor
    property string infoText: ""

    /* this is a damn workaround cause apparently everybody in the ShowManager rounds this real value to int except for an Item. */
    property int iTrackHeight: UISettings.mediumItemHeight

    onStartTimeChanged: x = TimeUtils.timeToSize(startTime, timeScale)
    onDurationChanged: width = TimeUtils.timeToSize(duration, timeScale)
    onTimeScaleChanged:
    {
        x = TimeUtils.timeToSize(startTime, timeScale)
        width = TimeUtils.timeToSize(duration, timeScale)
    }

    onGlobalColorChanged:
    {
        if (isSelected && sfRef)
            sfRef.color = globalColor
    }

    /* Locker image */
    Image
    {
        x: Math.max(0, itemRoot.width - 25)
        y: itemRoot.height - 27
        z: 3
        source: "qrc:/lock.svg"
        sourceSize: Qt.size(24, 24)
        visible: sfRef ? (sfRef.locked ? true : false) : false
    }

    Canvas
    {
        id: prCanvas
        z: 2
        anchors.fill: parent

        onPaint:
        {
            if (sfRef === null || funcRef === null)
                return

            var previewData = showManager.previewData(funcRef)

            if (previewData === null || previewData === undefined)
                return

            var ctx = prCanvas.getContext('2d')

            ctx.strokeStyle = "#ddd"
            ctx.fillStyle = "transparent"
            ctx.lineWidth = 1

            ctx.beginPath()
            ctx.clearRect(0, 0, width, height)

            //console.log("About to paint " + previewData.length + " values")

            var lastTime = 0
            var xPos = 0

            if (previewData[0] === ShowManager.RepeatingDuration)
            {
                var loopCount = funcRef.totalDuration ? Math.floor(sfRef.duration / funcRef.totalDuration) : 0
                for (var l = 0; l < loopCount; l++)
                {
                    lastTime += previewData[1]
                    xPos = TimeUtils.timeToSize(lastTime, timeScale)
                    ctx.moveTo(xPos, 0)
                    ctx.lineTo(xPos, itemRoot.height)
                }
                ctx.stroke()
                return
            }

            for (var i = 0; i < previewData.length; i+=2)
            {
                if (i + 1 >= previewData.length)
                    break

                switch(previewData[i])
                {
                    case ShowManager.FadeIn:
                        var fiEnd = TimeUtils.timeToSize(lastTime + previewData[i + 1], timeScale)
                        ctx.moveTo(xPos, itemRoot.height)
                        ctx.lineTo(fiEnd, 0)
                    break;
                    case ShowManager.StepDivider:
                        lastTime = previewData[i + 1]
                        xPos = TimeUtils.timeToSize(lastTime, timeScale)
                        ctx.moveTo(xPos, 0)
                        ctx.lineTo(xPos, itemRoot.height)
                    break;
                    case ShowManager.FadeOut:
                        var foEnd = TimeUtils.timeToSize(lastTime + previewData[i + 1], timeScale)
                        ctx.moveTo(xPos, 0)
                        ctx.lineTo(foEnd, itemRoot.height)
                    break;
                }

            }
            ctx.stroke()
        }
    }

    /* Body mouse area (covers the whole item) */
    MouseArea
    {
        id: sfMouseArea
        anchors.fill: parent
        hoverEnabled: true

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
                fontSize: UISettings.textSizeDefault * 0.5
                wrapText: true
            }

            RobotoText
            {
                id: infoTextBox
                x: 3
                y: itemRoot.height - height
                width: 100
                height: 20
                fontSize: 9
                wrapText: true
                label: infoText
            }
        }

        onPressed:
        {
            if (sfRef && sfRef.locked)
                return;
            console.log("Show Item drag started")
            showManager.enableFlicking(false)
            drag.target = showItemBody
            itemRoot.z = 2
            infoTextBox.height = 20
        }
        onPositionChanged:
        {
            if (drag.target !== null)
            {
                infoText = qsTr("Position: ") + TimeUtils.msToString(TimeUtils.posToMs(itemRoot.x + showItemBody.x, timeScale))
            }
        }
        onReleased:
        {
            if (drag.target !== null)
            {
                console.log("Show item drag finished: " + showItemBody.x + " " + showItemBody.y);
                drag.target = null
                var newTime = TimeUtils.posToMs(itemRoot.x + showItemBody.x, timeScale)
                var newTrackIdx = parseInt((itemRoot.y + showItemBody.y) / height)
                if (newTime >= 0)
                {
                    var res = showManager.checkAndMoveItem(sfRef, trackIndex, newTrackIdx, newTime)

                    if (res === true)
                        trackIndex = newTrackIdx

                    prCanvas.requestPaint()
                }

                showItemBody.x = 0
                showItemBody.y = 0
                infoText = ""
            }
            itemRoot.z = 1
            showManager.enableFlicking(true)
        }

        onClicked:
        {
            itemRoot.isSelected = !itemRoot.isSelected
            showManager.setItemSelection(trackIndex, sfRef, itemRoot, itemRoot.isSelected)
        }

        onExited: Tooltip.hideText()
        onCanceled: Tooltip.hideText()

        Timer
        {
           interval: 1000
           running: sfMouseArea.containsMouse
           onTriggered:
           {
               var tooltip = funcRef ? funcRef.name + "\n" : "0"
               tooltip += qsTr("Position: ") + TimeUtils.msToString(TimeUtils.posToMs(itemRoot.x + showItemBody.x, timeScale))
               tooltip += "\n" + qsTr("Duration: ") + TimeUtils.msToString(TimeUtils.posToMs(itemRoot.width, timeScale))
               Tooltip.showText(sfMouseArea, Qt.point(sfMouseArea.mouseX, sfMouseArea.mouseY), tooltip)
           }
        }
    }

    /* horizontal left handler */
    Rectangle
    {
        id: horLeftHandler
        z: 1
        width: 10
        height: itemRoot.height
        color: horLeftHdlMa.containsMouse ? "#7FFFFF00" : "transparent"
        visible: sfRef ? (sfRef.locked ? false : true) : false

        MouseArea
        {
            id: horLeftHdlMa
            anchors.fill: parent
            preventStealing: true
            hoverEnabled: true
            cursorShape: containsMouse ? Qt.SizeHorCursor : Qt.ArrowCursor

            drag.target: horLeftHandler
            drag.axis: Drag.XAxis

            onPositionChanged:
            {
                if (drag.active == true)
                {
                    var hdlPos = mapToItem(itemRoot.parent, horLeftHandler.x, horLeftHandler.y)
                    itemRoot.width = itemRoot.width + (itemRoot.x - hdlPos.x + mouse.x)
                    itemRoot.x = hdlPos.x - mouse.x
                    infoTextBox.height = 40
                    infoText = qsTr("Position: ") + TimeUtils.msToString(TimeUtils.posToMs(itemRoot.x + showItemBody.x, timeScale))
                    infoText += "\n" + qsTr("Duration: ") + TimeUtils.msToString(TimeUtils.posToMs(itemRoot.width, timeScale))
                    horLeftHandler.x = 0
                }
            }
            onReleased:
            {
                if (drag.active == true)
                {
                    if (sfRef)
                    {
                        if (itemRoot.x < 0)
                        {
                            itemRoot.width += itemRoot.x
                            itemRoot.x = 0
                        }
                        sfRef.startTime = TimeUtils.posToMs(itemRoot.x, timeScale)
                        sfRef.duration = TimeUtils.posToMs(itemRoot.width, timeScale)
                        if (funcRef && showManager.stretchFunctions === true)
                            funcRef.totalDuration = sfRef.duration

                        prCanvas.requestPaint()
                    }
                    infoText = ""
                    horLeftHandler.x = 0
                }
            }
        }
    }

    /* horizontal right handler */
    Rectangle
    {
        id: horRightHandler
        x: itemRoot.width - 10
        z: 1
        width: 10
        height: itemRoot.height
        color: horRightHdlMa.containsMouse ? "#7FFFFF00" : "transparent"
        visible: sfRef ? (sfRef.locked ? false : true) : false

        MouseArea
        {
            id: horRightHdlMa
            anchors.fill: parent
            preventStealing: true
            hoverEnabled: true
            cursorShape: containsMouse ? Qt.SizeHorCursor : Qt.ArrowCursor

            drag.target: horRightHandler
            drag.axis: Drag.XAxis

            onPositionChanged:
            {
                //var mp = mapToItem(itemRoot, mouseX, mouseY)
                //console.log("Mouse position: " + mp.x)
                if (drag.active == true)
                {
                    infoTextBox.height = 20
                    var obj = mapToItem(itemRoot, mouseX, mouseY)
                    //console.log("Mapped position: " + obj.x)
                    itemRoot.width = obj.x + (horRightHdlMa.width - mouse.x)
                    infoText = qsTr("Duration: ") + TimeUtils.msToString(TimeUtils.posToMs(itemRoot.width, timeScale))
                }
            }
            onReleased:
            {
                if (drag.active == true)
                {
                    if (sfRef)
                    {
                        sfRef.duration = TimeUtils.posToMs(itemRoot.width, timeScale)
                        if (funcRef && showManager.stretchFunctions === true)
                            funcRef.totalDuration = sfRef.duration

                        prCanvas.requestPaint()
                    }
                    infoText = ""
                }
            }
        }
    }
}
