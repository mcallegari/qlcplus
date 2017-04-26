/*
  Q Light Controller Plus
  DayTimeTool.qml

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
import QtQuick.Layouts 1.1

Rectangle
{
    id: dttRoot
    width: 220
    height: 40
    color: "transparent"

    property int tHours: 0
    property int tMinutes: 0
    property int tSeconds: 0

    // the day time value in seconds
    property int timeValue: 0

    onTimeValueChanged:
    {
        if (timeValue < 0)
            return

        var sTime = timeValue
        tHours = Math.floor(sTime / 3600)
        sTime -= (tHours * 3600)

        tMinutes = Math.floor(sTime / 60)
        sTime -= (tMinutes * 60)

        tSeconds = sTime
    }

    function updateTime()
    {
        var fTime = (tHours * 60 * 60) + (tMinutes * 60) + tSeconds
        timeValue = fTime
    }

    Row
    {
        id: dtRow
        spacing: 5
        property int fieldsWidth: (dttRoot.width / 3) - spacing

        CustomSpinBox
        {
            width: dtRow.fieldsWidth
            height: dttRoot.height
            from: 0
            to: 23
            suffix: "h"
            value: tHours
            onValueChanged:
            {
                tHours = value
                updateTime()
            }
        }
        CustomSpinBox
        {
            width: dtRow.fieldsWidth
            height: dttRoot.height
            from: 0
            to: 59
            suffix: "m"
            value: tMinutes
            onValueChanged:
            {
                tMinutes = value
                updateTime()
            }
        }
        CustomSpinBox
        {
            width: dtRow.fieldsWidth
            height: dttRoot.height
            from: 0
            to: 59
            suffix: "s"
            value: tSeconds
            onValueChanged:
            {
                tSeconds = value
                updateTime()
            }
        }
    }
}
