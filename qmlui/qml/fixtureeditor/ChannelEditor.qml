/*
  Q Light Controller Plus
  ChannelEditor.qml

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

import QtQuick 2.14
import QtQuick.Layouts 1.14
import QtQuick.Controls 2.14

import org.qlcplus.classes 1.0
import "."

GridLayout
{
    columns: 4

    property EditorRef fixtureEditor: null
    property ChannelEdit editor: null
    property QLCChannel channel: null

    function setItemName(name)
    {
        editor = fixtureEditor.requestChannelEditor(name)
        channel = editor.channel
        nameEdit.selectAndFocus()
    }

    // row 1
    RobotoText { label: qsTr("Name") }
    CustomTextEdit
    {
        id: nameEdit
        Layout.fillWidth: true
        Layout.columnSpan: 3
        text: channel ? channel.name : ""
        onTextChanged: if (channel) channel.name = text
    }

    // row 2
    RobotoText { label: qsTr("Preset") }
    CustomComboBox
    {
        Layout.fillWidth: true
        Layout.columnSpan: 3

        model: editor ? editor.channelPresetList : null
        currentIndex: channel ? channel.preset : 0
        onCurrentIndexChanged: if (channel) channel.preset = currentIndex
    }

    // row 3
    RobotoText { label: qsTr("Type") }
    CustomComboBox
    {
        Layout.fillWidth: true
        enabled: channel ? (channel.preset ? false : true) : false
        model: editor ? editor.channelTypeList : null
        currValue: channel ? channel.group : 0
    }

    RobotoText { label: qsTr("Role") }
    RowLayout
    {
        Layout.fillWidth: true

        ButtonGroup { id: roleGroup }

        CustomCheckBox
        {
            implicitHeight: UISettings.listItemHeight
            implicitWidth: implicitHeight
            ButtonGroup.group: roleGroup
            enabled: channel ? (channel.preset ? false : true) : false
            checked: channel ? !channel.controlByte : false
            onToggled: if (channel) channel.controlByte = 0
        }
        RobotoText { label: qsTr("Coarse (MSB)") }
        CustomCheckBox
        {
            implicitHeight: UISettings.listItemHeight
            implicitWidth: implicitHeight
            ButtonGroup.group: roleGroup
            enabled: channel ? (channel.preset ? false : true) : false
            checked: channel ? channel.controlByte : false
            onToggled: if (channel) channel.controlByte = 1
        }
        RobotoText { label: qsTr("Fine (LSB)") }
    }

    // row 4
    RobotoText { label: qsTr("Default value") }
    CustomSpinBox
    {
        Layout.columnSpan: 2
        from: 0
        to: 255
        stepSize: 1
        value: channel ? channel.defaultValue : 0
        onValueChanged: if (channel) channel.defaultValue = value
    }

    RowLayout
    {
        Layout.fillWidth: true
        Layout.alignment: Qt.AlignRight

        IconButton
        {
            id: removeCapButton
            imgSource: "qrc:/remove.svg"
            tooltip: qsTr("Delete the selected capabilities")
        }
    }

    // row 5 - capability editor
    Rectangle
    {
        Layout.columnSpan: 4
        Layout.fillHeight: true
        Layout.fillWidth: true
        color: "transparent"

        ListView
        {
            id: capsList
            z: 1
            anchors.fill: parent
            boundsBehavior: Flickable.StopAtBounds
            headerPositioning: ListView.OverlayHeader
            clip: true
            model: editor ? editor.capabilities : null

            property int selectedRow: -1

            function editRow(index, compIndex)
            {
                // capabilities cannot be edited on channel presets
                if (channel && channel.preset != 0)
                    return

                if (index < 0)
                {
                    editItem.visible = false
                    presetBox.presetType = QLCCapability.None
                    return
                }

                var item = capsList.itemAtIndex(index)
                selectedRow = index
                editItem.iMin = item.minValue
                editItem.iMax = item.maxValue
                editItem.sDesc = item.description
                editItem.x = Qt.binding(function() { return item.x })
                editItem.y = Qt.binding(function() { return item.y - capsList.contentY })
                editItem.focusItem(compIndex)

                // setup the capability preset items
                capPresetCombo.currentIndex = editor.getCapabilityPresetAtIndex(index)
                presetBox.presetType = editor.getCapabilityPresetType(index)

                switch (presetBox.presetType)
                {
                    case QLCCapability.SingleColor:
                        colorPreview.primary = editor.getCapabilityValueAt(index, 0)
                    break
                    case QLCCapability.DoubleColor:
                        colorPreview.primary = editor.getCapabilityValueAt(index, 0)
                        colorPreview.secondary = editor.getCapabilityValueAt(index, 1)
                    break
                    case QLCCapability.Picture:
                        goboPicture.source = "file://" + editor.getCapabilityValueAt(index, 0)
                    break
                    case QLCCapability.SingleValue:
                        pValueSpin.value = editor.getCapabilityValueAt(index, 0)
                        pValueSpin.suffix = editor.getCapabilityPresetUnits(index)
                    break
                    case QLCCapability.DoubleValue:
                        pValueSpin.value = editor.getCapabilityValueAt(index, 0)
                        pValueSpin.suffix = editor.getCapabilityPresetUnits(index)
                        sValueSpin.value = editor.getCapabilityValueAt(index, 1)
                        sValueSpin.suffix = pValueSpin.suffix
                    break
                    default:
                    break
                }

                editItem.visible = true
            }

            header:
                RowLayout
                {
                    z: 2
                    width: capsList.width
                    height: UISettings.listItemHeight

                    RobotoText
                    {
                        width: UISettings.bigItemHeight
                        height: UISettings.listItemHeight
                        label: qsTr("From")
                        color: UISettings.sectionHeader
                    }
                    Rectangle { width: 1; height: UISettings.listItemHeight }

                    RobotoText
                    {
                        width: UISettings.bigItemHeight
                        height: UISettings.listItemHeight
                        label: qsTr("To")
                        color: UISettings.sectionHeader
                    }
                    Rectangle { width: 1; height: UISettings.listItemHeight }

                    RobotoText
                    {
                        Layout.fillWidth: true
                        height: UISettings.listItemHeight
                        label: qsTr("Description")
                        color: UISettings.sectionHeader
                    }
                }

            delegate:
                Item
                {
                    width: capsList.width
                    height: UISettings.listItemHeight

                    property alias minValue: minValBox.label
                    property alias maxValue: maxValBox.label
                    property alias description: capDescription.label

                    RowLayout
                    {
                        id: delegateRow
                        anchors.fill: parent

                        RobotoText
                        {
                            id: minValBox
                            width: UISettings.bigItemHeight
                            height: UISettings.listItemHeight
                            label: modelData.iMin
                        }
                        Rectangle { width: 1; height: UISettings.listItemHeight }

                        RobotoText
                        {
                            id: maxValBox
                            width: UISettings.bigItemHeight
                            height: UISettings.listItemHeight
                            label: modelData.iMax
                        }
                        Rectangle { width: 1; height: UISettings.listItemHeight }

                        RobotoText
                        {
                            id: capDescription
                            Layout.fillWidth: true
                            height: UISettings.listItemHeight
                            label: modelData.sDesc
                        }
                    }

                    MouseArea
                    {
                        anchors.fill: parent
                        onClicked:
                        {
                            var compIdx = 0
                            var item = delegateRow.childAt(mouse.x, mouse.y)
                            if (item === minValBox)
                                compIdx = 0
                            else if (item === maxValBox)
                                compIdx = 1
                            else
                                compIdx = 2
                            capsList.editRow(index, compIdx)
                        }
                    }
                }

            ScrollBar.vertical: CustomScrollBar { }

            // default edit item. Moves over rows on editing
            Item
            {
                id: editItem
                y: UISettings.listItemHeight
                width: capsList.width
                height: UISettings.listItemHeight
                visible: false || capsList.count == 0

                property int iMin: 0
                property int iMax: 0
                property string sDesc: ""

                function focusItem(index)
                {
                    switch(index)
                    {
                        case 0:
                            defMinValBox.forceActiveFocus()
                        break
                        case 1:
                            defMaxValBox.forceActiveFocus()
                        break
                        case 2:
                            defCapDescription.selectAndFocus()
                        break
                    }
                }

                RowLayout
                {
                    width: capsList.width
                    height: UISettings.listItemHeight

                    CustomTextEdit
                    {
                        id: defMinValBox
                        implicitWidth: UISettings.bigItemHeight
                        text: editItem.iMin
                        KeyNavigation.backtab: defCapDescription
                        Keys.onBacktabPressed: capsList.editRow(capsList.selectedRow - 1, 2)
                    }
                    Rectangle { width: 1; height: UISettings.listItemHeight }

                    CustomTextEdit
                    {
                        id: defMaxValBox
                        implicitWidth: UISettings.bigItemHeight
                        text: editItem.iMax
                        Keys.onBacktabPressed: defMinValBox.forceActiveFocus()
                        Keys.onTabPressed: defCapDescription.selectAndFocus()
                    }
                    Rectangle { width: 1; height: UISettings.listItemHeight }

                    CustomTextEdit
                    {
                        id: defCapDescription
                        Layout.fillWidth: true
                        text: editItem.sDesc
                        Keys.onBacktabPressed: defMaxValBox.forceActiveFocus()
                        Keys.onTabPressed: capsList.editRow(capsList.selectedRow + 1, 0)
                    }
                } // RowLayout
            } // Rectangle
        } // ListView
    } // Rectangle

    // row 6 - capability preset
    GroupBox
    {
        //title: qsTr("Preset")
        Layout.columnSpan: 3
        Layout.fillWidth: true
        //font.family: UISettings.robotoFontName
        //font.pixelSize: UISettings.textSizeDefault
        //palette.windowText: UISettings.fgMain
        visible: editItem.visible

        GridLayout
        {
            id: presetBox
            width: parent.width
            columns: 3

            property int presetType: QLCCapability.None

            RobotoText { label: qsTr("Type") }
            CustomComboBox
            {
                id: capPresetCombo
                Layout.fillWidth: true
                model: editor ? editor.capabilityPresetList : null
                //currValue: channel ? channel.group : 0
            }

            GroupBox
            {
                id: previewBox
                visible: presetBox.presetType == QLCCapability.SingleColor ||
                         presetBox.presetType == QLCCapability.DoubleColor ||
                         presetBox.presetType == QLCCapability.Picture
                title: qsTr("Preview")
                font.family: UISettings.robotoFontName
                font.pixelSize: UISettings.textSizeDefault
                palette.windowText: UISettings.fgMain

                RowLayout
                {
                    IconButton
                    {
                        visible: presetBox.presetType == QLCCapability.SingleColor ||
                                 presetBox.presetType == QLCCapability.DoubleColor
                        imgSource: "qrc:/color.svg"
                        tooltip: qsTr("Primary color")
                        width: UISettings.iconSizeMedium
                        height: UISettings.iconSizeMedium
                        onClicked: {
                            pColTool.visible = !pColTool.visible
                        }

                        ColorTool
                        {
                            id: pColTool
                            x: -width
                            y: -height
                            visible: false
                            closeOnSelect: true
                            showPalette: false

                            //onColorChanged: fixtureManager.setColorValue(r * 255, g * 255, b * 255, w * 255, a * 255, uv * 255)
                        }
                    }

                    MultiColorBox
                    {
                        id: colorPreview
                        visible: presetBox.presetType == QLCCapability.SingleColor ||
                                 presetBox.presetType == QLCCapability.DoubleColor
                        width: UISettings.mediumItemHeight
                        height: UISettings.mediumItemHeight
                    }

                    GenericButton
                    {
                        visible: presetBox.presetType == QLCCapability.Picture
                        width: UISettings.iconSizeMedium
                        height: UISettings.iconSizeMedium
                        label: "..."
                    }

                    Rectangle
                    {
                        id: goboPreview
                        visible: presetBox.presetType == QLCCapability.Picture
                        width: UISettings.mediumItemHeight
                        height: UISettings.mediumItemHeight
                        radius: 2

                        Image
                        {
                            id: goboPicture
                            anchors.fill: parent
                            sourceSize: Qt.size(width, height)
                        }
                    }

                    IconButton
                    {
                        visible: presetBox.presetType == QLCCapability.DoubleColor
                        imgSource: "qrc:/color.svg"
                        tooltip: qsTr("Secondary color")
                        width: UISettings.iconSizeMedium
                        height: UISettings.iconSizeMedium

                        onClicked: {
                            sColTool.visible = !sColTool.visible
                        }

                        ColorTool
                        {
                            id: sColTool
                            x: -width
                            y: -height
                            visible: false
                            closeOnSelect: true
                            showPalette: false

                            //onColorChanged: fixtureManager.setColorValue(r * 255, g * 255, b * 255, w * 255, a * 255, uv * 255)
                        }
                    }
                }
            } // preview GroupBox

            GroupBox
            {
                id: valuesBox
                visible: presetBox.presetType == QLCCapability.SingleValue ||
                         presetBox.presetType == QLCCapability.DoubleValue
                title: qsTr("Value(s)")
                font.family: UISettings.robotoFontName
                font.pixelSize: UISettings.textSizeDefault
                palette.windowText: UISettings.fgMain

                GridLayout
                {
                    anchors.fill: parent
                    columns: 2

                    RobotoText
                    {
                        label: qsTr("Value 1")
                    }

                    CustomSpinBox
                    {
                        id: pValueSpin
                        Layout.fillWidth: true
                        from: -1000
                        to: 1000
                    }

                    RobotoText
                    {
                        visible: presetBox.presetType == QLCCapability.DoubleValue
                        label: qsTr("Value 2")
                    }

                    CustomSpinBox
                    {
                        id: sValueSpin
                        visible: presetBox.presetType == QLCCapability.DoubleValue
                        Layout.fillWidth: true
                        from: -1000
                        to: 1000
                    }
                }
            }
        }
    }
} // GridLayout
