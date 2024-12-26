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
import QtQuick.Controls 2.2

import org.qlcplus.classes 1.0
import "."

Rectangle
{
    id: fgmContainer
    objectName: "fixtureGroupManager"
    anchors.fill: parent
    color: "transparent"

    /** By default, fixtureManager is the model provider, unless a
      * specific provider is set here. In that case, modelProvider
      * must provide a 'groupsTreeModel' method */
    property var modelProvider: null
    property bool allowEditing: true

    signal doubleClicked(int ID, int type)

    function updateButtons(itemType, itemID)
    {
        // update info button
        infoButton.enabled = itemType !== App.HeadDragItem ? true : false
        updateInfoView()

        // update rename button
        renameButton.enabled = true

        // update linked button
        if (fixtureManager.propertyEditEnabled === false)
            return

        linkedButton.enabled = itemType === App.FixtureDragItem ? true : false
        var linkedIndex = fixtureManager.fixtureLinkedIndex(itemID)
        linkedButton.faSource = linkedIndex ? FontAwesome.fa_unlink : FontAwesome.fa_link
    }

    function updateInfoView()
    {
        if (gfhcDragItem.itemsList.length === 0)
            return

        if (!infoButton.checked)
            return

        switch(gfhcDragItem.itemsList[0].itemType)
        {
            case App.UniverseDragItem:
                fixtureManager.itemID = gfhcDragItem.itemsList[0].cRef.id
                fixtureAndFunctions.currentViewQML = "qrc:/UniverseSummary.qml"
            break;
            case App.FixtureGroupDragItem:
                fixtureGroupEditor.setEditGroup(gfhcDragItem.itemsList[0].cRef)
                fixtureAndFunctions.currentViewQML = "qrc:/FixtureGroupEditor.qml"
            break;
            case App.FixtureDragItem:
                fixtureManager.itemID = gfhcDragItem.itemsList[0].itemID
                fixtureAndFunctions.currentViewQML = "qrc:/FixtureSummary.qml"
            break;
        }
    }

    function showChannelModifierEditor(itemID, channelIndex, modifierName)
    {
        chModifierEditor.itemID = itemID
        chModifierEditor.chIndex = channelIndex
        chModifierEditor.modName = modifierName
        chModifierEditor.open()
    }

    CustomPopupDialog
    {
        id: fmGenericPopup
        visible: false
        title: qsTr("Error")
        message: ""
        onAccepted: {}
    }

    PopupChannelModifiers
    {
        id: chModifierEditor
        visible: false

        onAccepted: fixtureManager.setChannelModifier(itemID, chIndex)
    }

    ColumnLayout
    {
        anchors.fill: parent
        spacing: 0

        Rectangle
        {
            id: topBar
            implicitWidth: fgmContainer.width
            implicitHeight: UISettings.iconSizeMedium
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
                    visible: allowEditing
                    z: 2
                    width: height
                    height: topBar.height - 2
                    imgSource: "qrc:/add.svg"
                    tooltip: qsTr("Add a new fixture group")
                    onClicked: contextManager.createFixtureGroup()
                }
                IconButton
                {
                    id: delItemButton
                    visible: allowEditing
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

                            switch (item.itemType)
                            {
                                case App.UniverseDragItem:
                                break;
                                case App.FixtureGroupDragItem:
                                    fxGroupDeleteList.push(item.cRef.id)
                                break;
                                case App.FixtureDragItem:
                                    if (item.inGroup)
                                        fixtureManager.deleteFixtureInGroup(item.subID, item.itemID, item.nodePath)
                                    else
                                        fxDeleteList.push(item.itemID)
                                break;
                            }
                        }

                        if (fxDeleteList.length)
                        {
                            contextManager.resetFixtureSelection()
                            fixtureManager.deleteFixtures(fxDeleteList)
                        }

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
                    bgColor: UISettings.bgMedium
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
                    id: renameButton
                    visible: allowEditing
                    z: 2
                    width: height
                    height: topBar.height - 2
                    imgSource: "qrc:/rename.svg"
                    tooltip: qsTr("Rename the selected items")
                    enabled: false

                    onClicked:
                    {
                        renamePopup.showNumbering = gfhcDragItem.itemsList.length > 1 ? true : false
                        renamePopup.editText = gfhcDragItem.itemsList[0].textLabel
                        renamePopup.open()
                    }

                    PopupRenameItems
                    {
                        id: renamePopup
                        title: qsTr("Rename items")
                        onAccepted:
                        {
                            var item
                            var ret

                            if (numberingEnabled)
                            {
                                var currNum = startNumber
                                var i, zeroes = ""

                                for (i = 0; i < digits; i++)
                                    zeroes += '0'

                                for (i = 0; i < gfhcDragItem.itemsList.length; i++)
                                {
                                    item = gfhcDragItem.itemsList[i]
                                    var zerofilled = (zeroes + currNum).slice(-digits);
                                    var finalName = editText + " " + zerofilled
                                    currNum++

                                    if (item.itemType === App.FixtureDragItem)
                                        ret = fixtureManager.renameFixture(item.itemID, finalName)
                                    else if (item.itemType === App.FixtureGroupDragItem)
                                        ret = fixtureManager.renameFixtureGroup(item.itemID, finalName)

                                    if (ret === false)
                                        break
                                }
                            }
                            else
                            {
                                item = gfhcDragItem.itemsList[0];

                                if (item.itemType === App.FixtureDragItem)
                                    ret = fixtureManager.renameFixture(item.itemID, editText)
                                else if (item.itemType === App.FixtureGroupDragItem)
                                    ret = fixtureManager.renameFixtureGroup(item.itemID, editText)
                            }

                            if (ret === false)
                            {
                                fmGenericPopup.message = qsTr("An item with the same name already exists.\nPlease provide a different name.")
                                fmGenericPopup.open()
                            }
                        }
                    }
                }

                IconButton
                {
                    id: infoButton
                    visible: allowEditing
                    z: 2
                    width: height
                    height: topBar.height - 2
                    imgSource: "qrc:/info.svg"
                    tooltip: qsTr("Inspect the selected item")
                    enabled: false
                    checkable: true

                    property string previousView: ""

                    onToggled:
                    {
                        if (checked)
                        {
                            previousView = fixtureAndFunctions.currentViewQML
                            updateInfoView()
                        }
                        else
                        {
                            fixtureGroupEditor.setEditGroup(null)
                            fixtureAndFunctions.currentViewQML = previousView
                            previousView = ""
                        }
                    }
                }

                IconButton
                {
                    id: propsButton
                    visible: allowEditing
                    width: height
                    height: topBar.height - 2
                    imgSource: "qrc:/edit.svg"
                    tooltip: qsTr("Toggle fixtures and channels properties")
                    checkable: true

                    onToggled:
                    {
                        if (checked)
                            leftSidePanel.width += UISettings.sidePanelWidth
                        else
                            leftSidePanel.width -= UISettings.sidePanelWidth
                        fixtureManager.propertyEditEnabled = checked
                    }
                }

                IconButton
                {
                    id: linkedButton
                    visible: fixtureManager.propertyEditEnabled
                    enabled: false
                    width: height
                    height: topBar.height - 2
                    faSource: FontAwesome.fa_link
                    faColor: "white"
                    tooltip: qsTr("Add/Remove a linked fixture")
                    onClicked: contextManager.setLinkedFixture(gfhcDragItem.itemsList[0].itemID)
                }
            }
        } // RowLayout

        Rectangle
        {
            id: propertiesHeader
            visible: propsButton.checked
            implicitHeight: UISettings.iconSizeMedium
            implicitWidth: fgmContainer.width - (gEditScrollBar.visible ? gEditScrollBar.width : 0)
            z: 5
            color: UISettings.bgMedium

            RowLayout
            {
                anchors.fill: parent

                RobotoText { label: qsTr("Name"); Layout.fillWidth: true; height: parent.height }
                Rectangle { width: 1; height: parent.height }
                RobotoText { label: qsTr("Mode"); width: UISettings.chPropsModesWidth; height: parent.height }
                Rectangle { width: 1; height: parent.height }
                RobotoText { label: qsTr("Flags"); width: UISettings.chPropsFlagsWidth; height: parent.height }
                Rectangle { width: 1; height: parent.height }
                RobotoText { label: qsTr("Can fade"); width: UISettings.chPropsCanFadeWidth; height: parent.height }
                Rectangle { width: 1; height: parent.height }
                RobotoText { label: qsTr("Behaviour"); width: UISettings.chPropsPrecedenceWidth; height: parent.height }
                Rectangle { width: 1; height: parent.height }
                RobotoText { label: qsTr("Modifier"); width: UISettings.chPropsModifierWidth; height: parent.height }
            }
        }

        Rectangle
        {
            id: searchBox
            visible: searchItem.checked
            width: fgmContainer.width
            implicitHeight: UISettings.iconSizeMedium
            z: 5
            color: UISettings.bgMedium
            radius: 5
            border.width: 2
            border.color: UISettings.borderColorDark

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
            //implicitWidth: fgmContainer.width
            Layout.fillWidth: true
            Layout.fillHeight: true
            //height: fgmContainer.height - topBar.height -
            //        (searchBox.visible ? searchBox.height : 0) -
            //        (propertiesHeader.visible ? propertiesHeader.height : 0)
            z: 4
            boundsBehavior: Flickable.StopAtBounds

            property bool dragActive: false

            model: modelProvider ? modelProvider.groupsTreeModel : fixtureManager.groupsTreeModel
            delegate:
              Component
              {
                Loader
                {
                    //width: groupListView.width - (gEditScrollBar.visible ? gEditScrollBar.width : 0)
                    source: hasChildren ? "qrc:/TreeNodeDelegate.qml" : ""
                    onLoaded:
                    {
                        //console.log("[groupEditor] Item " + label + " has children: " + hasChildren)
                        item.width = Qt.binding(function() { return fgmContainer.width - (gEditScrollBar.visible ? gEditScrollBar.width : 0) })
                        item.cRef = classRef
                        item.textLabel = Qt.binding(function() { return label })
                        item.isSelected = Qt.binding(function() { return isSelected })
                        item.dragItem = gfhcDragItem

                        if (hasChildren)
                        {
                            item.itemIcon = "qrc:/group.svg"
                            if (type)
                            {
                                item.itemType = type
                                if (type === App.UniverseDragItem)
                                    isExpanded = true
                            }
                            item.isExpanded = isExpanded
                            item.nodePath = path
                            item.subTreeDelegate = "qrc:/FixtureNodeDelegate.qml"
                            item.childrenDelegate = "qrc:/FixtureNodeDelegate.qml"
                            item.nodeChildren = childrenModel
                        }
                    }
                    Connections
                    {
                        target: item

                        function onMouseEvent(type, iID, iType, qItem, mouseMods)
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
                                    }
                                break;
                                case App.Clicked:
                                    if (qItem === item)
                                    {
                                        model.isSelected = (mouseMods & Qt.ControlModifier) ? 2 : 1
                                        if (model.hasChildren)
                                            model.isExpanded = item.isExpanded
                                    }

                                    if (!(mouseMods & Qt.ControlModifier))
                                        contextManager.resetFixtureSelection()

                                    console.log("Item clicked. Type: " + qItem.itemType + ", id: " + iID)

                                    var itemID = iID

                                    switch (qItem.itemType)
                                    {
                                        case App.FixtureDragItem:
                                            contextManager.setFixtureSelection(iID, -1, true)
                                        break;
                                        case App.HeadDragItem:
                                            itemID = qItem.itemID
                                            contextManager.setFixtureSelection(qItem.itemID, iID, true);
                                        break;
                                        case App.UniverseDragItem:
                                            contextManager.setFixtureGroupSelection(iID, true, true)
                                        break;
                                        case App.FixtureGroupDragItem:
                                            contextManager.setFixtureGroupSelection(iID, true, false)
                                        break;
                                    }

                                    updateButtons(qItem.itemType, itemID)
                                break;
                                case App.DoubleClicked:
                                    if (allowEditing == false && qItem.itemType === App.FixtureDragItem)
                                        fgmContainer.doubleClicked(iID, qItem.itemType)
                                break;
                                case App.DragStarted:
                                    if (qItem === item && !model.isSelected)
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

            ScrollBar.vertical: CustomScrollBar {
                id: gEditScrollBar
            }

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
