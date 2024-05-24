/*
  Q Light Controller Plus
  TreeNodeDelegate.qml

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

import QtQuick 2.2

import org.qlcplus.classes 1.0
import "."

Column
{
    id: nodeContainer
    width: 350
    //height: nodeLabel.height + nodeChildrenView.height

    property var cRef
    property string textLabel
    property string itemIcon: "qrc:/folder.svg"
    property int itemType: App.GenericDragItem
    property int itemID: cRef ? cRef.id : -1

    property bool isExpanded: false
    property bool isSelected: false
    property bool isCheckable: false
    property bool isChecked: false

    property string nodePath
    property var nodeChildren
    property string childrenDelegate: "qrc:/FunctionDelegate.qml"
    property string subTreeDelegate: "qrc:/TreeNodeDelegate.qml"
    property Item dragItem
    property string dropKeys: ""

    signal toggled(bool expanded, int newHeight)
    signal mouseEvent(int type, int iID, int iType, var qItem, int mouseMods)
    signal pathChanged(string oldPath, string newPath)
    signal itemsDropped(string path)

    function getItemAtPos(x, y)
    {
        var child = nodeChildrenView.itemAt(x, y)
        if (child.item.hasOwnProperty("nodePath"))
            return child.item.getItemAtPos(x, y - child.item.y)

        return child.item
    }

    Rectangle
    {
        id: nodeBgRect
        color: nodeIconImg.visible ? "transparent" : UISettings.sectionHeader
        width: nodeContainer.width
        height: UISettings.listItemHeight

        // selection rectangle
        Rectangle
        {
            anchors.fill: parent
            radius: 3
            color: UISettings.highlight
            visible: isSelected || tnDropArea.containsDrag
        }

        Row
        {
            CustomCheckBox
            {
                id: nodeCheckBox
                visible: isCheckable
                implicitWidth: UISettings.listItemHeight
                implicitHeight: implicitWidth
                checked: isChecked
                onCheckedChanged: nodeContainer.mouseEvent(App.Checked, -1, checked, nodeContainer, 0)
            }

            Image
            {
                id: nodeIconImg
                visible: itemIcon == "" ? false : true
                width: nodeBgRect.height
                height: width
                source: itemIcon
                sourceSize: Qt.size(width, height)
            }

            CustomTextInput
            {
                id: nodeLabel
                width: nodeBgRect.width - x - 1
                text: cRef ? cRef.name : textLabel
                originalText: text

                onTextConfirmed: nodeContainer.pathChanged(nodePath, text)
            }
        } // Row

        MouseArea
        {
            x: nodeCheckBox.visible ? nodeCheckBox.width : 0
            width: parent.width
            height: parent.height

            property bool dragActive: drag.active

            onDragActiveChanged:
            {
                console.log("Drag changed on node: " + textLabel)
                nodeContainer.mouseEvent(dragActive ? App.DragStarted : App.DragFinished,
                                         cRef ? cRef.id : -1, nodeContainer.itemType, nodeContainer, 0)
            }

            drag.target: dragItem

            onPressed: nodeContainer.mouseEvent(App.Pressed, cRef ? cRef.id : -1, nodeContainer.itemType,
                                                nodeContainer, mouse.modifiers)
            onClicked:
            {
                nodeLabel.forceActiveFocus()
                nodeContainer.mouseEvent(App.Clicked, cRef ? cRef.id : -1, nodeContainer.itemType,
                                         nodeContainer, mouse.modifiers)
            }
            onDoubleClicked: isExpanded = !isExpanded
        }

        DropArea
        {
            id: tnDropArea
            anchors.fill: parent
            keys: [ nodeContainer.dropKeys ]

            onDropped:
            {
                console.log("Item dropped here. x: " + drop.x + " y: " + drop.y + ", items: " + drop.source.itemsList.length)
                nodeContainer.itemsDropped(nodePath)
            }
        }
    }

    Repeater
    {
        id: nodeChildrenView
        visible: isExpanded
        width: nodeContainer.width - 20
        model: visible ? nodeChildren : null
        delegate:
            Component
            {
                Loader
                {
                    width: nodeChildrenView.width
                    x: 20
                    //height: 35
                    source: hasChildren ? subTreeDelegate : childrenDelegate
                    onLoaded:
                    {
                        item.textLabel = Qt.binding(function() { return label })
                        item.isSelected = Qt.binding(function() { return model.isSelected })
                        item.isCheckable = model.isCheckable
                        item.isChecked = Qt.binding(function() { return model.isChecked })
                        item.dragItem = dragItem
                        if (model.hasOwnProperty("type") && item.hasOwnProperty("itemType"))
                            item.itemType = type

                        if (item.hasOwnProperty('itemIcon'))
                            item.itemIcon = nodeContainer.itemIcon

                        if (model.classRef !== undefined && item.hasOwnProperty('cRef'))
                            item.cRef = classRef

                        if (item.hasOwnProperty('itemID') && model.classRef !== undefined)
                            item.itemID = id

                        if (item.hasOwnProperty('inGroup'))
                            item.inGroup = inGroup

                        if (item.hasOwnProperty('subID'))
                            item.subID = subid

                        //console.log("Item flags: " + model.flags);
                        if (model.flags !== undefined && item.hasOwnProperty("itemFlags"))
                        {
                            item.showFlags = true
                            item.itemFlags = Qt.binding(function() { return model.flags })
                        }

                        if (hasChildren)
                        {
                            item.nodePath = Qt.binding(function() { return nodePath + '`' + path })
                            item.isExpanded = isExpanded
                            item.nodeChildren = childrenModel
                            if (item.hasOwnProperty('dropKeys'))
                                item.dropKeys = nodeContainer.dropKeys
                            if (item.hasOwnProperty('childrenDelegate'))
                                item.childrenDelegate = childrenDelegate

                            //console.log("Item path: " + item.nodePath + ", label: " + label)
                        }
                    }
                    Connections
                    {
                        target: item
                        function onMouseEvent(type, iID, iType, qItem, mouseMods)
                        {
                            console.log("Got generic tree node mouse event")
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
                                case App.DragStarted:
                                    if (qItem === item && !model.isSelected)
                                    {
                                        model.isSelected = 1
                                        // invalidate the modifiers to force a single selection
                                        mouseMods = -1
                                    }
                                break;
                            }

                            // forward the event to the parent node
                            nodeContainer.mouseEvent(type, iID, iType, qItem, mouseMods)
                        }
                    }
                    Connections
                    {
                        ignoreUnknownSignals: true
                        target: item
                        function onPathChanged(oldPath, newPath)
                        {
                            nodeContainer.pathChanged(oldPath, newPath)
                        }
                    }
                    Connections
                    {
                        ignoreUnknownSignals: true
                        target: item
                        function onItemsDropped(path)
                        {
                            nodeContainer.itemsDropped(path)
                        }
                    }
                }
        }
    }
}
