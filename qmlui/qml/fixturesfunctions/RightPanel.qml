/*
  Q Light Controller Plus
  RightPanel.qml

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

import com.qlcplus.classes 1.0
import "."

SidePanel
{
    id: rightSidePanel

    property int editorFuncID: -1

    function createFunctionAndEditor(fType, fEditor)
    {
        var newFuncID = functionManager.createFunction(fType)
        functionManager.setEditorFunction(newFuncID)
        if (fType === Function.Show)
        {
            showManager.currentShowID = newFuncID
            mainView.switchToContext("SHOWMGR", fEditor)
        }
        else
        {
            itemID = newFuncID
            loaderSource = fEditor
            animatePanel(true)
            addFunctionMenu.visible = false
            addFunction.checked = false
            funcEditor.checked = true
        }
    }

    onContentLoaded: item.functionID = itemID

    Rectangle
    {
        width: collapseWidth
        height: parent.height
        color: "transparent"
        z: 2

        Column
        {
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 3

            IconButton
            {
                id: funcEditor
                z: 2
                width: iconSize
                height: iconSize
                imgSource: "qrc:/functions.svg"
                tooltip: qsTr("Function Manager")
                checkable: true
                onToggled:
                {
                    if (checked)
                        loaderSource = "qrc:/FunctionManager.qml"
                    animatePanel(checked)
                }
            }
            IconButton
            {
                id: addFunction
                z: 2
                width: iconSize
                height: iconSize
                imgSource: "qrc:/add.svg"
                tooltip: qsTr("Add a new function")
                checkable: true
                onToggled: addFunctionMenu.visible = !addFunctionMenu.visible

                AddFunctionMenu
                {
                    id: addFunctionMenu
                    visible: false
                    x: -width

                    onEntryClicked: createFunctionAndEditor(fType, fEditor)
                }
            }
            IconButton
            {
                id: removeFunction
                z: 2
                width: iconSize
                height: iconSize
                imgSource: "qrc:/remove.svg"
                tooltip: qsTr("Remove the selected functions")
                counter: functionManager.selectionCount
                onClicked:
                {
                    var selNames = functionManager.selectedFunctionsName()
                    console.log(selNames)

                    actionManager.requestActionPopup(ActionManager.DeleteFunctions,
                                                     qsTr("Are you sure you want to remove the following functions ?\n" + selNames),
                                                     ActionManager.OK | ActionManager.Cancel,
                                                     functionManager.selectedFunctionsID())
                }
            }
            IconButton
            {
                id: sceneDump
                objectName: "dumpButton"
                z: 2
                width: iconSize
                height: iconSize
                imgSource: "qrc:/dmxdump.svg"
                tooltip: qsTr("Dump to a Scene")
                visible: false
                onClicked:
                {
                    contextManager.dumpDmxChannels()
                    loaderSource = "qrc:/FunctionManager.qml"
                    animatePanel(true)
                    funcEditor.checked = true
                }
            }
            IconButton
            {
                id: previewFunc
                objectName: "previewButton"
                z: 2
                width: iconSize
                height: iconSize
                imgSource: "qrc:/play.svg"
                tooltip: qsTr("Function Preview")
                checkable: true
                counter: functionManager.selectionCount
                onToggled: functionManager.setPreview(checked)
            }
        }
    }
}

