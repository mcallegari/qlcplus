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

Item
{
    id: boxRoot
    width: 330
    height: boxLayout.height

    property alias checked: fanningButton.checked
    property alias isPicking: colorPicker.checked
    property bool isEditing: false
    property QLCPalette palette: null
    property int paletteType: QLCPalette.Undefined

    onVisibleChanged:
    {
        if (isEditing)
            return

        if (visible)
            palette = paletteManager.getEditingPalette(paletteType)
    }

    Component.onDestruction:
    {
        paletteManager.releaseEditingPalette(paletteType)
        palette = null
    }

    function updateValue(value)
    {
        paletteManager.updatePalette(palette, value)
    }

    function updateValues(val1, val2)
    {
        paletteManager.updatePalette(palette, val1, val2)
    }

    function updatePreview()
    {
        paletteManager.previewPalette(palette)
    }

    function editPalette(ePalette)
    {
        isEditing = true
        palette = ePalette

        if (palette.fanningType == QLCPalette.Flat)
            return

        switch (palette.type)
        {
            case QLCPalette.Pan:
                panCheck.checked = true
                tiltCheck.checked = false
                valueSpin.value = palette.fanningValue
            break
            case QLCPalette.Tilt:
                panCheck.checked = false
                tiltCheck.checked = true
                valueSpin.value = palette.fanningValue
            break
            case QLCPalette.PanTilt:
                panCheck.checked = true
                tiltCheck.checked = true
                valueSpin.value = palette.fanningValue
            break
            case QLCPalette.Dimmer:
                valueSpin.value = palette.fanningValue
            break;

            case QLCPalette.Color:
                colorPreview.color = palette.fanningValue
            break;
        }
    }

    function showPalettePopup()
    {
        palettePopup.paletteObj = palette
        palettePopup.open()
        palettePopup.focusEditItem()
    }

    function setName(paletteName)
    {
        if (palette)
            palette.name = paletteName
    }

    function setColor(color)
    {
        colorPreview.color = color
        palette.fanningValue = color
    }

    PopupCreatePalette
    {
        id: palettePopup
    }

    Column
    {
        id: boxLayout
        width: parent.width

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

        GridLayout
        {
            id: paramsGrid
            visible: fanningButton.checked
            columns: 2
            x: 5
            y: 5

            // row 1
            RobotoText
            {
                height: UISettings.iconSizeMedium
                label: qsTr("Type")
            }
            RowLayout
            {
                Layout.fillWidth: true
                height: UISettings.iconSizeMedium

                IconPopupButton
                {
                    id: typeButton
                    implicitWidth: UISettings.iconSizeMedium
                    implicitHeight: implicitWidth

                    ListModel
                    {
                        id: typeModel
                        ListElement { mLabel: qsTr("Flat"); mIcon: "qrc:/algo-flat.svg"; mValue: QLCPalette.Flat }
                        ListElement { mLabel: qsTr("Linear"); mIcon: "qrc:/algo-linear.svg"; mValue: QLCPalette.Linear }
                        ListElement { mLabel: qsTr("Square"); mIcon: "qrc:/algo-square.svg"; mValue: QLCPalette.Square }
                        ListElement { mLabel: qsTr("Saw"); mIcon: "qrc:/algo-saw.svg"; mValue: QLCPalette.Saw }
                        ListElement { mLabel: qsTr("Sine"); mIcon: "qrc:/algo-sine.svg"; mValue: QLCPalette.Sine }
                    }
                    model: typeModel

                    currValue: boxRoot.palette ? boxRoot.palette.fanningType : QLCPalette.Flat
                    onValueChanged:
                    {
                        if (boxRoot.palette)
                        {
                            boxRoot.palette.fanningType = value
                            boxRoot.updatePreview()
                        }
                    }
                }

                RobotoText
                {
                    height: UISettings.iconSizeMedium
                    label: typeButton.currentText
                }
            }

            // row 2
            RobotoText
            {
                height: UISettings.iconSizeMedium
                label: qsTr("Layout")
            }
            RowLayout
            {
                Layout.fillWidth: true
                height: UISettings.iconSizeMedium

                IconPopupButton
                {
                    id: layoutButton
                    implicitWidth: UISettings.iconSizeMedium
                    implicitHeight: implicitWidth

                    ListModel
                    {
                        id: layoutModel
                        ListElement { mLabel: qsTr("X Ascending"); mIcon: "qrc:/layout-ltr.svg"; mValue: QLCPalette.XAscending }
                        ListElement { mLabel: qsTr("X Descending"); mIcon: "qrc:/layout-rtl.svg"; mValue: QLCPalette.XDescending }
                        //ListElement { mLabel: qsTr("X Centered"); mIcon: "qrc:/layout-center.svg"; mValue: QLCPalette.XCentered }
                        ListElement { mLabel: qsTr("Y Ascending"); mIcon: "qrc:/layout-btt.svg"; mValue: QLCPalette.YAscending }
                        ListElement { mLabel: qsTr("Y Descending"); mIcon: "qrc:/layout-ttb.svg"; mValue: QLCPalette.YDescending }
                        //ListElement { mLabel: qsTr("Y Centered"); mIcon: "qrc:/layout-center.svg"; mValue: QLCPalette.YCentered }
                        ListElement { mLabel: qsTr("Z Ascending"); mIcon: "qrc:/layout-ftb.svg"; mValue: QLCPalette.ZAscending }
                        ListElement { mLabel: qsTr("Z Descending"); mIcon: "qrc:/layout-btf.svg"; mValue: QLCPalette.ZDescending }
                        //ListElement { mLabel: qsTr("Z Centered"); mIcon: "qrc:/layout-center.svg"; mValue: QLCPalette.ZCentered }
                    }
                    model: layoutModel

                    currValue: boxRoot.palette ? boxRoot.palette.fanningLayout : QLCPalette.XAscending
                    onValueChanged:
                    {
                        if (boxRoot.palette)
                        {
                            boxRoot.palette.fanningLayout = value
                            boxRoot.updatePreview()
                        }
                    }
                }

                RobotoText
                {
                    height: UISettings.iconSizeMedium
                    label: layoutButton.currentText
                }
            }

            // row 3
            RobotoText
            {
                height: UISettings.iconSizeMedium
                label: qsTr("Amount")
            }
            CustomSpinBox
            {
                from: 0
                to: 1000
                value: boxRoot.palette ? boxRoot.palette.fanningAmount : 100
                suffix: "%"
                onValueChanged:
                {
                    if (boxRoot.palette)
                    {
                        boxRoot.palette.fanningAmount = value
                        boxRoot.updatePreview()
                    }
                }
            }

            // row 4
            RowLayout
            {
                Layout.columnSpan: 2
                visible: boxRoot.paletteType === QLCPalette.Pan ||
                         boxRoot.paletteType === QLCPalette.Tilt ||
                         boxRoot.paletteType === QLCPalette.PanTilt ? true : false

                CustomCheckBox
                {
                    id: panCheck
                    implicitHeight: UISettings.iconSizeMedium
                    implicitWidth: implicitHeight
                    checked: true
                    enabled: tiltCheck.checked ? true : false
                    onCheckedChanged:
                    {
                        if (checked)
                            boxRoot.paletteType = tiltCheck.checked ? QLCPalette.PanTilt : QLCPalette.Pan
                        else
                            boxRoot.paletteType = QLCPalette.Tilt

                        if (isEditing == false)
                            boxRoot.palette = paletteManager.getEditingPalette(boxRoot.paletteType)
                        updatePreview()
                    }
                }
                RobotoText
                {
                    label: "Pan"
                }
                CustomCheckBox
                {
                    id: tiltCheck
                    implicitHeight: UISettings.iconSizeMedium
                    implicitWidth: implicitHeight
                    checked: true
                    enabled: panCheck.checked ? true : false
                    onCheckedChanged:
                    {
                        if (checked)
                            boxRoot.paletteType = panCheck.checked ? QLCPalette.PanTilt : QLCPalette.Tilt
                        else
                            boxRoot.paletteType = QLCPalette.Pan

                        if (isEditing == false)
                            boxRoot.palette = paletteManager.getEditingPalette(boxRoot.paletteType)
                        updatePreview()
                    }
                }
                RobotoText
                {
                    label: "Tilt"
                }
            }

            // row 5
            RobotoText
            {
                label: qsTr("Value")
            }
            CustomSpinBox
            {
                id: valueSpin

                function suffixFromType()
                {
                    switch (boxRoot.paletteType)
                    {
                        case QLCPalette.Pan:
                        case QLCPalette.Tilt:
                        case QLCPalette.PanTilt:
                            return "°"
                        default:
                            return ""
                    }
                }

                function fromFromType()
                {
                    switch (boxRoot.paletteType)
                    {
                        case QLCPalette.Pan:
                        case QLCPalette.Tilt:
                        case QLCPalette.PanTilt:
                            return -360
                        default:
                            return 0
                    }
                }

                function toFromType()
                {
                    switch (boxRoot.paletteType)
                    {
                        case QLCPalette.Pan:
                        case QLCPalette.Tilt:
                        case QLCPalette.PanTilt:
                            return 360
                        default:
                            return 255
                    }
                }

                visible: boxRoot.paletteType != QLCPalette.Color
                from: fromFromType()
                to: toFromType()
                suffix: suffixFromType()
                onValueChanged:
                {
                    if (boxRoot.palette)
                    {
                        boxRoot.palette.fanningValue = value
                        boxRoot.updatePreview()
                    }
                }
            }
            RowLayout
            {
                visible: boxRoot.paletteType == QLCPalette.Color

                Rectangle
                {
                    id: colorPreview
                    height: UISettings.iconSizeDefault
                    Layout.fillWidth: true
                    color: "black"
                }
                IconButton
                {
                    id: colorPicker
                    faSource: FontAwesome.fa_eyedropper
                    faColor: "white"
                    imgMargins: UISettings.iconSizeDefault / 4
                    checkable: true
                    tooltip: qsTr("Pick the selected color")
                }
            }
        } // GridLayout
    } // Column
}
