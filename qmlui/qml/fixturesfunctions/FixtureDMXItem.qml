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

    width: channelsRow.width
    height: fxColumn.height
    color: "#777"
    border.width: 1
    border.color: "#222"

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
                        height: fxChIcon.height + fxChVal.height

                        property string dmxValue: "0"

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
                            id: fxChVal
                            anchors.horizontalCenter: parent.horizontalCenter
                            y: fxChIcon.height
                            //width: 30
                            height: UISettings.listItemHeight * 0.75
                            fontSize: UISettings.textSizeDefault
                            labelColor: "black"
                            label: dmxValue
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
            contextManager.setFixtureSelection(fixtureObj.id, isSelected)
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
             onClicked: clickTimer.start()
             onDoubleClicked:
             {
                 clickTimer.stop()
                 consoleLoader.source = ""
                 dmxItemRoot.width = channelsRow.width
                 dmxItemRoot.height = fxColumn.height
                 fxColumn.visible = true
             }
             onSizeChanged:
             {
                 if (w != 0 && h != 0)
                 {
                     dmxItemRoot.width = w
                     dmxItemRoot.height = h
                     //console.log("2- Item width: " + w + ", height: " + h)
                 }
             }
             onValueChanged:
             {
                 //console.log("Channel " + chIndex + " value changed " + value)
                 channelsRpt.itemAt(chIndex).dmxValue = value
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
