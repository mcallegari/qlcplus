/*
  Q Light Controller Plus
  FixtureNodeDelegate.qml

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

import com.qlcplus.classes 1.0
import "."

Column
{
    id: nodeContainer
    width: 350
    //height: nodeLabel.height + isExpanded ? nodeChildrenView.height : 0

    property Fixture cRef
    property string textLabel
    property string itemIcon
    property int itemType: App.FixtureDragItem
    property bool isExpanded: false
    property bool isSelected: false
    property string nodePath    
    property var nodeChildren
    property Item dragItem

    signal toggled(bool expanded, int newHeight)
    signal mouseEvent(int type, int iID, int iType, var qItem, int mouseMods)
    signal pathChanged(string oldPath, string newPath)

    onCRefChanged: itemIcon = cRef ? cRef.iconResource(true) : ""

    function getItemAtPos(x, y)
    {
        var child = nodeChildrenView.itemAt(x, y)
        if (child.item.hasOwnProperty("nodePath"))
            return child.item.getItemAtPos(x, y - child.item.y)

        return child.item
    }

    Rectangle
    {
        color: nodeIconImg.visible ? "transparent" : UISettings.sectionHeader
        width: nodeContainer.width
        height: UISettings.listItemHeight

        Rectangle
        {
            visible: itemIcon == "" ? false : true
            y: 1
            width: visible ? parent.height - 2 : 0
            height: width
            color: UISettings.bgLight
            radius: height / 4
            border.width: 1
            border.color: UISettings.fgMedium
        }

        // selection rectangle
        Rectangle
        {
            anchors.fill: parent
            radius: 3
            color: UISettings.highlight
            visible: isSelected
        }

        Image
        {
            id: nodeIconImg
            visible: itemIcon == "" ? false : true
            x: 1
            y: 1
            width: visible ? parent.height - 2 : 0
            height: width
            source: itemIcon
            sourceSize: Qt.size(width, height)
        }

        TextInput
        {
            property string originalText

            id: nodeLabel
            x: nodeIconImg.width + 2
            z: 0
            width: parent.width - nodeIconImg.width - 1
            height: UISettings.listItemHeight
            readOnly: true
            text: textLabel
            verticalAlignment: TextInput.AlignVCenter
            color: UISettings.fgMain
            font.family: UISettings.robotoFontName
            font.pixelSize: UISettings.textSizeDefault
            echoMode: TextInput.Normal
            selectByMouse: true
            selectionColor: "#4DB8FF"
            selectedTextColor: "#111"

            function disableEditing()
            {
                z = 0
                select(0, 0)
                readOnly = true
                cursorVisible = false
            }

            onEditingFinished:
            {
                disableEditing()
                nodeContainer.pathChanged(nodePath, text)
            }
            Keys.onEscapePressed:
            {
                disableEditing()
                nodeLabel.text = originalText
            }
        }

        RobotoText
        {
            anchors.right: parent.right
            height: UISettings.listItemHeight
            label: cRef ? "" + (cRef.address + 1) + "-" + (cRef.address + cRef.channels + 1) : ""

        }

        Timer
        {
            id: clickTimer
            interval: 200
            repeat: false
            running: false

            property int modifiers: 0

            onTriggered:
            {
                isExpanded = !isExpanded
                nodeContainer.mouseEvent(App.Clicked, cRef ? cRef.id : -1, -1, nodeContainer, modifiers)
                modifiers = 0
            }
        }

        MouseArea
        {
            anchors.fill: parent

            property bool dragActive: drag.active

            onDragActiveChanged:
            {
                console.log("Drag changed on node: " + textLabel)
                nodeContainer.mouseEvent(dragActive ? App.DragStarted : App.DragFinished, cRef ? cRef.id : -1, -1, nodeContainer, 0)
            }

            drag.target: dragItem

            onPressed: nodeContainer.mouseEvent(App.Pressed, cRef ? cRef.id : -1, -1, nodeContainer, mouse.modifiers)
            onClicked:
            {
                clickTimer.modifiers = mouse.modifiers
                clickTimer.start()
            }
            onDoubleClicked:
            {
                clickTimer.stop()
                clickTimer.modifiers = 0
                nodeLabel.originalText = textLabel
                nodeLabel.z = 5
                nodeLabel.readOnly = false
                nodeLabel.forceActiveFocus()
                nodeLabel.cursorPosition = nodeLabel.text.length
                nodeLabel.cursorVisible = true
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
                    source: type == App.ChannelDragItem ? "qrc:/FixtureChannelDelegate.qml" : "qrc:/FixtureHeadDelegate.qml"
                    onLoaded:
                    {
                        item.textLabel = label
                        item.isSelected = Qt.binding(function() { return isSelected })
                        item.dragItem = dragItem
                        item.itemType = type

                        if (type == App.ChannelDragItem)
                        {
                            item.isCheckable = isCheckable
                            item.isChecked = Qt.binding(function() { return isChecked })
                            item.chIndex = index
                            item.itemIcon = cRef ? fixtureManager.channelIcon(cRef.id, index) : ""
                        }
                        else
                        {
                            item.fixtureID = cRef ? cRef.id : -1
                            item.headIndex = head
                        }

                        if (item.hasOwnProperty('cRef'))
                            item.cRef = classRef
                    }
                    Connections
                    {
                        target: item
                        onMouseEvent:
                        {
                            console.log("Got tree node child mouse event")
                            switch (type)
                            {
                                case App.Clicked:
                                    if (qItem == item)
                                    {
                                        model.isSelected = (mouseMods & Qt.ControlModifier) ? 2 : 1
                                        if (model.hasChildren)
                                            model.isExpanded = item.isExpanded
                                    }
                                break;
                                case App.Checked:
                                    if (qItem == item)
                                    {
                                        console.log("Channel " + index + " got checked")
                                        model.isChecked = iType
                                    }

                                break;
                                case App.DragStarted:
                                    if (qItem == item && !model.isSelected)
                                    {
                                        model.isSelected = 1
                                        // invalidate the modifiers to force a single selection
                                        mouseMods = -1
                                    }
                                break;
                            }

                            nodeContainer.mouseEvent(type, iID, iType, qItem, mouseMods)
                        }
                    }
                    Connections
                    {
                        ignoreUnknownSignals: true
                        target: item
                        onPathChanged: nodeContainer.pathChanged(oldPath, newPath)
                    }
                }
        }
    }
}
