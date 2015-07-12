/*
  Q Light Controller Plus
  InputPatchItem.qml

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
import "PluginUtils.js" as PluginUtils

Rectangle
{
    width: parent.width
    height: profileBox.visible ? 110 : 80
    color: "transparent"

    property int universeID
    property InputPatch patch

    Rectangle
    {
        id: patchBox
        width: profileBox.visible ? parent.width - 10 : parent.width
        height: 80
        y: profileBox.visible ? 25 : 0
        x: profileBox.visible ? 5 : 0
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

        Rectangle
        {
            id: valueChangeBox
            x: parent.width - 30
            y: 10
            z: 1
            width: 20
            height: 20
            radius: 10
            border.width: 2
            border.color: "#333"
            color: "#666"

            ColorAnimation on color
            {
                id: cAnim
                from: "#00FF00"
                to: "#666"
                duration: 500
                running: false
            }

            Connections
            {
                id: valChangedSignal
                target: patch
                onInputValueChanged: cAnim.restart()
            }
        }

        Row
        {
            x: 8
            spacing: 3
            Image
            {
                id: pluginIcon
                y: 2
                height: patchBox.height - 6
                width: height
                source: patch ? PluginUtils.iconFromName(patch.pluginName) : ""
                sourceSize: Qt.size(width, height)
                fillMode: Image.Stretch
            }
            RobotoText
            {
                height: patchBox.height
                width: patchBox.width - pluginIcon.width - 6
                label: patch ? patch.inputName : ""
                labelColor: "black"
                wrapText: true
            }
        }
    }
    Rectangle
    {
        id: profileBox
        width: parent.width
        height: patchBox.height + 30
        visible: patch.profileName === "None" ? false : true

        border.width: 2
        border.color: "#222"
        color: "#269ABA"
        radius: 10

        RobotoText
        {
            x: 10
            y: 3
            height: 20
            width: parent.width - 20
            label: patch ? patch.profileName : ""
            labelColor: "black"
            //wrapText: true
        }
    }
}

