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
import QtQuick.Layouts 1.1

import org.qlcplus.classes 1.0
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
        color: UISettings.bgLighter
        border.width: 2
        border.color: UISettings.borderColorDark

        RowLayout
        {
            x: 8
            width: parent.width - 16
            spacing: 5

            Column
            {
                height: patchBox.height
                spacing: 1

                IconButton
                {
                    imgSource: checked ? "qrc:/pause.svg" : "qrc:/play.svg"
                    bgColor: "green"
                    checkedColor: "red"
                    checkable: true
                    checked: patch ? patch.paused : false
                    tooltip: qsTr("Play/Pause this output patch")
                    onToggled: if (patch) patch.paused = checked
                }
                IconButton
                {
                    faSource: checked ? FontAwesome.fa_eye_slash : FontAwesome.fa_eye
                    faColor: UISettings.fgMain
                    bgColor: "green"
                    checkedColor: "red"
                    checkable: true
                    checked: patch ? patch.blackout : false
                    tooltip: qsTr("Enable/Disable a blackout on this output patch")
                    onToggled: if (patch) patch.blackout = checked
                }
            }

            Image
            {
                Layout.alignment: Qt.AlignVCenter
                height: patchBox.height * 0.75
                width: height
                source: patch ? Helpers.pluginIconFromName(patch.pluginName) : ""
                sourceSize: Qt.size(width, height)
                fillMode: Image.Stretch
            }
            RobotoText
            {
                height: patchBox.height
                Layout.fillWidth: true
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
            color: patchDropTarget.containsDrag ? "#7F00FF00" : "transparent"
        }
    }
}
