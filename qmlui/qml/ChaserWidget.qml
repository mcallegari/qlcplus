/*
  Q Light Controller Plus
  ChaserWidget.qml

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

import QtQuick 2.12
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.1

import org.qlcplus.classes 1.0
import "TimeUtils.js" as TimeUtils
import "."

Column
{
    id: widgetRoot
    property bool isSequence: false
    property alias model: cStepsList.model
    property int playbackIndex: -1
    property int nextIndex: -1
    property alias speedType: timeEditTool.speedType
    property int tempoType: QLCFunction.Time
    property bool isRunning: false
    property alias containsDrag: cwDropArea.containsDrag
    property alias selector: ceSelector
    property bool isPrinting: false

    property int editStepIndex: -1
    property int editStepType
    property int selectionRequestIndex: -1

    signal indexChanged(int index)
    signal stepValueChanged(int index, int value, int type)
    signal noteTextChanged(int index, string text)
    signal addFunctions(var list, int index)
    signal moveSteps(var list, int index)
    signal requestEditor(int funcID)
    signal dragEntered(var item)
    signal dragExited(var item)
    signal enterPressed(int index)

    onPlaybackIndexChanged: ceSelector.selectItem(playbackIndex, cStepsList.model, false)

    function editStepTime(stepIndex, stepItem, type)
    {
        var title, timeValueString

        cStepsList.currentIndex = stepIndex

        if (stepItem.isSelected === false)
            ceSelector.selectItem(stepIndex, cStepsList.model, false)

        editStepIndex = stepIndex
        editStepType = type

        timeEditTool.tempoType = widgetRoot.tempoType
        timeEditTool.indexInList = stepIndex

        if (type === QLCFunction.FadeIn)
        {
            title = "#" + (stepIndex + 1) + " " + fInCol.label
            timeValueString = stepItem.stepFadeIn
            timeEditTool.allowFractions = QLCFunction.AllFractions
        }
        else if (type === QLCFunction.Hold)
        {
            title = "#" + (stepIndex + 1) + " " + holdCol.label
            timeValueString = stepItem.stepHold
            timeEditTool.allowFractions = QLCFunction.NoFractions
        }
        else if (type === QLCFunction.FadeOut)
        {
            title = "#" + (stepIndex + 1) + " " + fOutCol.label
            timeValueString = stepItem.stepFadeOut
            timeEditTool.allowFractions = QLCFunction.AllFractions
        }
        else if (type === QLCFunction.Duration)
        {
            title = "#" + (stepIndex + 1) + " " + durCol.label
            timeValueString = stepItem.stepDuration
            timeEditTool.allowFractions = QLCFunction.NoFractions
        }

        timeEditTool.show(-1, stepItem.mapToItem(mainView, 0, 0).y, title, timeValueString, type)
    }

    function selectStep(stepIndex, multiSelect)
    {
        ceSelector.selectItem(stepIndex, cStepsList.model, multiSelect)
    }

    ModelSelector
    {
        id: ceSelector
        onItemsCountChanged: console.log("Chaser Editor selected items: " + itemsCount)
    }

    TimeEditTool
    {
        id: timeEditTool
        parent: mainView ? mainView : widgetRoot
        x: rightSidePanel ? rightSidePanel.x - width : -width
        z: 99
        visible: false

        onValueChanged: widgetRoot.stepValueChanged(indexInList, val, speedType)
        onClosed: editStepIndex = -1
        onTabPressed:
        {
            var typeArray = [ QLCFunction.FadeIn, QLCFunction.Hold, QLCFunction.FadeOut, QLCFunction.Duration ]
            var currType = typeArray.indexOf(editStepType) + (forward ? 1 : -1)

            if (currType < 0)
            {
                // need to select the previous step
                if (cStepsList.currentIndex > 0)
                {
                    cStepsList.currentIndex--
                    editStepTime(cStepsList.currentIndex, cStepsList.currentItem, QLCFunction.Duration)
                }
            }
            else if (currType >= typeArray.length)
            {
                // need to select the next step
                cStepsList.currentIndex++
                editStepTime(cStepsList.currentIndex, cStepsList.currentItem, QLCFunction.FadeIn)
            }
            else
            {
                // same step, other field
                editStepTime(editStepIndex, cStepsList.currentItem, typeArray[currType])
            }
        }
    }

    Rectangle
    {
        id: chListHeader
        width: parent.width
        height: UISettings.listItemHeight
        color: UISettings.bgLight
        property int fSize: UISettings.textSizeDefault * 0.75

        Row
        {
            height: UISettings.listItemHeight
            spacing: 2

            // Step number column
            RobotoText
            {
                id: numCol
                width: UISettings.iconSizeMedium
                height: parent.height
                label: "#"
                wrapText: true
                textHAlign: Text.AlignHCenter
                fontSize: chListHeader.fSize
            }
            Rectangle { height: parent.height; width: 1; color: UISettings.fgMedium }

            // Step Function name column
            RobotoText
            {
                id: nameCol
                visible: !isSequence
                width: UISettings.bigItemHeight * 1.5
                height: parent.height
                label: qsTr("Function")
                wrapText: true
                textHAlign: Text.AlignHCenter
                fontSize: chListHeader.fSize
            }
            Rectangle
            {
                id: nameColDrag
                visible: !isSequence
                height: parent.height
                width: 1
                color: UISettings.fgMedium

                MouseArea
                {
                    anchors.fill: parent
                    cursorShape: Qt.SizeHorCursor
                    onPressed:
                    {
                        drag.target = nameColDrag
                        drag.minimumX = 0
                        drag.axis = Drag.XAxis
                    }
                    onPositionChanged:
                    {
                        if (drag.target == null)
                            return
                        nameCol.width = nameColDrag.x - nameCol.x - 1
                    }
                    onReleased: drag.target = null
                }
            }

            // Step fade in column
            RobotoText
            {
                id: fInCol
                width: UISettings.bigItemHeight * 0.5
                height: parent.height
                label: qsTr("Fade In")
                wrapText: true
                textHAlign: Text.AlignHCenter
                fontSize: chListHeader.fSize
            }
            Rectangle
            {
                id: fInColDrag
                height: parent.height
                width: 1
                color: UISettings.fgMedium

                MouseArea
                {
                    anchors.fill: parent
                    cursorShape: Qt.SizeHorCursor
                    onPressed:
                    {
                        drag.target = fInColDrag
                        drag.minimumX = 0
                        drag.axis = Drag.XAxis
                    }
                    onPositionChanged:
                    {
                        if (drag.target == null)
                            return
                        fInCol.width = fInColDrag.x - fInCol.x - 1
                    }
                    onReleased: drag.target = null
                }
            }

            // Step hold column
            RobotoText
            {
                id: holdCol
                width: UISettings.bigItemHeight * 0.5
                height: parent.height
                label: qsTr("Hold")
                wrapText: true
                textHAlign: Text.AlignHCenter
                fontSize: chListHeader.fSize
            }
            Rectangle
            {
                id: holdColDrag
                height: parent.height
                width: 1
                color: UISettings.fgMedium

                MouseArea
                {
                    anchors.fill: parent
                    cursorShape: Qt.SizeHorCursor
                    onPressed:
                    {
                        drag.target = holdColDrag
                        drag.minimumX = 0
                        drag.axis = Drag.XAxis
                    }
                    onPositionChanged:
                    {
                        if (drag.target == null)
                            return
                        holdCol.width = holdColDrag.x - holdCol.x - 1
                    }
                    onReleased: drag.target = null
                }
            }

            // Step fade out column
            RobotoText
            {
                id: fOutCol
                width: UISettings.bigItemHeight * 0.5
                height: parent.height
                label: qsTr("Fade Out")
                wrapText: true
                textHAlign: Text.AlignHCenter
                fontSize: chListHeader.fSize
            }
            Rectangle
            {
                id: fOutColDrag
                height: parent.height
                width: 1
                color: UISettings.fgMedium

                MouseArea
                {
                    anchors.fill: parent
                    cursorShape: Qt.SizeHorCursor
                    onPressed:
                    {
                        drag.target = fOutColDrag
                        drag.minimumX = 0
                        drag.axis = Drag.XAxis
                    }
                    onPositionChanged:
                    {
                        if (drag.target == null)
                            return
                        fOutCol.width = fOutColDrag.x - fOutCol.x - 1
                    }
                    onReleased: drag.target = null
                }
            }

            // Step duration column
            RobotoText
            {
                id: durCol
                width: UISettings.bigItemHeight * 0.5
                height: parent.height
                label: qsTr("Duration")
                wrapText: true
                textHAlign: Text.AlignHCenter
                fontSize: chListHeader.fSize
            }
            Rectangle
            {
                id: durColDrag
                height: parent.height
                width: 1
                color: UISettings.fgMedium

                MouseArea
                {
                    anchors.fill: parent
                    cursorShape: Qt.SizeHorCursor
                    onPressed:
                    {
                        drag.target = durColDrag
                        drag.minimumX = 0
                        drag.axis = Drag.XAxis
                    }
                    onPositionChanged:
                    {
                        if (drag.target == null)
                            return
                        durCol.width = durColDrag.x - durCol.x - 1
                    }
                    onReleased: drag.target = null
                }
            }

            // Step note column
            RobotoText
            {
                id: noteCol
                width: UISettings.bigItemHeight * 2
                height: parent.height
                label: qsTr("Note")
                fontSize: chListHeader.fSize
                //Layout.fillWidth: true
            }
        }
    } // Rectangle header

    ListView
    {
        id: cStepsList
        width: parent.width
        height: widgetRoot.height - chListHeader.height
        boundsBehavior: Flickable.StopAtBounds
        clip: true

        property bool dragActive: false
        property int dragInsertIndex: -1

        //onCurrentIndexChanged: ceSelector.selectItem(currentIndex, model, false)

        CustomTextEdit
        {
            id: noteTextEdit
            visible: false

            function show(stepIndex, item)
            {
                editStepIndex = stepIndex
                x = item.x
                y = item.mapToItem(cStepsList, 0, 0).y
                width = item.width
                height = item.height

                text = item.label
                visible = true
                selectAndFocus()
            }

            onAccepted:
            {
                widgetRoot.noteTextChanged(editStepIndex, text)
                editStepIndex = -1
                visible = false
            }

            Keys.onPressed:
            {
                if (event.key === Qt.Key_Escape)
                {
                    editStepIndex = -1
                    visible = false
                }
            }
        }

        delegate:
            Item
            {
                id: itemRoot
                width: cStepsList.width
                height: UISettings.listItemHeight

                Keys.onPressed:
                {
                    if (event.key === Qt.Key_Return ||
                        event.key === Qt.Key_Enter)
                    {
                        widgetRoot.enterPressed(index)
                        event.accepted = true
                    }
                }

                MouseArea
                {
                    id: delegateRoot
                    width: cStepsList.width
                    height: parent.height

                    drag.target: csDragItem
                    drag.threshold: height / 4

                    property bool dragActive: drag.active

                    onPressed:
                    {
                        var posInList = delegateRoot.mapToItem(widgetRoot, mouse.x, mouse.y)
                        csDragItem.parent = widgetRoot
                        csDragItem.x = posInList.x
                        csDragItem.y = posInList.y
                        csDragItem.z = 10

                        if (model.isSelected)
                            return

                        ceSelector.selectItem(index, cStepsList.model, mouse.modifiers & Qt.ControlModifier)
                        if (mouse.modifiers == 0)
                        {
                            widgetRoot.indexChanged(index)
                            csDragItem.itemsList = []
                        }

                        csDragItem.itemsList = ceSelector.itemsList()
                        itemRoot.forceActiveFocus()
                    }

                    onDoubleClicked: csDelegate.handleDoubleClick(mouse.x, mouse.y)

                    onDragActiveChanged:
                    {
                        if (dragActive)
                        {
                            csDragItem.itemLabel = csDelegate.func.name
                            csDragItem.itemIcon = functionManager.functionIcon(csDelegate.func.type)
                            cStepsList.dragActive = true
                        }
                        else
                        {
                            csDragItem.Drag.drop()
                        }
                    }

                    ChaserStepDelegate
                    {
                        id: csDelegate
                        width: widgetRoot.width
                        showFunctionName: !isSequence
                        functionID: model.funcID
                        isSelected: model.isSelected
                        stepFadeIn: TimeUtils.timeToQlcString(model.fadeIn, widgetRoot.tempoType)
                        stepHold: TimeUtils.timeToQlcString(model.hold, widgetRoot.tempoType)
                        stepFadeOut: TimeUtils.timeToQlcString(model.fadeOut, widgetRoot.tempoType)
                        stepDuration: TimeUtils.timeToQlcString(model.duration, widgetRoot.tempoType)
                        stepNote: model.note

                        col1Width: numCol.width
                        col2Width: nameCol.width
                        col3Width: fInCol.width
                        col4Width: holdCol.width
                        col5Width: fOutCol.width
                        col6Width: durCol.width

                        isPrinting: widgetRoot.isPrinting
                        indexInList: index
                        highlightIndex: cStepsList.dragInsertIndex
                        highlightEditTime: editStepIndex === index ? editStepType : -1
                        nextIndex: widgetRoot.nextIndex

                        onDoubleClicked:
                        {
                            console.log("Double clicked: " + indexInList + ", " + type)
                            if (type === QLCFunction.Name)
                                widgetRoot.requestEditor(ID)
                            else if (type === QLCFunction.Notes)
                                noteTextEdit.show(indexInList, qItem)
                            else
                                widgetRoot.editStepTime(indexInList, this, type)
                        }
                    } // ChaserStepDelegate
                } // MouseArea
            } // Item

        GenericMultiDragItem
        {
            id: csDragItem

            property bool fromFunctionManager: false

            visible: cStepsList.dragActive

            Drag.active: cStepsList.dragActive
            Drag.source: csDragItem
            Drag.keys: [ "function" ]
        }

        DropArea
        {
            id: cwDropArea
            anchors.fill: parent
            // accept only functions
            keys: [ "function" ]

            onEntered: widgetRoot.dragEntered(widgetRoot)
            onExited:
            {
                cStepsList.dragInsertIndex = -1
                widgetRoot.dragExited(widgetRoot)
            }

            onDropped:
            {
                console.log("Item dropped here. x: " + drag.x + " y: " + drag.y)

                /* Check if the dragging was started from a Function Manager */
                if (drag.source.fromFunctionManager === true)
                {
                    widgetRoot.addFunctions(drag.source.itemsList, cStepsList.dragInsertIndex)
                }
                else
                {
                    widgetRoot.moveSteps(ceSelector.itemsList(), cStepsList.dragInsertIndex)
                    cStepsList.currentIndex = -1
                }

                cStepsList.dragInsertIndex = -1
                cStepsList.dragActive = false
            }
            onPositionChanged:
            {
                var idx = cStepsList.indexAt(drag.x, drag.y)
                var item = cStepsList.itemAt(drag.x, drag.y)
                var itemY = item.mapToItem(cStepsList, 0, 0).y
                //console.log("Item index:" + idx)

                if (drag.y < (itemY + item.height) / 2)
                    cStepsList.dragInsertIndex = idx
                else
                    cStepsList.dragInsertIndex = idx + 1
            }
        }
        ScrollBar.vertical: CustomScrollBar { }
    } // end of ListView
}
