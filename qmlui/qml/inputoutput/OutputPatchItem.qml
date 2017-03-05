/*
  Q Light Controller Plus
  OutputPatchItem.qml

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

import com.qlcplus.classes 1.0
import "GenericHelpers.js" as Helpers
import "."

Rectangle
{
    width: parent.width
    height: UISettings.bigItemHeight * 0.9
    color: "transparent"

    property int universeID
    property OutputPatch patch
    property int patchIndex

    Rectangle
    {
        id: patchBox
        width: parent.width
        height: parent.height
        z: 1
        radius: 3
        gradient: Gradient
        {
            id: bgGradient
            GradientStop { position: 0.75 ; color: "#999" }
            GradientStop { position: 1 ; color: "#333" }
        }
        border.width: 2
        border.color: "#111"

        Row
        {
            x: 8
            spacing: 3

            Image
            {
                id: pluginIcon
                anchors.verticalCenter: parent.verticalCenter
                height: patchBox.height * 0.75
                width: height
                source: patch ? Helpers.pluginIconFromName(patch.pluginName) : ""
                sourceSize: Qt.size(width, height)
                fillMode: Image.Stretch
            }
            RobotoText
            {
                height: patchBox.height
                width: patchBox.width - pluginIcon.width - 6
                label: patch ? patch.outputName : ""
                labelColor: "black"
                wrapText: true
                fontSize: UISettings.textSizeDefault
            }
        }
    }

    DropArea
    {
        id: patchDropTarget
        anchors.fill: parent
        z: 5

        // this key must match the one in PluginList, to avoid dropping
        // an input plugin on output and vice-versa
        keys: [ "output-" + universeID ]

        onDropped:
        {
            console.log("Requested to replace an output patch")
            ioManager.setOutputPatch(drag.source.pluginUniverse, drag.source.pluginName,
                                     drag.source.pluginLine, patchIndex)
        }

        Rectangle
        {
            id: outDropRect
            anchors.fill: parent
            color: patchDropTarget.containsDrag ? "#33FF9B3E" : "transparent"
        }
    }
}
