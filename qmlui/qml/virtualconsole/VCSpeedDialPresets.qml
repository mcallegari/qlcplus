/*
  Q Light Controller Plus
  VCSpeedDialPresets.qml

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
import "TimeUtils.js" as TimeUtils
import "."

Rectangle
{
    id: presetsRoot
    color: "transparent"
    height: presetsColumn.height

    property VCSpeedDial widgetRef: null
    property int selectedPresetId: -1
    property bool editingExistingPreset: false
    property bool updatingFields: false
    property bool editorReady: false
    property int presetTimeValue: 0

    function selectPresetById(presetId, markEditing)
    {
        if (markEditing === undefined)
            markEditing = true

        editingExistingPreset = (presetId >= 0 && markEditing)
        selectedPresetId = presetId
        updateEditorFields()
    }

    function updateEditorFields()
    {
        if (!widgetRef || !editorReady || typeof presetNameEdit === "undefined")
            return

        var preset = null
        for (var i = 0; i < widgetRef.presetsList.length; i++)
        {
            if (widgetRef.presetsList[i].id === selectedPresetId)
            {
                preset = widgetRef.presetsList[i]
                break
            }
        }

        updatingFields = true
        if (preset)
        {
            presetNameEdit.text = preset.name
            presetTimeValue = preset.value
        }
        else
        {
            presetNameEdit.text = ""
            presetTimeValue = 0
            selectedPresetId = -1
        }
        updatingFields = false
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

                    GridLayout
                    {
                        id: presetEditorGrid
                        width: parent.width
                        columns: 3
                        columnSpacing: 5
                        rowSpacing: 4

                        function showTimeTool(item, titleLabel)
                        {
                            timeEditTool.allowFractions = QLCFunction.ByTwoFractions
                            timeEditTool.show(-1, item.mapToItem(mainView, 0, 0).y - timeEditTool.height,
                                              titleLabel,
                                              TimeUtils.timeToQlcString(presetTimeValue, QLCFunction.Time),
                                              QLCFunction.Duration)
                        }

                        RobotoText
                        {
                            height: UISettings.listItemHeight
                            label: qsTr("Preset name")
                        }
                        CustomTextEdit
                        {
                            id: presetNameEdit
                            Layout.fillWidth: true
                            Layout.columnSpan: 2
                            height: UISettings.listItemHeight
                            onTextEdited:
                            {
                                if (updatingFields || !widgetRef || !editingExistingPreset)
                                    return

                                widgetRef.setPresetName(selectedPresetId, text)
                            }
                        }

                        RobotoText
                        {
                            height: UISettings.listItemHeight
                            label: qsTr("Preset time")
                        }
                        Rectangle
                        {
                            Layout.fillWidth: true
                            height: UISettings.listItemHeight
                            color: UISettings.bgMedium

                            RobotoText
                            {
                                x: 3
                                height: parent.height
                                label: TimeUtils.timeToQlcString(presetTimeValue, QLCFunction.Time)
                            }
                            MouseArea
                            {
                                anchors.fill: parent
                                onDoubleClicked: presetEditorGrid.showTimeTool(this, qsTr("Preset time"))
                            }
                        }
                        IconButton
                        {
                            width: height
                            height: UISettings.listItemHeight
                            faSource: FontAwesome.fa_clock
                            faColor: UISettings.fgMain
                            onClicked: presetEditorGrid.showTimeTool(this, qsTr("Preset time"))
                        }
                    }

                    Rectangle
                    {
                        width: parent.width
                        height: UISettings.iconSizeMedium
                        color: UISettings.bgMedium

                        RowLayout
                        {
                            anchors.fill: parent
                            spacing: 5

                            Rectangle
                            {
                                Layout.fillWidth: true
                                height: parent.height
                                color: "transparent"
                            }

                            IconButton
                            {
                                width: height * 2
                                height: parent.height
                                faSource: FontAwesome.fa_plus
                                faColor: "limegreen"
                                tooltip: qsTr("Add a preset")
                                enabled: presetNameEdit.text.length > 0 && presetTimeValue > 0
                                onClicked:
                                {
                                    if (!widgetRef)
                                        return

                                    var newId = widgetRef.addPreset(presetNameEdit.text, presetTimeValue)
                                    selectPresetById(newId, false)
                                }
                            }

                            IconButton
                            {
                                width: height * 2
                                height: parent.height
                                faSource: FontAwesome.fa_minus
                                faColor: "crimson"
                                tooltip: qsTr("Remove the selected preset")
                                enabled: selectedPresetId >= 0
                                onClicked:
                                {
                                    if (!widgetRef || selectedPresetId < 0)
                                        return

                                    widgetRef.removePreset(selectedPresetId)
                                    selectPresetById(-1)
                                }
                            }
                        }
                    }

                    ListView
                    {
                        id: presetsList
                        width: parent.width
                        height: count ? (count + 1) * UISettings.listItemHeight : UISettings.bigItemHeight
                        boundsBehavior: Flickable.StopAtBounds
                        headerPositioning: ListView.OverlayHeader

                        property real nameColWidth: presetsList.width * 0.65
                        property real valueColWidth: presetsList.width * 0.35

                        model: widgetRef ? widgetRef.presetsList : null

                        header:
                            Row
                            {
                                z: 2
                                width: presetsList.width
                                height: UISettings.listItemHeight

                                RobotoText
                                {
                                    width: presetsList.nameColWidth
                                    height: parent.height
                                    label: qsTr("Name")
                                    color: UISettings.sectionHeader
                                }
                                Rectangle { width: 1; height: parent.height; color: UISettings.fgMedium }

                                RobotoText
                                {
                                    width: presetsList.valueColWidth - 1
                                    height: parent.height
                                    label: qsTr("Time")
                                    color: UISettings.sectionHeader
                                }
                            }

                        delegate:
                            Rectangle
                            {
                                width: presetsList.width
                                height: UISettings.listItemHeight
                                color: modelData.id === presetsRoot.selectedPresetId ? UISettings.highlight : "transparent"

                                Row
                                {
                                    anchors.fill: parent

                                    RobotoText
                                    {
                                        width: presetsList.nameColWidth
                                        height: parent.height
                                        label: modelData.name
                                    }
                                    Rectangle { width: 1; height: parent.height; color: UISettings.fgMedium }
                                    RobotoText
                                    {
                                        width: presetsList.valueColWidth - 1
                                        height: parent.height
                                        label: TimeUtils.timeToQlcString(modelData.value, QLCFunction.Time)
                                    }
                                }

                                MouseArea
                                {
                                    anchors.fill: parent
                                    onClicked: presetsRoot.selectPresetById(modelData.id, true)
                                }
                            }

                        onModelChanged:
                        {
                            if (presetsRoot.editorReady)
                                updateEditorFields()
                        }
                    }

                    Item
                    {
                        width: 1
                        height: 1
                        Component.onCompleted: presetsRoot.editorReady = true
                        Component.onDestruction: presetsRoot.editorReady = false
                    }
                }
        }
    }

    Connections
    {
        target: widgetRef
        function onPresetsListChanged()
        {
            if (presetsRoot.editorReady)
                updateEditorFields()
        }
    }

    TimeEditTool
    {
        id: timeEditTool
        parent: mainView
        z: 99
        x: rightSidePanel.x - width
        visible: false

        onValueChanged: (val) =>
        {
            presetTimeValue = val
            if (widgetRef && editingExistingPreset)
                widgetRef.setPresetValue(selectedPresetId, val)
        }
    }
}
