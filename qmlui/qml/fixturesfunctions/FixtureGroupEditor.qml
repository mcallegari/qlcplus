/*
  Q Light Controller Plus
  FixtureGroupEditor.qml

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
import QtQuick.Layouts 1.0
import QtQuick.Controls 2.1

import org.qlcplus.classes 1.0
import "."

Rectangle
{
    id: fGroupEditor
    anchors.fill: parent
    color: "transparent"

    property int viewMargin: UISettings.listItemHeight * 0.5

    function hasSettings() { return false }

    Rectangle
    {
        height: UISettings.iconSizeDefault
        width: parent.width
        color: UISettings.bgStrong
        z: 1

        RowLayout
        {
            id: editToolbar
            anchors.fill: parent
            anchors.leftMargin: viewMargin
            anchors.rightMargin: viewMargin

            TextInput
            {
                id: fNameEdit
                height: parent.height
                Layout.fillWidth: true
                color: UISettings.fgLight
                clip: true
                verticalAlignment: TextInput.AlignVCenter
                font.family: UISettings.robotoFontName
                font.pixelSize: UISettings.textSizeDefault * 1.5
                font.bold: true
                selectByMouse: true
                text: fixtureGroupEditor.groupName
                onTextChanged: fixtureGroupEditor.groupName = text
            }

            Rectangle { color: "transparent"; Layout.fillWidth: true; }

            RobotoText
            {
                label: qsTr("Group size")
            }

            CustomSpinBox
            {
                id: gridWidthSpin
                from: 1
                to: 999
                value: fixtureGroupEditor.groupSize.width
                onValueModified: fixtureGroupEditor.groupSize = Qt.size(value, gridHeightSpin.value)
            }

            RobotoText
            {
                label: "x"
            }

            CustomSpinBox
            {
                id: gridHeightSpin
                from: 1
                to: 999
                value: fixtureGroupEditor.groupSize.height
                onValueModified: fixtureGroupEditor.groupSize = Qt.size(gridWidthSpin.value, value)
            }

            IconButton
            {
                id: posButton
                imgSource: "qrc:/position.svg"
                tooltip: qsTr("Transform the selected items")
                onClicked: transformMenu.open()
            }

            IconButton
            {
                imgSource: "qrc:/remove.svg"
                tooltip: qsTr("Remove the selected items")
                onClicked:
                {
                    fixtureGroupEditor.deleteSelection()
                    // TODO: update groups tree
                    //fixtureManager.updateFixtureGroup(fixtureGroupEditor.groupID, itemID, -1)
                    var empty = []
                    groupGrid.setSelectionData(empty)
                }
            }

            IconButton
            {
                faSource: FontAwesome.fa_remove
                faColor: UISettings.bgControl
                tooltip: qsTr("Reset the entire group")
                onClicked: fixtureGroupEditor.resetGroup()
            }
        }
    }

    Popup
    {
        id: transformMenu
        x: posButton.x - width + viewMargin
        padding: 0

        background:
            Rectangle
            {
                color: UISettings.bgStrong
                border.color: UISettings.bgStronger
            }

        Column
        {
            ContextMenuEntry
            {
                entryText: qsTr("Rotate 90° clockwise")
                onClicked:
                {
                    fixtureGroupEditor.transformSelection(FixtureGroupEditor.rotate90)
                    groupGrid.setSelectionData(fixtureGroupEditor.selectionData)
                }
            }
            ContextMenuEntry
            {
                entryText: qsTr("Rotate 180° clockwise")
                onClicked:
                {
                    fixtureGroupEditor.transformSelection(FixtureGroupEditor.rotate180)
                    groupGrid.setSelectionData(fixtureGroupEditor.selectionData)
                }
            }
            ContextMenuEntry
            {
                entryText: qsTr("Rotate 270° clockwise")
                onClicked:
                {
                    fixtureGroupEditor.transformSelection(FixtureGroupEditor.rotate270)
                    groupGrid.setSelectionData(fixtureGroupEditor.selectionData)
                }
            }
            ContextMenuEntry
            {
                entryText: qsTr("Flip horizontally")
                onClicked:
                {
                    fixtureGroupEditor.transformSelection(FixtureGroupEditor.HorizontalFlip)
                    groupGrid.setSelectionData(fixtureGroupEditor.selectionData)
                }
            }
            ContextMenuEntry
            {
                entryText: qsTr("Flip vertically")
                onClicked:
                {
                    fixtureGroupEditor.transformSelection(FixtureGroupEditor.VerticalFlip)
                    groupGrid.setSelectionData(fixtureGroupEditor.selectionData)
                }
            }
        }
    }

    Flickable
    {
        id: gridFlickable
        x: viewMargin
        y: editToolbar.y + editToolbar.height + viewMargin
        width: parent.width - x
        height: parent.height - editToolbar.height - viewMargin

        boundsBehavior: Flickable.StopAtBounds
        ScrollBar.vertical: CustomScrollBar { }
        ScrollBar.horizontal : CustomScrollBar { orientation: Qt.Horizontal }

        GridEditor
        {
            id: groupGrid
            width: fGroupEditor.width - (viewMargin * 2)
            height: parent.height - (viewMargin * 3)

            onHeightChanged: gridFlickable.contentHeight = height + cellSize
            onWidthChanged: gridFlickable.contentWidth = width + cellSize

            function getItemIcon(fixtureID, headNumber)
            {
                return fixtureManager.fixtureIcon(fixtureID)
            }

            function getTooltip(xPos, yPos)
            {
                return fixtureGroupEditor.getTooltip(xPos, yPos)
            }

            gridSize: fixtureGroupEditor.groupSize
            fillDirection: Qt.Horizontal | Qt.Vertical
            minimumCellSize: UISettings.iconSizeDefault * 1.5
            labelsFontSize: cellSize / 6
            gridData: fixtureGroupEditor.groupMap
            gridLabels: fixtureGroupEditor.groupLabels
            evenColor: UISettings.fgLight

            Component.onCompleted: forceActiveFocus()

            Keys.onPressed:
            {
                if (event.key === Qt.Key_Delete)
                {
                    fixtureGroupEditor.deleteSelection()
                    var empty = []
                    setSelectionData(empty)
                    event.accepted = true
                }
            }

            onPressed:
            {
                var empty = []
                focus = true

                if (xPos < 0 && yPos < 0)
                {
                    setSelectionData(empty)
                    fixtureGroupEditor.resetSelection()
                }
                else
                {
                    gridFlickable.interactive = false
                    var absIdx = (yPos * gridSize.width) + xPos
                    if (selectionData && selectionData.indexOf(absIdx) >= 0)
                        return
                    if (contextManager.multipleSelection === false && mods == 0)
                    {
                        setSelectionData(empty)
                        fixtureGroupEditor.resetSelection()
                    }
                    setSelectionData(fixtureGroupEditor.groupSelection(xPos, yPos, mods))
                }
            }

            onReleased:
            {
                gridFlickable.interactive = true

                if (currentItemID === -1 || offset === 0)
                    return

                if (externalDrag == false)
                {
                    fixtureGroupEditor.moveSelection(xPos, yPos, offset)
                    setSelectionData(fixtureGroupEditor.groupSelection(xPos, yPos, mods))
                }
            }

            onPositionChanged:
            {
                validSelection = fixtureGroupEditor.checkSelection(xPos, yPos, offset)
            }

            onDragEntered:
            {
                console.log("Drag entered at " + xPos + ", " + yPos)
                var tmp
                var mods = 0
                for (var i = 0; i < dragEvent.source.itemsList.length; i++)
                {
                    console.log("Item #" + i + " type: " + dragEvent.source.itemsList[i].itemType)
                    switch(dragEvent.source.itemsList[i].itemType)
                    {
                        case App.FixtureDragItem:
                            tmp = fixtureGroupEditor.fixtureSelection(dragEvent.source.itemsList[i].cRef, xPos, yPos, mods)
                        break;
                        case App.HeadDragItem:
                            tmp = fixtureGroupEditor.headSelection(xPos, yPos, mods)
                        break;
                    }
                    mods = 1
                }

                validSelection = fixtureGroupEditor.checkSelection(xPos, yPos, 0)
                setSelectionData(tmp)
            }

            onDragPositionChanged:
            {
                validSelection = fixtureGroupEditor.checkSelection(xPos, yPos, offset)
            }

            onDragDropped:
            {
                console.log("Drag dropped at " + xPos + ", " + yPos)
                for (var i = 0; i < dragEvent.source.itemsList.length; i++)
                {
                    var itemID = dragEvent.source.itemsList[i].itemID

                    switch(dragEvent.source.itemsList[i].itemType)
                    {
                        case App.FixtureDragItem:
                            if (fixtureGroupEditor.addFixture(dragEvent.source.itemsList[i].cRef, xPos, yPos) === true)
                                fixtureManager.updateFixtureGroup(fixtureGroupEditor.groupID, itemID, -1)
                        break;
                        case App.HeadDragItem:
                            if (fixtureGroupEditor.addHead(itemID, dragEvent.source.itemsList[i].headIndex, xPos, yPos) === true)
                                fixtureManager.updateFixtureGroup(fixtureGroupEditor.groupID, itemID,
                                                                  dragEvent.source.itemsList[i].headIndex)
                        break;
                    }
                }
            }
        } // GridEditor
    } // Flickable
}
