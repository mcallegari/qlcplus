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
import QtQuick.Layouts 1.0

import org.qlcplus.classes 1.0
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
    property int itemID
    property int subID
    property int linkedIndex: 0
    property bool inGroup: false
    property bool isExpanded: false
    property bool isSelected: false
    property bool isCheckable: false
    property bool isChecked: false
    property bool showFlags: false
    property int itemFlags: 0
    property string nodePath
    property var nodeChildren
    property Item dragItem

    signal toggled(bool expanded, int newHeight)
    signal mouseEvent(int type, int iID, int iType, var qItem, int mouseMods)
    signal pathChanged(string oldPath, string newPath)

    onCRefChanged: itemIcon = cRef ? cRef.iconResource(true) : ""
    onItemIDChanged: linkedIndex = fixtureManager.fixtureLinkedIndex(itemID)

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
        z: 1

        // icon background for contrast
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

        RowLayout
        {
            width: parent.width

            CustomCheckBox
            {
                visible: isCheckable
                implicitWidth: UISettings.listItemHeight
                implicitHeight: implicitWidth
                checked: isChecked
                onCheckedChanged: nodeContainer.mouseEvent(App.Checked, -1, -1, nodeContainer, 0)
            }

            Image
            {
                id: nodeIconImg
                visible: itemIcon == "" ? false : true
                x: 1
                y: 1
                width: nodeBgRect.height - 2
                height: width
                source: itemIcon
                sourceSize: Qt.size(width, height)
            }

            Text
            {
                visible: linkedIndex
                color: UISettings.fgMain
                font.family: "FontAwesome"
                font.pixelSize: UISettings.listItemHeight - 6
                text: FontAwesome.fa_link
            }

            CustomTextInput
            {
                id: nodeLabel
                Layout.fillWidth: true
                text: textLabel
                originalText: text

                onTextConfirmed:
                {
                    if (fixtureManager.renameFixture(itemID, text) === false)
                    {
                        fmGenericPopup.message = qsTr("An item with the same name already exists.\nPlease provide a different name.")
                        fmGenericPopup.open()
                        nodeLabel.text = textLabel
                    }
                    else
                    {
                        nodeContainer.pathChanged(nodePath, text)
                    }
                }
            }

            // DMX address range
            RobotoText
            {
                visible: !showFlags
                implicitWidth: width
                implicitHeight: UISettings.listItemHeight
                label: cRef ? "" + (cRef.address + 1) + "-" + (cRef.address + cRef.channels) : ""
            }

            // divider
            Rectangle
            {
                visible: showFlags
                width: 1
                height: parent.height
            }

            // fixture modes
            Rectangle
            {
                id: fxModes
                visible: showFlags
                width: UISettings.chPropsModesWidth
                height: parent.height
                color: "transparent"
                z: 1

                CustomComboBox
                {
                    visible: showFlags
                    implicitWidth: parent.width
                    height: UISettings.listItemHeight
                    textRole: ""
                    model: showFlags ? fixtureManager.fixtureModes(itemID) : null
                    currentIndex: showFlags ? fixtureManager.fixtureModeIndex(itemID) : -1

                    onActivated:
                    {
                        if (!visible)
                            return

                        if (fixtureManager.setFixtureModeIndex(itemID, index) === false)
                        {
                            // show error popup on failure
                            fmGenericPopup.message = qsTr("Mode <" + currentText + "> overlaps with another fixture!")
                            fmGenericPopup.open()
                            currentIndex = fixtureManager.fixtureModeIndex(itemID)
                        }
                    }
                }
            }

            // divider
            Rectangle
            {
                visible: showFlags
                width: 1
                height: parent.height
            }

            // fixture flags
            Rectangle
            {
                id: fxFlags
                visible: showFlags
                width: UISettings.chPropsFlagsWidth
                height: parent.height
                color: "transparent"
                z: 1

                Row
                {
                    height: parent.height
                    spacing: 2

                    IconButton
                    {
                        height: parent.height - 2
                        width: height
                        border.width: 0
                        faSource: checked ? FontAwesome.fa_eye : FontAwesome.fa_eye_slash
                        faColor: checked ? "#00FF00" : UISettings.fgMedium
                        bgColor: "transparent"
                        checkedColor: "transparent"
                        checkable: true
                        checked: itemFlags & MonitorProperties.HiddenFlag ? false : true
                        tooltip: qsTr("Show/Hide this fixture")
                        onToggled:
                        {
                            if (itemFlags & MonitorProperties.HiddenFlag)
                                fixtureManager.setItemRoleData(itemID, -1, "flags", (itemFlags & ~MonitorProperties.HiddenFlag))
                            else
                                fixtureManager.setItemRoleData(itemID, -1, "flags", itemFlags | MonitorProperties.HiddenFlag)
                        }
                    }

                    IconButton
                    {
                        height: parent.height - 2
                        width: height
                        border.width: 0
                        faSource: FontAwesome.fa_arrows_h
                        faColor: checked ? "#00FF00" : UISettings.fgMedium
                        bgColor: "transparent"
                        checkedColor: "transparent"
                        checkable: true
                        checked: itemFlags & MonitorProperties.InvertedPanFlag ? true : false
                        tooltip: qsTr("Invert Pan")
                        onToggled:
                        {
                            if (itemFlags & MonitorProperties.InvertedPanFlag)
                                fixtureManager.setItemRoleData(itemID, -1, "flags", (itemFlags & ~MonitorProperties.InvertedPanFlag))
                            else
                                fixtureManager.setItemRoleData(itemID, -1, "flags", itemFlags | MonitorProperties.InvertedPanFlag)
                        }
                    }


                    IconButton
                    {
                        height: parent.height - 2
                        width: height
                        border.width: 0
                        faSource: FontAwesome.fa_arrows_v
                        faColor: checked ? "#00FF00" : UISettings.fgMedium
                        bgColor: "transparent"
                        checkedColor: "transparent"
                        checkable: true
                        checked: itemFlags & MonitorProperties.InvertedTiltFlag ? true : false
                        tooltip: qsTr("Invert Tilt")
                        onToggled:
                        {
                            if (itemFlags & MonitorProperties.InvertedTiltFlag)
                                fixtureManager.setItemRoleData(itemID, -1, "flags", (itemFlags & ~MonitorProperties.InvertedTiltFlag))
                            else
                                fixtureManager.setItemRoleData(itemID, -1, "flags", itemFlags | MonitorProperties.InvertedTiltFlag)
                        }
                    }
                }
            }

            Rectangle { visible: showFlags; width: 1; height: parent.height } // divider
            Rectangle { visible: showFlags; width: UISettings.chPropsCanFadeWidth; height: parent.height; color: "transparent" } // stub
            Rectangle { visible: showFlags; width: 1; height: parent.height } // divider
            Rectangle { visible: showFlags; width: UISettings.chPropsPrecedenceWidth; height: parent.height; color: "transparent" } // stub
            Rectangle { visible: showFlags; width: 1; height: parent.height } // divider
            Rectangle { visible: showFlags; width: UISettings.chPropsModifierWidth; height: parent.height; color: "transparent" } // stub
        } // RowLayout

        MouseArea
        {
            width: showFlags ? fxModes.x : parent.width
            height: parent.height

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
                nodeLabel.forceActiveFocus()
                nodeContainer.mouseEvent(App.Clicked, itemID, -1, nodeContainer, mouse.modifiers)
            }
            onDoubleClicked:
            {
                nodeContainer.mouseEvent(App.DoubleClicked, itemID, -1, nodeContainer, mouse.modifiers)
                isExpanded = !isExpanded
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
                    //width: nodeChildrenView.width
                    x: 20
                    //height: 35
                    source: type == App.ChannelDragItem ? "qrc:/FixtureChannelDelegate.qml" : "qrc:/FixtureHeadDelegate.qml"
                    onLoaded:
                    {
                        item.width = Qt.binding(function() { return nodeChildrenView.width })
                        item.textLabel = label
                        item.isSelected = Qt.binding(function() { return model.isSelected })
                        item.isCheckable = model.isCheckable
                        item.isChecked = Qt.binding(function() { return model.isChecked })
                        item.dragItem = dragItem
                        item.itemType = type

                        if (item.hasOwnProperty('cRef'))
                            item.cRef = classRef

                        item.itemID = id

                        if (type == App.ChannelDragItem)
                        {
                            console.log("Channel node, fixture " + cRef + " index: " + chIdx + " label: " + label)
                            item.isCheckable = isCheckable
                            item.isChecked = Qt.binding(function() { return isChecked })
                            item.chIndex = chIdx
                            item.itemIcon = cRef ? fixtureManager.channelIcon(cRef.id, chIdx) : ""

                            if (model.flags !== undefined && item.hasOwnProperty("itemFlags"))
                            {
                                item.showFlags = true
                                item.itemFlags = Qt.binding(function() { return model.flags })
                                item.canFade = Qt.binding(function() { return model.canFade })
                                item.precedence = Qt.binding(function() { return model.precedence })
                                item.modifier = Qt.binding(function() { return model.modifier })
                            }
                        }
                        else
                        {
                            console.log("Head node, fixture " + cRef + " index: " + chIdx + " label: " + label)
                            item.headIndex = chIdx
                        }
                    }
                    Connections
                    {
                        target: item
                        function onMouseEvent(type, iID, iType, qItem, mouseMods)
                        {
                            console.log("Got fixture tree node child mouse event")
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
                                    {
                                        console.log("Channel " + index + " got checked")
                                        model.isChecked = iType
                                    }
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
                }
        }
    }
}
