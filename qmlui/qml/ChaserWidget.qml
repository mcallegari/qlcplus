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

import QtQuick 2.0
import QtQuick.Layouts 1.1
import QtQuick.Controls 1.2

import org.qlcplus.classes 1.0
import "TimeUtils.js" as TimeUtils
import "."

Column
{
    id: widgetRoot
    property bool isSequence: false
    property alias model: cStepsList.model
    property alias playbackIndex: cStepsList.currentIndex
    property int tempoType: Function.Time
    property bool isRunning: false
    property alias containsDrag: cwDropArea.containsDrag
    property alias selector: ceSelector

    property int editStepIndex: -1
    property int editStepType

    signal indexChanged(int index)
    signal stepValueChanged(int index, int value, int type)
    signal noteTextChanged(int index, string text)
    signal addFunctions(var list, int index)
    signal requestEditor(int funcID)
    signal dragEntered(var item)
    signal dragExited(var item)

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

        if (type === Function.FadeIn)
        {
            title = "#" + (stepIndex + 1) + " " + fInCol.label
            timeValueString = stepItem.stepFadeIn
            timeEditTool.allowFractions = Function.AllFractions
        }
        else if (type === Function.Hold)
        {
            title = "#" + (stepIndex + 1) + " " + holdCol.label
            timeValueString = stepItem.stepHold
            timeEditTool.allowFractions = Function.NoFractions
        }
        else if (type === Function.FadeOut)
        {
            title = "#" + (stepIndex + 1) + " " + fOutCol.label
            timeValueString = stepItem.stepFadeOut
            timeEditTool.allowFractions = Function.AllFractions
        }
        else if (type === Function.Duration)
        {
            title = "#" + (stepIndex + 1) + " " + durCol.label
            timeValueString = stepItem.stepDuration
            timeEditTool.allowFractions = Function.NoFractions
        }

        timeEditTool.show(-1, stepItem.mapToItem(mainView, 0, 0).y, title, timeValueString, type)
    }

    ModelSelector
    {
        id: ceSelector
        onItemsCountChanged: console.log("Chaser Editor selected items changed !")
    }

    TimeEditTool
    {
        id: timeEditTool
        parent: mainView
        x: rightSidePanel.x - width
        z: 99
        visible: false

        onValueChanged: widgetRoot.stepValueChanged(indexInList, val, speedType)
        onClosed: editStepIndex = -1
        onTabPressed:
        {
            var typeArray = [ Function.FadeIn, Function.Hold, Function.FadeOut, Function.Duration ]
            var currType = editStepType + (forward ? 1 : -1)

            if (currType < 0)
            {
                // need to select the previous step
                if (cStepsList.currentIndex > 0)
                {
                    cStepsList.currentIndex--
                    editStepTime(cStepsList.currentIndex, cStepsList.currentItem, Function.Duration)
                }
            }
            else if (currType >= typeArray.length)
            {
                // need to select the next step
                cStepsList.currentIndex++
                editStepTime(cStepsList.currentIndex, cStepsList.currentItem, Function.FadeIn)
            }
            else
            {
                // same step, other field
                editStepTime(editStepIndex, cStepsList.currentItem, currType)
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
                        if (drag.target === null)
                            return;
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
                        if (drag.target === null)
                            return;
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
                        if (drag.target === null)
                            return;
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
                        if (drag.target === null)
                            return;
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
                        if (drag.target === null)
                            return;
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

        property int dragInsertIndex: -1

        onCurrentIndexChanged: ceSelector.selectItem(currentIndex, model, 0)

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

                inputText = item.label
                visible = true
                selectAndFocus()
            }

            onEnterPressed:
            {
                widgetRoot.noteTextChanged(editStepIndex, inputText)
                editStepIndex = -1
                visible = false
            }
        }

        delegate:
            ChaserStepDelegate
            {
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

                indexInList: index
                highlightIndex: cStepsList.dragInsertIndex
                highlightEditTime: editStepIndex === index ? editStepType : -1

                onClicked:
                {
                    ceSelector.selectItem(indexInList, cStepsList.model, mouseMods & Qt.ControlModifier)
                    console.log("mouse mods: " + mouseMods)
                    if ((mouseMods & Qt.ControlModifier) == 0)
                        widgetRoot.indexChanged(index)
                }

                onDoubleClicked:
                {
                    console.log("Double clicked: " + indexInList + ", " + type)
                    if (type === Function.Name)
                        widgetRoot.requestEditor(ID)
                    else if (type === Function.Notes)
                        noteTextEdit.show(indexInList, qItem)
                    else
                        widgetRoot.editStepTime(indexInList, this, type)
                }
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
                if (drag.source.hasOwnProperty("fromFunctionManager"))
                {
                    widgetRoot.addFunctions(drag.source.itemsList, cStepsList.dragInsertIndex)
                    cStepsList.dragInsertIndex = -1
                }
            }
            onPositionChanged:
            {
                var idx = cStepsList.indexAt(drag.x, drag.y)
                //console.log("Item index:" + idx)
                cStepsList.dragInsertIndex = idx
            }
        }
        CustomScrollBar { flickable: cStepsList }
    } // end of ListView
}
