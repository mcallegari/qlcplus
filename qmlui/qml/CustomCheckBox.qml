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
import QtQuick.Controls 2.1

import "."

RadioButton
{
    id: checkBoxRoot
    implicitWidth: UISettings.iconSizeDefault
    implicitHeight: UISettings.iconSizeDefault
    hoverEnabled: true
    autoExclusive: false

    property color bgColor: UISettings.bgControl
    property color hoverColor: UISettings.bgLight
    property color pressColor: UISettings.highlightPressed
    property string tooltip: ""

    Rectangle
    {
        anchors.fill: parent
        z: 3
        color: "black"
        opacity: 0.4
        visible: !parent.enabled
    }

    ToolTip
    {
        visible: tooltip && hovered
        text: tooltip
        delay: 1000
        timeout: 5000
        background:
            Rectangle
            {
                color: UISettings.bgMain
                border.width: 1
                border.color: UISettings.bgLight
            }
        contentItem:
            Text
            {
              text: tooltip
              color: "white"
          }
    }
/*
    contentItem:
        RobotoText
        {
            label: checkBoxRoot.text
            opacity: checkBoxRoot.enabled ? 1.0 : 0.3
            //color: control.down ? "#17a81a" : "#21be2b"
        }
*/
    background:
        Rectangle
        {
            id: cbBody
            color: hovered ? hoverColor : bgColor
            radius: 5
            border.color: "#1D1D1D"
            border.width: 2
        }

    indicator:
        Image
        {
            id: cbIcon
            visible: checked
            anchors.fill: parent
            anchors.margins: 3
            source: "qrc:/apply.svg"
            sourceSize: Qt.size(width, height)
        }
}
