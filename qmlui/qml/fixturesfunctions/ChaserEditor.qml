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
import QtQuick.Controls 2.13

import org.qlcplus.classes 1.0
import "TimeUtils.js" as TimeUtils
import "."

Rectangle
{
    id: ceContainer
    anchors.fill: parent
    color: "transparent"

    property int functionID: -1
    property bool isSequence: chaserEditor.isSequence

    signal requestView(int ID, string qmlSrc)

    function deleteSelectedItems()
    {
        deleteItemsPopup.open()
    }

    CustomPopupDialog
    {
        id: deleteItemsPopup
        title: qsTr("Delete steps")
        message: qsTr("Are you sure you want to remove the selected steps?")
        onAccepted: functionManager.deleteEditorItems(chWidget.selector.itemsList())
    }

    SplitView
    {
        anchors.fill: parent

        Loader
        {
            id: funcMgrLoader
            width: UISettings.sidePanelWidth
            SplitView.preferredWidth: UISettings.sidePanelWidth
            visible: false
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
            SplitView.fillWidth: true

            EditorTopBar
            {
                id: topbar
                visible: !isSequence
                text: chaserEditor.functionName
                onTextChanged: chaserEditor.functionName = text

                onBackClicked:
                {
                    if (funcMgrLoader.visible)
                    {
                        funcMgrLoader.source = ""
                        funcMgrLoader.visible = false
                        rightSidePanel.width -= funcMgrLoader.width
                    }

                    var prevID = chaserEditor.previousID
                    functionManager.setEditorFunction(prevID, false, true)
                    requestView(prevID, functionManager.getEditorResource(prevID))
                }

                IconButton
                {
                    width: height
                    height: UISettings.iconSizeMedium - 2
                    imgSource: "qrc:/back.svg"
                    tooltip: qsTr("Preview the previous step")
                    visible: chaserEditor.previewEnabled
                    onClicked: chaserEditor.gotoPreviousStep()
                }

                IconButton
                {
                    width: height
                    height: UISettings.iconSizeMedium - 2
                    imgSource: "qrc:/forward.svg"
                    tooltip: qsTr("Preview the next step")
                    visible: chaserEditor.previewEnabled
                    onClicked: chaserEditor.gotoNextStep()
                }

                IconButton
                {
                    id: addFunc
                    width: height
                    height: UISettings.iconSizeMedium - 2
                    imgSource: "qrc:/add.svg"
                    checkable: true
                    enabled: !chaserEditor.previewEnabled
                    tooltip: qsTr("Add a new step")

                    onCheckedChanged:
                    {
                        if (checked)
                        {
                            if (!funcMgrLoader.visible)
                                rightSidePanel.width += UISettings.sidePanelWidth
                            funcMgrLoader.visible = true
                            funcMgrLoader.source = "qrc:/FunctionManager.qml"
                        }
                        else
                        {
                            rightSidePanel.width -= funcMgrLoader.width
                            funcMgrLoader.source = ""
                            funcMgrLoader.visible = false
                        }
                    }
                }

                IconButton
                {
                    width: height
                    height: UISettings.iconSizeMedium - 2
                    imgSource: "qrc:/edit-copy.svg"
                    tooltip: qsTr("Duplicate the selected step(s)")
                    enabled: !chaserEditor.previewEnabled && chWidget.selector.itemsCount
                    onClicked: chaserEditor.duplicateSteps(chWidget.selector.itemsList())
                }

                IconButton
                {
                    id: removeFunc
                    width: height
                    height: UISettings.iconSizeMedium - 2
                    imgSource: "qrc:/remove.svg"
                    tooltip: qsTr("Remove the selected steps")
                    enabled: !chaserEditor.previewEnabled && chWidget.selector.itemsCount
                    onClicked: deleteSelectedItems()
                }

                IconButton
                {
                    id: printButton
                    width: height
                    height: UISettings.iconSizeMedium - 2
                    imgSource: "qrc:/printer.svg"
                    tooltip: qsTr("Print the Chaser steps")
                    onClicked:
                    {
                        chWidget.isPrinting = true
                        qlcplus.printItem(chWidget)
                    }
                }
            }

            ChaserWidget
            {
                id: chWidget
                objectName: "chaserEditorWidget"
                isSequence: ceContainer.isSequence
                width: ceContainer.width
                height: ceContainer.height - (topbar.visible ? topbar.height : 0) - chModes.height
                model: chaserEditor.stepsList
                playbackIndex: chaserEditor.playbackIndex
                speedType: chaserEditor.stepsDuration
                tempoType: chaserEditor.tempoType
                isRunning: chaserEditor.previewEnabled

                onIndexChanged: chaserEditor.playbackIndex = index
                onStepValueChanged: chaserEditor.setStepSpeed(index, value, type)
                onNoteTextChanged: chaserEditor.setStepNote(index, text)
                onAddFunctions: chaserEditor.addFunctions(list, index)
                onMoveSteps: chaserEditor.moveSteps(list, index)
                onRequestEditor:
                {
                    functionManager.setEditorFunction(funcID, false, false)
                    requestView(funcID, functionManager.getEditorResource(funcID))
                }
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
                            ListElement { mLabel: qsTr("Loop"); mIcon: "qrc:/loop.svg"; mValue: QLCFunction.Loop }
                            ListElement { mLabel: qsTr("Single Shot"); mIcon: "qrc:/arrow-end.svg"; mValue: QLCFunction.SingleShot }
                            ListElement { mLabel: qsTr("Ping Pong"); mIcon: "qrc:/pingpong.svg"; mValue: QLCFunction.PingPong }
                            ListElement { mLabel: qsTr("Random"); mIcon: "qrc:/random.svg"; mValue: QLCFunction.Random }
                        }
                        model: runOrderModel

                        currValue: chaserEditor.runOrder
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
                            ListElement { mLabel: qsTr("Forward"); mIcon: "qrc:/forward.svg"; mValue: QLCFunction.Forward }
                            ListElement { mLabel: qsTr("Backward"); mIcon: "qrc:/back.svg"; mValue: QLCFunction.Backward }
                        }
                        model: directionModel

                        currValue: chaserEditor.direction
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
                            ListElement { mLabel: qsTr("Time"); mTextIcon: "T"; mValue: QLCFunction.Time }
                            ListElement { mLabel: qsTr("Beats"); mTextIcon: "B"; mValue: QLCFunction.Beats }
                        }
                        model: tempoModel

                        currValue: chaserEditor.tempoType
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

                        currValue: chaserEditor.stepsFadeIn
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

                        currValue: chaserEditor.stepsFadeOut
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

                        currValue: chaserEditor.stepsDuration
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
