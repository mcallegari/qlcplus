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

import org.qlcplus.classes 1.0
import "."

Rectangle
{
    id: consoleRoot
    width: channelsRow.width
    height: parent.height
    color: UISettings.bgStrong

    onWidthChanged: consoleRoot.sizeChanged(width, height)
    onHeightChanged:
    {
        if (height < UISettings.iconSizeDefault * 3)
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
    property bool externalChange: false

    signal doubleClicked
    signal clicked
    signal sizeChanged(var w, var h)
    signal valueChanged(var fixtureID, var chIndex, var value)

    onValuesChanged:
    {
        externalChange = true
        for (var i = 0; i < values.length; i++)
        {
            //console.log("Value " + i + " = " + values[i]);
            if (dmxValues)
                channelsRpt.itemAt(i).dmxValue = values[i]
            else
                channelsRpt.itemAt(i).dmxValue = (values[i] / 255) * 100
        }
        externalChange = false
    }

    function setChannelValue(channel, value)
    {
        if (showEnablers == true)
            channelsRpt.itemAt(channel).isEnabled = true
        externalChange = true
        channelsRpt.itemAt(channel).dmxValue = value
        externalChange = false
    }

    function updateChannels()
    {
        for (var i = 0; i < channelsRpt.count; i++)
            channelsRpt.itemAt(i).updateChannel()
    }

    Column
    {
        id: fxColumn
        height: parent.height
        visible: false

        Rectangle
        {
            id: fxNameBar
            color: UISettings.bgStronger
            width: parent.width
            height: UISettings.listItemHeight
            clip: true

            RobotoText
            {
                anchors.verticalCenter: parent.verticalCenter
                x: 2
                label: fixtureObj ? fixtureObj.name : ""
                fontSize: UISettings.textSizeDefault
            }
            DMXPercentageButton
            {
                x: parent.width - width - 3
                y: 1
                z: 2
                height: parent.height - 2
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
                        width: UISettings.iconSizeDefault
                        height: channelsRow.height

                        property alias dmxValue: chValueSpin.value
                        property bool dmxMode: true
                        property bool isEnabled: showEnablers ? false : true

                        function updateChannel()
                        {
                            chIcon.source = fixtureObj ? fixtureManager.channelIcon(fixtureObj.id, index) : ""
                        }

                        onDmxValueChanged:
                        {
                            if (externalChange == true)
                                return

                            var val = dmxMode ? dmxValue : dmxValue * 2.55
                            if (sceneConsole == false)
                                fixtureManager.setChannelValue(fixtureObj.id, index, val)
                            else
                                functionManager.setChannelValue(fixtureObj.id, index, val)

                            consoleRoot.valueChanged(fixtureObj.id, index, val)
                        }

                        // This is the black overlay to "simulate" a disabled channel
                        Rectangle
                        {
                            x: 1
                            y: chIcon.y
                            z: 2
                            width: parent.width - 2
                            height: chDelegate.height - enableCheckBox.height
                            color: "black"
                            opacity: 0.7
                            visible: showEnablers ? !isEnabled : false
                        }

                        Rectangle
                        {
                            width: parent.width
                            height: (showEnablers ? enableCheckBox.height : 0) + chIcon.height + 1
                            color: UISettings.bgLight
                        }

                        Column
                        {
                            id: chColumn
                            x: 1
                            y: 1
                            width: parent.width - 2
                            height: parent.height
                            spacing: 1

                            Rectangle
                            {
                                id: enableCheckBox
                                x: (parent.width - width) / 2
                                width: UISettings.iconSizeMedium
                                height: UISettings.iconSizeMedium / 2
                                radius: 2
                                visible: showEnablers
                                color: isEnabled ? UISettings.highlight : UISettings.bgLight
                                border.width: 1
                                border.color: isEnabled ? "white" : UISettings.bgLighter
                                //Layout.alignment: Qt.AlignCenter

                                MouseArea
                                {
                                    anchors.fill: parent
                                    onClicked:
                                    {
                                        isEnabled = !isEnabled
                                        if (sceneConsole == true)
                                        {
                                            if (isEnabled == true)
                                                functionManager.setChannelValue(fixtureObj.id, index, dmxMode ? dmxValue : dmxValue * 2.55)
                                            else
                                                sceneEditor.unsetChannel(fixtureObj.id, index)
                                        }
                                    }
                                }
                            }

                            Image
                            {
                                id: chIcon
                                x: (parent.width - width) / 2
                                width: UISettings.iconSizeMedium
                                height: width
                                //Layout.alignment: Qt.AlignCenter
                                sourceSize: Qt.size(width, height)
                                source: fixtureObj ? fixtureManager.channelIcon(fixtureObj.id, index) : ""
                            }

                            QLCPlusFader
                            {
                                id: slider
                                x: (parent.width - width) / 2
                                width: parent.width * 0.95
                                height: chDelegate.height - (showEnablers ? enableCheckBox.height : 0) - chIcon.height - chValueSpin.height
                                from: 0
                                to: dmxMode ? 255 : 100
                                value: dmxValue
                                enabled: showEnablers ? isEnabled : true
                                onPositionChanged: dmxValue = valueAt(position)

                                Component.onCompleted:
                                {
                                    if (sceneConsole)
                                    {
                                        if (showEnablers && sceneEditor.hasChannel(fixtureObj.id, index) === true)
                                            isEnabled = true

                                        dmxValue = sceneEditor.channelValue(fixtureObj.id, index)
                                    }
                                }
                            }

                            CustomSpinBox
                            {
                                id: chValueSpin
                                width: parent.width
                                height: UISettings.listItemHeight * 0.75
                                from: 0
                                to: dmxMode ? 255 : 100
                                suffix: dmxMode ? "" : "%"
                                showControls: false
                                padding: 0
                                horizontalAlignment: Qt.AlignHCenter
                                onValueChanged: dmxValue = value
                            }
                        }
                    }
            }
        }
    }
}

