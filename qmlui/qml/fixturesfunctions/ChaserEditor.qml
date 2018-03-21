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
    property bool isSequence: chaserEditor.isSequence

    signal requestView(int ID, string qmlSrc)

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

                    var prevID = chaserEditor.previousID
                    functionManager.setEditorFunction(prevID, false, true)
                    requestView(prevID, functionManager.getEditorResource(prevID))
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
                    onClicked: deleteItemsPopup.open()

                    CustomPopupDialog
                    {
                        id: deleteItemsPopup
                        title: qsTr("Delete steps")
                        message: qsTr("Are you sure you want to remove the selected steps ?")
                        onAccepted: functionManager.deleteEditorItems(chWidget.selector.itemsList())
                    }
                }
            }

            ChaserWidget
            {
                id: chWidget
                isSequence: ceContainer.isSequence
                width: ceContainer.width
                height: ceContainer.height - (topbar.visible ? topbar.height : 0) - chModes.height
                model: chaserEditor.stepsList
                playbackIndex: chaserEditor.playbackIndex
                tempoType: chaserEditor.tempoType
                isRunning: chaserEditor.previewEnabled

                onIndexChanged: chaserEditor.playbackIndex = index
                onStepValueChanged: chaserEditor.setStepSpeed(index, value, type)
                onNoteTextChanged: chaserEditor.setStepNote(index, text)
                onAddFunctions: chaserEditor.addFunctions(list, index)
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
