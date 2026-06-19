/*
  Q Light Controller Plus
  EffectToggleRow.qml

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

import QtQuick
import QtQuick.Layouts
import "."

Rectangle
{
    id: row

    property var effectData: ({})
    readonly property bool isEnabled:   effectData.enabled   || false
    readonly property bool isAvailable: effectData.available !== false

    height: UISettings.listItemHeight * 1.05
    color: hovered && isAvailable ? "#151530" : "#0E0E20"
    opacity: isAvailable ? 1.0 : 0.4

    property bool hovered: false

    Behavior on color   { ColorAnimation { duration: 100 } }
    Behavior on opacity { NumberAnimation { duration: 150 } }

    MouseArea
    {
        anchors.fill: parent
        enabled: row.isAvailable
        hoverEnabled: true
        onEntered: row.hovered = true
        onExited:  row.hovered = false
        onClicked:
        {
            if (stageWizard && row.isAvailable)
                stageWizard.setEffectEnabled(effectData.flag, !row.isEnabled)
        }
    }

    RowLayout
    {
        anchors.fill: parent
        anchors.leftMargin: 12
        anchors.rightMargin: 12
        spacing: 12

        // Toggle pill
        Rectangle
        {
            width:  UISettings.listItemHeight * 1.4
            height: UISettings.listItemHeight * 0.6
            radius: height / 2
            color:  row.isEnabled ? "#E94560" : "#222244"
            border.color: row.isEnabled ? "transparent" : "#333366"
            Behavior on color { ColorAnimation { duration: 150 } }

            // Knob
            Rectangle
            {
                width: parent.height - 4
                height: width
                radius: width / 2
                color: "white"
                anchors.verticalCenter: parent.verticalCenter
                x: row.isEnabled ? parent.width - width - 2 : 2
                Behavior on x { NumberAnimation { duration: 150; easing.type: Easing.InOutQuad } }
            }
        }

        // Effect name
        RobotoText
        {
            label: effectData.name || ""
            fontSize: UISettings.textSizeDefault
            fontBold: row.isEnabled
            labelColor: row.isEnabled ? "white" : "#666688"
            width: UISettings.bigItemHeight * 2
            Behavior on labelColor { ColorAnimation { duration: 150 } }
        }

        Item { Layout.fillWidth: true }

        // Preview count
        RobotoText
        {
            label: effectData.preview || ""
            fontSize: UISettings.textSizeDefault * 0.82
            labelColor: "#555577"
            textHAlign: Text.AlignRight
        }

        // Not-available badge
        Rectangle
        {
            visible: !row.isAvailable
            height: UISettings.textSizeDefault * 1.4
            width: naText.implicitWidth + 12
            radius: height / 2
            color: "#3A2A2A"

            RobotoText
            {
                id: naText
                anchors.centerIn: parent
                label: qsTr("No compatible fixtures")
                fontSize: UISettings.textSizeDefault * 0.75
                labelColor: "#AA6666"
            }
        }
    }

    // Bottom separator line
    Rectangle
    {
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.leftMargin: 12
        height: 1
        color: "#1A1A30"
    }
}
