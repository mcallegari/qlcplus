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
import QtQuick.Layouts 1.0

import com.qlcplus.classes 1.0

Rectangle
{
    id: consoleRoot
    width: channelsRow.width
    height: parent.height
    color: "#222"

    onWidthChanged: consoleRoot.sizeChanged(width, height)
    onHeightChanged:
    {
        if (height < 80)
            fxColumn.visible = false
        else
            fxColumn.visible = true
        consoleRoot.sizeChanged(width, height)
    }

    property Fixture fixtureObj
    property variant values
    property bool dmxValues: true
    property bool isSelected: false
    property bool showEnablers: false
    property bool sceneConsole: false

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

    function setChannelValue(channel, value)
    {
        if (showEnablers == true)
            channelsRpt.itemAt(channel).isEnabled = true
        channelsRpt.itemAt(channel).dmxValue = value
    }

    Column
    {
        id: fxColumn
        height: parent.height
        visible: false
        Rectangle
        {
            id: fxNameBar
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
            height: parent.height - fxNameBar.height
            Repeater
            {
                id: channelsRpt
                model: fixtureObj ? fixtureObj.channels : null
                delegate:
                    Rectangle
                    {
                        id: chDelegate
                        color: "transparent"
                        border.width: 1
                        border.color: "#111"
                        width: 40
                        height: channelsRow.height

                        property alias dmxValue: slider.value
                        property bool dmxMode: true
                        property bool isEnabled: showEnablers ? false : true

                        onDmxValueChanged:
                        {
                            var val = dmxMode ? dmxValue : dmxValue * 2.55
                            if (sceneConsole == false)
                                fixtureManager.setChannelValue(fixtureObj.id, index, val)
                            else
                                sceneEditor.setChannelValue(fixtureObj.id, index, val)
                        }

                        // This is the black overlay to "emulate" a disabled channel
                        Rectangle
                        {
                            x: 1
                            y: 16
                            z: 2
                            width: parent.width - 2
                            height: chColumn.height - 16
                            color: "black"
                            opacity: 0.7
                            visible: showEnablers ? !isEnabled : false
                        }

                        ColumnLayout
                        {
                            id: chColumn
                            x: 1
                            y: 1
                            width: parent.width - 2
                            spacing: 1

                            Rectangle
                            {
                                id: enableCheckBox
                                width: 34
                                height: 15
                                radius: 2
                                visible: showEnablers
                                color: isEnabled ? "#0978FF" : "#333"
                                border.width: 1
                                border.color: isEnabled ? "white" : "#555"
                                Layout.alignment: Qt.AlignCenter

                                MouseArea
                                {
                                    anchors.fill: parent
                                    onClicked:
                                    {
                                        isEnabled = !isEnabled
                                        if (sceneConsole == true)
                                        {
                                            if (isEnabled == true)
                                                sceneEditor.setChannelValue(fixtureObj.id, index, 0)
                                            else
                                                sceneEditor.unsetChannel(fixtureObj.id, index)
                                        }
                                    }
                                }
                            }

                            Image
                            {
                                width: 32
                                height: 32
                                Layout.alignment: Qt.AlignCenter
                                sourceSize: Qt.size(width, height)
                                source: fixtureObj ? fixtureManager.channelIcon(fixtureObj.id, index) : ""
                            }
                            QLCPlusFader
                            {
                                id: slider
                                width: 32
                                Layout.preferredHeight:
                                    chDelegate.height ? chDelegate.height - 32 - 25 - (showEnablers ? 15 : 0) - 4 : 0
                                Layout.alignment: Qt.AlignCenter
                                minimumValue: 0
                                maximumValue: dmxMode ? 255 : 100
                                enabled: showEnablers ? isEnabled : true

                                Component.onCompleted:
                                {
                                    if (sceneConsole)
                                    {
                                        if (sceneEditor.hasChannel(fixtureObj.id, index) === true)
                                        {
                                            if (showEnablers)
                                                isEnabled = true
                                            slider.value = sceneEditor.channelValue(fixtureObj.id, index)
                                        }
                                    }
                                }
                            }
                            CustomSpinBox
                            {
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
}

