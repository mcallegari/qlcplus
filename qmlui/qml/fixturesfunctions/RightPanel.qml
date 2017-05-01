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
import QtQuick.Layouts 1.0

import com.qlcplus.classes 1.0
import "."

SidePanel
{
    id: rightSidePanel
    objectName: "funcRightPanel"

    function createFunctionAndEditor(fType)
    {
        // reset the currently loaded item first
        loaderSource = ""

        var fEditor = functionManager.getEditorResource(fType)
        var newFuncID = functionManager.createFunction(fType)
        functionManager.setEditorFunction(newFuncID, false)

        if (fType === Function.ShowType)
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

    function requestEditor(funcID, funcType)
    {
        // reset the currently loaded item first
        loaderSource = ""
        itemID = funcID
        loaderSource = functionManager.getEditorResource(funcType)
        animatePanel(true)
    }

    onContentLoaded:
    {
        if (item.hasOwnProperty("functionID"))
            item.functionID = itemID
    }

    Rectangle
    {
        width: collapseWidth
        height: parent.height
        color: "transparent"
        z: 2

        ColumnLayout
        {
            anchors.horizontalCenter: parent.horizontalCenter
            height: parent.height
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
                    else
                    {
                        functionManager.selectFunctionID(-1, false)
                        functionManager.setEditorFunction(-1, false)
                    }
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

                    onEntryClicked: createFunctionAndEditor(fType)
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
                counter: functionManager.selectionCount && !functionManager.isEditing
                onClicked:
                {
                    var selNames = functionManager.selectedFunctionsName()
                    console.log(selNames)

                    actionManager.requestActionPopup(ActionManager.DeleteFunctions,
                                                     qsTr("Are you sure you want to remove the following functions ?\n") + selNames,
                                                     ActionManager.OK | ActionManager.Cancel,
                                                     functionManager.selectedFunctionsID())
                }
            }
            IconButton
            {
                id: renameFunction
                z: 2
                width: iconSize
                height: iconSize
                imgSource: "qrc:/rename.svg"
                tooltip: qsTr("Rename the selected functions")
                counter: functionManager.selectionCount && !functionManager.isEditing
                onClicked:
                {
                    var selNames = functionManager.selectedFunctionsName()
                    var dataArray = functionManager.selectedFunctionsID()
                    // push the first selected name at the beginning of the array
                    dataArray.unshift(selNames[0])

                    actionManager.requestActionPopup(ActionManager.RenameFunctions,
                                                     "qrc:/PopupTextRequest.qml",
                                                     ActionManager.OK | ActionManager.Cancel, dataArray)
                }
            }
            IconButton
            {
                id: sceneDump
                z: 2
                width: iconSize
                height: iconSize
                imgSource: "qrc:/dmxdump.svg"
                tooltip: qsTr("Dump to a Scene")
                counter: functionManager.dumpValuesCount

                Rectangle
                {
                    x: -3
                    y: -3
                    width: sceneDump.width * 0.4
                    height: width
                    color: "red"
                    border.width: 1
                    border.color: UISettings.fgMain
                    radius: 3
                    clip: true

                    RobotoText
                    {
                        anchors.centerIn: parent
                        height: parent.height * 0.7
                        label: functionManager.dumpValuesCount
                        fontSize: height
                    }

                }

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

            /* filler object */
            Rectangle
            {
                Layout.fillHeight: true
                width: iconSize
                color: "transparent"
            }

            IconButton
            {
                z: 2
                width: iconSize
                height: iconSize
                imgSource: "qrc:/reset.svg"
                tooltip: qsTr("Reset dump channels") + " (CTRL+R)"
                onClicked: contextManager.resetDumpValues()
            }
        }
    }
}

