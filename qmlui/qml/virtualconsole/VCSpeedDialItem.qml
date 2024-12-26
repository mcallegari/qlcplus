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

import QtQuick 2.14
import QtQuick.Layouts 1.14
import QtQuick.Controls 2.14

import org.qlcplus.classes 1.0
import "TimeUtils.js" as TimeUtils
import "."

VCWidgetItem
{
    id: speedRoot
    property VCSpeedDial speedObj: null

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

        // row 1
        QLCPlusKnob
        {
            property int lastValue: 0

            Layout.columnSpan: tapButton.visible ? 4 : 6
            Layout.rowSpan: 2
            Layout.fillWidth: true
            Layout.fillHeight: true
            visible: speedObj ? speedObj.visibilityMask & VCSpeedDial.Dial : false
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
                    speedObj.currentTime += diff
                }
            }
        }

        GenericButton
        {
            Layout.fillWidth: true
            Layout.fillHeight: true
            label: "1/16"
            visible: speedObj ? speedObj.visibilityMask & VCSpeedDial.Beats : false
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
            visible: speedObj ? speedObj.visibilityMask & VCSpeedDial.Beats : false
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
            visible: speedObj ? speedObj.visibilityMask & VCSpeedDial.Beats : false
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
            visible: speedObj ? speedObj.visibilityMask & VCSpeedDial.Beats : false
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
            Layout.rowSpan: xpadControl.visible ? 3 : 2
            Layout.fillHeight: true

            label: "TAP"
            visible: speedObj ? speedObj.visibilityMask & VCSpeedDial.Tap : false

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

        CustomSpinBox
        {
            id: hoursSpin
            Layout.fillWidth: true
            visible: speedObj ? speedObj.visibilityMask & VCSpeedDial.Hours : false
            from: 0
            to: 999
            suffix: "h"
        }
        CustomSpinBox
        {
            id: minutesSpin
            Layout.fillWidth: true
            visible: speedObj ? speedObj.visibilityMask & VCSpeedDial.Minutes : false
            from: 0
            to: 59
            suffix: "m"
        }
        CustomSpinBox
        {
            id: secondsSpin
            Layout.fillWidth: true
            visible: speedObj ? speedObj.visibilityMask & VCSpeedDial.Seconds : false
            from: 0
            to: 59
            suffix: "s"
        }
        CustomSpinBox
        {
            id: msSpin
            Layout.fillWidth: true
            visible: speedObj ? speedObj.visibilityMask & VCSpeedDial.Milliseconds : false
            from: 0
            to: 999
            suffix: "ms"
        }

        // row 2
        Rectangle
        {
            // TODO: X-Pad
            id: xpadControl
            visible: false
            Layout.columnSpan: 4
            Layout.fillWidth: true
            height: UISettings.iconSizeMedium
        }

        // row 3
        GenericButton
        {
            Layout.fillWidth: true
            Layout.fillHeight: true
            label: "2"
            visible: speedObj ? speedObj.visibilityMask & VCSpeedDial.Beats : false
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
            visible: speedObj ? speedObj.visibilityMask & VCSpeedDial.Beats : false
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
            visible: speedObj ? speedObj.visibilityMask & VCSpeedDial.Beats : false
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
            visible: speedObj ? speedObj.visibilityMask & VCSpeedDial.Beats : false
            bgColor: speedObj ? (speedObj.currentFactor === VCSpeedDial.Sixteen ? speedRoot.activeColor : UISettings.bgControl) : UISettings.bgControl
            onClicked:
            {
                if (speedObj)
                    speedObj.currentFactor = VCSpeedDial.Sixteen
            }
        }

        // row 4
        GenericButton
        {
            Layout.fillWidth: true
            Layout.fillHeight: true
            label: "-"
            visible: speedObj ? speedObj.visibilityMask & VCSpeedDial.Multipliers : false
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
            visible: speedObj ? speedObj.visibilityMask & VCSpeedDial.Multipliers : false
        }

        GenericButton
        {
            Layout.fillWidth: true
            Layout.fillHeight: true
            label: "+"
            visible: speedObj ? speedObj.visibilityMask & VCSpeedDial.Multipliers : false
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
            faSource: FontAwesome.fa_times
            visible: speedObj ? speedObj.visibilityMask & VCSpeedDial.Multipliers : false
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
            visible: speedObj ? speedObj.visibilityMask & VCSpeedDial.Apply : false
            onClicked:
            {
                if (speedObj)
                    speedObj.applyFunctionsTime()
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
