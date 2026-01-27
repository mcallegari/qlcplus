/*
  Q Light Controller Plus
  VCSpeedDialItem.qml

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
import QtQuick.Controls

import org.qlcplus.classes 1.0
import "TimeUtils.js" as TimeUtils
import "."

VCWidgetItem
{
    id: speedRoot
    property VCSpeedDial speedObj: null
    property int vMask: speedObj ? speedObj.visibilityMask : VCSpeedDial.Nothing

    // needed for tapping
    property double tapTimeValue: 0
    property int tapCount: 0
    property double lastTap: 0
    property var tapHistory: []

    property color activeColor: "green"
    property var speedLabels: [ "", "", "1/16", "1/8", "1/4", "1/2", "1", "2", "4", "8", "16" ]

    property int currTime: speedObj ? speedObj.currentTime : 0

    clip: true

    onSpeedObjChanged:
    {
        setCommonProperties(speedObj)
    }

    onCurrTimeChanged:
    {
        var value = currTime
        var h = Math.floor(value / 3600000);
        value -= (h * 3600000);

        var m = Math.floor(value / 60000);
        value -= (m * 60000);

        var s = Math.floor(value / 1000);
        value -= (s * 1000);

        hoursSpin.value = h
        minutesSpin.value = m
        secondsSpin.value = s
        msSpin.value = value
    }

    function tap()
    {
        var currTime = new Date().getTime()

        if (lastTap != 0 && currTime - lastTap < 1500)
        {
            var newTime = currTime - lastTap

            tapHistory.push(newTime)

            tapTimeValue = TimeUtils.calculateBPMByTapIntervals(tapHistory)

            if (speedObj)
                speedObj.currentTime = tapTimeValue

            tapTimer.interval = tapTimeValue
            tapTimer.restart()
        }
        else
        {
            lastTap = 0
            tapHistory = []
        }
        lastTap = currTime
    }

    function updateTime()
    {
        var newTime = 0
        if (hoursSpin.visible)
            newTime += hoursSpin.value * 3600000
        if (minutesSpin.visible)
            newTime += minutesSpin.value * 60000
        if (secondsSpin.visible)
            newTime += secondsSpin.value * 1000
        if (msSpin.visible)
            newTime += msSpin.value

        tapTimer.stop()
        speedObj.currentTime = newTime
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
                tapButton.border.color = "#00FF00"
            else
                tapButton.border.color = UISettings.bgMedium
        }
    }

    GridLayout
    {
        id: itemsLayout
        anchors.fill: parent

        columns: tapButton.visible ? 6 : 4

        // row 1 widget name text box
        Text
        {
            id: sliderText
            visible: (speedObj && speedObj.caption.length) ? true : false
            Layout.columnSpan: itemsLayout.columns
            Layout.fillWidth: true
            height: UISettings.listItemHeight
            font: speedObj ? speedObj.font : ""
            text: speedObj ? speedObj.caption : ""
            color: speedObj ? speedObj.foregroundColor : "white"
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }

        // row 2
        QLCPlusKnob
        {
            property int lastValue: 0

            Layout.columnSpan: tapButton.visible ? 4 : 6
            Layout.rowSpan: 2
            Layout.fillWidth: true
            Layout.fillHeight: true
            visible: vMask & VCSpeedDial.Dial
            drawOuterLevel: false
            from: 0
            to: 1000
            wrap: true
            
            onMoved: 
            {
                if (speedObj)
                {
                    var diff = value - lastValue
                    var THRESHOLD = 50

                    // handle wrapping
                    if (diff > THRESHOLD)
                        diff = -stepSize;
                    else if (diff < (-THRESHOLD))
                        diff = stepSize;

                    lastValue = value
                    tapTimer.stop()
                    speedObj.currentTime += diff
                }
            }
        }

        GenericButton
        {
            Layout.fillWidth: true
            Layout.fillHeight: true
            label: "1/16"
            visible: vMask & VCSpeedDial.Beats
            bgColor: speedObj ? (speedObj.currentFactor === VCSpeedDial.OneSixteenth ? speedRoot.activeColor : UISettings.bgControl) : UISettings.bgControl
            onClicked:
            {
                if (speedObj)
                    speedObj.currentFactor = VCSpeedDial.OneSixteenth
            }
        }

        GenericButton
        {
            Layout.fillWidth: true
            Layout.fillHeight: true
            label: "1/8"
            visible: vMask & VCSpeedDial.Beats
            bgColor: speedObj ? (speedObj.currentFactor === VCSpeedDial.OneEighth ? speedRoot.activeColor : UISettings.bgControl) : UISettings.bgControl
            onClicked:
            {
                if (speedObj)
                    speedObj.currentFactor = VCSpeedDial.OneEighth
            }
        }

        GenericButton
        {
            Layout.fillWidth: true
            Layout.fillHeight: true
            label: "1/4"
            visible: vMask & VCSpeedDial.Beats
            bgColor: speedObj ? (speedObj.currentFactor === VCSpeedDial.OneFourth ? speedRoot.activeColor : UISettings.bgControl) : UISettings.bgControl
            onClicked:
            {
                if (speedObj)
                    speedObj.currentFactor = VCSpeedDial.OneFourth
            }
        }

        GenericButton
        {
            Layout.fillWidth: true
            Layout.fillHeight: true
            label: "1/2"
            visible: vMask & VCSpeedDial.Beats
            bgColor: speedObj ? (speedObj.currentFactor === VCSpeedDial.Half ? speedRoot.activeColor : UISettings.bgControl) : UISettings.bgControl
            onClicked:
            {
                if (speedObj)
                    speedObj.currentFactor = VCSpeedDial.Half
            }
        }

        GenericButton
        {
            id: tapButton
            Layout.columnSpan: 2
            Layout.rowSpan: 2
            Layout.fillHeight: true

            label: "TAP"
            visible: vMask & VCSpeedDial.Tap

            onClicked:
            {
                /* right click resets the current TAP time */
                if (mouseButton === Qt.RightButton)
                {
                    tapTimer.stop()
                    tapButton.border.color = UISettings.bgMedium
                    lastTap = 0
                    tapHistory = []
                }
                else
                {
                    speedRoot.tap()
                }
            }
        }

        // row 3
        GenericButton
        {
            Layout.fillWidth: true
            Layout.fillHeight: true
            label: "2"
            visible: vMask & VCSpeedDial.Beats
            bgColor: speedObj ? (speedObj.currentFactor === VCSpeedDial.Two ? speedRoot.activeColor : UISettings.bgControl) : UISettings.bgControl
            onClicked:
            {
                if (speedObj)
                    speedObj.currentFactor = VCSpeedDial.Two
            }
        }

        GenericButton
        {
            Layout.fillWidth: true
            Layout.fillHeight: true
            label: "4"
            visible: vMask & VCSpeedDial.Beats
            bgColor: speedObj ? (speedObj.currentFactor === VCSpeedDial.Four ? speedRoot.activeColor : UISettings.bgControl) : UISettings.bgControl
            onClicked:
            {
                if (speedObj)
                    speedObj.currentFactor = VCSpeedDial.Four
            }
        }

        GenericButton
        {
            Layout.fillWidth: true
            Layout.fillHeight: true
            label: "8"
            visible: vMask & VCSpeedDial.Beats
            bgColor: speedObj ? (speedObj.currentFactor === VCSpeedDial.Eight ? speedRoot.activeColor : UISettings.bgControl) : UISettings.bgControl
            onClicked:
            {
                if (speedObj)
                    speedObj.currentFactor = VCSpeedDial.Eight
            }
        }

        GenericButton
        {
            Layout.fillWidth: true
            Layout.fillHeight: true
            label: "16"
            visible: vMask & VCSpeedDial.Beats
            bgColor: speedObj ? (speedObj.currentFactor === VCSpeedDial.Sixteen ? speedRoot.activeColor : UISettings.bgControl) : UISettings.bgControl
            onClicked:
            {
                if (speedObj)
                    speedObj.currentFactor = VCSpeedDial.Sixteen
            }
        }

        // row 4
        RowLayout
        {
            visible: vMask & VCSpeedDial.Hours | vMask & VCSpeedDial.Minutes | vMask & VCSpeedDial.Seconds | vMask & VCSpeedDial.Milliseconds
            Layout.columnSpan: itemsLayout.columns
            Layout.fillWidth: true
            height: UISettings.listItemHeight

            CustomSpinBox
            {
                id: hoursSpin
                Layout.fillWidth: true
                visible: vMask & VCSpeedDial.Hours
                from: 0
                to: 999
                suffix: "h"
                onValueModified: updateTime()
            }
            CustomSpinBox
            {
                id: minutesSpin
                Layout.fillWidth: true
                visible: vMask & VCSpeedDial.Minutes
                from: 0
                to: 59
                suffix: "m"
                onValueModified: updateTime()
            }
            CustomSpinBox
            {
                id: secondsSpin
                Layout.fillWidth: true
                visible: vMask & VCSpeedDial.Seconds
                from: 0
                to: 59
                suffix: "s"
                onValueModified: updateTime()
            }
            CustomSpinBox
            {
                id: msSpin
                Layout.fillWidth: true
                visible: vMask & VCSpeedDial.Milliseconds
                from: 0
                to: 999
                suffix: "ms"
                onValueModified: updateTime()
            }
        }

        // row 5
        GenericButton
        {
            Layout.fillWidth: true
            Layout.fillHeight: true
            label: "-"
            visible: vMask & VCSpeedDial.Multipliers
            onClicked:
            {
                if (speedObj)
                    speedObj.decreaseSpeedFactor();
            }
        }

        RobotoText
        {
            Layout.columnSpan: 2
            Layout.fillWidth: true
            Layout.fillHeight: true
            textHAlign: Text.AlignHCenter
            label: speedLabels[speedObj.currentFactor] + "x\n" + TimeUtils.timeToQlcString(speedObj.currentTime, QLCFunction.Time)
            visible: vMask & VCSpeedDial.Multipliers
        }

        GenericButton
        {
            Layout.fillWidth: true
            Layout.fillHeight: true
            label: "+"
            visible: vMask & VCSpeedDial.Multipliers
            onClicked:
            {
                if (speedObj)
                    speedObj.increaseSpeedFactor();
            }
        }

        IconButton
        {
            Layout.columnSpan: 2
            Layout.fillWidth: true
            Layout.fillHeight: true
            faSource: FontAwesome.fa_xmark
            visible: vMask & VCSpeedDial.Multipliers
            onClicked:
            {
                if (speedObj)
                    speedObj.currentFactor = VCSpeedDial.One
            }
        }
        GenericButton
        {
            Layout.columnSpan: itemsLayout.columns
            Layout.fillWidth: true
            label: qsTr("Apply")
            visible: vMask & VCSpeedDial.Apply
            onClicked:
            {
                if (speedObj)
                    speedObj.applyFunctionsTime()
            }
        }

        Flow
        {
            Layout.columnSpan: itemsLayout.columns
            Layout.fillWidth: true
            spacing: 4
            visible: speedObj && speedObj.presetsList && speedObj.presetsList.length > 0

            Repeater
            {
                model: speedObj ? speedObj.presetsList : []

                GenericButton
                {
                    height: UISettings.listItemHeight
                    width: Math.max(UISettings.iconSizeDefault * 3, implicitWidth)
                    label: modelData.name
                    bgColor: speedObj && speedObj.currentTime === modelData.value ?
                                speedRoot.activeColor : UISettings.bgControl
                    onClicked:
                    {
                        if (speedObj)
                            speedObj.currentTime = modelData.value
                    }
                }
            }
        }
    }

    DropArea
    {
        id: dropArea
        anchors.fill: parent
        z: 2 // this area must be above the VCWidget resize controls
        keys: [ "function" ]

        onDropped:
        {
            // attach functions here
            if (drag.source.hasOwnProperty("fromFunctionManager"))
            {
                for (var i = 0; i < drag.source.itemsList.length; i++)
                    speedObj.addFunction(drag.source.itemsList[i])
            }
        }

        states: [
            State
            {
                when: dropArea.containsDrag
                PropertyChanges
                {
                    target: speedRoot
                    color: UISettings.activeDropArea
                }
            }
        ]
    }
}
