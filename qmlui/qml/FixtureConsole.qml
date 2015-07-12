/*
  Q Light Controller Plus
  FixtureConsole.qml

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

import QtQuick 2.2
import com.qlcplus.classes 1.0

Rectangle
{
    id: consoleRoot
    width: channelsRow.width
    height: fxColumn.height
    color: "#222"
    border.width: 1
    border.color: "#aaa"

    onWidthChanged: consoleRoot.sizeChanged(width, height)
    onHeightChanged: consoleRoot.sizeChanged(width, height)

    property Fixture fixtureObj
    property variant values
    property bool dmxValues: true
    property bool isSelected: false

    signal doubleClicked
    signal clicked
    signal sizeChanged(var w, var h)
    signal valueChanged(var fixtureID, var chIndex, var value)

    onValuesChanged:
    {
        for (var i = 0; i < values.length; i++)
        {
            //console.log("Value " + i + " = " + values[i]);
            if (dmxValues)
                channelsRpt.itemAt(i).dmxValue = values[i]
            else
                channelsRpt.itemAt(i).dmxValue = (values[i] / 255) * 100
        }
    }

    Column
    {
        id: fxColumn
        Rectangle
        {
            color: "#111"
            width: parent.width
            height: 27
            clip: true

            RobotoText
            {
                anchors.verticalCenter: parent.verticalCenter
                x: 2
                label: fixtureObj ? fixtureObj.name : ""
                fontSize: 17
            }
            DMXPercentageButton
            {
                x: parent.width - width - 3
                y: 1
                z: 2
                height: 25
                dmxMode: dmxValues
                onClicked:
                {
                    dmxValues = !dmxValues
                    for (var i = 0; i < channelsRpt.count; i++)
                    {
                        var newVal;
                        if (dmxValues == false)
                            newVal = (channelsRpt.itemAt(i).dmxValue / 255) * 100
                        else
                            newVal = (channelsRpt.itemAt(i).dmxValue / 100) * 255
                        channelsRpt.itemAt(i).dmxMode = dmxValues
                        channelsRpt.itemAt(i).dmxValue = newVal
                    }
                }
            }

            MouseArea
            {
                anchors.fill: parent
                z: 1
                onClicked: consoleRoot.clicked()
                onDoubleClicked: consoleRoot.doubleClicked()
            }
        }
        Row
        {
            id: channelsRow
            Repeater
            {
                id: channelsRpt
                model: fixtureObj ? fixtureObj.channels : null
                delegate:
                    Rectangle
                    {
                        color: "transparent"
                        border.width: 1
                        border.color: "#333"
                        width: 40
                        height: 190

                        property alias dmxValue: slider.value
                        property bool dmxMode: true

                        onDmxValueChanged:
                        {
                            if (dmxMode)
                                fixtureManager.setChannelValue(fixtureObj.id, index, dmxValue)
                            else
                                fixtureManager.setChannelValue(fixtureObj.id, index, dmxValue * 2.55)
                        }

                        Image
                        {
                            x: 1
                            y: 1
                            width: 32
                            height: 32
                            anchors.horizontalCenter: parent.horizontalCenter
                            sourceSize: Qt.size(width, height)
                            source: fixtureObj ? fixtureManager.channelIcon(fixtureObj.id, index) : ""
                        }
                        QLCPlusFader
                        {
                            id: slider
                            x: 1
                            y: 35
                            width: 32
                            height: 130
                            anchors.horizontalCenter: parent.horizontalCenter
                            minimumValue: 0
                            maximumValue: dmxMode ? 255 : 100
                        }
                        CustomSpinBox
                        {
                            y: 167
                            x: 1
                            width: 38
                            height: 25
                            minimumValue: 0
                            maximumValue: dmxMode ? 255 : 100
                            showControls: false
                            value: slider.value
                            onValueChanged: dmxValue = value
                        }
                    }
            }
        }
    }
}

