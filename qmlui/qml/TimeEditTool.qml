/*
  Q Light Controller Plus
  TimeEditTool.qml

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

import com.qlcplus.classes 1.0
import "TimeUtils.js" as TimeUtils

import "."

GridLayout
{
    id: toolRoot
    columns: 5
    rows: 4
    columnSpacing: 0
    rowSpacing: 0

    property color buttonsBgColor: "#05438E"
    property int btnFontSize: UISettings.textSizeDefault
    property string title
    property string timeValueString

    property int timeValue: 0
    property bool timeValueCalcNeeded: true

    /* The TAP time counter */
    property double tapTimeValue: 0

    /* If needed, this property can be used to recognize which type
       of speed value is being edited */
    property int speedType

    /* The type of the tempo being edited. Can be Time or Beats */
    property int tempoType: Function.Time
    property int allowFractions: Function.NoFractions
    property int currentFraction: 0

    /* If needed, this can be the reference index of an item in a list */
    property int indexInList

    signal valueChanged(int val)

    function show(tX, tY, tTitle, tStrValue, tType)
    {
        timeValueCalcNeeded = false
        tapTimeValue = 0
        tapTimer.stop()
        title = tTitle
        speedType = tType
        timeValueString = tStrValue
        timeValue = TimeUtils.qlcStringToTime(timeValueString, tempoType)
        if (allowFractions !== Function.NoFractions)
            currentFraction = (timeValue % 1000)

        if (tX >= 0)
            x = tX
        if (tY >= 0)
            y = tY

        visible = true
        timeBox.selectAndFocus()
    }

    onTimeValueStringChanged:
    {
        if (timeValueCalcNeeded == true)
            timeValue = TimeUtils.qlcStringToTime(timeValueString, tempoType)
        console.log("Time value: " + timeValue)
        timeValueCalcNeeded = false
    }

    onTimeValueChanged:
    {
        //console.log("ms time: " + timeValue)
        timeValueCalcNeeded = false
        timeValueString = TimeUtils.timeToQlcString(timeValue, tempoType)
        toolRoot.valueChanged(timeValue)
    }

    Timer
    {
        id: tapTimer
        repeat: true
        running: false
        interval: 500

        onTriggered:
        {
            if (tapButton.border.color == UISettings.bgMedium)
                tapButton.border.color = "green"
            else
                tapButton.border.color = UISettings.bgMedium
        }
    }

    // title bar + close button
    Rectangle
    {
        height: UISettings.iconSizeDefault
        Layout.fillWidth: true
        Layout.columnSpan: 5
        gradient:
            Gradient
            {
                GradientStop { position: 0; color: UISettings.toolbarStartSub }
                GradientStop { position: 1; color: UISettings.toolbarEnd }
            }

        RobotoText
        {
            id: titleBox
            anchors.fill: parent
            anchors.margins: 3

            label: title
            fontSize: UISettings.textSizeDefault * 0.75
        }
        // allow the tool to be dragged around
        // by holding it on the title bar
        MouseArea
        {
            anchors.fill: parent
            drag.target: toolRoot
        }
        GenericButton
        {
            width: height
            height: parent.height
            anchors.right: parent.right
            border.color: UISettings.bgMedium
            //bgColor: buttonsBgColor
            useFontawesome: true
            label: FontAwesome.fa_times

            onClicked:
            {
                tapTimer.stop()
                toolRoot.visible = false
            }
        }
    }

    // top row: tap, increase values
    GenericButton
    {
        id: tapButton
        width: UISettings.iconSizeDefault
        Layout.fillHeight: true
        Layout.rowSpan: 2
        bgColor: buttonsBgColor
        fontSize: btnFontSize
        label: "Tap"

        onClicked:
        {
            /* right click resets the current TAP time */
            if (mouseButton === Qt.RightButton)
            {
                tapTimer.stop()
                tapButton.border.color = UISettings.bgMedium
                tapTimeValue = 0
            }
            else
            {
                var currTime = new Date().getTime()
                if (tapTimeValue != 0)
                {
                    timeValue = currTime - tapTimeValue
                    tapTimer.interval = timeValue
                    tapTimer.restart()
                }
                tapTimeValue = currTime
            }
        }
    }

    GenericButton
    {
        visible: tempoType === Function.Time
        width: height
        height: UISettings.iconSizeDefault
        border.color: UISettings.bgMedium
        bgColor: buttonsBgColor
        fontSize: btnFontSize
        label: "+M"
        repetition: true
        onClicked: timeValue += (60 * 1000)
    }

    GenericButton
    {
        visible: tempoType === Function.Time
        width: height * 1.2
        height: UISettings.iconSizeDefault
        border.color: UISettings.bgMedium
        bgColor: buttonsBgColor
        fontSize: btnFontSize
        label: "+S"
        repetition: true
        onClicked: timeValue += 1000
    }

    GenericButton
    {
        visible: tempoType === Function.Time
        width: height * 1.2
        height: UISettings.iconSizeDefault
        border.color: UISettings.bgMedium
        bgColor: buttonsBgColor
        fontSize: btnFontSize
        label: "+ms"
        repetition: true
        onClicked: timeValue++
    }

    GenericButton
    {
        visible: tempoType === Function.Beats
        height: UISettings.iconSizeDefault
        Layout.fillWidth: true
        Layout.columnSpan: allowFractions !== Function.NoFractions ? 2 : 4
        border.color: UISettings.bgMedium
        bgColor: buttonsBgColor
        fontSize: btnFontSize
        label: "+"
        repetition: true
        onClicked: timeValue += 1000
    }

    GenericButton
    {
        visible: tempoType === Function.Beats && allowFractions !== Function.NoFractions
        height: UISettings.iconSizeDefault
        Layout.fillWidth: true
        Layout.columnSpan: 2
        border.color: UISettings.bgMedium
        bgColor: buttonsBgColor
        fontSize: btnFontSize
        label: allowFractions === Function.AllFractions ? "+1/8" : "+2x"
        repetition: true
        onClicked:
        {
            if (allowFractions === Function.AllFractions)
                timeValue += 125
            else
            {
                var newfraction = 0
                if (currentFraction == 0)
                    newfraction = 125
                else if (currentFraction != 500)
                    newfraction = currentFraction * 2

                if (newfraction == 0)
                    timeValue += 1000

                timeValue = timeValue - currentFraction + newfraction
                currentFraction = newfraction
            }
        }
    }

    // middle row: tap, time value
    Rectangle
    {
        height: UISettings.iconSizeDefault
        color: "#444"
        border.color: UISettings.bgMedium
        Layout.fillWidth: true
        Layout.columnSpan: 4

        CustomTextEdit
        {
            id: timeBox
            anchors.fill: parent
            //anchors.fill: parent
            textAlignment: TextInput.AlignHCenter
            radius: 0
            inputText: timeValueString
            fontSize: btnFontSize

            onEnterPressed:
            {
                timeValueCalcNeeded = true
                timeValueString = TimeUtils.timeToQlcString(inputText, tempoType)
            }
            Keys.onEscapePressed:
            {
                tapTimer.stop()
                toolRoot.visible = false
            }
        }
    }

    // bottom row: infinite, decrease values
    GenericButton
    {
        width: height
        height: UISettings.iconSizeDefault
        border.color: UISettings.bgMedium
        bgColor: buttonsBgColor
        fontSize: btnFontSize
        label: "∞"
        onClicked: timeValue = -2
    }

    GenericButton
    {
        visible: tempoType === Function.Time
        width: height
        height: UISettings.iconSizeDefault
        border.color: UISettings.bgMedium
        bgColor: buttonsBgColor
        fontSize: btnFontSize
        label: "-M"
        repetition: true
        onClicked:
        {
            if (timeValue < 60000)
                return
            timeValue -= (60 * 1000)
        }
    }

    GenericButton
    {
        visible: tempoType === Function.Time
        width: height * 1.2
        height: UISettings.iconSizeDefault
        border.color: UISettings.bgMedium
        bgColor: buttonsBgColor
        fontSize: btnFontSize
        label: "-S"
        repetition: true
        onClicked:
        {
            if (timeValue < 1000)
                return
            timeValue -= 1000
        }
    }

    GenericButton
    {
        visible: tempoType === Function.Time
        width: height * 1.2
        height: UISettings.iconSizeDefault
        border.color: UISettings.bgMedium
        bgColor: buttonsBgColor
        fontSize: btnFontSize
        label: "-ms"
        repetition: true
        onClicked:
        {
            if (timeValue == 0)
                return
            timeValue--
        }
    }

    GenericButton
    {
        visible: tempoType === Function.Beats
        height: UISettings.iconSizeDefault
        Layout.fillWidth: true
        Layout.columnSpan: allowFractions !== Function.NoFractions ? 2 : 4
        border.color: UISettings.bgMedium
        bgColor: buttonsBgColor
        fontSize: btnFontSize
        label: "-"
        repetition: true
        onClicked: timeValue -= 1000
    }

    GenericButton
    {
        visible: tempoType === Function.Beats && allowFractions !== Function.NoFractions
        height: UISettings.iconSizeDefault
        Layout.fillWidth: true
        Layout.columnSpan: 2
        border.color: UISettings.bgMedium
        bgColor: buttonsBgColor
        fontSize: btnFontSize
        label: allowFractions === Function.AllFractions ? "-1/8" : "-x/2"
        repetition: true
        onClicked:
        {
            if (allowFractions === Function.AllFractions)
            {
                if (timeValue == 0)
                    return

                timeValue -= 125
            }
            else
            {
                var newfraction = 0
                if (currentFraction == 0)
                {
                    newfraction = 500
                    if (timeValue > 0)
                        timeValue -= 1000
                }
                else if (currentFraction != 125)
                    newfraction = currentFraction / 2

                timeValue = timeValue - currentFraction + newfraction
                currentFraction = newfraction
            }
        }
    }
}
