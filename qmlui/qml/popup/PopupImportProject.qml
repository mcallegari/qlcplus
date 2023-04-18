/*
  Q Light Controller Plus
  PopupImportProject.qml

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

CustomPopupDialog
{
    id: popupRoot
    width: mainView.width / 2
    title: qsTr("Import from project")
    standardButtons: Dialog.Cancel | Dialog.Apply

    onClicked:
    {
        if (role === Dialog.Cancel)
        {
            qlcplus.cancelImport()
        }
        else if (role === Dialog.Apply)
        {
            qlcplus.importFromWorkspace()
            close()
        }
    }

    contentItem:
        GridLayout
        {
            columnSpacing: UISettings.iconSizeMedium

            columns: 2
            rows: 2

            // row 1
            RobotoText
            {
                label: qsTr("Fixtures")
            }
            RobotoText
            {
                label: qsTr("Functions")
            }

            // row 2
            Rectangle
            {
                width: mainView.width / 3
                height: UISettings.iconSizeMedium
                z: 5
                color: UISettings.bgMedium
                radius: 5
                border.width: 2
                border.color: UISettings.borderColorDark

                Text
                {
                    id: fxSearchIcon
                    x: 6
                    width: height
                    height: parent.height - 6
                    anchors.verticalCenter: parent.verticalCenter
                    color: "gray"
                    font.family: "FontAwesome"
                    font.pixelSize: height - 6
                    text: FontAwesome.fa_search
                }

                TextInput
                {
                    x: fxSearchIcon.width + 14
                    y: 3
                    height: parent.height - 6
                    width: parent.width - x
                    color: UISettings.fgMain
                    text: importManager.fixtureSearchFilter
                    font.family: "Roboto Condensed"
                    font.pixelSize: height - 6
                    selectionColor: UISettings.highlightPressed
                    selectByMouse: true

                    onTextChanged: importManager.fixtureSearchFilter = text
                }
            }

            Rectangle
            {
                width: mainView.width / 3
                height: UISettings.iconSizeMedium
                z: 5
                color: UISettings.bgMedium
                radius: 5
                border.width: 2
                border.color: UISettings.borderColorDark

                Text
                {
                    id: funcSearchIcon
                    x: 6
                    width: height
                    height: parent.height - 6
                    anchors.verticalCenter: parent.verticalCenter
                    color: "gray"
                    font.family: "FontAwesome"
                    font.pixelSize: height - 6
                    text: FontAwesome.fa_search
                }

                TextInput
                {
                    x: funcSearchIcon.width + 14
                    y: 3
                    height: parent.height - 6
                    width: parent.width - x
                    color: UISettings.fgMain
                    text: importManager.functionSearchFilter
                    font.family: "Roboto Condensed"
                    font.pixelSize: height - 6
                    selectionColor: UISettings.highlightPressed
                    selectByMouse: true

                    onTextChanged: importManager.functionSearchFilter = text
                }
            }

            // row 3
            ListView
            {
                id: groupListView
                width: mainView.width / 3
                height: mainView.height * 0.6
                clip: true
                z: 4
                boundsBehavior: Flickable.StopAtBounds

                property bool dragActive: false

                model: popupRoot.visible ? importManager.groupsTreeModel : null
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
                            item.width = Qt.binding(function() { return groupListView.width - (gEditScrollBar.visible ? gEditScrollBar.width : 0) })
                            item.cRef = classRef
                            item.textLabel = label
                            item.isSelected = Qt.binding(function() { return isSelected })
                            item.isCheckable = isCheckable
                            item.isChecked = Qt.binding(function() { return isChecked })

                            if (hasChildren)
                            {
                                item.itemIcon = "qrc:/group.svg"
                                if (type)
                                {
                                    item.itemType = type
                                    if (type === App.UniverseDragItem)
                                        isExpanded = true
                                }
                                item.nodePath = path
                                item.isExpanded = isExpanded
                                item.subTreeDelegate = "qrc:/FixtureNodeDelegate.qml"
                                item.childrenDelegate = "qrc:/FixtureDelegate.qml"
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
                                    case App.Clicked:
                                        if (qItem === item)
                                        {
                                            model.isSelected = (mouseMods & Qt.ControlModifier) ? 2 : 1
                                            if (model.hasChildren)
                                                model.isExpanded = item.isExpanded
                                        }
                                    break;
                                    case App.Checked:
                                        console.log("Item checked " + iType)
                                        if (qItem === item)
                                            item.isChecked = iType
                                    break;
                                }
                            }
                        }
                    } // Loader
                  } // Component
                ScrollBar.vertical: CustomScrollBar { id: gEditScrollBar }
            } // ListView

            ListView
            {
                id: functionsListView
                width: mainView.width / 3
                height: mainView.height * 0.6
                z: 4
                clip: true
                boundsBehavior: Flickable.StopAtBounds


                model: popupRoot.visible ? importManager.functionsTreeModel : null
                delegate:
                    Component
                    {
                        Loader
                        {
                            width: functionsListView.width
                            source: hasChildren ? "qrc:/TreeNodeDelegate.qml" : "qrc:/FunctionDelegate.qml"

                            onLoaded:
                            {
                                item.width = Qt.binding(function() { return functionsListView.width - (fMgrScrollBar.visible ? fMgrScrollBar.width : 0) })
                                item.textLabel = label
                                item.isSelected = Qt.binding(function() { return isSelected })
                                item.isCheckable = isCheckable
                                item.isChecked = Qt.binding(function() { return isChecked })

                                if (hasChildren)
                                {
                                    console.log("Item path: " + path + ",label: " + label)
                                    item.itemType = App.FolderDragItem
                                    item.nodePath = path
                                    item.isExpanded = isExpanded
                                    item.nodeChildren = childrenModel
                                    item.dropKeys = "function"
                                }
                                else
                                {
                                    item.cRef = classRef
                                    item.itemType = App.FunctionDragItem
                                    //item.functionType = funcType
                                }
                            }
                            Connections
                            {
                                target: item

                                function onMouseEvent(type, iID, iType, qItem, mouseMods)
                                {
                                    switch (type)
                                    {
                                        case App.Clicked:
                                            if (qItem === item)
                                            {
                                                model.isSelected = (mouseMods & Qt.ControlModifier) ? 2 : 1
                                                if (model.hasChildren)
                                                    model.isExpanded = item.isExpanded
                                            }
                                        break;
                                        case App.Checked:
                                            if (qItem === item)
                                                model.isChecked = iType
                                        break;
                                    }
                                }
                            }
                        } // Loader
                    } // Component
                ScrollBar.vertical: CustomScrollBar { id: fMgrScrollBar }
            } // ListView
        }
}
