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

import com.qlcplus.classes 1.0
import "."

Rectangle
{
    id: ceContainer
    anchors.fill: parent
    color: "transparent"

    property int functionID: -1
    // the index of the step currently being edited
    property int timeEditStepIndex: -1
    // the type of time editing currently being performed
    property string timeEditType: ""

    signal requestView(int ID, string qmlSrc)


    ModelSelector
    {
        id: ceSelector

        onItemsCountChanged:
        {
            console.log("Chaser Editor selected items changed !")
        }
    }

    TimeEditTool
    {
        id: timeEditTool
        //parent: mainView
        z: 99
        visible: false

        onTimeValueChanged:
        {

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

            Rectangle
            {
                color: UISettings.bgMedium
                width: parent.width
                height: 40
                z: 2

                Rectangle
                {
                    id: backBox
                    width: 40
                    height: 40
                    color: "transparent"

                    Image
                    {
                        id: leftArrow
                        anchors.fill: parent
                        rotation: 180
                        source: "qrc:/arrow-right.svg"
                        sourceSize: Qt.size(width, height)
                    }
                    MouseArea
                    {
                        anchors.fill: parent
                        hoverEnabled: true
                        onEntered: backBox.color = "#666"
                        onExited: backBox.color = "transparent"
                        onClicked:
                        {
                            if (funcMgrLoader.width)
                            {
                                funcMgrLoader.source = "";
                                funcMgrLoader.width = 0;
                                rightSidePanel.width = rightSidePanel.width / 2
                            }

                            requestView(-1, "qrc:/FunctionManager.qml")
                        }
                    }
                }
                TextInput
                {
                    id: cNameEdit
                    x: leftArrow.width + 5
                    height: 40
                    width: ceContainer.width - backBox.width - addFunc.width - removeFunc.width - 10
                    color: UISettings.fgMain
                    clip: true
                    text: chaserEditor.chaserName
                    verticalAlignment: TextInput.AlignVCenter
                    font.family: "Roboto Condensed"
                    font.pixelSize: 20
                    selectByMouse: true
                    Layout.fillWidth: true

                    onTextChanged: chaserEditor.chaserName = text
                }

                IconButton
                {
                    id: addFunc
                    x: parent.width - 90
                    width: height
                    height: 38
                    imgSource: "qrc:/add.svg"
                    checkable: true
                    tooltip: qsTr("Add a function")
                    onCheckedChanged:
                    {
                        if (checked)
                        {
                            rightSidePanel.width += 350
                            funcMgrLoader.width = 350
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
                    x: parent.width - 45
                    width: height
                    height: 38
                    imgSource: "qrc:/remove.svg"
                    tooltip: qsTr("Remove the selected function")
                    onClicked: {   }
                }
            }

            Rectangle
            {
                id: chListHeader
                width: parent.width
                height: 35
                color: UISettings.bgLight
                property int fSize: 11

                Row
                {
                    height: 35
                    spacing: 2

                    // Step number column
                    RobotoText
                    {
                        id: numCol
                        width: 25
                        label: "#"
                        wrapText: true
                        textAlign: Text.AlignHCenter
                        fontSize: chListHeader.fSize
                    }
                    Rectangle { height: 35; width: 1; color: UISettings.fgMedium }

                    // Step Function name column
                    RobotoText
                    {
                        id: nameCol
                        width: 120
                        label: qsTr("Function")
                        wrapText: true
                        textAlign: Text.AlignHCenter
                        fontSize: chListHeader.fSize
                    }
                    Rectangle
                    {
                        id: nameColDrag
                        height: 35
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
                        width: 60
                        label: qsTr("Fade In")
                        wrapText: true
                        textAlign: Text.AlignHCenter
                        fontSize: chListHeader.fSize
                    }
                    Rectangle
                    {
                        id: fInColDrag
                        height: 35
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
                        width: 60
                        label: qsTr("Hold")
                        wrapText: true
                        textAlign: Text.AlignHCenter
                        fontSize: chListHeader.fSize
                    }
                    Rectangle
                    {
                        id: holdColDrag
                        height: 35
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
                        width: 60
                        label: qsTr("Fade Out")
                        wrapText: true
                        textAlign: Text.AlignHCenter
                        fontSize: chListHeader.fSize
                    }
                    Rectangle
                    {
                        id: fOutColDrag
                        height: 35
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
                        width: 60
                        label: qsTr("Duration")
                        wrapText: true
                        textAlign: Text.AlignHCenter
                        fontSize: chListHeader.fSize
                    }
                    Rectangle
                    {
                        id: durColDrag
                        height: 35
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
                        width: 200
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
                height: ceContainer.height - 40 - chListHeader.height - chModes.height
                boundsBehavior: Flickable.StopAtBounds
                clip: true

                property int dragInsertIndex: -1

                model: chaserEditor.stepsList
                delegate:
                    ChaserStepDelegate
                    {
                        width: ceContainer.width
                        functionID: modelData.funcID
                        stepFadeIn: modelData.fadeIn
                        stepHold: modelData.hold
                        stepFadeOut: modelData.fadeOut
                        stepDuration: modelData.duration
                        stepNote: modelData.note

                        col1Width: numCol.width
                        col2Width: nameCol.width
                        col3Width: fInCol.width
                        col4Width: holdCol.width
                        col5Width: fOutCol.width
                        col6Width: durCol.width

                        indexInList: index
                        highlightIndex: cStepsList.dragInsertIndex

                        onClicked:
                        {
                            ceSelector.selectItem(ID, qItem, mouseMods & Qt.ControlModifier)
                        }
                        onDoubleClicked:
                        {
                            console.log("Double clicked: " + indexInList + ", " + type)
                            ceContainer.timeEditStepIndex = indexInList
                            ceContainer.timeEditType = type

                            if (type == "FI")
                            {
                                timeEditTool.x = fInCol.x - 35
                                timeEditTool.title = fInCol.label
                                timeEditTool.timeValueString = stepFadeIn
                            }
                            else if (type == "H")
                            {
                                timeEditTool.x = holdCol.x - 35
                                timeEditTool.title = holdCol.label
                                timeEditTool.timeValueString = stepHold
                            }
                            else if (type == "FO")
                            {
                                timeEditTool.x = fOutCol.x - 35
                                timeEditTool.title = fOutCol.label
                                timeEditTool.timeValueString = stepFadeOut
                            }
                            else if (type == "D")
                            {
                                timeEditTool.x = durCol.x - 35
                                timeEditTool.title = durCol.label
                                timeEditTool.timeValueString = stepDuration
                            }


                            timeEditTool.y = height * indexInList - cStepsList.contentY + cStepsList.y
                            //timeEditTool.y = height * indexInList - cStepsList.contentY + cStepsList.y - 70
                            timeEditTool.visible = true
                            height = timeEditTool.height
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
                        console.log("Item fID: " + drag.source.funcID)
                        chaserEditor.addFunction(drag.source.funcID, cStepsList.dragInsertIndex)
                        cStepsList.dragInsertIndex = -1
                    }
                    onPositionChanged:
                    {
                        var idx = cStepsList.indexAt(drag.x, drag.y)
                        //console.log("Item index:" + idx)
                        cStepsList.dragInsertIndex = idx
                    }
                }
                ScrollBar { flickable: cStepsList }
            }

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

                    Rectangle { height: 30; color: "transparent" }
                    Rectangle { height: 30; color: "transparent"; Layout.fillWidth: true }

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
            } // end of Rectangle
        } // end of Column
    } // end of SplitView
}
