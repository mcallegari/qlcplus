/*
  Q Light Controller Plus
  ChaserEditor.qml

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

Rectangle
{
    id: ceContainer
    anchors.fill: parent
    color: "transparent"

    property int functionID: -1
    property int editStepIndex: -1
    property int editStepType
    property bool isSequence: chaserEditor.isSequence

    signal requestView(int ID, string qmlSrc)

    function editStepTime(stepIndex, stepItem, type)
    {
        var title, timeValueString

        cStepsList.currentIndex = stepIndex

        if (stepItem.isSelected === false)
            ceSelector.selectItem(stepIndex, cStepsList.model, false)

        editStepIndex = stepIndex
        editStepType = type

        timeEditTool.tempoType = chaserEditor.tempoType
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

        onValueChanged: chaserEditor.setStepSpeed(indexInList, val, speedType)
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

    SplitView
    {
        anchors.fill: parent

        Loader
        {
            id: funcMgrLoader
            visible: width
            width: 0
            height: ceContainer.height
            source: ""

            Rectangle
            {
                width: 2
                height: parent.height
                x: parent.width - 2
                color: UISettings.bgLighter
            }
        }

        Column
        {
            Layout.fillWidth: true

            EditorTopBar
            {
                id: topbar
                visible: !isSequence
                text: chaserEditor.functionName
                onTextChanged: chaserEditor.functionName = text

                onBackClicked:
                {
                    if (funcMgrLoader.width)
                    {
                        funcMgrLoader.source = ""
                        funcMgrLoader.width = 0
                        rightSidePanel.width = rightSidePanel.width / 2
                    }

                    functionManager.setEditorFunction(-1, false)
                    requestView(-1, "qrc:/FunctionManager.qml")
                }

                IconButton
                {
                    id: addFunc
                    width: height
                    height: UISettings.iconSizeMedium - 2
                    imgSource: "qrc:/add.svg"
                    checkable: true
                    tooltip: qsTr("Add a new step")

                    onCheckedChanged:
                    {
                        if (checked)
                        {
                            rightSidePanel.width += mainView.width / 3
                            funcMgrLoader.width = mainView.width / 3
                            funcMgrLoader.source = "qrc:/FunctionManager.qml"
                        }
                        else
                        {
                            rightSidePanel.width = rightSidePanel.width - funcMgrLoader.width
                            funcMgrLoader.source = ""
                            funcMgrLoader.width = 0
                        }
                    }
                }

                IconButton
                {
                    id: removeFunc
                    width: height
                    height: UISettings.iconSizeMedium - 2
                    imgSource: "qrc:/remove.svg"
                    tooltip: qsTr("Remove the selected steps")
                    onClicked: {   }
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
            }

            ListView
            {
                id: cStepsList
                width: parent.width
                height: ceContainer.height - (topbar.visible ? topbar.height : 0) - chListHeader.height - chModes.height
                boundsBehavior: Flickable.StopAtBounds
                clip: true

                property int dragInsertIndex: -1
                property int playbackIndex: chaserEditor.playbackIndex

                model: chaserEditor.stepsList

                onPlaybackIndexChanged:
                {
                    if (chaserEditor.previewEnabled)
                        ceSelector.selectItem(playbackIndex, model, 0)
                }

                delegate:
                    ChaserStepDelegate
                    {
                        width: ceContainer.width
                        showFunctionName: !isSequence
                        functionID: model.funcID
                        isSelected: model.isSelected
                        stepFadeIn: TimeUtils.timeToQlcString(model.fadeIn, chaserEditor.tempoType)
                        stepHold: TimeUtils.timeToQlcString(model.hold, chaserEditor.tempoType)
                        stepFadeOut: TimeUtils.timeToQlcString(model.fadeOut, chaserEditor.tempoType)
                        stepDuration: TimeUtils.timeToQlcString(model.duration, chaserEditor.tempoType)
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
                                chaserEditor.playbackIndex = index
                        }

                        onDoubleClicked:
                        {
                            console.log("Double clicked: " + indexInList + ", " + type)
                            ceContainer.editStepTime(indexInList, this, type)
                        }
                    }

                DropArea
                {
                    anchors.fill: parent
                    // accept only functions
                    keys: [ "function" ]

                    onDropped:
                    {
                        console.log("Item dropped here. x: " + drag.x + " y: " + drag.y)

                        /* Check if the dragging was started from a Function Manager */
                        if (drag.source.hasOwnProperty("fromFunctionManager"))
                        {
                            chaserEditor.addFunctions(drag.source.itemsList, cStepsList.dragInsertIndex)
                            cStepsList.dragInsertIndex = -1
                        }
                    }
                    onPositionChanged:
                    {
                        var idx = cStepsList.indexAt(drag.x, drag.y)
                        //console.log("Item index:" + idx)
                        cStepsList.dragInsertIndex = idx
                    }
                    onExited: cStepsList.dragInsertIndex = -1
                }
                CustomScrollBar { flickable: cStepsList }
            } // end of ListView

            SectionBox
            {
                id: chModes
                width: parent.width
                isExpanded: false
                sectionLabel: qsTr("Run properties")

                sectionContents:
                GridLayout
                {
                    x: 4
                    width: parent.width - 8
                    columns: 6
                    columnSpacing: 4
                    rowSpacing: 4

                    // Row 1
                    IconPopupButton
                    {
                        ListModel
                        {
                            id: runOrderModel
                            ListElement { mLabel: qsTr("Loop"); mIcon: "qrc:/loop.svg"; mValue: Function.Loop }
                            ListElement { mLabel: qsTr("Single Shot"); mIcon: "qrc:/arrow-end.svg"; mValue: Function.SingleShot }
                            ListElement { mLabel: qsTr("Ping Pong"); mIcon: "qrc:/pingpong.svg"; mValue: Function.PingPong }
                            ListElement { mLabel: qsTr("Random"); mIcon: "qrc:/random.svg"; mValue: Function.Random }
                        }
                        model: runOrderModel

                        currentValue: chaserEditor.runOrder
                        onValueChanged: chaserEditor.runOrder = value
                    }
                    RobotoText
                    {
                        label: qsTr("Run Order")
                        Layout.fillWidth: true
                    }

                    IconPopupButton
                    {
                        ListModel
                        {
                            id: directionModel
                            ListElement { mLabel: qsTr("Forward"); mIcon: "qrc:/forward.svg"; mValue: Function.Forward }
                            ListElement { mLabel: qsTr("Backward"); mIcon: "qrc:/back.svg"; mValue: Function.Backward }
                        }
                        model: directionModel

                        currentValue: chaserEditor.direction
                        onValueChanged: chaserEditor.direction = value
                    }
                    RobotoText
                    {
                        label: qsTr("Direction")
                        Layout.fillWidth: true
                    }

                    IconPopupButton
                    {
                        ListModel
                        {
                            id: tempoModel
                            ListElement { mLabel: qsTr("Time"); mTextIcon: "T"; mValue: Function.Time }
                            ListElement { mLabel: qsTr("Beats"); mTextIcon: "B"; mValue: Function.Beats }
                        }
                        model: tempoModel

                        currentValue: chaserEditor.tempoType
                        onValueChanged: chaserEditor.tempoType = value
                    }
                    RobotoText
                    {
                        label: qsTr("Tempo")
                        Layout.fillWidth: true
                    }

                    // Row 2
                    IconPopupButton
                    {
                        ListModel
                        {
                            id: fadeInModel
                            ListElement { mLabel: qsTr("Default"); mTextIcon: "D"; mValue: Chaser.Default }
                            ListElement { mLabel: qsTr("Common"); mTextIcon: "C"; mValue: Chaser.Common }
                            ListElement { mLabel: qsTr("Per Step"); mTextIcon: "S"; mValue: Chaser.PerStep }
                        }
                        model: fadeInModel

                        currentValue: chaserEditor.stepsFadeIn
                        onValueChanged: chaserEditor.stepsFadeIn = value
                    }
                    RobotoText
                    {
                        label: qsTr("Fade In")
                        Layout.fillWidth: true
                    }

                    IconPopupButton
                    {
                        ListModel
                        {
                            id: fadeOutModel
                            ListElement { mLabel: qsTr("Default"); mTextIcon: "D"; mValue: Chaser.Default }
                            ListElement { mLabel: qsTr("Common"); mTextIcon: "C"; mValue: Chaser.Common }
                            ListElement { mLabel: qsTr("Per Step"); mTextIcon: "S"; mValue: Chaser.PerStep }
                        }
                        model: fadeOutModel

                        currentValue: chaserEditor.stepsFadeOut
                        onValueChanged: chaserEditor.stepsFadeOut = value
                    }
                    RobotoText
                    {
                        label: qsTr("Fade Out")
                        Layout.fillWidth: true
                    }

                    IconPopupButton
                    {
                        ListModel
                        {
                            id: durationModel
                            ListElement { mLabel: qsTr("Common"); mTextIcon: "C"; mValue: Chaser.Common }
                            ListElement { mLabel: qsTr("Per Step"); mTextIcon: "S"; mValue: Chaser.PerStep }
                        }
                        model: durationModel

                        currentValue: chaserEditor.stepsDuration
                        onValueChanged: chaserEditor.stepsDuration = value
                    }
                    RobotoText
                    {
                        label: qsTr("Duration")
                        Layout.fillWidth: true
                    }
                } // end of GridLayout
            } // end of SectionBox
        } // end of Column
    } // end of SplitView
}
