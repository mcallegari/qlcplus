/*
  Q Light Controller Plus
  PaletteFanningBox.qml

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
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.1

import org.qlcplus.classes 1.0
import "."

Rectangle
{
    id: boxRoot
    width: 330
    height: UISettings.iconSizeMedium
    color: "transparent"

    property int paletteType: QLCPalette.Dimmer
    property int value1: 0
    property int value2: 0

    function showPalettePopup()
    {
        palettePopup.type = boxRoot.paletteType
        palettePopup.value1 = boxRoot.value1
        palettePopup.value2 = boxRoot.value2
        palettePopup.open()
        palettePopup.focusEditItem()
    }

    function stringFromType()
    {
        switch(curveButton.currentValue)
        {
            case QLCPalette.Flat: return qsTr("Flat")
            case QLCPalette.Linear: return qsTr("Linear")
            case QLCPalette.Square: return qsTr("Square")
            case QLCPalette.Saw: return qsTr("Saw")
            case QLCPalette.Sine: return qsTr("Sine")
        }
    }

    PopupCreatePalette
    {
        id: palettePopup
    }

    RowLayout
    {
        width: boxRoot.width
        height: UISettings.iconSizeMedium

        IconButton
        {
            id: fanningButton
            width: UISettings.iconSizeMedium
            height: width
            imgSource: "qrc:/fanning.svg"
            tooltip: qsTr("Show/Hide fanning options")
            checkable: true
            checked: false
        }

        Rectangle
        {
            color: "transparent"
            Layout.fillWidth: true
            height: UISettings.iconSizeMedium
        }

        IconButton
        {
            width: UISettings.iconSizeMedium
            height: width
            imgSource: "qrc:/palette.svg"
            tooltip: qsTr("Create a new palette")
            onClicked: boxRoot.showPalettePopup()
        }
    }

    Rectangle
    {
        visible: fanningButton.checked
        y: UISettings.iconSizeMedium + 2
        width: paramsGrid.width + 10
        height: paramsGrid.height + 10
        color: UISettings.bgMedium
        border.color: UISettings.bgLight
        border.width: 2

        GridLayout
        {
            id: paramsGrid
            columns: 2
            x: 5
            y: 5

            // row 1
            IconPopupButton
            {
                id: curveButton
                implicitWidth: UISettings.iconSizeMedium
                implicitHeight: implicitWidth

                ListModel
                {
                    id: curveModel
                    ListElement { mLabel: qsTr("Flat"); mIcon: "qrc:/algo-flat.svg"; mValue: QLCPalette.Flat }
                    ListElement { mLabel: qsTr("Linear"); mIcon: "qrc:/algo-linear.svg"; mValue: QLCPalette.Linear }
                    ListElement { mLabel: qsTr("Square"); mIcon: "qrc:/algo-square.svg"; mValue: QLCPalette.Square }
                    ListElement { mLabel: qsTr("Saw"); mIcon: "qrc:/algo-saw.svg"; mValue: QLCPalette.Saw }
                    ListElement { mLabel: qsTr("Sine"); mIcon: "qrc:/algo-sine.svg"; mValue: QLCPalette.Sine }
                }
                model: curveModel

                //currentValue: efxEditor.runOrder
                //onValueChanged: efxEditor.runOrder = value
            }

            RobotoText
            {
                height: UISettings.iconSizeMedium
                label: stringFromType()
            }

            // row 2
            RobotoText
            {
                height: UISettings.iconSizeMedium
                label: qsTr("Amount")
            }
            CustomSpinBox
            {
                from: 0
                to: 100
                value: 100
                suffix: "%"
            }

            // row 3
            RowLayout
            {
                Layout.columnSpan: 2
                visible: boxRoot.paletteType === QLCPalette.Pan ||
                         boxRoot.paletteType === QLCPalette.Tilt ||
                         boxRoot.paletteType === QLCPalette.PanTilt ? true : false

                CustomCheckBox
                {
                    implicitHeight: UISettings.iconSizeMedium
                    implicitWidth: implicitHeight
                    checked: true
                }
                RobotoText
                {
                    label: "Pan"
                }
                CustomCheckBox
                {
                    implicitHeight: UISettings.iconSizeMedium
                    implicitWidth: implicitHeight
                    checked: true
                }
                RobotoText
                {
                    label: "Tilt"
                }
            }

            // row 4
            RobotoText
            {
                label: qsTr("Value")
            }
            CustomSpinBox
            {
                visible: boxRoot.paletteType != QLCPalette.Color
                from: 0
                to: 255
                suffix: boxRoot.paletteType === QLCPalette.Pan ||
                        boxRoot.paletteType === QLCPalette.Tilt ||
                        boxRoot.paletteType === QLCPalette.PanTilt ? "°" : ""
            }
        } // GridLayout
    } // Rectangle
}
