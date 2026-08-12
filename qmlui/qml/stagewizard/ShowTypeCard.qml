/*
  Q Light Controller Plus
  ShowTypeCard.qml

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
    id: card

    property var  showData: ({})
    property bool isSelected: false

    signal clicked()

    implicitHeight: contentArea.y + contentArea.height + UISettings.listItemHeight * 0.4
    radius: 10

    clip: true
    color: isSelected ? "#1E1040" : (hovered ? "#1A1A3A" : "#111128")
    border.color: isSelected ? "#0978FF" : (hovered ? "#555577" : "#2A2A44")
    border.width: isSelected ? 2 : 1

    Behavior on color        { ColorAnimation { duration: 150 } }
    Behavior on border.color { ColorAnimation { duration: 150 } }

    property bool hovered: false

    MouseArea
    {
        anchors.fill: parent
        hoverEnabled: true
        onEntered:  card.hovered = true
        onExited:   card.hovered = false
        onClicked:  card.clicked()
    }

    // ── Colour stripe at top ──────────────────────────────────────────────
    Row
    {
        id: stripe
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: 8
        spacing: 0

        Repeater
        {
            model: showData.palette ? showData.palette : []
            Rectangle
            {
                width:  stripe.width / (showData.palette ? showData.palette.length : 1)
                height: parent.height
                color:  modelData
                radius: index === 0 ? 12 : (index === (showData.palette.length - 1) ? 12 : 0)
            }
        }
    }

    // ── Content ────────────────────────────────────────────────────────────
    Column
    {
        id: contentArea
        anchors.top: stripe.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: UISettings.listItemHeight * 0.4
        spacing: 6

        // Emoji icon + name
        Row
        {
            width: parent.width
            spacing: 8

            Text
            {
                text: showData.icon || ""
                font.pixelSize: UISettings.textSizeDefault * 1.5
            }

            RobotoText
            {
                width: parent.width - UISettings.textSizeDefault * 1.5 - parent.spacing
                label: showData.name || ""
                fontSize: UISettings.textSizeDefault * 1.1
                fontBold: true
                labelColor: card.isSelected ? "#0978FF" : "white"
                Behavior on labelColor { ColorAnimation { duration: 150 } }
            }
        }

        // Description
        RobotoText
        {
            width: parent.width
            label: showData.description || ""
            fontSize: UISettings.textSizeDefault * 0.82
            labelColor: "#999ABB"
            wrapText: true
        }

        // Stage label
        Row
        {
            spacing: 4
            RobotoText
            {
                label: qsTr("Stage:")
                fontSize: UISettings.textSizeDefault * 0.78
                labelColor: "#555577"
            }
            RobotoText
            {
                label: showData.stage || ""
                fontSize: UISettings.textSizeDefault * 0.78
                labelColor: "#8888AA"
            }
        }

        // Tags
        Row
        {
            //width: parent.width
            spacing: 3

            Repeater
            {
                model: showData.tags ? showData.tags : []

                Rectangle
                {
                    height: UISettings.textSizeDefault * 1.5
                    width: tagText.width + UISettings.iconSizeMedium / 3
                    radius: height / 3
                    color: "#2A2A50"

                    RobotoText
                    {
                        id: tagText
                        anchors.centerIn: parent
                        label: modelData
                        fontSize: UISettings.textSizeDefault * 0.72
                        labelColor: "#8888CC"
                    }
                }
            }
        }
    }

    // Selected indicator
    Rectangle
    {
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.topMargin: 20
        anchors.rightMargin: 15
        width: UISettings.listItemHeight * 0.9
        height: width
        radius: width / 2
        color: "#0978FF"
        visible: card.isSelected

        RobotoText
        {
            anchors.centerIn: parent
            label: "✓"
            labelColor: "white"
            fontBold: true
            fontSize: UISettings.textSizeDefault * 0.9
        }
    }
}
