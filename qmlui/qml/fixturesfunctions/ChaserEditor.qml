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

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import org.qlcplus.classes 1.0
import "TimeUtils.js" as TimeUtils
import "."

Rectangle
{
    id: ceContainer
    anchors.fill: parent
    color: "transparent"

    property int functionID: -1
    property bool isSequence: chaserEditor ? chaserEditor.isSequence : false

    signal requestView(int ID, string qmlSrc, bool back)

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
                text: chaserEditor ? chaserEditor.functionName : ""
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
                    requestView(prevID, functionManager.getEditorResource(prevID), true)
                }

                IconButton
                {
                    width: height
                    height: UISettings.iconSizeMedium - 2
                    faSource: FontAwesome.fa_circle_left
                    faColor: "lightcyan"
                    tooltip: qsTr("Preview the previous step")
                    visible: chaserEditor ? chaserEditor.previewEnabled : false
                    onClicked: chaserEditor.gotoPreviousStep()
                }

                IconButton
                {
                    width: height
                    height: UISettings.iconSizeMedium - 2
                    faSource: FontAwesome.fa_circle_right
                    faColor: "lightcyan"
                    tooltip: qsTr("Preview the next step")
                    visible: chaserEditor ? chaserEditor.previewEnabled : false
                    onClicked: chaserEditor.gotoNextStep()
                }

                IconButton
                {
                    id: addFunc
                    width: height
                    height: UISettings.iconSizeMedium - 2
                    faSource: FontAwesome.fa_plus
                    faColor: "limegreen"
                    checkable: true
                    enabled: chaserEditor ? !chaserEditor.previewEnabled : true
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
                    faSource: FontAwesome.fa_clone
                    faColor: UISettings.fgMain
                    tooltip: qsTr("Duplicate the selected step(s)")
                    enabled: !chaserEditor.previewEnabled && chWidget.selector.itemsCount
                    onClicked: chaserEditor.duplicateSteps(chWidget.selector.itemsList())
                }

                IconButton
                {
                    width: height
                    height: UISettings.iconSizeMedium - 2
                    faSource: FontAwesome.fa_shuffle
                    faColor: "gold"
                    tooltip: qsTr("Randomize the selected step(s) order")
                    enabled: chaserEditor ? !chaserEditor.previewEnabled : true
                    onClicked: chaserEditor.shuffleSteps(chWidget.selector.itemsList())
                }

                IconButton
                {
                    id: removeFunc
                    width: height
                    height: UISettings.iconSizeMedium - 2
                    faSource: FontAwesome.fa_minus
                    faColor: "crimson"
                    tooltip: qsTr("Remove the selected steps")
                    enabled: chaserEditor && !chaserEditor.previewEnabled && chWidget.selector.itemsCount
                    onClicked: deleteSelectedItems()
                }

                IconButton
                {
                    id: printButton
                    width: height
                    height: UISettings.iconSizeMedium - 2
                    faSource: FontAwesome.fa_print
                    faColor: UISettings.fgMain
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

                onIndexChanged: (index) => chaserEditor.playbackIndex = index
                onStepValueChanged: (index, value, type) => chaserEditor.setStepSpeed(index, value, type)
                onNoteTextChanged: (index, text) => chaserEditor.setStepNote(index, text)
                onAddFunctions: (list, index) => chaserEditor.addFunctions(list, index)
                onMoveSteps: (list, index) => chaserEditor.moveSteps(list, index)
                onRequestEditor: (funcID) =>
                {
                    requestView(funcID, functionManager.getEditorResource(funcID), false)
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
                        model: [
                            { mLabel: qsTr("Loop"), faIcon: FontAwesome.fa_retweet, mValue: QLCFunction.Loop },
                            { mLabel: qsTr("Single Shot"), faIcon: FontAwesome.fa_right_long, mValue: QLCFunction.SingleShot },
                            { mLabel: qsTr("Ping Pong"), faIcon: FontAwesome.fa_right_left, mValue: QLCFunction.PingPong },
                            { mLabel: qsTr("Random"), faIcon: FontAwesome.fa_shuffle, mValue: QLCFunction.Random }
                        ]

                        currValue: chaserEditor.runOrder
                        onValueChanged: (value) => chaserEditor.runOrder = value
                    }
                    RobotoText
                    {
                        label: qsTr("Run Order")
                        Layout.fillWidth: true
                    }

                    IconPopupButton
                    {
                        model: [
                            { mLabel: qsTr("Forward"), faIcon: FontAwesome.fa_angles_right, mValue: QLCFunction.Forward },
                            { mLabel: qsTr("Backward"), faIcon: FontAwesome.fa_angles_left, mValue: QLCFunction.Backward }
                        ]

                        currValue: chaserEditor.direction
                        onValueChanged: (value) => chaserEditor.direction = value
                    }
                    RobotoText
                    {
                        label: qsTr("Direction")
                        Layout.fillWidth: true
                    }

                    IconPopupButton
                    {
                        model: [
                            { mLabel: qsTr("Time"), mTextIcon: "T", mValue: QLCFunction.Time },
                            { mLabel: qsTr("Beats"), mTextIcon: "B", mValue: QLCFunction.Beats }
                        ]

                        currValue: chaserEditor.tempoType
                        onValueChanged: (value) => chaserEditor.tempoType = value
                    }
                    RobotoText
                    {
                        label: qsTr("Tempo")
                        Layout.fillWidth: true
                    }

                    // Row 2
                    IconPopupButton
                    {
                        model: [
                            { mLabel: qsTr("Default"), mTextIcon: "D", mValue: Chaser.Default },
                            { mLabel: qsTr("Common"), mTextIcon: "C", mValue: Chaser.Common },
                            { mLabel: qsTr("Per Step"), mTextIcon: "S", mValue: Chaser.PerStep }
                        ]

                        currValue: chaserEditor.stepsFadeIn
                        onValueChanged: (value) => chaserEditor.stepsFadeIn = value
                    }
                    RobotoText
                    {
                        label: qsTr("Fade In")
                        Layout.fillWidth: true
                    }

                    IconPopupButton
                    {
                        model: [
                            { mLabel: qsTr("Default"), mTextIcon: "D", mValue: Chaser.Default },
                            { mLabel: qsTr("Common"), mTextIcon: "C", mValue: Chaser.Common },
                            { mLabel: qsTr("Per Step"), mTextIcon: "S", mValue: Chaser.PerStep }
                        ]

                        currValue: chaserEditor.stepsFadeOut
                        onValueChanged: (value) => chaserEditor.stepsFadeOut = value
                    }
                    RobotoText
                    {
                        label: qsTr("Fade Out")
                        Layout.fillWidth: true
                    }

                    IconPopupButton
                    {
                        model: [
                            { mLabel: qsTr("Common"), mTextIcon: "C", mValue: Chaser.Common },
                            { mLabel: qsTr("Per Step"), mTextIcon: "S", mValue: Chaser.PerStep }
                        ]

                        currValue: chaserEditor.stepsDuration
                        onValueChanged: (value) => chaserEditor.stepsDuration = value
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
