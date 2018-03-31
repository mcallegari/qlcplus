/*
  Q Light Controller Plus
  FixtureGroupManager.qml

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

import org.qlcplus.classes 1.0
import "."

Rectangle
{
    id: fgmContainer
    anchors.fill: parent
    color: "transparent"

    /** By default, fixtureManager is the model provider, unless a
      * specific provider is set here. In that case, modelProvider
      * must provide a 'groupsTreeModel' method */
    property var modelProvider: null

    ColumnLayout
    {
        anchors.fill: parent
        spacing: 3

        Rectangle
        {
            id: topBar
            width: fgmContainer.width
            height: UISettings.iconSizeMedium
            z: 5
            gradient: Gradient
            {
                GradientStop { position: 0; color: UISettings.toolbarStartSub }
                GradientStop { position: 1; color: UISettings.toolbarEnd }
            }

            RowLayout
            {
                id: topBarRowLayout
                width: parent.width
                y: 1

                spacing: 4

                IconButton
                {
                    id: addGrpButton
                    z: 2
                    width: height
                    height: topBar.height - 2
                    imgSource: "qrc:/add.svg"
                    tooltip: qsTr("Add a new group")
                    onClicked: contextManager.createFixtureGroup()
                }
                IconButton
                {
                    id: delItemButton
                    z: 2
                    width: height
                    height: topBar.height - 2
                    imgSource: "qrc:/remove.svg"
                    tooltip: qsTr("Remove the selected items")
                    onClicked:
                    {
                        if (gfhcDragItem.itemsList.length === 0)
                            return;

                        var fxDeleteList = []
                        var fxGroupDeleteList = []

                        for (var i = 0; i < gfhcDragItem.itemsList.length; i++)
                        {
                            var item = gfhcDragItem.itemsList[i]

                            switch(item.itemType)
                            {
                                case App.UniverseDragItem:
                                break;
                                case App.FixtureGroupDragItem:
                                    fxGroupDeleteList.push(item.cRef.id)
                                break;
                                case App.FixtureDragItem:
                                    fxDeleteList.push(item.cRef.id)
                                break;
                            }
                        }

                        if (fxDeleteList.length)
                            fixtureManager.deleteFixtures(fxDeleteList)

                        if (fxGroupDeleteList.length)
                            fixtureManager.deleteFixtureGroups(fxGroupDeleteList)
                    }
                }
                Rectangle { Layout.fillWidth: true }
                IconButton
                {
                    id: searchItem
                    z: 2
                    width: height
                    height: topBar.height - 2
                    bgColor: UISettings.bgMain
                    faColor: checked ? "white" : "gray"
                    faSource: FontAwesome.fa_search
                    checkable: true
                    tooltip: qsTr("Set a Group/Fixture/Channel search filter")
                    onToggled:
                    {
                        fixtureManager.searchFilter = ""
                        if (checked)
                            sTextInput.forceActiveFocus()
                    }
                }
                IconButton
                {
                    id: infoButton
                    z: 2
                    width: height
                    height: topBar.height - 2
                    imgSource: "qrc:/info.svg"
                    tooltip: qsTr("Inspect the selected item")
                    checkable: true

                    property string previousView: ""

                    onToggled:
                    {
                        if (gfhcDragItem.itemsList.length === 0)
                            return;

                        if (checked)
                            previousView = fixtureAndFunctions.currentViewQML

                        switch(gfhcDragItem.itemsList[0].itemType)
                        {
                            case App.UniverseDragItem:
                                if (checked)
                                {
                                    fixtureManager.itemID = gfhcDragItem.itemsList[0].cRef.id
                                    fixtureAndFunctions.currentViewQML = "qrc:/UniverseSummary.qml"
                                }
                            break;
                            case App.FixtureGroupDragItem:
                                if (checked)
                                {
                                    fixtureGroupEditor.setEditGroup(gfhcDragItem.itemsList[0].cRef)
                                    fixtureAndFunctions.currentViewQML = "qrc:/FixtureGroupEditor.qml"
                                }
                                else
                                {
                                    fixtureGroupEditor.setEditGroup(null)
                                }

                            break;
                            case App.FixtureDragItem:
                                if (checked)
                                {
                                    fixtureManager.itemID = gfhcDragItem.itemsList[0].cRef.id
                                    fixtureAndFunctions.currentViewQML = "qrc:/FixtureSummary.qml"
                                }
                            break;
                        }

                        if (!checked)
                        {
                            fixtureAndFunctions.currentViewQML = previousView
                            previousView = ""
                        }
                    }
                }
            }
        }

        Rectangle
        {
            id: searchBox
            visible: searchItem.checked
            width: fgmContainer.width
            height: UISettings.iconSizeMedium
            z: 5
            color: UISettings.bgMain
            radius: 5
            border.width: 2
            border.color: "#111"

            TextInput
            {
                id: sTextInput
                y: 3
                height: parent.height - 6
                width: parent.width
                color: UISettings.fgMain
                text: modelProvider ? modelProvider.searchFilter : fixtureManager.searchFilter
                font.family: "Roboto Condensed"
                font.pixelSize: parent.height - 6
                selectionColor: UISettings.highlightPressed
                selectByMouse: true

                onTextChanged: modelProvider ? modelProvider.searchFilter = text : fixtureManager.searchFilter = text
            }
        }

        ListView
        {
            id: groupListView
            width: fgmContainer.width
            height: fgmContainer.height - topBar.height - (searchBox.visible ? searchBox.height : 0)
            z: 4
            boundsBehavior: Flickable.StopAtBounds

            property bool dragActive: false

            model: modelProvider ? modelProvider.groupsTreeModel : fixtureManager.groupsTreeModel
            delegate:
              Component
              {
                Loader
                {
                    width: groupListView.width - (gEditScrollBar.visible ? gEditScrollBar.width : 0)
                    source: hasChildren ? "qrc:/TreeNodeDelegate.qml" : ""
                    onLoaded:
                    {
                        //console.log("[groupEditor] Item " + label + " has children: " + hasChildren)
                        item.cRef = classRef
                        item.textLabel = label
                        item.isSelected = Qt.binding(function() { return isSelected })
                        item.dragItem = gfhcDragItem

                        if (hasChildren)
                        {
                            item.itemIcon = "qrc:/group.svg"
                            //if (modelData.hasOwnProperty("type"))
                            if (type)
                                item.itemType = type
                            item.nodePath = path
                            item.isExpanded = isExpanded
                            item.subTreeDelegate = "qrc:/FixtureNodeDelegate.qml"
                            item.nodeChildren = childrenModel
                        }
                    }
                    Connections
                    {
                        target: item

                        onMouseEvent:
                        {
                            switch (type)
                            {
                                case App.Pressed:
                                    var posnInWindow = qItem.mapToItem(mainView, qItem.x, qItem.y)
                                    gfhcDragItem.parent = mainView
                                    gfhcDragItem.x = posnInWindow.x - (gfhcDragItem.width / 4)
                                    gfhcDragItem.y = posnInWindow.y - (gfhcDragItem.height / 4)
                                    if (!qItem.isSelected)
                                    {
                                        if ((mouseMods & Qt.ControlModifier) == 0)
                                            gfhcDragItem.itemsList = []

                                        gfhcDragItem.itemsList.push(qItem)
                                        //console.log("[TOP LEVEL] Got item press event: " + gfhcDragItem.itemsList.length)

                                        if (gfhcDragItem.itemsList.length === 1)
                                        {
                                            gfhcDragItem.itemLabel = qItem.textLabel
                                            if (qItem.hasOwnProperty("itemIcon"))
                                                gfhcDragItem.itemIcon = qItem.itemIcon
                                            else
                                                gfhcDragItem.itemIcon = ""
                                        }
                                        gfhcDragItem.multipleItems = gfhcDragItem.itemsList.length > 1 ? true : false
                                    }
                                break;
                                case App.Clicked:
                                    if (qItem == item)
                                    {
                                        model.isSelected = (mouseMods & Qt.ControlModifier) ? 2 : 1
                                        if (model.hasChildren)
                                            model.isExpanded = item.isExpanded
                                    }

                                    if (infoButton.checked)
                                        fixtureManager.itemID = iID

                                    if (!(mouseMods & Qt.ControlModifier))
                                        contextManager.resetFixtureSelection()

                                    console.log("Item clicked. Type: " + qItem.itemType + ", id: " + iID)

                                    switch (qItem.itemType)
                                    {
                                        case App.FixtureDragItem:
                                            contextManager.setFixtureSelection(iID, true)
                                        break;
                                        case App.UniverseDragItem:
                                            contextManager.setFixtureGroupSelection(iID, true, true)
                                        break;
                                        case App.FixtureGroupDragItem:
                                            contextManager.setFixtureGroupSelection(iID, true, false)
                                        break;
                                    }
                                break;
                                case App.DragStarted:
                                    if (qItem == item && !model.isSelected)
                                    {
                                        model.isSelected = 1
                                        // invalidate the modifiers to force a single selection
                                        mouseMods = -1
                                    }

                                    groupListView.dragActive = true
                                break;
                                case App.DragFinished:
                                    gfhcDragItem.Drag.drop()
                                    gfhcDragItem.parent = groupListView
                                    gfhcDragItem.x = 0
                                    gfhcDragItem.y = 0
                                    groupListView.dragActive = false
                                    //gfhcDragItem.itemsList = []
                                break;
                            }
                        }
                    }
                } // Loader
              } // Component
            CustomScrollBar { id: gEditScrollBar; flickable: groupListView }

            // Group / Fixture / Head / Channel draggable item
            GenericMultiDragItem
            {
                id: gfhcDragItem

                visible: groupListView.dragActive

                Drag.active: groupListView.dragActive
                Drag.source: gfhcDragItem
                Drag.keys: [ "fixture" ]
            }
        } // ListView
    } // ColumnLayout
}
