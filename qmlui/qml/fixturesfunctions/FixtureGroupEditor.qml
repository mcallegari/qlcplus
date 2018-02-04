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

    function hasSettings() { return false; }

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
                onValueChanged: fixtureGroupEditor.groupSize = Qt.size(value, gridHeightSpin.value)
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
                onValueChanged: fixtureGroupEditor.groupSize = Qt.size(gridWidthSpin.value, value)
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
                imgSource: "qrc:/reset.svg"
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

        GridEditor
        {
            id: groupGrid
            width: parent.width
            height: parent.height - (viewMargin * 3)

            onHeightChanged: gridFlickable.contentHeight = height + cellSize
            onWidthChanged: gridFlickable.contentWidth = width + cellSize

            function getItemIcon(fixtureID, headNumber)
            {
                return fixtureManager.fixtureIcon(fixtureID)
            }

            gridSize: fixtureGroupEditor.groupSize
            fillDirection: Qt.Horizontal | Qt.Vertical
            mininumCellSize: UISettings.iconSizeDefault * 1.5
            gridData: fixtureGroupEditor.groupMap
            gridLabels: fixtureGroupEditor.groupLabels
            labelsFontSize: cellSize / 5
            evenColor: UISettings.fgLight

            onPressed:
            {
                if (xPos < 0 && yPos < 0)
                {
                    var empty = []
                    setSelectionData(empty)
                    fixtureGroupEditor.resetSelection()
                }
                else
                {
                    gridFlickable.interactive = false
                    setSelectionData(fixtureGroupEditor.groupSelection(xPos, yPos, mods))
                }
            }

            onReleased:
            {
                gridFlickable.interactive = true

                if (currentItemID === -1)
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
                            tmp = fixtureGroupEditor.dragSelection(dragEvent.source.itemsList[i].cRef, xPos, yPos, mods)
                        break;
                        case App.HeadDragItem:
                            efxEditor.addHead(dragEvent.source.itemsList[i].fixtureID, dragEvent.source.itemsList[i].headIndex)
                        break;
                    }
                    mods = 1
                }
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
                    switch(dragEvent.source.itemsList[i].itemType)
                    {
                        case App.FixtureDragItem:
                            fixtureGroupEditor.addFixture(dragEvent.source.itemsList[i].cRef, xPos, yPos)
                        break;
                    }
                }
            }
        } // GridEditor
    } // Flickable

    CustomScrollBar
    {
        anchors.right: parent.right
        flickable: gridFlickable
        doubleBars: true
    }
    CustomScrollBar
    {
        flickable: gridFlickable
        orientation: Qt.Horizontal
    }
}
