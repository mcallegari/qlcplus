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

import QtQuick 2.2
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.1
import QtQuick.Dialogs 1.3
import org.qlcplus.classes 1.0
import "."

Rectangle
{
    visible: true
    width: 800
    height: 600
    anchors.fill: parent
    color: UISettings.bgMedium

    FontLoader
    {
        source: "qrc:/RobotoCondensed-Regular.ttf"
    }

    // Load the "FontAwesome" font for the monochrome icons
    FontLoader
    {
        source: "qrc:/FontAwesome.otf"
    }

    FileDialog
    {
        id: openDialog
        visible: false
        title: qsTr("Open a fixture definition")
        nameFilters: [ qsTr("Fixture definition files") + " (*.qxf)", qsTr("All files") + " (*)" ]

        onAccepted:
        {
            fixtureEditor.workingPath = folder.toString()
            if (fixtureEditor.loadDefinition(fileUrl) === false)
            {
                editor.visible = false
                messagePopup.message = qsTr("An error occurred while loading the selected file.<br>" +
                                            "It could be invalid or corrupted.")
                messagePopup.open()
            }
        }
    }

    FileDialog
    {
        id: saveDialog
        visible: false
        title: qsTr("Save definition as...")
        selectExisting: false
        nameFilters: [ qsTr("Fixture definition files") + " (*.qxf)", qsTr("All files") + " (*)" ]
        defaultSuffix: "qxf"
        //fileMode: FileDialog.SaveFile

        onAccepted:
        {
            console.log("You chose: " + fileUrl)
            editor.save(fileUrl)
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
        {
            if (role === Dialog.Yes)
            {
                if (editRef.fileName === "")
                {
                    saveDialog.folder = fixtureEditor.workingPath
                    //saveDialog.currentFile = "file:///" + editor.editorView.fileName
                    saveDialog.open()
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
                imgSource: "qrc:/arrow-right.svg"
                entryText: qsTr("Back to QLC+")
                iconRotation: 180
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
                    openDialog.folder = fixtureEditor.workingPath
                    openDialog.open()
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
                        saveDialog.folder = fixtureEditor.workingPath
                        //saveDialog.currentFile = "file:///" + editor.editorView.fileName
                        saveDialog.open()
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
                    saveDialog.folder = fixtureEditor.workingPath + "/" + editor.editorView.fileName
                    //saveDialog.currentFile = "file:///" + editor.editorView.fileName
                    saveDialog.open()
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

                onItemAdded: item.clicked()

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

                        onClicked:
                        {
                            editor.editorId = modelData.id
                            editor.editorView = modelData.cRef
                            editor.initialize()
                            checked = true
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
