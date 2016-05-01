/*
  Q Light Controller Plus
  VCClockItem.qml

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

VCWidgetItem
{
    id: clockRoot
    property VCClock clockObj: null
    property var locale: Qt.locale()
    property string timeString: new Date().toLocaleString(locale, "hh:mm:ss");
    property int clockType: clockObj ? clockObj.clockType : VCClock.Clock
    property int targetTime: clockObj ? clockObj.targetTime : 0
    property int currentTime: 0

    clip: true

    onClockObjChanged:
    {
        setCommonProperties(clockObj)
        currentTime = clockObj.targetTime
        updateTime(false)
    }

    onTargetTimeChanged:
    {
        if (clockTimer.running == false)
            currentTime = targetTime
        updateTime(false)
    }

    onClockTypeChanged:
    {
        switch(clockType)
        {
            case VCClock.Stopwatch:
                clockTimer.interval = 100
                clockTimer.running = false
            break;
            case VCClock.Countdown:
                clockTimer.interval = 100
                clockTimer.running = false
            break;
            case VCClock.Clock:
                clockTimer.interval = 1000
                clockTimer.running = true
            break;
        }
        updateTime(false)
    }

    function updateTime(modify)
    {
        switch(clockType)
        {
        case VCClock.Stopwatch:
            if (modify)
                currentTime += 100
            timeString = TimeUtils.msToStringWithPrecision(currentTime, 1)
        break;
        case VCClock.Countdown:
            if (modify)
                currentTime -= 100
            timeString = TimeUtils.msToStringWithPrecision(currentTime, 1)
            if (currentTime == 0)
                clockTimer.running = false
        break;
        case VCClock.Clock:
            timeString = new Date().toLocaleString(locale, "hh:mm:ss");
            clockTimer.running = true
        break;
        }
    }

    Text
    {
        x: 2
        width: parent.width - 4
        height: parent.height
        font: clockObj ? clockObj.font : ""
        text: timeString
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignHCenter
        wrapMode: Text.Wrap
        lineHeight: 0.8
        color: clockObj ? clockObj.foregroundColor : "#111"
    }

    MouseArea
    {
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        onClicked:
        {
            if (clockType == VCClock.Stopwatch || clockType == VCClock.Countdown)
            {
                if (mouse.button === Qt.LeftButton)
                {
                    clockTimer.running = !clockTimer.running
                    return;
                }
                else
                {
                    if (clockType == VCClock.Stopwatch)
                        currentTime = 0
                    else
                        currentTime = clockObj.targetTime
                    updateTime(false)
                }
            }
        }
    }

    Timer
    {
        id: clockTimer
        running: false
        interval: 1000
        repeat: true
        onTriggered: updateTime(true)
    }
}
