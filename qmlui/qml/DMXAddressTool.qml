/*
  Q Light Controller Plus
  DMXAddressTool.qml

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
import QtQuick.Layouts 1.2

import "."

GridLayout
{
    id: toolRoot
    implicitHeight: 100
    implicitWidth: 400

    columns: 4

    property alias flipHorizontally: flipHoriz.checked
    property alias flipVertically: flipVert.checked
    property alias value: dmxValue.value
    property string currentColor: "red"

    // row 1
    DMXAddressWidget
    {
        Layout.columnSpan: 4
        Layout.fillWidth: true

        color: currentColor
        flipHorizontally: toolRoot.flipHorizontally
        flipVertically: toolRoot.flipVertically
        currentValue: toolRoot.value
        onValueChanged: toolRoot.value = value
    }

    // row 2
    RobotoText
    {
        Layout.columnSpan: 2
        Layout.fillWidth: true
        label: qsTr("Address")
        textHAlign: Text.AlignRight
    }

    CustomSpinBox
    {
        id: dmxValue
        Layout.columnSpan: 2
        Layout.fillWidth: true
        from: 1
        to: 511
        //value: 1
    }

    // row 3
    CustomCheckBox
    {
        id: flipVert
        autoExclusive: false
        implicitHeight: UISettings.iconSizeMedium
        implicitWidth: implicitHeight
    }
    RobotoText
    {
        label: qsTr("Reverse vertically")
    }

    CustomCheckBox
    {
        id: flipHoriz
        autoExclusive: false
        implicitHeight: UISettings.iconSizeMedium
        implicitWidth: implicitHeight

    }
    RobotoText
    {
        label: qsTr("Reverse horizontally")
    }

    // row 4
    RobotoText
    {
        Layout.columnSpan: 2
        Layout.fillWidth: true
        label: qsTr("Color")
        textHAlign: Text.AlignRight
    }

    Row
    {
        Layout.columnSpan: 2
        Layout.fillWidth: true
        spacing: 5

        IconButton
        {
            bgColor: "red"
            hoverColor: bgColor
            onClicked: currentColor = bgColor
        }
        IconButton
        {
            bgColor: "blue"
            hoverColor: bgColor
            onClicked: currentColor = bgColor
        }
        IconButton
        {
            bgColor: "black"
            hoverColor: bgColor
            onClicked: currentColor = bgColor
        }
    }
}
