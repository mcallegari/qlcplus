/*
  Q Light Controller Plus
  WizardOptionRow.qml

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

// A wizard option card: toggle pill + bold title + wrapped explanation.
// Clicking anywhere on the card flips it. Same toggle styling as
// EffectToggleRow, which is the compact single-line variant used in step 4.
Rectangle
{
    id: optRoot

    property bool checked: false
    property string title
    property string body
    property bool rowEnabled: true

    signal toggled(bool value)

    property bool hovered: false

    // Card height follows its content row. The row anchors to the top rather
    // than the vertical centre: centring makes the row's position depend on the
    // card height, which is derived from the row — an indirect binding loop.
    // Set as implicitHeight, not height, so a parent ColumnLayout honours it
    // (layouts size children from their implicit/Layout.* hints and overwrite
    // a plain height).
    implicitHeight: optRow.implicitHeight + 20
    radius: 8
    color: hovered && rowEnabled ? "#151530" : "#0E0E20"
    opacity: rowEnabled ? 1.0 : 0.4

    Behavior on color   { ColorAnimation { duration: 100 } }
    Behavior on opacity { NumberAnimation { duration: 150 } }

    MouseArea
    {
        anchors.fill: parent
        enabled: optRoot.rowEnabled
        hoverEnabled: true
        onEntered: optRoot.hovered = true
        onExited:  optRoot.hovered = false
        onClicked: optRoot.toggled(!optRoot.checked)
    }

    RowLayout
    {
        id: optRow
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.topMargin: 10
        anchors.leftMargin: 12
        anchors.rightMargin: 12
        spacing: 12

        // Toggle pill
        Rectangle
        {
            Layout.alignment: Qt.AlignTop
            width:  UISettings.listItemHeight * 1.4
            height: UISettings.listItemHeight * 0.6
            radius: height / 2
            color:  optRoot.checked ? "#E94560" : "#222244"
            border.color: optRoot.checked ? "transparent" : "#333366"
            Behavior on color { ColorAnimation { duration: 150 } }

            // Knob
            Rectangle
            {
                width: parent.height - 4
                height: width
                radius: width / 2
                color: "white"
                anchors.verticalCenter: parent.verticalCenter
                x: optRoot.checked ? parent.width - width - 2 : 2
                Behavior on x { NumberAnimation { duration: 150; easing.type: Easing.InOutQuad } }
            }
        }

        ColumnLayout
        {
            id: optCol
            Layout.fillWidth: true
            spacing: 3

            RobotoText
            {
                label: optRoot.title
                fontBold: true
                fontSize: UISettings.textSizeDefault
                labelColor: optRoot.checked ? "white" : "#8888AA"
                Behavior on labelColor { ColorAnimation { duration: 150 } }
            }
            RobotoText
            {
                Layout.fillWidth: true
                label: optRoot.body
                fontSize: UISettings.textSizeDefault * 0.85
                labelColor: "#777799"
                wrapText: true
            }
        }
    }
}
