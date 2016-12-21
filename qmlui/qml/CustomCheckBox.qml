/*
  Q Light Controller Plus
  CustomCheckBox.qml

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
import QtQuick.Controls 1.2
import QtQuick.Controls.Private 1.0

import "."

Rectangle
{
    id: checkBoxRoot
    width: UISettings.iconSizeDefault
    height: UISettings.iconSizeDefault

    property color bgColor: UISettings.bgMedium
    property color hoverColor: UISettings.bgLight
    property color pressColor: "#054A9E"
    property string tooltip: ""

    property bool checked: false

    property ExclusiveGroup exclusiveGroup: null

    signal toggle(bool status)

    color: bgColor
    radius: 5
    border.color: "#1D1D1D"
    border.width: 2

    onExclusiveGroupChanged:
    {
        if (exclusiveGroup)
            exclusiveGroup.bindCheckable(checkBoxRoot)
    }

    Image
    {
        id: cbIcon
        visible: checked
        anchors.fill: parent
        anchors.margins: 3
        source: "qrc:/apply.svg"
        sourceSize: Qt.size(width, height)
    }

    MouseArea
    {
        id: mouseArea1
        anchors.fill: parent
        hoverEnabled: true
        onEntered: { checkBoxRoot.color = hoverColor }
        onExited: { checkBoxRoot.color = bgColor; Tooltip.hideText() }
        onReleased:
        {
            if (exclusiveGroup && checked == true)
                return

            checked = !checked
            checkBoxRoot.toggle(checked)
        }
        onCanceled: Tooltip.hideText()

        Timer
        {
           interval: 1000
           running: mouseArea1.containsMouse && tooltip.length
           onTriggered: Tooltip.showText(mouseArea1, Qt.point(mouseArea1.mouseX, mouseArea1.mouseY), tooltip)
        }
    }
}
