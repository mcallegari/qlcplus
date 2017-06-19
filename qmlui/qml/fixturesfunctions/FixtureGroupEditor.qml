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

import org.qlcplus.classes 1.0
import "."

Flickable
{
    id: fGroupEditor
    anchors.fill: parent
    anchors.margins: viewMargin
    boundsBehavior: Flickable.StopAtBounds

    property int viewMargin: UISettings.listItemHeight * 0.5

    function hasSettings() { return false; }

    Rectangle
    {
        height: UISettings.iconSizeDefault
        width: parent.width
        color: UISettings.bgStrong

        RowLayout
        {
            id: editToolbar
            anchors.fill: parent
            anchors.leftMargin: viewMargin
            anchors.rightMargin: viewMargin

            RobotoText
            {
                height: parent.height
                label: fixtureGroupEditor.groupName
                labelColor: UISettings.fgLight
                fontSize: UISettings.textSizeDefault * 1.5
                fontBold: true
                textVAlign: Text.AlignVCenter
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
                checkable: true
            }

            IconButton
            {
                imgSource: "qrc:/reset.svg"
                tooltip: qsTr("Reset the entire group")
                onClicked: fixtureGroupEditor.resetGroup()
            }
        }
    }

    Rectangle
    {
        id: transformMenu
        visible: posButton.checked
        x: posButton.x - width
        z: 2
        border.color: UISettings.bgStronger
        color: UISettings.bgStrong
        width: menuBox.width
        height: menuBox.height

        Column
        {
            id: menuBox
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

    GridEditor
    {
        id: groupGrid
        y: editToolbar.y + editToolbar.height + viewMargin
        width: parent.width
        height: parent.height - editToolbar.height - (viewMargin * 3)

        function getItemIcon(fixtureID, headNumber)
        {
            return fixtureManager.fixtureIcon(fixtureID)
        }

        gridSize: fixtureGroupEditor.groupSize
        fillDirection: Qt.Horizontal | Qt.Vertical
        gridData: fixtureGroupEditor.groupMap
        gridLabels: fixtureGroupEditor.groupLabels
        labelsFontSize: cellSize / 5
        evenColor: UISettings.fgLight

        onPressed:
        {
            fGroupEditor.interactive = false
            setSelectionData(fixtureGroupEditor.groupSelection(xPos, yPos, mods))
        }

        onReleased:
        {
            if (currentItemID === -1)
                return

            if (externalDrag == false)
            {
                fixtureGroupEditor.moveSelection(xPos, yPos, offset)
                setSelectionData(fixtureGroupEditor.groupSelection(xPos, yPos, mods))
            }
            fGroupEditor.interactive = true
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
    }
}
