/*
  Q Light Controller Plus
  SingleAxisTool.qml

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
import QtQuick.Layouts 1.0

import org.qlcplus.classes 1.0
import "."

Rectangle
{
    id: boxRoot
    width: UISettings.bigItemHeight * 3
    height: UISettings.bigItemHeight
    color: UISettings.bgMedium
    border.color: "#222"
    border.width: 2

    property real maxDegrees: 360
    property int currentValue: 0 // as DMX value
    property int currentDegrees: Math.round((currentValue * maxDegrees) / 255.0)
    property bool closeOnSelect: false
    property bool showPalette: false

    signal valueChanged(int value)
    signal close()

    GridLayout
    {
        anchors.fill: parent
        anchors.margins: 10
        columns: 3
        rows: 2

        CustomSlider
        {
            id: axisSlider
            Layout.columnSpan: 3
            Layout.fillWidth: true
            from: 0
            to: maxDegrees
            value: currentDegrees
            onMoved:
            {
                currentValue = Math.round((valueAt(position) * 255.0) / maxDegrees)
                boxRoot.valueChanged(currentValue)
            }
            onPressedChanged:
            {
                if (!pressed && closeOnSelect)
                    boxRoot.close()
            }
        }

        RobotoText
        {
            height: UISettings.listItemHeight
            label: "0°"
        }

        CustomSpinBox
        {
            id: wSpin
            width: UISettings.bigItemHeight * 0.7
            height: UISettings.listItemHeight
            Layout.alignment: Qt.AlignHCenter
            from: 0
            to: maxDegrees
            suffix: "°"
            value: currentDegrees
            onValueModified:
            {
                currentValue = Math.round((value * 255.0) / maxDegrees)
                boxRoot.valueChanged(currentValue)
            }
        }

        RobotoText
        {
            height: UISettings.listItemHeight
            Layout.alignment: Qt.AlignRight
            label: maxDegrees + "°"
        }
    }
}

