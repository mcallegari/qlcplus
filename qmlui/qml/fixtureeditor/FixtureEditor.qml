/*
  Q Light Controller Plus
  FixtureEditor.qml

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
import QtQuick.Dialogs

import org.qlcplus.classes 1.0
import "."

Rectangle
{
    id: fixtureEditorView
    visible: true
    width: 800
    height: 600
    anchors.fill: parent
    color: UISettings.bgMedium

    function setDimScreen(enable)
    {
        dimScreen.visible = enable
    }

    FontLoader
    {
        source: "qrc:/RobotoCondensed-Regular.ttf"
    }

    // Load the "FontAwesome" font for the monochrome icons
    FontLoader
    {
        source: "qrc:/FontAwesome7-Free-Solid-900.otf"
    }

    property string dialogTitle
    property url dialogCurrentFolder: fixtureEditor.workingPath
    property url dialogSelectedFile
    property var dialogNameFilters: [ qsTr("Fixture definition files") + " (*.qxf)", qsTr("All files") + " (*)" ]
    property int dialogFileMode: FileDialog.OpenFile
    property int dialogOpMode: App.OpenMode

    function openDialog(opMode)
    {
        var saveFilename = ""

        dialogOpMode = opMode
        switch (dialogOpMode)
        {
            case App.OpenMode:
                dialogTitle = qsTr("Open a fixture definition")
                dialogFileMode = FileDialog.OpenFile
            break
            case App.SaveMode:
            case App.SaveAsMode:
                dialogTitle = qsTr("Save definition as...")
                dialogFileMode = FileDialog.SaveFile
                saveFilename = editor.editorView.fileName
                if (saveFilename === "")
                    saveFilename = editor.editorView.manufacturer.replace(" ", "-") + "-" +
                                   editor.editorView.model.replace(" ", "-")
            break
        }

        if (Qt.platform.os === "linux")
        {
            if (saveFilename !== "")
                customDialog.selectedFile = saveFilename
            customDialog.open()
        }
        else
        {
            if (saveFilename !== "")
                nativeDialog.selectedFile = saveFilename
            nativeDialog.open()
        }
    }

    function handleAccept()
    {
        console.log("Selected file: " + dialogSelectedFile)

        switch (dialogOpMode)
        {
            case App.OpenMode:
            {
                fixtureEditor.workingPath = dialogCurrentFolder.toString()

                if (fixtureEditor.loadDefinition(dialogSelectedFile) === false)
                {
                    editor.visible = false
                    messagePopup.message = qsTr("An error occurred while loading the selected file.<br>" +
                                                "It could be invalid or corrupted.")
                    messagePopup.open()
                }
            }
            break
            case App.SaveMode:
            case App.SaveAsMode:
            {
                editor.save(dialogSelectedFile)
                qlcplus.reloadFixture(dialogSelectedFile)
            }
            break
        }
    }

    FileDialog
    {
        id: nativeDialog
        title: dialogTitle
        fileMode: dialogFileMode
        currentFolder: "file:///" + dialogCurrentFolder
        nameFilters: dialogNameFilters

        onAccepted:
        {
            dialogSelectedFile = selectedFile
            dialogCurrentFolder = currentFolder
            handleAccept()
        }
    }

    PopupFolderBrowser
    {
        id: customDialog
        title: dialogTitle
        currentFolder: dialogCurrentFolder
        nameFilters: dialogNameFilters
        standardButtons: Dialog.Cancel |
            ((dialogOpMode === App.SaveMode | dialogOpMode === App.SaveAsMode) ? Dialog.Save : Dialog.Open)

        onAccepted:
        {
            dialogSelectedFile = currentFolder + folderSeparator() + selectedFile
            dialogCurrentFolder = currentFolder
            handleAccept()
        }
    }

    CustomPopupDialog
    {
        id: messagePopup
        standardButtons: Dialog.Ok
        title: qsTr("Error")
        onAccepted: close()
    }

    CustomPopupDialog
    {
        id: saveBeforeExitPopup
        title: qsTr("Warning")
        message: editRef ? qsTr("Do you wish to save the following definition first?<br>" +
                 "<i>" + editRef.manufacturer + " - " + editRef.model + "</i><br>" +
                 "Changes will be lost if you don't save them.") : ""
        standardButtons: Dialog.Yes | Dialog.No | Dialog.Cancel

        property var editRef

        onClicked:
            function (role)
            {
                if (role === Dialog.Yes)
                {
                    if (editRef.fileName === "")
                    {
                        dialogCurrentFolder = fixtureEditor.workingPath
                        openDialog(App.SaveMode)
                    }
                    else
                    {
                        editRef.save("")
                    }
                }
                else if (role === Dialog.No)
                {
                    fixtureEditor.deleteEditor(editRef.id)
                }
                else if (role === Dialog.Cancel)
                {
                    return
                }

                close()
                checkBeforeExit()
            }
    }

    function checkBeforeExit()
    {
        var modifiedCount = false

        for (var i = 0; i < editorsRepeater.count; i++)
        {
            var iEdit = editorsRepeater.model[i].cRef
            if (iEdit.isModified)
            {
                console.log("Editor " + i + " is modified")
                saveBeforeExitPopup.editRef = iEdit
                saveBeforeExitPopup.open()
                modifiedCount = true
                break
            }
        }

        if (modifiedCount === false)
            qlcplus.closeFixtureEditor()
    }

    Rectangle
    {
        id: mainToolbar
        width: parent.width
        height: UISettings.iconSizeDefault
        z: 50
        gradient: Gradient
        {
            GradientStop { position: 0; color: UISettings.toolbarStartMain }
            GradientStop { position: 1; color: UISettings.toolbarEnd }
        }

        RowLayout
        {
            spacing: 5
            anchors.fill: parent

            ButtonGroup { id: menuBarGroup }

            MenuBarEntry
            {
                faSource: FontAwesome.fa_chevron_left
                faColor: UISettings.fgLight
                entryText: qsTr("Back to QLC+")
                autoExclusive: false
                checkable: false
                onClicked: checkBeforeExit()
            }
            MenuBarEntry
            {
                imgSource: "qrc:/filenew.svg"
                entryText: qsTr("New definition")
                autoExclusive: false
                checkable: false
                onClicked: fixtureEditor.createDefinition()
            }
            MenuBarEntry
            {
                id: fileOpen
                imgSource: "qrc:/fileopen.svg"
                entryText: qsTr("Open definition")
                onClicked:
                {
                    dialogCurrentFolder = fixtureEditor.workingPath
                    openDialog(App.OpenMode)
                }
                autoExclusive: false
                checkable: false
            }
            MenuBarEntry
            {
                imgSource: "qrc:/filesave.svg"
                entryText: qsTr("Save definition")
                autoExclusive: false
                checkable: false
                onClicked:
                {
                    if (editor.editorView.fileName === "")
                    {
                        dialogCurrentFolder = fixtureEditor.workingPath
                        openDialog(App.SaveMode)
                    }
                    else
                    {
                        editor.save("")
                    }
                }
            }
            MenuBarEntry
            {
                imgSource: "qrc:/filesaveas.svg"
                entryText: qsTr("Save definition as...")
                autoExclusive: false
                checkable: false
                onClicked:
                {
                    dialogCurrentFolder = fixtureEditor.workingPath + "/" + editor.editorView.fileName
                    openDialog(App.SaveAsMode)
                }
            }
            // filler
            Rectangle
            {
                Layout.fillWidth: true
                color: "transparent"
            }
        }
    }

    Rectangle
    {
        id: feToolbar
        y: mainToolbar.height
        width: parent.width
        height: UISettings.iconSizeMedium
        z: 10
        gradient: Gradient
        {
            GradientStop { position: 0; color: UISettings.toolbarStartSub }
            GradientStop { position: 1; color: UISettings.toolbarEnd }
        }

        RowLayout
        {
            anchors.fill: parent
            spacing: 5
            ButtonGroup { }

            Repeater
            {
                id: editorsRepeater
                model: fixtureEditor ? fixtureEditor.editorsList : null

                onItemAdded: (index,item) => item.clicked()

                delegate:
                    MenuBarEntry
                    {
                        property string fxManuf: modelData.cRef.manufacturer
                        property string fxModel: modelData.cRef.model

                        //width: contentItem.width + (modIcon.visible ? modIcon.width + 8 : 0)

                        entryText: (fxManuf ? fxManuf : qsTr("Unknown")) + " - " + (fxModel ? fxModel : qsTr("Unknown"))
                        imgSource: modelData.cRef.isModified ? "qrc:/filesave.svg" : ""
                        iconSize: height * 0.75
                        checkable: true
                        autoExclusive: true
                        padding: 5
                        rightPadding: height * 1.1

                        onClicked:
                        {
                            editor.editorId = modelData.id
                            editor.editorView = modelData.cRef
                            editor.initialize()
                            checked = true
                        }

                        GenericButton
                        {
                            width: height
                            height: parent.height * 0.9
                            anchors.right: parent.right
                            border.color: UISettings.bgMedium
                            useFontawesome: true
                            label: FontAwesome.fa_xmark
                            onClicked: fixtureEditor.deleteEditor(modelData.id)
                        }
                    }
            } // Repeater
            Rectangle
            {
                Layout.fillWidth: true
                height: parent.height
                color: "transparent"
            }
        } // RowLayout
    } // Rectangle

    EditorView
    {
        id: editor
        y: mainToolbar.height + feToolbar.height
        width: parent.width
        height: parent.height - (mainToolbar.height + feToolbar.height)
    }

    /* Rectangle covering the whole window to
     * have a dimmered background for popups */
    Rectangle
    {
        id: dimScreen
        anchors.fill: parent
        visible: false
        z: 99
        color: Qt.rgba(0, 0, 0, 0.5)
    }
}
