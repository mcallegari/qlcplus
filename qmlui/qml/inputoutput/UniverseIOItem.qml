/*
  Q Light Controller Plus
  UniverseIOItem.qml

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

import org.qlcplus.classes 1.0
import "."

Rectangle
{
    id: uniItem
    width: parent.width
    height: itemHeight ? itemHeight : UISettings.bigItemHeight
    color: isSelected ? UISettings.highlightPressed : "transparent"
    border.width: 2
    border.color: isSelected ? UISettings.selection : "transparent"
    z: isSelected ? 5 : 1

    property int itemHeight: Math.max(inputPatchesNumber, outputPatchesNumber) * UISettings.bigItemHeight
    property Universe universe
    property bool isSelected: universe && ioManager.selectedIndex === universe.id ? true : false
    property int outputPatchesNumber: universe ? universe.outputPatchesCount : 0
    property int inputPatchesNumber: iPatch == null ? 0 : 1
    property int wireBoxWidth: (uniItem.width - uniBox.width) / 8 // one quarter of a uniItem side
    property InputPatch iPatch: universe ? universe.inputPatch : null

    signal selected(int index)
    signal patchDragging(bool status)

    onIsSelectedChanged:
    {
        if (isSelected == false)
            uniNameEdit.setEditingStatus(false)
    }

    // area containing the input patches
    Column
    {
        id: inputBox
        x: 14
        z: 1
        anchors.verticalCenter: uniItem.verticalCenter

        Repeater
        {
            model: inputPatchesNumber
            delegate:
                Item
                {
                    id: ipRoot
                    width: inWireBox.width * 3
                    height: ipItem.height

                    MouseArea
                    {
                        id: ipMouseArea
                        anchors.fill: parent
                        propagateComposedEvents: true

                        drag.target: ipItem
                        drag.threshold: 30

                        onClicked: mouse.accepted = false
                        onPositionChanged: if (ipMouseArea.drag.active) uniItem.patchDragging(true)
                        onReleased:
                        {
                            uniItem.patchDragging(false)
                            if (ipItem.Drag.target !== null)
                            {
                                ipItem.Drag.active = false
                                ioManager.removeInputPatch(universe.id)
                            }
                            else
                            {
                                // return the item to its original position
                                ipItem.x = 0
                                ipItem.y = 0
                            }
                        }

                        InputPatchItem
                        {
                            id: ipItem
                            width: ipRoot.width

                            universeID: universe ? universe.id : -1
                            patch: universe ? universe.inputPatch : null

                            Drag.active: ipMouseArea.drag.active
                            Drag.source: ipMouseArea
                            Drag.hotSpot.x: width / 2
                            Drag.hotSpot.y: height / 2
                            Drag.keys: [ "removePatch" ]

                            onRemoveProfile: ioManager.setInputProfile(universe.id, "")

                            DropArea
                            {
                                id: profDropTarget
                                z: 2
                                anchors.fill: parent
                                keys: [ universe ? "profile-" + universe.id : "" ]

                                Rectangle
                                {
                                    id: profDropRect
                                    anchors.fill: parent
                                    color: "transparent"
                                    states: [
                                        State
                                        {
                                            when: profDropTarget.containsDrag
                                            PropertyChanges
                                            {
                                                target: profDropRect
                                                color: "#33FFEC55"
                                            }
                                        }
                                    ]
                                } // Rectangle
                            } // DropArea
                        } // InputPatchItem
                    } // MouseArea
                } // Item
        } // Repeater
    } // Column

    // Input patches wires box
    PatchWireBox
    {
        id: inWireBox
        x: uniBox.x - width + 6
        width: wireBoxWidth
        height: uniItem.height
        z: 10

        patchesNumber: inputPatchesNumber
        showFeedback: universe ? universe.hasFeedback : false
    }

    // Input patch drop area
    DropArea
    {
        id: inputDropTarget
        x: 2
        y: 2
        width: ((uniItem.width - uniBox.width) / 2) - 6
        height: uniItem.height - 4

        // this key must match the one in PluginList, to avoid dropping
        // an input plugin on output and vice-versa
        keys: [ universe ? "input-" + universe.id : "" ]

        Rectangle
        {
            id: inDropRect
            anchors.fill: parent
            color: inputDropTarget.containsDrag ? UISettings.highlight : "transparent"
        }
    }

    // representation of the central Universe block
    Rectangle
    {
        id: uniBox
        anchors.centerIn: parent
        z: 5
        width: UISettings.bigItemHeight * 1.2
        height: UISettings.bigItemHeight * 0.8
        radius: 5
        //color: "#1C2255"
        gradient:
            Gradient
            {
                id: bgGradient
                GradientStop { position: 0 ; color: UISettings.highlightPressed }
                GradientStop { position: 1 ; color: UISettings.highlight }
            }
        border.width: 2
        border.color: UISettings.borderColorDark

        CustomTextInput
        {
            id: uniNameEdit
            anchors.centerIn: parent
            width: parent.width
            height: parent.height
            text: universe ? universe.name : ""
            horizontalAlignment: Text.AlignHCenter
            wrapMode: TextInput.Wrap
            allowDoubleClick: true

            onTextConfirmed: if(universe) universe.name = text
            onClicked:
            {
                ioManager.selectedIndex = universe.id
                uniItem.selected(universe.id)
            }
        }

        Canvas
        {
            id: passthrough
            anchors.fill: parent
            visible: ptCheckButton.checked
            contextType: "2d"

            onPaint:
            {
                var vCenter = (height / 2) - 4
                var wireMargin = width / 20
                context.strokeStyle = "yellow"
                context.lineWidth = 3
                context.beginPath()
                context.clearRect(0, 0, width, height)

                context.moveTo(0, vCenter)
                context.lineTo(wireMargin, vCenter)
                context.lineTo(wireMargin, height - wireMargin)
                context.lineTo(width - wireMargin, height - wireMargin)
                context.lineTo(width - wireMargin, vCenter)
                context.lineTo(width, vCenter)
                context.stroke()
            }
        }

        IconButton
        {
            id: ptCheckButton
            anchors.bottom: parent.bottom
            anchors.right: parent.right
            checkedColor: UISettings.selection
            width: UISettings.iconSizeMedium * 0.8
            height: UISettings.iconSizeMedium * 0.8
            faSource: FontAwesome.fa_long_arrow_right
            checkable: true
            tooltip: qsTr("Enable/Disable passthrough")
            checked: universe ? universe.passthrough : false
            onToggled: if (universe) universe.passthrough = checked
        }

        IconButton
        {
            id: fbButton
            z: 2
            visible: inputPatchesNumber
            anchors.bottom: parent.bottom
            width: UISettings.iconSizeMedium * 0.8
            height: UISettings.iconSizeMedium * 0.8
            checkedColor: "green"
            imgSource: ""
            checkable: true
            checked: universe ? universe.hasFeedback : false
            tooltip: qsTr("Enable/Disable feedback")
            onToggled:
            {
                if (universe)
                    ioManager.setFeedbackPatch(universe.id, checked)
            }

            RobotoText
            {
                anchors.centerIn: parent
                label: "F"
                fontSize: UISettings.textSizeDefault * 1.1
                fontBold: true
            }
        }
    }

    // Output patches wires box
    PatchWireBox
    {
        id: outWireBox
        x: uniBox.x + uniBox.width - 6
        width: wireBoxWidth
        height: uniItem.height
        z: 10

        patchesNumber: outputPatchesNumber
    }

    // area containing the output patches
    Column
    {
        id: outputBox
        x: outWireBox.x + outWireBox.width - 8
        y: ((outputPatchesNumber * UISettings.bigItemHeight) - outputBox.height) / 2
        z: 1
        spacing: 5

        Repeater
        {
            id: outRpt
            model: outputPatchesNumber
            delegate:
                Item
                {
                    id: opRoot
                    width: outWireBox.width * 3
                    height: opItem.height

                    MouseArea
                    {
                        id: opMouseArea
                        anchors.fill: parent
                        propagateComposedEvents: true

                        drag.target: opItem
                        drag.threshold: 30

                        onClicked: mouse.accepted = false
                        onPositionChanged: if (opMouseArea.drag.active) uniItem.patchDragging(true)
                        onReleased:
                        {
                            uniItem.patchDragging(false)
                            if (opItem.Drag.target !== null)
                            {
                                opItem.Drag.active = false
                                ioManager.removeOutputPatch(universe.id, model.index)
                            }
                            else
                            {
                                // return the item to its original position
                                opItem.x = 0
                                opItem.y = 0
                            }
                        }

                        OutputPatchItem
                        {
                            id: opItem
                            width: opRoot.width

                            universeID: universe.id
                            patch: universe ? universe.outputPatch(index) : null
                            patchIndex: index

                            Drag.active: opMouseArea.drag.active
                            Drag.source: opMouseArea
                            Drag.hotSpot.x: width / 2
                            Drag.hotSpot.y: height / 2
                            Drag.keys: [ "removePatch" ]
                        }
                    }
                }
        }
    } // Column

    // New output patch drop area
    DropArea
    {
        id: outputDropTarget
        x: outWireBox.x + 6
        y: outputBox.y + outputBox.height + 2
        width: ((uniItem.width - uniBox.width) / 2) - 6
        height: UISettings.bigItemHeight * 0.9

        // this key must match the one in PluginList, to avoid dropping
        // an input plugin on output and vice-versa
        keys: [ universe ? "output-" + universe.id : "" ]

        onDropped:
        {
            console.log("Requested to add a new output patch")
            ioManager.setOutputPatch(drag.source.pluginUniverse, drag.source.pluginName,
                                     drag.source.pluginLine, outputPatchesNumber)
        }

        Rectangle
        {
            id: outDropRect
            anchors.fill: parent
            color: outputDropTarget.containsDrag ? UISettings.highlight : "transparent"
            states: [
                State
                {
                    when: outputDropTarget.containsDrag

                    PropertyChanges
                    {
                        target: uniItem
                        itemHeight: (outputPatchesNumber + 1) * UISettings.bigItemHeight
                    }
                }
            ]
        }
    }

    // Global mouse area to select this Universe item
    MouseArea
    {
        anchors.fill: parent

        onClicked:
        {
            ioManager.selectedIndex = universe.id
            uniItem.selected(universe.id);
        }
    }

    // items divider
    Rectangle
    {
        width: parent.width
        height: 2
        y: parent.height - 2
        color: isSelected ? UISettings.selection : UISettings.bgLight
    }
}

