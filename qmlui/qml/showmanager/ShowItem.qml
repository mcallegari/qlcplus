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

import org.qlcplus.classes 1.0
import "TimeUtils.js" as TimeUtils
import "."

Item
{
    id: itemRoot
    height: UISettings.mediumItemHeight
    y: trackIndex >= 0 ? parseInt(height) * trackIndex : 0
    z: 2

    property ShowFunction sfRef: null
    property QLCFunction funcRef: null
    property int startTime: sfRef ? sfRef.startTime : -1
    property int duration: sfRef ? sfRef.duration : -1
    property int trackIndex: -1
    property real timeScale: showManager.timeScale
    property real tickSize: showManager.tickSize
    property bool isSelected: false
    property color globalColor: showManager.itemsColor
    property string infoText: ""

    onStartTimeChanged: x = TimeUtils.timeToSize(startTime, timeScale, tickSize)
    onDurationChanged: width = TimeUtils.timeToSize(duration, timeScale, tickSize)
    onTimeScaleChanged:
    {
        x = TimeUtils.timeToSize(startTime, timeScale, tickSize)
        width = TimeUtils.timeToSize(duration, timeScale, tickSize)
    }

    onGlobalColorChanged:
    {
        if (isSelected && sfRef)
            sfRef.color = globalColor
    }

    /* Locker image */
    Image
    {
        x: Math.max(0, itemRoot.width - width - 1)
        y: itemRoot.height - height - 3
        z: 4
        width: itemRoot.height / 3
        height: width
        source: "qrc:/lock.svg"
        sourceSize: Qt.size(width, height)
        visible: sfRef ? (sfRef.locked ? true : false) : false
    }

    Canvas
    {
        id: prCanvas
        z: 3
        anchors.fill: parent
        contextType: "2d"

        onPaint:
        {
            if (sfRef === null || funcRef === null)
                return

            var previewData = showManager.previewData(funcRef)

            if (previewData === null || previewData === undefined)
                return

            context.strokeStyle = "#ddd"
            context.fillStyle = "transparent"
            context.lineWidth = 1

            context.beginPath()
            context.clearRect(0, 0, width, height)

            //console.log("About to paint " + previewData.length + " values")

            var lastTime = 0
            var xPos = 0

            if (previewData[0] === ShowManager.RepeatingDuration)
            {
                var loopCount = funcRef.totalDuration ? Math.floor(sfRef.duration / funcRef.totalDuration) : 0
                for (var l = 0; l < loopCount; l++)
                {
                    lastTime += previewData[1]
                    xPos = TimeUtils.timeToSize(lastTime, timeScale, tickSize)
                    context.moveTo(xPos, 0)
                    context.lineTo(xPos, itemRoot.height)
                }
                context.stroke()
                return
            }

            for (var i = 0; i < previewData.length; i+=2)
            {
                if (i + 1 >= previewData.length)
                    break

                switch(previewData[i])
                {
                    case ShowManager.FadeIn:
                        var fiEnd = TimeUtils.timeToSize(lastTime + previewData[i + 1], timeScale, tickSize)
                        context.moveTo(xPos, itemRoot.height)
                        context.lineTo(fiEnd, 0)
                    break;
                    case ShowManager.StepDivider:
                        lastTime = previewData[i + 1]
                        xPos = TimeUtils.timeToSize(lastTime, timeScale, tickSize)
                        context.moveTo(xPos, 0)
                        context.lineTo(xPos, itemRoot.height)
                    break;
                    case ShowManager.FadeOut:
                        var foEnd = TimeUtils.timeToSize(lastTime + previewData[i + 1], timeScale, tickSize)
                        context.moveTo(xPos, 0)
                        context.lineTo(foEnd, itemRoot.height)
                    break;
                }

            }
            context.stroke()
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
            clip: true

            Drag.active: sfMouseArea.drag.active
            Drag.keys: [ "function" ]

            Image
            {
                x: 3
                y: itemRoot.height - height - 3
                visible: infoText ? false : true
                width: itemRoot.height / 3
                height: width
                source: funcRef ? functionManager.functionIcon(funcRef.type) : ""
                sourceSize: Qt.size(width, height)
            }

            RobotoText
            {
                x: 3
                y: 3
                width: parent.width - 6
                height: parent.height - 6
                label: funcRef ? funcRef.name : ""
                fontSize: UISettings.textSizeDefault * 0.7
                textVAlign: Text.AlignTop
                wrapText: true
            }

            RobotoText
            {
                id: infoTextBox
                x: 3
                y: itemRoot.height - height - 3
                width: itemRoot.width - 6
                height: itemRoot.height / 4
                fontSize: UISettings.textSizeDefault * 0.6
                textHAlign: Text.AlignLeft
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
            itemRoot.z++
            infoTextBox.height = itemRoot.height / 4
            infoTextBox.textHAlign = Text.AlignLeft
        }
        onPositionChanged:
        {
            if (drag.target !== null)
            {
                infoText = qsTr("Position: ") + TimeUtils.msToString(TimeUtils.posToMs(itemRoot.x + showItemBody.x, timeScale, tickSize))
            }
        }
        onReleased:
        {
            if (sfRef && sfRef.locked)
                return;

            if (drag.target !== null)
            {
                console.log("Show item drag finished: " + showItemBody.x + " " + showItemBody.y)
                drag.target = null
                infoText = ""

                var newTime = TimeUtils.posToMs(itemRoot.x + showItemBody.x, timeScale, tickSize)
                var newTrackIdx = Math.round((itemRoot.y + showItemBody.y) / itemRoot.height)

                // dragging to 0 might not be accurate...
                if (newTime < 0)
                    newTime = 0

                if (newTrackIdx >= 0)
                {
                    var res = showManager.checkAndMoveItem(sfRef, trackIndex, newTrackIdx, newTime)

                    if (res === true)
                        trackIndex = newTrackIdx

                    prCanvas.requestPaint()
                }

                showItemBody.x = 0
                showItemBody.y = 0
            }
            itemRoot.z--
            showManager.enableFlicking(true)
        }

        onClicked:
        {
            itemRoot.isSelected = !itemRoot.isSelected
            showManager.setItemSelection(trackIndex, sfRef, itemRoot, itemRoot.isSelected)
        }

        onDoubleClicked: functionManager.setEditorFunction(sfRef.functionID, true, false)

        onExited: Tooltip.hideText()
        onCanceled: Tooltip.hideText()

        Timer
        {
           interval: 1000
           running: sfMouseArea.containsMouse
           onTriggered:
           {
               var tooltip = funcRef ? funcRef.name + "\n" : "0"
               tooltip += qsTr("Position: ") + TimeUtils.msToString(TimeUtils.posToMs(itemRoot.x + showItemBody.x, timeScale, tickSize))
               tooltip += "\n" + qsTr("Duration: ") + TimeUtils.msToString(TimeUtils.posToMs(itemRoot.width, timeScale, tickSize))
               Tooltip.showText(sfMouseArea, Qt.point(sfMouseArea.mouseX, sfMouseArea.mouseY), tooltip)
           }
        }
    }

    /* horizontal left handler */
    Rectangle
    {
        id: horLeftHandler
        z: 2
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
            drag.maximumX: horRightHandler.x

            onPositionChanged:
            {
                if (drag.active == true)
                {
                    var hdlPos = mapToItem(itemRoot.parent, horLeftHandler.x, horLeftHandler.y)
                    itemRoot.width = itemRoot.width + (itemRoot.x - hdlPos.x + mouse.x)
                    itemRoot.x = hdlPos.x - mouse.x
                    infoTextBox.height = itemRoot.height / 2
                    infoTextBox.textHAlign = Text.AlignLeft
                    infoText = qsTr("Position: ") + TimeUtils.msToString(TimeUtils.posToMs(itemRoot.x + showItemBody.x, timeScale, tickSize))
                    infoText += "\n" + qsTr("Duration: ") + TimeUtils.msToString(TimeUtils.posToMs(itemRoot.width, timeScale, tickSize))
                    horLeftHandler.x = 0
                }
            }
            onReleased:
            {
                if (drag.active == false)
                    return

                if (sfRef)
                {
                    if (itemRoot.x < 0)
                    {
                        itemRoot.width += itemRoot.x
                        itemRoot.x = 0
                    }

                    // check grid snapping
                    if (itemRoot.x && showManager.gridEnabled)
                    {
                        var currX = itemRoot.x
                        itemRoot.x = Math.round(itemRoot.x / tickSize) * tickSize
                        itemRoot.width += (currX - itemRoot.x)
                    }

                    sfRef.startTime = TimeUtils.posToMs(itemRoot.x, timeScale, tickSize)
                    sfRef.duration = TimeUtils.posToMs(itemRoot.width, timeScale, tickSize)
                    if (funcRef && showManager.stretchFunctions === true)
                        funcRef.totalDuration = sfRef.duration

                    prCanvas.requestPaint()
                }
                infoText = ""
                horLeftHandler.x = 0
            }
        }
    }

    /* horizontal right handler */
    Rectangle
    {
        id: horRightHandler
        x: itemRoot.width - 10
        z: 2
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
            drag.minimumX: horLeftHandler.x + width

            onPositionChanged:
            {
                //var mp = mapToItem(itemRoot, mouseX, mouseY)
                //console.log("Mouse position: " + mp.x)
                if (drag.active == true)
                {
                    var obj = mapToItem(itemRoot, mouseX, mouseY)
                    //console.log("Mapped position: " + obj.x)
                    itemRoot.width = obj.x + (horRightHdlMa.width - mouse.x)
                    infoTextBox.height = itemRoot.height / 4
                    infoTextBox.textHAlign = Text.AlignRight
                    infoText = qsTr("Duration: ") + TimeUtils.msToString(TimeUtils.posToMs(itemRoot.width, timeScale, tickSize))
                }
            }
            onReleased:
            {
                if (drag.active == false)
                    return

                if (sfRef)
                {
                    // check grid snapping
                    if (showManager.gridEnabled)
                    {
                        var snappedEndPos = Math.round((itemRoot.x + itemRoot.width) / tickSize) * tickSize
                        itemRoot.width = snappedEndPos - itemRoot.x
                    }

                    sfRef.duration = TimeUtils.posToMs(itemRoot.width, timeScale, tickSize)
                    if (funcRef && showManager.stretchFunctions === true)
                        funcRef.totalDuration = sfRef.duration

                    prCanvas.requestPaint()
                }
                infoText = ""
            }
        }
    }
}
