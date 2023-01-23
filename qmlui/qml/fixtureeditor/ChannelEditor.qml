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
import QtQuick.Dialogs 1.3

import org.qlcplus.classes 1.0
import "."

GridLayout
{
    columns: 2
    rowSpacing: 5

    property EditorRef editorView: null
    property ChannelEdit editor: null
    property QLCChannel channel: null

    function setItemName(name)
    {
        editor = editorView.requestChannelEditor(name)
        channel = editor.channel
        nameEdit.selectAndFocus()
    }

    function updatePresetBox(capIndex)
    {
        capPresetCombo.currentIndex = editor.getCapabilityPresetAtIndex(capIndex)
        presetBox.presetType = editor.getCapabilityPresetType(capIndex)

        switch (presetBox.presetType)
        {
            case QLCCapability.SingleColor:
                colorPreview.biColor = false
                colorPreview.primary = editor.getCapabilityValueAt(capIndex, 0)
                colorPreview.secondary = "black"
            break
            case QLCCapability.DoubleColor:
                colorPreview.biColor = true
                colorPreview.primary = editor.getCapabilityValueAt(capIndex, 0)
                colorPreview.secondary = editor.getCapabilityValueAt(capIndex, 1)
            break
            case QLCCapability.Picture:
                goboPicture.source = "file://" + editor.getCapabilityValueAt(capIndex, 0)
            break
            case QLCCapability.SingleValue:
                pValueSpin.value = editor.getCapabilityValueAt(capIndex, 0)
                pValueSpin.suffix = editor.getCapabilityPresetUnits(capIndex)
            break
            case QLCCapability.DoubleValue:
                pValueSpin.value = editor.getCapabilityValueAt(capIndex, 0)
                pValueSpin.suffix = editor.getCapabilityPresetUnits(capIndex)
                sValueSpin.value = editor.getCapabilityValueAt(capIndex, 1)
                sValueSpin.suffix = pValueSpin.suffix
            break
            default:
            break
        }
    }

    FileDialog
    {
        id: openDialog
        visible: false
        title: qsTr("Open a picture file")
        folder: "file://" + qlcplus.goboSystemPath()
        nameFilters: [ qsTr("Gobo pictures") + " (*.jpg *.jpeg *.png *.bmp *.svg)", qsTr("All files") + " (*)" ]

        onAccepted:
        {
            var str = fileUrl.toString().slice(7)
            editor.setCapabilityValueAt(editItem.indexInList, 0, str)
            updatePresetBox(editItem.indexInList)
        }
    }

    // row 1
    RobotoText { label: qsTr("Name"); height: UISettings.listItemHeight }
    CustomTextEdit
    {
        id: nameEdit
        Layout.fillWidth: true
        text: channel ? channel.name : ""
        onTextChanged: if (channel) channel.name = text
    }

    // row 2
    RobotoText { label: qsTr("Preset"); height: UISettings.listItemHeight }
    CustomComboBox
    {
        Layout.fillWidth: true
        model: editor ? editor.channelPresetList : null
        currentIndex: channel ? channel.preset : 0
        onCurrentIndexChanged: if (channel) channel.preset = currentIndex
    }

    // row 3
    RobotoText { label: qsTr("Type"); height: UISettings.listItemHeight }
    CustomComboBox
    {
        Layout.fillWidth: true
        enabled: channel ? (channel.preset ? false : true) : false
        model: editor ? editor.channelTypeList : null
        currValue: editor ? editor.group : 0
        onValueChanged: if (editor) editor.group = value
    }

    // row 4
    RobotoText { label: qsTr("Role"); height: UISettings.listItemHeight }
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
        RobotoText { label: qsTr("Coarse (MSB)"); height: UISettings.listItemHeight }
        CustomCheckBox
        {
            implicitHeight: UISettings.listItemHeight
            implicitWidth: implicitHeight
            ButtonGroup.group: roleGroup
            enabled: channel ? (channel.preset ? false : true) : false
            checked: channel ? channel.controlByte : false
            onToggled: if (channel) channel.controlByte = 1
        }
        RobotoText { label: qsTr("Fine (LSB)"); height: UISettings.listItemHeight }
    }

    // row 5
    RobotoText { label: qsTr("Default value"); height: UISettings.listItemHeight }
    RowLayout
    {
        Layout.fillWidth: true
        Layout.alignment: Qt.AlignRight

        CustomSpinBox
        {
            from: 0
            to: 255
            stepSize: 1
            value: channel ? channel.defaultValue : 0
            onValueChanged: if (channel) channel.defaultValue = value
        }

        Rectangle
        {
            Layout.fillWidth: true
            height: UISettings.listItemHeight
            color: "transparent"
        }

        IconButton
        {
            id: removeCapButton
            imgSource: "qrc:/remove.svg"
            tooltip: qsTr("Delete the selected capabilities")
            onClicked: {
                editItem.visible = false
                editor.removeCapabilityAtIndex(editItem.indexInList)
            }
        }

        IconButton
        {
            id: chWizButton
            imgSource: "qrc:/wizard.svg"
            tooltip: qsTr("Capability wizard")
            onClicked: wizardPopup.open()

            PopupChannelWizard
            {
                id: wizardPopup
                chEdit: editor
                capabilityWizard: true
            }
        }
    }

    // row 5 - capability editor
    Rectangle
    {
        Layout.columnSpan: 2
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

            function editRow(index, fieldIndex)
            {
                // capabilities cannot be edited on channel presets
                if (channel && channel.preset != 0)
                    return

                if (index < 0)
                {
                    // hide edit item
                    editItem.visible = false
                    presetBox.presetType = QLCCapability.None
                    return
                }
                else if (index === capsList.count)
                {
                    // create a new capability
                    editor.addNewCapability()
                }

                var item = capsList.itemAtIndex(index)
                selectedRow = index
                editItem.indexInList = index
                editItem.editCap = item.cap
                editItem.x = Qt.binding(function() { return item.x })
                editItem.y = Qt.binding(function() { return item.y - capsList.contentY })
                editItem.focusItem(fieldIndex)

                // setup the capability preset items
                updatePresetBox(index)
                editItem.visible = true
            }

            function updateValues(index, min, max, text)
            {
                var item = capsList.itemAtIndex(index)
                var capRef = item.cap
                var convMin = parseInt(min, 10)
                var convMax = parseInt(max, 10)

                if (!isNaN(convMin))
                    capRef.min = convMin
                if (!isNaN(convMax))
                    capRef.max = convMax
                capRef.name = text
            }

            function warningDescription(type)
            {
                switch (type)
                {
                    case QLCCapability.NoWarning: return ""
                    case QLCCapability.EmptyName: return qsTr("Empty description provided")
                    case QLCCapability.Overlapping: return qsTr("Overlapping with another capability")
                }
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
                    Rectangle { width: 1; height: UISettings.listItemHeight; color: UISettings.fgMedium }

                    RobotoText
                    {
                        width: UISettings.bigItemHeight
                        height: UISettings.listItemHeight
                        label: qsTr("To")
                        color: UISettings.sectionHeader
                    }
                    Rectangle { width: 1; height: UISettings.listItemHeight; color: UISettings.fgMedium }

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

                    property QLCCapability cap: modelData.cRef

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
                            label: cap ? cap.min : 0
                        }
                        Rectangle { width: 1; height: UISettings.listItemHeight; color: UISettings.fgMedium }

                        RobotoText
                        {
                            id: maxValBox
                            width: UISettings.bigItemHeight
                            height: UISettings.listItemHeight
                            label: cap ? cap.max : 255
                        }
                        Rectangle { width: 1; height: UISettings.listItemHeight; color: UISettings.fgMedium }

                        RobotoText
                        {
                            id: capDescription
                            Layout.fillWidth: true
                            height: UISettings.listItemHeight
                            label: cap ? cap.name : ""
                        }

                        IconButton
                        {
                            visible: cap ? cap.warning : false
                            height: UISettings.listItemHeight
                            width: height
                            border.width: 0
                            faSource: FontAwesome.fa_warning
                            faColor: "yellow"
                            tooltip: visible ? capsList.warningDescription(cap.warning) : ""
                        }
                    }

                    Rectangle
                    {
                        width: capsList.width
                        height: 1
                        y: UISettings.listItemHeight - 1
                        color: UISettings.fgMedium

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

                property QLCCapability editCap: null
                property int indexInList: 0

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

                function updateValues()
                {
                    capsList.updateValues(indexInList, defMinValBox.text, defMaxValBox.text, defCapDescription.text)
                    editor.checkCapabilities()
                }

                RowLayout
                {
                    width: capsList.width
                    height: UISettings.listItemHeight

                    CustomTextEdit
                    {
                        id: defMinValBox
                        implicitWidth: UISettings.bigItemHeight
                        text: editItem.editCap ? editItem.editCap.min : ""
                        KeyNavigation.backtab: defCapDescription
                        Keys.onBacktabPressed: capsList.editRow(capsList.selectedRow - 1, 2)
                        onTextEdited: editItem.updateValues()
                    }
                    Rectangle { width: 1; height: UISettings.listItemHeight }

                    CustomTextEdit
                    {
                        id: defMaxValBox
                        implicitWidth: UISettings.bigItemHeight
                        text: editItem.editCap ? editItem.editCap.max : ""
                        Keys.onBacktabPressed: defMinValBox.forceActiveFocus()
                        Keys.onTabPressed: defCapDescription.selectAndFocus()
                        onTextEdited: editItem.updateValues()
                    }
                    Rectangle { width: 1; height: UISettings.listItemHeight }

                    CustomTextEdit
                    {
                        id: defCapDescription
                        Layout.fillWidth: true
                        text: editItem.editCap ? editItem.editCap.name : 0
                        Keys.onBacktabPressed: defMaxValBox.forceActiveFocus()
                        Keys.onTabPressed: capsList.editRow(capsList.selectedRow + 1, 0)
                        onTextEdited: editItem.updateValues()
                    }

                    IconButton
                    {
                        visible: editItem.editCap ? editItem.editCap.warning : false
                        height: UISettings.listItemHeight
                        width: height
                        border.width: 0
                        faSource: FontAwesome.fa_warning
                        faColor: "yellow"
                        tooltip: editItem.editCap ? capsList.warningDescription(editItem.editCap.warning) : ""
                    }
                } // RowLayout
            } // Rectangle
        } // ListView
    } // Rectangle

    // row 6 - capability preset
    GroupBox
    {
        //title: qsTr("Preset")
        Layout.columnSpan: 2
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
                onValueChanged:
                {
                    editor.setCapabilityPresetAtIndex(editItem.indexInList, value)
                    updatePresetBox(editItem.indexInList)
                }
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

                            onColorChanged: {
                                editor.setCapabilityValueAt(editItem.indexInList, 0, Qt.rgba(r, g, b, 1.0))
                                updatePresetBox(editItem.indexInList)
                            }
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
                        onClicked: openDialog.open()
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

                            onColorChanged: {
                                editor.setCapabilityValueAt(editItem.indexInList, 1, Qt.rgba(r, g, b, 1.0))
                                updatePresetBox(editItem.indexInList)
                            }
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

                        onValueChanged: editor.setCapabilityValueAt(editItem.indexInList, 0, value)
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

                        onValueChanged: editor.setCapabilityValueAt(editItem.indexInList, 1, value)
                    }
                }
            }
        }
    }
} // GridLayout
