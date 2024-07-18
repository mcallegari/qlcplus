/*
  Q Light Controller Plus
  PresetCapabilityItem.qml

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

import org.qlcplus.classes 1.0
import "."

Rectangle
{
    id: iRoot
    width: UISettings.bigItemHeight * 1.5
    height: UISettings.iconSizeDefault * 1.2
    color: capMouseArea.pressed ? UISettings.bgLight : "white"

    border.width: 1
    border.color: UISettings.borderColorDark

    property QLCCapability capability
    property int capIndex

    signal valueChanged(int value)

    onCapabilityChanged:
    {
        if (capability === null)
            return

        var resArray = capability.resources
        capName.label = capability.name

        if (resArray.length === 0)
        {
            capIdx.label = capIndex.toString()
            return
        }

        if (resArray[0].toString().startsWith('#'))
        {
            col1.color = resArray[0]
            col2.color = resArray[0]

            if (resArray.length > 1 && resArray[1].toString().startsWith('#'))
                col2.color = resArray[1]
        }
        else if (resArray[0].indexOf('.') !== -1)
        {
            if (Qt.platform.os === "android")
                pic.source = resArray[0]
            else
                pic.source = "file:" + resArray[0]
        }
    }

    Row
    {
        spacing: 5

        Rectangle
        {
            width: UISettings.iconSizeDefault
            height: width
            border.width: 1
            border.color: UISettings.borderColorDark

            Rectangle
            {
                id: col1
                x: 1
                y: 1
                width: parent.width / 2
                height: parent.height - 2
                color: "transparent"
            }
            Rectangle
            {
                id: col2
                x: parent.width / 2
                y: 1
                width: parent.width / 2
                height: parent.height - 2
                color: "transparent"
            }
            RobotoText
            {
                id: capIdx
                anchors.centerIn: parent
                z: 2
                label: ""
                labelColor: "black"
            }
            Image
            {
                id: pic
                x: 1
                y: 1
                width: parent.width - 2
                height: width
                anchors.centerIn: parent
                source: ""
            }
        }
        RobotoText
        {
            id: capName
            width: iRoot.width - UISettings.iconSizeDefault
            height: iRoot.height
            //label: ""
            labelColor: "black"
            fontSize: UISettings.textSizeDefault * 0.75
            wrapText: true
        }
    }
    Rectangle
    {
        id: capBar
        y: parent.height - height
        width: 0
        height: 5
        z: 10
        color: "blue"
    }
    MouseArea
    {
        id: capMouseArea
        anchors.fill: parent
        hoverEnabled: true
        preventStealing: false

        onPositionChanged: capBar.width = mouse.x
        onExited: capBar.width = 0
        onClicked:
        {
            var value = ((capability.max - capability.min) * capBar.width) / iRoot.width
            //console.log("max: " + capability.max + " min: " + capability.min + " value: " + value)
            valueChanged(value + capability.min)
        }
    }
}
