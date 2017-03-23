/*
  Q Light Controller Plus
  GroupEditor.qml

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
import QtQuick.Controls 1.2

import com.qlcplus.classes 1.0
import "."

Rectangle
{
    id: geContainer
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
            width: geContainer.width
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
                }
                Rectangle { Layout.fillWidth: true }
            }
        }

        ListView
        {
            id: groupListView
            width: geContainer.width
            height: geContainer.height - topBar.height
            z: 4
            boundsBehavior: Flickable.StopAtBounds

            property bool dragActive: false

            model: modelProvider ? modelProvider.groupsTreeModel : fixtureManager.groupsTreeModel
            delegate:
              Component
              {
                Loader
                {
                    width: groupListView.width
                    source: hasChildren ? "qrc:/TreeNodeDelegate.qml" : ""
                    onLoaded:
                    {
                        console.log("[groupEditor] Item " + label + " has children: " + hasChildren)
                        item.textLabel = label
                        item.isSelected = Qt.binding(function() { return isSelected })
                        item.dragItem = gfhcDragItem

                        if (item.hasOwnProperty('cRef'))
                            item.cRef = classRef

                        if (hasChildren)
                        {
                            item.itemIcon = "qrc:/group.svg"
                            item.itemType = App.GroupDragItem
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
                                    gfhcDragItem.modifiers = mouseMods
                                break;
                                case App.Clicked:
                                    if (qItem == item)
                                    {
                                        model.isSelected = (mouseMods & Qt.ControlModifier) ? 2 : 1
                                        if (model.hasChildren)
                                            model.isExpanded = item.isExpanded
                                    }
                                    gfhcDragItem.itemsList = []
                                break;
                                case App.DragStarted:
                                    if (qItem == item && !model.isSelected)
                                    {
                                        model.isSelected = 1
                                        // invalidate the modifiers to force a single selection
                                        mouseMods = -1
                                        gfhcDragItem.itemsList = []
                                    }

                                    gfhcDragItem.itemLabel = qItem.textLabel
                                    if (qItem.hasOwnProperty("itemIcon"))
                                        gfhcDragItem.itemIcon = qItem.itemIcon
                                    else
                                        gfhcDragItem.itemIcon = ""
                                    gfhcDragItem.itemsList.push(qItem)
                                    groupListView.dragActive = true
                                break;
                                case App.DragFinished:
                                    gfhcDragItem.Drag.drop()
                                    gfhcDragItem.parent = groupListView
                                    gfhcDragItem.x = 0
                                    gfhcDragItem.y = 0
                                    groupListView.dragActive = false
                                break;
                            }
                        }
                    }
                } // Loader
              } // Component
            ScrollBar { id: gEditScrollBar; flickable: groupListView }

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
