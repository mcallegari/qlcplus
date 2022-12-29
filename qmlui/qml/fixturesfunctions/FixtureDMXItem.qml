/*
  Q Light Controller Plus
  FixtureDMXItem.qml

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
import org.qlcplus.classes 1.0
import "."

Rectangle
{
    id: dmxItemRoot
    property Fixture fixtureObj
    property variant values
    property bool isSelected: false

    //signal requestTool(var item, var fixtureID, var chIndex, var value)

    onValuesChanged:
    {
        for (var i = 0; i < values.length; i++)
        {
            //console.log("Value " + i + " = " + values[i]);
            if (fxColumn.visible == true)
                channelsRpt.itemAt(i).dmxValue = values[i]
            else
                consoleLoader.setValues(values)
        }
    }

    function updateChannels()
    {
        if (fxColumn.visible == false)
            consoleLoader.item.updateChannels()

        for (var i = 0; i < channelsRpt.count; i++)
            channelsRpt.itemAt(i).updateChannel()
    }

    width: channelsRow.width
    height: fxColumn.height
    color: UISettings.bgLighter
    border.width: 1
    border.color: "#222"

    onWidthChanged:
    {
        dmxItemRoot.parent.itemWidthChanged(width)
    }

    Column
    {
        id: fxColumn
        anchors.margins: 1

        Rectangle
        {
            color: "#111"
            width: parent.width
            height: UISettings.listItemHeight * 0.75
            clip: true

            RobotoText
            {
                anchors.verticalCenter: parent.verticalCenter
                x: 2
                label: fixtureObj ? fixtureObj.name : ""
                fontSize: UISettings.textSizeDefault
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
                        width: UISettings.iconSizeMedium
                        height: chColumn.height

                        property string dmxValue: "0"

                        function updateChannel()
                        {
                            fxChIcon.source = fixtureObj ? fixtureManager.channelIcon(fixtureObj.id, index) : ""
                        }

                        Column
                        {
                            id: chColumn
                            width: parent.width

                            Image
                            {
                                id: fxChIcon
                                width: parent.width
                                height: width
                                sourceSize: Qt.size(width, height)
                                source: fixtureObj ? fixtureManager.channelIcon(fixtureObj.id, index) : ""
                            }
                            RobotoText
                            {
                                id: fxChAddress
                                visible: ViewDMX.showAddresses
                                anchors.horizontalCenter: parent.horizontalCenter
                                height: UISettings.listItemHeight * 0.75
                                fontSize: UISettings.textSizeDefault
                                labelColor: "black"
                                fontBold: true
                                label: ViewDMX.relativeAddresses ? (index + 1) : (fixtureObj ? fixtureObj.address + index + 1 : 0)
                            }
                            RobotoText
                            {
                                id: fxChVal
                                anchors.horizontalCenter: parent.horizontalCenter
                                height: UISettings.listItemHeight * 0.75
                                fontSize: UISettings.textSizeDefault
                                labelColor: "black"
                                label: dmxValue
                            }
                        }

                        // vertical divider between channels
                        Rectangle
                        {
                            visible: (index == fixtureObj.channels - 1) ? false : true
                            width: 1
                            height: parent.height
                            x: parent.width - 1
                            color: "#222"
                        }
                    }
            }
        }
    }
    Timer
    {
        id: clickTimer
        interval: 200
        repeat: false
        running: false
        onTriggered:
        {
            isSelected = !isSelected
            contextManager.setFixtureIDSelection(fixtureObj.id, isSelected)
        }
    }

    MouseArea
    {
        anchors.fill: parent
        onClicked: clickTimer.start()
        onDoubleClicked:
        {
            clickTimer.stop()
            fxColumn.visible = false
            consoleLoader.source = "qrc:/FixtureConsole.qml"
        }
    }
    Loader
    {
        id: consoleLoader
        anchors.fill: parent

        function setValues(values)
        {
            item.values = values
        }

        onLoaded:
        {
            item.fixtureObj = fixtureObj
            item.isSelected = isSelected
            item.values = values
            item.height = UISettings.bigItemHeight * 2.3
        }
        Connections
        {
             target: consoleLoader.item
             function onClicked()
             {
                clickTimer.start()
             }
             function onDoubleClicked()
             {
                 clickTimer.stop()
                 consoleLoader.source = ""
                 dmxItemRoot.width = channelsRow.width
                 dmxItemRoot.height = fxColumn.height
                 fxColumn.visible = true
             }
             function onSizeChanged(w, h)
             {
                 if (w !== 0 && h !== 0)
                 {
                     dmxItemRoot.width = w
                     dmxItemRoot.height = h
                     //console.log("2- Item width: " + w + ", height: " + h)
                 }
             }
             function onValueChanged(fixtureID, chIndex, value)
             {
                 //console.log("Channel " + chIndex + " value changed " + value)
                 channelsRpt.itemAt(chIndex).dmxValue = value
             }

             function onRequestTool(item, fixtureID, chIndex, value)
             {
                 //dmxItemRoot.requestTool(item, fixtureID, chIndex, value)
                 dmxItemRoot.parent.loadTool(item, fixtureID, chIndex, value)
             }
        }
    }

    Rectangle
    {
        anchors.fill: parent
        z: 5
        color: "transparent"
        border.width: isSelected ? 2 : 1
        border.color: isSelected ? UISettings.selection : "transparent"
    }
}
