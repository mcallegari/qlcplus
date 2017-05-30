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

import org.qlcplus.classes 1.0
import "GenericHelpers.js" as Helpers
import "."

Rectangle
{
    width: parent.width
    height: UISettings.bigItemHeight * 0.9
    color: "transparent"

    property int universeID
    property InputPatch patch

    Rectangle
    {
        id: patchBox
        width: profileBox.visible ? parent.width - 10 : parent.width
        height: profileBox.visible ? UISettings.bigItemHeight * 0.6 : UISettings.bigItemHeight * 0.8
        y: profileBox.visible ? UISettings.bigItemHeight * 0.2 : 0
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

        /* LED kind-of signal indicator */
        Rectangle
        {
            id: valueChangeBox
            x: parent.width - width - 10
            y: 10
            z: 1
            width: UISettings.iconSizeMedium * 0.75
            height: width
            radius: height / 2
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
                label: patch ? patch.inputName : ""
                labelColor: "black"
                wrapText: true
                fontSize: UISettings.textSizeDefault
            }
        }
    }

    Rectangle
    {
        id: profileBox
        width: parent.width
        height: UISettings.bigItemHeight * 0.8
        visible: patch ? (patch.profileName === "None" ? false : true) : false

        border.width: 2
        border.color: "#222"
        color: "#269ABA"
        radius: 10

        RobotoText
        {
            x: 10
            y: 3
            height: UISettings.bigItemHeight * 0.2
            width: parent.width - 20
            label: patch ? patch.profileName : ""
            labelColor: "black"
            fontSize: height ? height * 0.7 : 20
            //wrapText: true
        }
    }
}

