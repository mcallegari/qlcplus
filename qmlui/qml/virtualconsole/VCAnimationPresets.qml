/*
  Q Light Controller Plus
  VCAnimationPresets.qml

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
import QtQuick.Controls

import org.qlcplus.classes 1.0
import "."

Rectangle
{
    id: presetsRoot
    color: "transparent"
    height: presetsColumn.height

    property VCAnimation widgetRef: null
    property int selectedPresetId: -1
    property int colorSlot: 0

    function selectedPreset()
    {
        if (!widgetRef || selectedPresetId < 0)
            return null

        var list = widgetRef.presetsList
        for (var i = 0; i < list.length; i++)
        {
            if (list[i].id === selectedPresetId)
                return list[i]
        }

        return null
    }

    function presetLabel(modelData)
    {
        if (!modelData)
            return ""

        switch (modelData.typeString)
        {
            case "Animation": return modelData.resource
            case "Text": return qsTr("Text: %1").arg(modelData.resource)
            default: return modelData.typeString
        }
    }

    Column
    {
        id: presetsColumn
        width: parent.width
        spacing: 5

        SectionBox
        {
            width: parent.width
            sectionLabel: qsTr("Presets")

            sectionContents:
                Column
                {
                    width: parent.width
                    spacing: 5

                    // Color slot selector
                    GridLayout
                    {
                        width: parent.width
                        columns: 2
                        columnSpacing: 5
                        rowSpacing: 4

                        RobotoText
                        {
                            height: UISettings.listItemHeight
                            label: qsTr("Color slot")
                        }

                        CustomComboBox
                        {
                            id: colorSlotCombo
                            Layout.fillWidth: true
                            height: UISettings.listItemHeight
                            textRole: ""
                            model: [ qsTr("Color 1"), qsTr("Color 2"), qsTr("Color 3"),
                                     qsTr("Color 4"), qsTr("Color 5") ]
                            currValue: presetsRoot.colorSlot
                            onActivated: (index) => presetsRoot.colorSlot = index
                        }
                    }

                    // Add buttons toolbar
                    RowLayout
                    {
                        width: parent.width
                        height: UISettings.listItemHeight
                        spacing: 0

                        IconButton
                        {
                            id: addColorButton
                            Layout.preferredWidth: height
                            Layout.fillHeight: true
                            faSource: FontAwesome.fa_palette
                            faColor: UISettings.fgMain
                            tooltip: qsTr("Add a fixed-color preset for the selected color slot")
                            onClicked:
                            {
                                if (widgetRef)
                                    colorTool.visible = !colorTool.visible
                            }
                        }

                        IconButton
                        {
                            id: addKnobsButton
                            Layout.preferredWidth: height
                            Layout.fillHeight: true
                            faSource: FontAwesome.fa_sliders
                            faColor: UISettings.fgMain
                            tooltip: qsTr("Add R/G/B knobs for the selected color slot")
                            onClicked:
                            {
                                if (!widgetRef)
                                    return

                                var presetId = widgetRef.addColorKnobsPreset(presetsRoot.colorSlot)
                                if (presetId >= 0)
                                    selectedPresetId = presetId
                            }
                        }

                        IconButton
                        {
                            id: addAlgorithmButton
                            Layout.preferredWidth: height
                            Layout.fillHeight: true
                            faSource: FontAwesome.fa_scroll
                            faColor: UISettings.fgMain
                            tooltip: qsTr("Add a script algorithm preset")
                            onClicked:
                            {
                                if (!widgetRef)
                                    return

                                var popup = presetPopupComponent.createObject(mainView, { "widgetRef": widgetRef })
                                popup.open()
                            }
                        }

                        IconButton
                        {
                            id: addTextButton
                            Layout.preferredWidth: height
                            Layout.fillHeight: true
                            faSource: FontAwesome.fa_font
                            faColor: UISettings.fgMain
                            tooltip: qsTr("Add a Text preset using the text on the right")
                            enabled: textInput.text.length > 0
                            onClicked:
                            {
                                if (!widgetRef || textInput.text.length === 0)
                                    return

                                var presetId = widgetRef.addTextPreset(textInput.text)
                                if (presetId >= 0)
                                {
                                    selectedPresetId = presetId
                                    textInput.text = ""
                                }
                            }
                        }

                        CustomTextEdit
                        {
                            id: textInput
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            Layout.margins: 3
                            text: ""
                        }

                        IconButton
                        {
                            id: removePreset
                            Layout.preferredWidth: height
                            Layout.fillHeight: true
                            faSource: FontAwesome.fa_minus
                            faColor: "crimson"
                            tooltip: qsTr("Remove selected preset")
                            enabled: selectedPresetId >= 0
                            onClicked:
                            {
                                if (widgetRef && selectedPresetId >= 0)
                                {
                                    widgetRef.removePreset(selectedPresetId)
                                    selectedPresetId = -1
                                }
                            }
                        }

                        IconButton
                        {
                            id: moveDownPreset
                            Layout.preferredWidth: height
                            Layout.fillHeight: true
                            faSource: FontAwesome.fa_arrow_down
                            faColor: UISettings.fgMain
                            tooltip: qsTr("Move selected preset down")
                            enabled: selectedPresetId >= 0
                            onClicked:
                            {
                                if (widgetRef && selectedPresetId >= 0)
                                    selectedPresetId = widgetRef.movePresetDown(selectedPresetId)
                            }
                        }

                        IconButton
                        {
                            id: moveUpPreset
                            Layout.preferredWidth: height
                            Layout.fillHeight: true
                            faSource: FontAwesome.fa_arrow_up
                            faColor: UISettings.fgMain
                            tooltip: qsTr("Move selected preset up")
                            enabled: selectedPresetId >= 0
                            onClicked:
                            {
                                if (widgetRef && selectedPresetId >= 0)
                                    selectedPresetId = widgetRef.movePresetUp(selectedPresetId)
                            }
                        }
                    }

                    ListView
                    {
                        id: presetsList
                        width: parent.width
                        height: count ? count * UISettings.listItemHeight : UISettings.bigItemHeight
                        boundsBehavior: Flickable.StopAtBounds
                        model: widgetRef ? widgetRef.presetsList : null

                        delegate:
                            Rectangle
                            {
                                width: presetsList.width
                                height: UISettings.listItemHeight
                                color: modelData && modelData.id === selectedPresetId ? UISettings.highlight : "transparent"

                                RowLayout
                                {
                                    width: parent.width
                                    height: parent.height
                                    spacing: 5

                                    Rectangle
                                    {
                                        Layout.leftMargin: 3
                                        width: UISettings.listItemHeight * 0.6
                                        height: width
                                        radius: 3
                                        border.width: 1
                                        border.color: UISettings.bgLight
                                        visible: modelData && modelData.colorIndex >= 0
                                        color: modelData && modelData.color ? modelData.color : "transparent"
                                    }

                                    RobotoText
                                    {
                                        Layout.fillWidth: true
                                        height: parent.height
                                        label: presetsRoot.presetLabel(modelData)
                                    }

                                    RobotoText
                                    {
                                        Layout.preferredWidth: presetsList.width * 0.3
                                        height: parent.height
                                        textHAlign: Text.AlignHCenter
                                        label: modelData && modelData.isKnob ? qsTr("Knob") : qsTr("Button")
                                    }
                                }

                                MouseArea
                                {
                                    anchors.fill: parent
                                    onClicked:
                                    {
                                        if (modelData)
                                            selectedPresetId = modelData.id
                                    }
                                }
                            }
                    }
                }
        }
    }

    ColorTool
    {
        id: colorTool
        parent: presetsRoot
        z: 10000
        visible: false
        closeOnSelect: true

        // the "Full" color picker emits toolColorChanged continuously while
        // dragging, so just remember the latest color here and create a single
        // preset once the tool is closed
        property color pickedColor
        property bool colorPicked: false

        onToolColorChanged:
            function(r, g, b, w, a, uv)
            {
                pickedColor = Qt.rgba(r, g, b, 1.0)
                colorPicked = true
            }

        onVisibleChanged:
        {
            if (visible)
            {
                colorPicked = false
                return
            }

            if (!widgetRef || !colorPicked)
                return

            var presetId = widgetRef.addColorPreset(presetsRoot.colorSlot, pickedColor)
            if (presetId >= 0)
                selectedPresetId = presetId
        }

        onClose: visible = false
    }

    Component
    {
        id: presetPopupComponent

        PopupAnimationPreset
        {
            onClosed: destroy()
        }
    }

    Connections
    {
        target: widgetRef
        function onPresetsListChanged()
        {
            if (selectedPresetId < 0)
                return

            var exists = false
            var list = widgetRef.presetsList
            for (var i = 0; i < list.length; i++)
            {
                if (list[i].id === selectedPresetId)
                {
                    exists = true
                    break
                }
            }

            if (!exists)
                selectedPresetId = -1
        }
    }
}
