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
import QtQuick.Controls 2.1

import org.qlcplus.classes 1.0
import "TimeUtils.js" as TimeUtils
import "."

Rectangle
{
    id: seContainer
    anchors.fill: parent
    color: "transparent"

    property int functionID
    property bool showToolBar: true

    signal requestView(int ID, string qmlSrc)

    ModelSelector
    {
        id: seSelector
        onItemsCountChanged: console.log("Scene Editor selected items changed!")
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

    Column
    {
        EditorTopBar
        {
            id: toolbar
            visible: showToolBar
            text: sceneEditor ? sceneEditor.functionName : ""
            onTextChanged: sceneEditor.functionName = text

            onBackClicked:
            {
                var prevID = sceneEditor.previousID
                functionManager.setEditorFunction(prevID, false, true)
                requestView(prevID, functionManager.getEditorResource(prevID))
            }

            IconButton
            {
                id: removeFxButton
                x: parent.width - UISettings.iconSizeMedium - 5
                width: height
                height: UISettings.iconSizeMedium
                imgSource: "qrc:/remove.svg"
                tooltip: qsTr("Remove the selected items")
                onClicked: deleteItemsPopup.open()

                CustomPopupDialog
                {
                    id: deleteItemsPopup
                    title: qsTr("Delete items")
                    message: qsTr("Are you sure you want to remove the selected items?")
                    onAccepted: functionManager.deleteEditorItems(seSelector.itemsList())
                }
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
                                seSelector.selectItem(index, sfxList.model, mouse.modifiers & Qt.ControlModifier)

                                if (compDelegate.itemType === App.FixtureDragItem)
                                {
                                    if (!(mouse.modifiers & Qt.ControlModifier))
                                        contextManager.resetFixtureSelection()

                                    contextManager.setFixtureIDSelection(model.cRef.id, true)
                                    sceneEditor.setFixtureSelection(model.cRef.id)
                                }
                            }
                            //onDoubleClicked: fxDelegate.mouseEvent(App.DoubleClicked, cRef.id, cRef.type, fxDelegate, -1)
                        }
                    }
                }
/*
                FixtureDelegate
                {
                    cRef: model.cRef
                    width: seContainer.width
                    isSelected: model.isSelected
                    Component.onCompleted: contextManager.setFixtureIDSelection(cRef.id, true)
                    Component.onDestruction: if (contextManager) contextManager.setFixtureIDSelection(cRef.id, false)
                    onMouseEvent:
                    {
                        if (type === App.Clicked)
                        {
                            seSelector.selectItem(index, sfxList.model, mouseMods & Qt.ControlModifier)

                            if (!(mouseMods & Qt.ControlModifier))
                                contextManager.resetFixtureSelection()

                            contextManager.setFixtureIDSelection(cRef.id, true)
                            sceneEditor.setFixtureSelection(cRef.id)
                        }
                    }
                }
*/

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
                        console.log("Item type: " + drag.source.itemsList[i].itemType + ", id: " + drag.source.itemsList[i].cRef.id)
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
    }
}
