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
import QtQuick.Layouts 1.0

import org.qlcplus.classes 1.0

import "TimeUtils.js" as TimeUtils
import "."

VCWidgetItem
{
    id: clockRoot
    property VCClock clockObj: null
    property var locale: Qt.locale()
    property string timeString: new Date().toLocaleString(locale, "hh:mm:ss");
    property int clockType: clockObj ? clockObj.clockType : VCClock.Clock

    /** The target time from where to start a countdown or a stopwatch */
    property int targetTime: clockObj ? clockObj.targetTime : 0

    /** This is the Countdown/Stopwatch time in milliseconds */
    property int timeCounter: 0

    /** The current day time in seconds, bound to the C++ timer */
    property int currentDayTime: clockObj ? clockObj.currentTime : 0

    clip: true

    onClockObjChanged:
    {
        setCommonProperties(clockObj)
        timeCounter = clockObj.targetTime
        updateTime(false)
    }

    onCurrentDayTimeChanged: updateTime(false)

    onTargetTimeChanged:
    {
        if (clockTimer.running === false)
            timeCounter = targetTime
        updateTime(false)
    }

    onClockTypeChanged:
    {
        timeCounter = clockObj.targetTime
        clockTimer.running = false
        updateTime(false)
    }

    function updateTime(modify)
    {
        switch(clockType)
        {
            case VCClock.Stopwatch:
                if (modify)
                    timeCounter += 100
                timeString = TimeUtils.msToStringWithPrecision(timeCounter, 1)
            break;
            case VCClock.Countdown:
                if (modify)
                    timeCounter -= 100
                timeString = TimeUtils.msToStringWithPrecision(timeCounter, 1)
                if (timeCounter == 0)
                    clockTimer.running = false
            break;
            case VCClock.Clock:
                timeString = new Date().toLocaleString(locale, "hh:mm:ss");
            break;
        }
    }

    Row
    {
        anchors.fill: parent

        Text
        {
            width: parent.width - enableChk.width
            height: clockRoot.height
            font: clockObj ? clockObj.font : ""
            text: timeString
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.Wrap
            lineHeight: 0.8
            color: clockObj ? clockObj.foregroundColor : "#111"

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
                                timeCounter = 0
                            else
                                timeCounter = clockObj.targetTime
                            updateTime(false)
                        }
                    }
                }
            }

            Timer
            {
                id: clockTimer
                running: false
                interval: 100
                repeat: true
                onTriggered: updateTime(true)
            }
        }

        // enable button
        IconButton
        {
            id: enableChk
            anchors.verticalCenter: parent.verticalCenter
            height: Math.min(clockRoot.height, UISettings.iconSizeDefault)
            width: height
            checkable: true
            tooltip: qsTr("Enable/Disable this scheduler")
            imgSource: "qrc:/apply.svg"
            checked: clockObj ? clockObj.enableSchedule : false
            onToggled: if (clockObj) clockObj.enableSchedule = checked
        }
    }
}
