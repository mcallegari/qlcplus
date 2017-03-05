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
            model: fixtureManager.groupsTreeModel
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

                        if (hasChildren)
                        {
                            item.nodePath = path
                            item.nodeIcon = "qrc:/group.svg"
                            item.isExpanded = isExpanded
                            item.subTreeDelegate = "qrc:/FixtureNodeDelegate.qml"
                            item.nodeChildren = childrenModel
                        }

                        if (item.hasOwnProperty('cRef'))
                            item.cRef = classRef
                    }
                    Connections
                    {
                        target: item

                        onMouseEvent:
                        {
                            if (type === App.Clicked && qItem == item)
                            {
                                model.isSelected = (mouseMods & Qt.ControlModifier) ? 2 : 1
                                if (model.hasChildren)
                                    model.isExpanded = item.isExpanded
                            }
                        }
                    }
                    /*
                    Connections
                    {
                          target: item
                          onDoubleClicked: { }
                    }
                    */
                } // Loader
              } // Component
        } // ListView
    } // ColumnLayout
}
