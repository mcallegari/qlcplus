/*
  Q Light Controller Plus
  VCXYPadPresets.qml

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

    property VCXYPad widgetRef: null
    property int selectedPresetId: -1
    property bool panelOpenByThisView: false

    function selectedPreset()
    {
        if (!widgetRef || selectedPresetId < 0)
            return null

        for (var i = 0; i < widgetRef.presetsList.length; i++)
        {
            if (widgetRef.presetsList[i].id === selectedPresetId)
                return widgetRef.presetsList[i]
        }

        return null
    }

    function presetIcon(type)
    {
        switch (type)
        {
            case "Scene": return "qrc:/scene.svg"
            case "EFX": return "qrc:/efx.svg"
            case "FixtureGroup": return "qrc:/group.svg"
            default: return "qrc:/position.svg"
        }
    }

    function openSidePanel(qmlSource, modelProvider)
    {
        if (!sideLoader.visible)
        {
            rightSidePanel.width += UISettings.sidePanelWidth
            panelOpenByThisView = true
        }
        else if (!panelOpenByThisView)
        {
            panelOpenByThisView = true
        }

        sideLoader.visible = true
        sideLoader.modelProvider = modelProvider
        sideLoader.source = qmlSource
    }

    function closeSidePanel()
    {
        if (!panelOpenByThisView)
            return

        if (sideLoader.visible)
            rightSidePanel.width -= sideLoader.width

        sideLoader.source = ""
        sideLoader.visible = false
        panelOpenByThisView = false
    }

    Component.onDestruction: closeSidePanel()

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

                    Rectangle
                    {
                        width: parent.width
                        height: UISettings.listItemHeight
                        color: UISettings.bgMedium

                        IconButton
                        {
                            id: addFunctionPreset
                            anchors.top: parent.top
                            anchors.left: parent.left
                            width: height
                            height: parent.height
                            imgSource: "qrc:/functions.svg"
                            checkable: true
                            tooltip: qsTr("Drag Scene/EFX functions as presets")

                            onCheckedChanged:
                            {
                                if (checked)
                                {
                                    addFixturePreset.checked = false
                                    presetsRoot.openSidePanel("qrc:/FunctionManager.qml", null)
                                }
                                else if (!addFixturePreset.checked)
                                {
                                    presetsRoot.closeSidePanel()
                                }
                            }
                        }

                        IconButton
                        {
                            id: addFixturePreset
                            anchors.top: parent.top
                            anchors.left: addFunctionPreset.right
                            width: height
                            height: parent.height
                            imgSource: "qrc:/group.svg"
                            checkable: true
                            tooltip: qsTr("Drag fixture items as fixture-group presets")

                            onCheckedChanged:
                            {
                                if (checked)
                                {
                                    addFunctionPreset.checked = false
                                    presetsRoot.openSidePanel("qrc:/FixtureGroupManager.qml", widgetRef)
                                }
                                else if (!addFunctionPreset.checked)
                                {
                                    presetsRoot.closeSidePanel()
                                }
                            }
                        }

                        IconButton
                        {
                            id: addPositionPreset
                            anchors.top: parent.top
                            anchors.left: addFixturePreset.right
                            width: height
                            height: parent.height
                            imgSource: "qrc:/position.svg"
                            tooltip: qsTr("Create a position preset from current XY position")
                            onClicked:
                            {
                                if (!widgetRef)
                                    return

                                var presetId = widgetRef.addPositionPreset()
                                if (presetId >= 0)
                                    selectedPresetId = presetId
                            }
                        }

                        IconButton
                        {
                            anchors.top: parent.top
                            anchors.right: moveDownPreset.left
                            width: height
                            height: parent.height
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
                            anchors.top: parent.top
                            anchors.right: moveUpPreset.left
                            width: height
                            height: parent.height
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
                            anchors.top: parent.top
                            anchors.right: parent.right
                            width: height
                            height: parent.height
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

                    GridLayout
                    {
                        width: parent.width
                        columns: 2
                        columnSpacing: 5
                        rowSpacing: 4

                        RobotoText
                        {
                            height: UISettings.listItemHeight
                            label: qsTr("Preset name")
                        }
                        CustomTextEdit
                        {
                            Layout.fillWidth: true
                            height: UISettings.listItemHeight
                            enabled: selectedPresetId >= 0
                            text: selectedPreset() ? selectedPreset().name : ""
                            onTextEdited:
                            {
                                if (widgetRef && selectedPresetId >= 0)
                                    widgetRef.setPresetName(selectedPresetId, text)
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

                                    IconTextEntry
                                    {
                                        width: presetsList.width * 0.7
                                        height: parent.height
                                        iSrc: presetsRoot.presetIcon(modelData.typeString)
                                        tLabel: modelData.name
                                    }

                                    RobotoText
                                    {
                                        width: (presetsList.width * 0.3) - 1
                                        height: parent.height
                                        textHAlign: Text.AlignHCenter
                                        label: modelData.typeString === "FixtureGroup"
                                               ? qsTr("%1 heads").arg(modelData ? modelData.headsCount : 0)
                                               : modelData.typeString
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

                    Rectangle
                    {
                        id: functionDropBox
                        width: parent.width
                        height: UISettings.bigItemHeight * 0.6
                        visible: addFunctionPreset.checked
                        color: functionDrop.containsDrag ? UISettings.activeDropArea : UISettings.bgMedium
                        radius: 10
                        opacity: 0.85

                        RobotoText
                        {
                            anchors.centerIn: parent
                            label: qsTr("Drop Scene/EFX functions here")
                            labelColor: functionDrop.containsDrag ? UISettings.bgStronger : UISettings.fgMain
                            fontBold: functionDrop.containsDrag
                        }

                        DropArea
                        {
                            id: functionDrop
                            anchors.fill: parent
                            keys: [ "function" ]

                            onDropped:
                            {
                                if (!widgetRef || !drag.source.hasOwnProperty("fromFunctionManager"))
                                    return

                                var addedPresetId = -1
                                for (var i = 0; i < drag.source.itemsList.length; i++)
                                {
                                    var presetId = widgetRef.addFunctionPreset(drag.source.itemsList[i])
                                    if (presetId >= 0)
                                        addedPresetId = presetId
                                }

                                if (addedPresetId >= 0)
                                    selectedPresetId = addedPresetId
                            }
                        }
                    }

                    Rectangle
                    {
                        id: fixtureDropBox
                        width: parent.width
                        height: UISettings.bigItemHeight * 0.6
                        visible: addFixturePreset.checked
                        color: fixtureDrop.containsDrag ? UISettings.activeDropArea : UISettings.bgMedium
                        radius: 10
                        opacity: 0.85

                        RobotoText
                        {
                            anchors.centerIn: parent
                            label: qsTr("Drop fixture groups/heads here")
                            labelColor: fixtureDrop.containsDrag ? UISettings.bgStronger : UISettings.fgMain
                            fontBold: fixtureDrop.containsDrag
                        }

                        DropArea
                        {
                            id: fixtureDrop
                            anchors.fill: parent
                            keys: [ "fixture" ]

                            onDropped:
                            {
                                if (!widgetRef || !drag.source.itemsList || !drag.source.itemsList.length)
                                    return

                                var addedPresetId = -1
                                for (var i = 0; i < drag.source.itemsList.length; i++)
                                {
                                    var item = drag.source.itemsList[i]
                                    var presetId = -1

                                    switch (item.itemType)
                                    {
                                        case App.UniverseDragItem:
                                        case App.FixtureGroupDragItem:
                                        case App.FixtureDragItem:
                                            presetId = widgetRef.addFixtureGroupPreset(item.cRef)
                                        break
                                        case App.HeadDragItem:
                                            presetId = widgetRef.addFixtureGroupHeadPreset(item.itemID, item.headIndex)
                                        break
                                    }

                                    if (presetId >= 0)
                                        addedPresetId = presetId
                                }

                                if (addedPresetId >= 0)
                                    selectedPresetId = addedPresetId
                            }
                        }
                    }

                    Connections
                    {
                        target: presetsRoot
                        function onVisibleChanged()
                        {
                            if (!presetsRoot.visible)
                            {
                                addFunctionPreset.checked = false
                                addFixturePreset.checked = false
                                presetsRoot.closeSidePanel()
                            }
                        }
                    }
                }
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
            for (var i = 0; i < widgetRef.presetsList.length; i++)
            {
                if (widgetRef.presetsList[i].id === selectedPresetId)
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
