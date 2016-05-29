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

import "TimeUtils.js" as TimeUtils

import "."

GridLayout
{
    id: toolRoot
    property color buttonsBgColor: "#05438E"
    property int btnFontSize: 12
    property string title
    property string timeValueString

    property int msTime: 0
    property bool msTimeCalcNeeded: true

    columns: 5
    rows: 4
    columnSpacing: 0
    rowSpacing: 0

    signal timeValueChanged(int ms)

    onVisibleChanged: if (visible) timeBox.selectAndFocus()

    onTimeValueStringChanged:
    {
        if (msTimeCalcNeeded == true)
            msTime = TimeUtils.qlcStringToMs(timeValueString)
        //console.log("Time value ms: " + msTime)
        msTimeCalcNeeded = false
    }

    onMsTimeChanged:
    {
        //console.log("ms time: " + msTime)
        msTimeCalcNeeded = false
        timeValueString = TimeUtils.msToQlcString(msTime)
        toolRoot.timeValueChanged(msTime)
    }

    // title bar
    Rectangle
    {
        height: 35
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
            fontSize: 14
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
            width: 35
            height: 35
            anchors.right: parent.right
            border.width: 1
            border.color: "#333"
            //bgColor: buttonsBgColor
            useFontawesome: true
            label: FontAwesome.fa_times

            onClicked: toolRoot.visible = false
        }
    }

    // top row: close, increase values
    GenericButton
    {
        width: 35
        Layout.fillHeight: true
        Layout.rowSpan: 2
        border.width: 1
        border.color: "#333"
        bgColor: buttonsBgColor
        fontSize: btnFontSize
        label: "Tap"
    }

    GenericButton
    {
        width: 35
        height: 35
        border.width: 1
        border.color: "#333"
        bgColor: buttonsBgColor
        fontSize: btnFontSize
        label: "+H"
        repetition: true
        onClicked:
        {
            msTime += (60 * 60 * 1000)
        }
    }

    GenericButton
    {
        width: 35
        height: 35
        border.width: 1
        border.color: "#333"
        bgColor: buttonsBgColor
        fontSize: btnFontSize
        label: "+M"
        repetition: true
        onClicked:
        {
            msTime += (60 * 1000)
        }
    }

    GenericButton
    {
        width: 40
        height: 35
        border.width: 1
        border.color: "#333"
        bgColor: buttonsBgColor
        fontSize: btnFontSize
        label: "+S"
        repetition: true
        onClicked:
        {
            msTime += 1000
        }
    }

    GenericButton
    {
        width: 40
        height: 35
        border.width: 1
        border.color: "#333"
        bgColor: buttonsBgColor
        fontSize: btnFontSize
        label: "+ms"
        repetition: true
        onClicked:
        {
            msTime++
        }
    }

    // middle row: tap, time value
    Rectangle
    {
        height: 35
        color: "#444"
        border.width: 1
        border.color: "#333"
        Layout.fillWidth: true
        Layout.columnSpan: 4

        CustomTextEdit
        {
            id: timeBox
            width: parent.width
            height: 35
            //anchors.fill: parent
            textAlignment: TextInput.AlignHCenter
            radius: 0
            inputText: timeValueString
            fontSize: btnFontSize
        }
    }

    // bottom row: infinite, decrease values
    GenericButton
    {
        width: 35
        height: 35
        border.width: 1
        border.color: "#333"
        bgColor: buttonsBgColor
        fontSize: btnFontSize
        label: "∞"
        onClicked:
        {
            timeValueString = "∞"
        }
    }

    GenericButton
    {
        width: 35
        height: 35
        border.width: 1
        border.color: "#333"
        bgColor: buttonsBgColor
        fontSize: btnFontSize
        label: "-H"
        repetition: true
        onClicked:
        {
            if (msTime < 60 * 60 * 1000)
                return
            msTime -= (60 * 60 * 1000)
        }
    }

    GenericButton
    {
        width: 35
        height: 35
        border.width: 1
        border.color: "#333"
        bgColor: buttonsBgColor
        fontSize: btnFontSize
        label: "-M"
        repetition: true
        onClicked:
        {
            if (msTime < 60000)
                return
            msTime -= (60 * 1000)
        }
    }

    GenericButton
    {
        width: 40
        height: 35
        border.width: 1
        border.color: "#333"
        bgColor: buttonsBgColor
        fontSize: btnFontSize
        label: "-S"
        repetition: true
        onClicked:
        {
            if (msTime < 1000)
                return
            msTime -= 1000
        }
    }

    GenericButton
    {
        width: 40
        height: 35
        border.width: 1
        border.color: "#333"
        bgColor: buttonsBgColor
        fontSize: btnFontSize
        label: "-ms"
        repetition: true
        onClicked:
        {
            if (msTime == 0)
                return
            msTime--
        }
    }
}
