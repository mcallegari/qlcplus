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
import QtQuick.Dialogs 1.1

import org.qlcplus.classes 1.0
import "."

SidePanel
{
    id: rightSidePanel
    objectName: "funcRightPanel"

    function createFunctionAndEditor(fType)
    {
        var i
        // reset the currently loaded item first
        loaderSource = ""

        if (fType === Function.AudioType)
        {
            var extList = functionManager.audioExtensions
            var exts = qsTr("Audio files") + " ("
            for (i = 0; i < extList.length; i++)
                exts += extList[i] + " "
            exts += ")"

            openFileDialog.fType = fType
            openFileDialog.nameFilters = [ exts, qsTr("All files") + " (*)" ]
            openFileDialog.open()
            return
        }
        else if (fType === Function.VideoType)
        {
            var videoExtList = functionManager.videoExtensions
            var picExtList = functionManager.pictureExtensions
            var vexts = qsTr("Video files") + " ("
            for (i = 0; i < videoExtList.length; i++)
                vexts += videoExtList[i] + " "
            vexts += ")"
            var pexts = qsTr("Picture files") + " ("
            for (i = 0; i < picExtList.length; i++)
                pexts += picExtList[i] + " "
            pexts += ")"

            openFileDialog.fType = fType
            openFileDialog.nameFilters = [ vexts, pexts, qsTr("All files") + " (*)" ]
            openFileDialog.open()
            return
        }


        var newFuncID = functionManager.createFunction(fType)
        var fEditor = functionManager.getEditorResource(newFuncID)
        functionManager.setEditorFunction(newFuncID, false, false)

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
            addFunction.checked = false
            funcEditor.checked = true
        }
    }

    function requestEditor(funcID, funcType)
    {
        if (!(qlcplus.accessMask & App.AC_FunctionEditing))
            return

        // reset the currently loaded item first
        loaderSource = ""
        itemID = funcID
        loaderSource = functionManager.getEditorResource(funcID)
        animatePanel(true)
    }

    onContentLoaded:
    {
        if (item.hasOwnProperty("functionID"))
            item.functionID = itemID
    }

    FileDialog
    {
        id: openFileDialog
        visible: false
        selectMultiple: true

        property int fType

        onAccepted:
        {

            var strArray = []
            for (var i = 0; i < fileUrls.length; i++)
                strArray.push("" + fileUrls[i])

            console.log("File list: " + strArray)

            if (strArray.length === 1)
            {
                itemID = functionManager.createFunction(fType, strArray)
                functionManager.setEditorFunction(itemID, false, false)
                loaderSource = functionManager.getEditorResource(itemID)
            }
            else
            {
                functionManager.createFunction(fType, strArray)
                loaderSource = "qrc:/FunctionManager.qml"
            }

            animatePanel(true)
            addFunction.checked = false
            funcEditor.checked = true
        }
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
            width: iconSize
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
                        functionManager.setEditorFunction(-1, false, false)
                    }
                    animatePanel(checked)
                }
            }
            IconButton
            {
                id: addFunction
                visible: qlcplus.accessMask & App.AC_FunctionEditing
                z: 2
                width: iconSize
                height: iconSize
                imgSource: "qrc:/add.svg"
                tooltip: qsTr("Add a new function")
                checkable: true

                AddFunctionMenu
                {
                    id: addFunctionMenu
                    visible: addFunction.checked
                    x: -width

                    onEntryClicked:
                    {
                        close()
                        createFunctionAndEditor(fType)
                    }
                    onClosed: addFunction.checked = false
                }
            }
            IconButton
            {
                id: removeFunction
                z: 2
                width: iconSize
                height: iconSize
                imgSource: "qrc:/remove.svg"
                tooltip: qsTr("Delete the selected functions")
                counter: functionManager.selectionCount && !functionManager.isEditing
                onClicked:
                {
                    var selNames = functionManager.selectedFunctionsName()
                    //console.log(selNames)
                    deleteItemsPopup.message = qsTr("Are you sure you want to delete the following functions ?") + "\n" + selNames
                    deleteItemsPopup.open()
                }

                CustomPopupDialog
                {
                    id: deleteItemsPopup
                    title: qsTr("Delete functions")
                    onAccepted: functionManager.deleteFunctions(functionManager.selectedFunctionsID())
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
                    renameFuncPopup.baseName = selNames[0]
                    renameFuncPopup.functionIDs = functionManager.selectedFunctionsID()
                    renameFuncPopup.open()
                }

                PopupRenameFunctions
                {
                    id: renameFuncPopup
                    title: qsTr("Rename functions")
                }
            }

            IconButton
            {
                z: 2
                width: iconSize
                height: iconSize
                faSource: FontAwesome.fa_sitemap
                faColor: UISettings.fgMedium
                tooltip: qsTr("Show function usage")
                counter: functionManager.selectionCount
                onClicked:
                {
                    var idList = functionManager.selectedFunctionsID()
                    loaderSource = ""
                    itemID = idList[0]
                    loaderSource = "qrc:/UsageList.qml"
                }
            }

            IconButton
            {
                id: sceneDump
                z: 2
                width: iconSize
                height: iconSize
                imgSource: "qrc:/dmxdump.svg"
                tooltip: qsTr("Dump on a new Scene")
                counter: contextManager ? contextManager.dumpValuesCount && (qlcplus.accessMask & App.AC_FunctionEditing) : 0

                onClicked:
                {
                    if (dmxDumpDialog.show)
                    {
                        dmxDumpDialog.sceneID = -1
                        dmxDumpDialog.open()
                        dmxDumpDialog.focusEditItem()
                    }
                    else
                    {
                        contextManager.dumpDmxChannels("")
                        loaderSource = "qrc:/FunctionManager.qml"
                        animatePanel(true)
                        funcEditor.checked = true
                    }
                }

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
                        label: contextManager ? contextManager.dumpValuesCount : ""
                        fontSize: height
                    }
                }

                MouseArea
                {
                    id: dumpDragArea
                    anchors.fill: parent
                    propagateComposedEvents: true
                    drag.target: dumpDragItem
                    drag.threshold: 10
                    onClicked: mouse.accepted = false

                    property bool dragActive: drag.active

                    onDragActiveChanged:
                    {
                        console.log("Drag active changed: " + dragActive)
                        if (dragActive == false)
                        {
                            dumpDragItem.Drag.drop()
                            dumpDragItem.parent = sceneDump
                            dumpDragItem.x = 0
                            dumpDragItem.y = 0
                        }
                        dumpDragItem.Drag.active = dragActive
                    }
                }

                Item
                {
                    id: dumpDragItem
                    visible: dumpDragArea.drag.active

                    Drag.source: dumpDragItem
                    Drag.keys: [ "dumpValues" ]

                    function itemDropped(id, name)
                    {
                        console.log("Dump values dropped on " + id)
                        dmxDumpDialog.sceneID = id
                        dmxDumpDialog.sceneName = name
                        dmxDumpDialog.open()
                        dmxDumpDialog.focusEditItem()
                    }

                    Rectangle
                    {
                        width: UISettings.iconSizeMedium
                        height: width
                        radius: width / 4
                        color: "red"

                        RobotoText
                        {
                            anchors.centerIn: parent
                            label: contextManager ? contextManager.dumpValuesCount : ""
                        }
                    }
                }

                PopupDMXDump
                {
                    id: dmxDumpDialog

                    property int sceneID: -1

                    onAccepted:
                    {
                        if (sceneID == -1)
                            contextManager.dumpDmxChannels(sceneName, getChannelsMask())
                        else
                            contextManager.dumpDmxChannels(sceneID, getChannelsMask())
                        loaderSource = "qrc:/FunctionManager.qml"
                        animatePanel(true)
                        funcEditor.checked = true
                    }
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
                faSource: FontAwesome.fa_times
                tooltip: qsTr("Reset dump channels") + " (CTRL+R)"
                onClicked: contextManager.resetDumpValues()
            }
        }
    }
}

