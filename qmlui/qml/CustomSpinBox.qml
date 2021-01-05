/*
  Q Light Controller Plus
  CustomSpinBox.qml

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

import QtQuick 2.3
import QtQuick.Controls 2.14
import "."

SpinBox
{
    id: control
    font.family: UISettings.robotoFontName
    font.pixelSize: UISettings.textSizeDefault
    width: UISettings.bigItemHeight
    height: UISettings.listItemHeight
    implicitWidth: UISettings.bigItemHeight
    implicitHeight: UISettings.listItemHeight
    editable: true
    from: 0
    to: 255
    clip: true
    wheelEnabled: true

    property bool showControls: true
    property string suffix: ""
    property alias horizontalAlignment: textControl.horizontalAlignment
    property int controlWidth: showControls ? Math.min(UISettings.iconSizeMedium, control.width / 3) : 0

    onFromChanged: if (value < from) control.value = from
    onToChanged: if (value > to) control.value = to

    onFocusChanged:
    {
        if (focus) contentItem.selectAll()
    }

    textFromValue: function(value) {
        return value + suffix
    }

    valueFromText: function(text) {
        return parseInt(text.replace(suffix, ""))
    }

    Rectangle
    {
        anchors.fill: parent
        z: 3
        color: "black"
        opacity: 0.6
        visible: !parent.enabled
    }

    background: Rectangle {
        implicitWidth: parent.width
        color: UISettings.bgControl
        border.color: "#222"
        radius: 3
    }

    contentItem: TextInput {
        id: textControl
        z: 2
        height: control.height
        font: control.font
        text: control.textFromValue(control.value, control.locale)
        color: UISettings.fgMain
        selectByMouse: true
        selectionColor: UISettings.highlightPressed
        selectedTextColor: "white"
        horizontalAlignment: Qt.AlignRight
        verticalAlignment: Qt.AlignVCenter

        readOnly: !control.editable
        validator: control.validator
        inputMethodHints: Qt.ImhFormattedNumbersOnly
    }

    up.indicator: Rectangle {
        visible: showControls
        x: parent.width - width
        implicitHeight: parent.height / 2
        implicitWidth: controlWidth
        color: up.pressed ? UISettings.bgLight : UISettings.bgControl
        border.color: UISettings.bgStrong

        Image
        {
            anchors.centerIn: parent
            source: "qrc:/arrow-up.svg"
            width: height * 2
            height: parent.height - 8
            sourceSize: Qt.size(width, height)
        }
    }

    down.indicator: Rectangle {
        visible: showControls
        x: parent.width - width
        y: parent.height / 2
        implicitWidth: controlWidth
        implicitHeight: parent.height / 2
        color: down.pressed ? UISettings.bgLight : UISettings.bgControl
        border.color: UISettings.bgStrong

        Image
        {
            anchors.centerIn: parent
            source: "qrc:/arrow-up.svg"
            rotation: 180
            width: height * 2
            height: parent.height - 8
            sourceSize: Qt.size(width, height)
        }
    }
}
