/*
  Q Light Controller Plus
  SceneEditor.qml

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
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.13

import org.qlcplus.classes 1.0
import "TimeUtils.js" as TimeUtils
import "."

Rectangle
{
    id: seContainer
    anchors.fill: parent
    color: "transparent"

    property int functionID
    property bool boundToSequence: false

    signal requestView(int ID, string qmlSrc)

    function deleteSelectedItems()
    {
        deleteItemsPopup.open()
    }

    CustomPopupDialog
    {
        id: deleteItemsPopup
        title: qsTr("Delete items")
        message: qsTr("Are you sure you want to remove the selected items?")
        onAccepted:
        {
            if (boundToSequence)
                functionManager.deleteSequenceFixtures(seSelector.itemsList())
            else
                functionManager.deleteEditorItems(seSelector.itemsList())
        }
    }

    ModelSelector
    {
        id: seSelector
        //onItemsCountChanged: console.log("Scene Editor selected items changed!")
        onItemSelectionChanged:
        {
            var item = sfxList.itemAtIndex(itemIndex)
            if (item.itemType === App.FixtureDragItem)
            {
                contextManager.setFixtureIDSelection(item.itemId, selected)
                if (selected)
                    sceneEditor.setFixtureSelection(item.itemId)
            }
        }
    }

    TimeEditTool
    {
        id: timeEditTool

        parent: mainView
        z: 99
        x: rightSidePanel.x - width
        visible: false
        tempoType: sceneEditor ? sceneEditor.tempoType : 0

        onValueChanged:
        {
            if (speedType == QLCFunction.FadeIn)
                sceneEditor.fadeInSpeed = val
            else if (speedType == QLCFunction.Hold)
                sceneEditor.holdSpeed = val
            else if (speedType == QLCFunction.FadeOut)
                sceneEditor.fadeOutSpeed = val
        }
    }

    SplitView
    {
        anchors.fill: parent

        Loader
        {
            id: sideLoader
            width: UISettings.sidePanelWidth
            SplitView.preferredWidth: UISettings.sidePanelWidth
            visible: false
            height: seContainer.height
            source: ""

            onLoaded:
            {
                if (source)
                    item.allowEditing = false
            }

            Rectangle
            {
                width: 2
                height: parent.height
                x: parent.width - 2
                color: UISettings.bgLight
            }
        }

        Column
        {
            SplitView.fillWidth: true

            EditorTopBar
            {
                id: toolbar
                visible: !boundToSequence
                text: sceneEditor ? sceneEditor.functionName : ""
                onTextChanged: if (sceneEditor) sceneEditor.functionName = text

                onBackClicked:
                {
                    if (sideLoader.visible)
                    {
                        sideLoader.source = ""
                        sideLoader.visible = false
                        rightSidePanel.width -= sideLoader.width
                    }

                    var prevID = sceneEditor.previousID
                    functionManager.setEditorFunction(prevID, false, true)
                    requestView(prevID, functionManager.getEditorResource(prevID))
                }

                IconButton
                {
                    id: addFixture
                    width: height
                    height: UISettings.iconSizeMedium
                    imgSource: "qrc:/fixture.svg"
                    checkable: true
                    tooltip: qsTr("Add a fixture/group")

                    Image
                    {
                        x: parent.width - width - 2
                        y: 2
                        width: parent.height / 3
                        height: width
                        source: "qrc:/add.svg"
                        sourceSize: Qt.size(width, height)
                    }

                    onCheckedChanged:
                    {
                        if (checked)
                        {
                            if (!sideLoader.visible)
                                rightSidePanel.width += UISettings.sidePanelWidth
                            sideLoader.visible = true
                            sideLoader.source = "qrc:/FixtureGroupManager.qml"
                        }
                        else
                        {
                            rightSidePanel.width -= sideLoader.width
                            sideLoader.source = ""
                            sideLoader.visible = false
                        }
                    }
                }

                IconButton
                {
                    id: addPalette
                    width: height
                    height: UISettings.iconSizeMedium
                    imgSource: "qrc:/palette.svg"
                    checkable: true
                    tooltip: qsTr("Add a palette")

                    Image
                    {
                        x: parent.width - width - 2
                        y: 2
                        width: parent.height / 3
                        height: width
                        source: "qrc:/add.svg"
                        sourceSize: Qt.size(width, height)
                    }

                    onCheckedChanged:
                    {
                        if (checked)
                        {
                            if (!sideLoader.visible)
                                rightSidePanel.width += UISettings.sidePanelWidth
                            sideLoader.visible = true
                            sideLoader.source = "qrc:/PaletteManager.qml"
                        }
                        else
                        {
                            rightSidePanel.width -= sideLoader.width
                            sideLoader.source = ""
                            sideLoader.visible = false
                        }
                    }
                }

                IconButton
                {
                    id: removeFxButton
                    x: parent.width - UISettings.iconSizeMedium - 5
                    width: height
                    height: UISettings.iconSizeMedium
                    imgSource: "qrc:/remove.svg"
                    tooltip: qsTr("Remove the selected items")
                    onClicked: deleteSelectedItems()
                }
            }

            ListView
            {
                id: sfxList
                width: seContainer.width
                height: seContainer.height - UISettings.iconSizeMedium - speedSection.height
                y: toolbar.height
                boundsBehavior: Flickable.StopAtBounds
                model: sceneEditor ? sceneEditor.componentList : null
                delegate:
                    Rectangle
                    {
                        id: compDelegate
                        width: seContainer.width
                        height: UISettings.listItemHeight
                        color: "transparent"

                        property int itemType: model.type
                        property int itemId: model.cRef.id

                        Rectangle
                        {
                            anchors.fill: parent
                            radius: 3
                            color: UISettings.highlight
                            visible: model.isSelected
                        }

                        IconTextEntry
                        {
                            width: parent.width
                            height: UISettings.listItemHeight

                            tLabel: model.cRef.name
                            iSrc:
                            {
                                switch(compDelegate.itemType)
                                {
                                    case App.FixtureGroupDragItem:
                                        "qrc:/group.svg"
                                    break;
                                    case App.PaletteDragItem:
                                    case App.FixtureDragItem:
                                        model.cRef.iconResource(true)
                                    break;
                                }
                            }

                            MouseArea
                            {
                                anchors.fill: parent

                                onClicked:
                                {
                                    if (compDelegate.itemType === App.FixtureDragItem)
                                    {
                                        if (!mouse.modifiers)
                                            contextManager.resetFixtureSelection()
                                    }

                                    seSelector.selectItem(index, sfxList.model, mouse.modifiers)
                                }
                            }
                        }
                    }

                DropArea
                {
                    id: cfDropArea
                    anchors.fill: parent
                    keys: [ "fixture", "palette" ]

                    onDropped:
                    {
                        console.log("[SceneEditor] Item dropped at x: " + drag.x + " y: " + drag.y)
                        for (var i = 0; i < drag.source.itemsList.length; i++)
                        {
                            console.log("[SE] Item type: " + drag.source.itemsList[i].itemType + ", id: " + drag.source.itemsList[i].cRef.id)
                            sceneEditor.addComponent(drag.source.itemsList[i].itemType, drag.source.itemsList[i].cRef.id)
                        }
                    }
                }

                ScrollBar.vertical: CustomScrollBar { }
            }

            SectionBox
            {
                id: speedSection
                width: parent.width
                isExpanded: false
                sectionLabel: qsTr("Speed")
                sectionContents:
                    GridLayout
                    {
                        width: parent.width
                        columns: 2
                        columnSpacing: 5
                        rowSpacing: 4

                        // Row 1
                        RobotoText
                        {
                            id: fiLabel
                            label: qsTr("Fade in")
                            height: UISettings.listItemHeight
                        }

                        Rectangle
                        {
                            Layout.fillWidth: true
                            height: UISettings.listItemHeight
                            color: UISettings.bgMedium

                            RobotoText
                            {
                                anchors.fill: parent
                                label: TimeUtils.timeToQlcString(sceneEditor.fadeInSpeed, sceneEditor.tempoType)

                                MouseArea
                                {
                                    anchors.fill: parent
                                    onDoubleClicked:
                                    {
                                        timeEditTool.allowFractions = QLCFunction.ByTwoFractions
                                        timeEditTool.show(-1, this.mapToItem(mainView, 0, 0).y - timeEditTool.height,
                                                          fiLabel.label, parent.label, QLCFunction.FadeIn)
                                    }
                                }
                            }
                        }

                        // Row 2
                        RobotoText
                        {
                            id: foLabel
                            height: UISettings.listItemHeight
                            label: qsTr("Fade out")
                        }

                        Rectangle
                        {
                            Layout.fillWidth: true
                            height: UISettings.listItemHeight
                            color: UISettings.bgMedium

                            RobotoText
                            {
                                anchors.fill: parent
                                label: TimeUtils.timeToQlcString(sceneEditor.fadeOutSpeed, sceneEditor.tempoType)

                                MouseArea
                                {
                                    anchors.fill: parent
                                    onDoubleClicked:
                                    {
                                        timeEditTool.allowFractions = QLCFunction.ByTwoFractions
                                        timeEditTool.show(-1, this.mapToItem(mainView, 0, 0).y - timeEditTool.height,
                                                          foLabel.label, parent.label, QLCFunction.FadeOut)
                                    }
                                }
                            }
                        }
                    } // GridLayout
            }
        } // Column
    } // Splitview
}
