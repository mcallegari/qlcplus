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

import QtQuick
import QtQuick.Layouts

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
    property int targetTime: clockObj ? clockObj.targetTime : 0
    /** Seconds in Clock mode, milliseconds in Stopwatch/Countdown */
    property int clockTimeValue: clockObj ? clockObj.currentTime : 0
    property int scheduledDaysMask: computeScheduledDaysMask()

    clip: true

    onClockObjChanged:
    {
        setCommonProperties(clockObj)
        updateTime()
    }

    onClockTimeValueChanged: updateTime()

    onTargetTimeChanged: updateTime()

    onClockTypeChanged:
    {
        updateTime()
    }

    function updateTime()
    {
        switch(clockType)
        {
            case VCClock.Stopwatch:
                timeString = TimeUtils.msToStringWithPrecision(Math.max(0, clockTimeValue), 1)
            break;
            case VCClock.Countdown:
                timeString = TimeUtils.msToStringWithPrecision(Math.max(0, clockTimeValue), 1)
            break;
            case VCClock.Clock:
                timeString = new Date().toLocaleString(locale, "hh:mm:ss");
            break;
        }
    }

    function computeScheduledDaysMask()
    {
        if (!clockObj || clockType !== VCClock.Clock || !clockObj.scheduleList)
            return 0

        var mask = 0
        for (var i = 0; i < clockObj.scheduleList.length; i++)
        {
            var sch = clockObj.scheduleList[i]
            if (!sch)
                continue

            var flags = sch.weekFlags & 0x7F
            // Do not display day initials for "all days" schedules.
            if (flags !== 0 && flags !== 0x7F)
                mask |= flags
        }

        return mask
    }

    function isDayScheduled(index)
    {
        return (scheduledDaysMask & (1 << index)) !== 0
    }

    Row
    {
        anchors.fill: parent

        Column
        {
            width: parent.width - enableChk.width
            height: clockRoot.height
            spacing: 0

            Text
            {
                width: parent.width
                height: parent.height - (daysRow.visible ? daysRow.height : 0)
                font: clockObj ? clockObj.font : Qt.font({ family: UISettings.robotoFontName })
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
                    onClicked: (mouse) =>
                    {
                        if (clockType == VCClock.Stopwatch || clockType == VCClock.Countdown)
                        {
                            if (mouse.button === Qt.LeftButton)
                            {
                                if (clockType === VCClock.Countdown && clockTimeValue <= 0)
                                    return

                                if (clockObj)
                                    clockObj.playPauseTimer()
                                return
                            }
                            else
                            {
                                if (clockObj)
                                    clockObj.resetTimer()
                            }
                        }
                    }
                }
            }

            Row
            {
                id: daysRow
                visible: clockType === VCClock.Clock && scheduledDaysMask !== 0
                width: parent.width
                height: Math.min(UISettings.listItemHeight * 0.65, parent.height * 0.35)
                spacing: 2

                Repeater
                {
                    model:
                    [
                        qsTr("M", "As in Monday"),
                        qsTr("T", "As in Tuesday"),
                        qsTr("W", "As in Wednesday"),
                        qsTr("T", "As in Thursday"),
                        qsTr("F", "As in Friday"),
                        qsTr("S", "As in Saturday"),
                        qsTr("S", "As in Sunday")
                    ]

                    Text
                    {
                        width: Math.floor((daysRow.width - (daysRow.spacing * 6)) / 7)
                        height: daysRow.height
                        text: modelData
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        font.family: UISettings.robotoFontName
                        font.pixelSize: Math.max(8, Math.round(daysRow.height * 0.75))
                        font.bold: clockRoot.isDayScheduled(index)
                        color: clockRoot.isDayScheduled(index) ? UISettings.highlight : (clockObj ? clockObj.foregroundColor : "#111")
                        opacity: clockRoot.isDayScheduled(index) ? 1.0 : 0.45
                    }
                }
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
            faSource: FontAwesome.fa_check
            faColor: "lime"
            checked: clockObj ? clockObj.enableSchedule : false
            onToggled: if (clockObj) clockObj.enableSchedule = checked
        }
    }
}
