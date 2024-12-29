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
    property real maxValue: 255
    property bool isSelected: false
    property bool showEnablers: false
    property bool sceneConsole: false
    property bool externalChange: false
    property bool multipleSelection: false

    signal doubleClicked
    signal clicked
    signal sizeChanged(var w, var h)
    signal valueChanged(var fixtureID, var chIndex, var value)
    signal requestTool(var item, var fixtureID, var chIndex, var value)

    onValuesChanged:
    {
        externalChange = true
        for (var i = 0; i < values.length; i++)
        {
            //console.log("Value " + i + " = " + values[i]);
            channelsRpt.itemAt(i).dmxValue = values[i]
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
                    if (dmxValues)
                    {
                        dmxValues = false
                        maxValue = 100
                    }
                    else
                    {
                        maxValue = 255
                        dmxValues = true
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
                        color: isSelected ? UISettings.selection : "transparent"
                        border.width: 1
                        border.color: UISettings.borderColorDark
                        width: UISettings.iconSizeDefault
                        height: channelsRow.height

                        property real dmxValue
                        property bool isEnabled: showEnablers ? false : true
                        property bool isSelected: false

                        function updateChannel()
                        {
                            chIcon.source = fixtureObj ? fixtureManager.channelIcon(fixtureObj.id, index) : ""
                            chIcon.tooltip = fixtureObj ? fixtureManager.channelName(fixtureObj.id, index) : ""
                        }

                        onDmxValueChanged:
                        {
                            if (externalChange == true)
                                return

                            if (sceneConsole == false)
                                fixtureManager.setChannelValue(fixtureObj.id, index, dmxValue)
                            else
                                functionManager.setChannelValue(fixtureObj.id, index, dmxValue)

                            consoleRoot.valueChanged(fixtureObj.id, index, dmxValue)
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

                            // channel enable/disable
                            Rectangle
                            {
                                id: enableCheckBox
                                x: (parent.width - width) / 2
                                width: UISettings.iconSizeMedium
                                height: UISettings.iconSizeMedium / 2
                                radius: 2
                                visible: showEnablers
                                color: isEnabled ? (chDelegate.isSelected ? UISettings.selection : UISettings.highlight) : UISettings.bgLight
                                border.width: 1
                                border.color: isEnabled ? "white" : UISettings.bgLighter
                                //Layout.alignment: Qt.AlignCenter

                                MouseArea
                                {
                                    anchors.fill: parent
                                    onClicked:
                                    {
                                        if (isEnabled && ((mouse.modifiers & Qt.ControlModifier) || multipleSelection))
                                        {
                                            chDelegate.isSelected = !chDelegate.isSelected
                                            sceneEditor.setChannelSelection(fixtureObj.id, index, chDelegate.isSelected);
                                            return
                                        }

                                        isEnabled = !isEnabled
                                        if (sceneConsole == true)
                                        {
                                            if (isEnabled == true)
                                                functionManager.setChannelValue(fixtureObj.id, index, dmxValue)
                                            else
                                                sceneEditor.unsetChannel(fixtureObj.id, index)
                                        }
                                    }
                                }
                            }

                            // channel icon
                            IconButton
                            {
                                id: chIcon
                                x: (parent.width - width) / 2
                                width: UISettings.iconSizeMedium
                                height: width
                                border.width: 0
                                tooltip: fixtureObj ? fixtureManager.channelName(fixtureObj.id, index) : ""
                                imgSource: fixtureObj ? fixtureManager.channelIcon(fixtureObj.id, index) : ""

                                onClicked:
                                {
                                    if (fixtureObj)
                                        consoleRoot.requestTool(chColumn, fixtureObj.id, index, dmxValue)
                                }
                            }

                            // channel fader
                            QLCPlusFader
                            {
                                id: slider
                                x: (parent.width - width) / 2
                                width: parent.width * 0.95
                                height: chDelegate.height - (showEnablers ? enableCheckBox.height : 0) - chIcon.height - chValueSpin.height
                                from: 0
                                to: 255
                                value: dmxValue
                                enabled: showEnablers ? isEnabled : true
                                onMoved: dmxValue = valueAt(position)

                                Component.onCompleted:
                                {
                                    if (sceneConsole)
                                    {
                                        if (showEnablers && sceneEditor.hasChannel(fixtureObj.id, index) === true)
                                            isEnabled = true
                                    }
                                }
                            }

                            // channel value
                            CustomSpinBox
                            {
                                id: chValueSpin
                                width: parent.width
                                height: UISettings.listItemHeight * 0.75
                                from: 0
                                to: maxValue
                                suffix: dmxValues ? "" : "%"
                                showControls: false
                                padding: 0
                                horizontalAlignment: Qt.AlignHCenter
                                value: dmxValues ? dmxValue : (dmxValue / 255.0) * 100.0
                                onValueModified: dmxValue = dmxValues ? value : value * 2.55
                            }
                        }
                    }
            }
        }
    }
}

