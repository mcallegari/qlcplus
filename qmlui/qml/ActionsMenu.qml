/*
  Q Light Controller Plus
  ActionsMenu.qml

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
import QtQuick.Controls 2.1
import QtQuick.Dialogs 1.2

import "."

Popup
{
    id: menuRoot
    padding: 0

    property Item submenuItem: null

    onClosed: submenuItem = null

    FileDialog
    {
        id: openDialog
        visible: false
        title: qsTr("Open a workspace")
        folder: "file://" + qlcplus.workingPath
        nameFilters: [ qsTr("Workspace files") + " (*.qxw)", qsTr("All files") + " (*)" ]

        onAccepted:
        {
            console.log("You chose: " + openDialog.fileUrl)
            qlcplus.loadWorkspace(openDialog.fileUrl)
            console.log("Folder: " + folder.toString())
            qlcplus.workingPath = folder.toString()
        }
        onRejected:
        {
            console.log("Canceled")
        }
    }

    FileDialog
    {
        id: saveDialog
        visible: false
        title: qsTr("Save workspace as")
        selectExisting: false
        nameFilters: [ qsTr("Workspace files") + " (*.qxw)", qsTr("All files") + " (*)" ]

        onAccepted:
        {
            console.log("You chose: " + saveDialog.fileUrl)
            qlcplus.saveWorkspace(saveDialog.fileUrl)
        }
        onRejected:
        {
            console.log("Canceled")
        }
    }

    CustomPopupDialog
    {
        id: saveFirstPopup
        title: qsTr("Your project has changes")
        message: qsTr("Do you wish to save the current workspace first ?\nChanges will be lost if you don't save them.")
        standardButtons: Dialog.Yes | Dialog.No | Dialog.Cancel

        property bool openAction: false

        onClicked:
        {
            if (role === Dialog.Yes)
            {
                if (qlcplus.fileName())
                    qlcplus.saveWorkspace(qlcplus.fileName())
                else
                {
                    //saveDialog.visible = true
                    saveDialog.open()
                }
            }
            else if (role === Dialog.No)
            {
                if (openAction)
                    openDialog.open()
                else
                    qlcplus.newWorkspace()
            }
            else if (role === Dialog.Cancel)
            {
                console.log("Cancel clicked")
            }
        }
    }

    background:
        Rectangle
        {
            //radius: 2
            border.width: 1
            border.color: UISettings.bgStronger
            color: UISettings.bgStrong
        }

    Column
    {
        id: actionsMenuEntries
        ContextMenuEntry
        {
            id: fileNew
            imgSource: "qrc:/filenew.svg"
            entryText: qsTr("New project")
            onClicked:
            {
                menuRoot.close()

                if (qlcplus.docModified)
                {
                    saveFirstPopup.openAction = false
                    saveFirstPopup.open()
                }
                else
                    qlcplus.newWorkspace()
            }
            onEntered: submenuItem = null
        }
        ContextMenuEntry
        {
            id: fileOpen
            imgSource: "qrc:/fileopen.svg"
            entryText: qsTr("Open project")
            onClicked:
            {
                menuRoot.close()

                if (qlcplus.docModified)
                {
                    saveFirstPopup.openAction = true
                    saveFirstPopup.open()
                }
                else
                    openDialog.open()
            }
            onEntered: submenuItem = recentMenu

            Rectangle
            {
                id: recentMenu
                x: menuRoot.width
                width: recentColumn.width
                height: recentColumn.height
                color: UISettings.bgStrong
                visible: submenuItem === recentMenu

                Column
                {
                    id: recentColumn
                    Repeater
                    {
                        model: qlcplus.recentFiles
                        delegate:
                            ContextMenuEntry
                            {
                                entryText: modelData
                                onClicked:
                                {
                                    menuRoot.close()
                                    qlcplus.loadWorkspace(entryText)
                                }
                            }
                        }
                }
            }
        }
        ContextMenuEntry
        {
            id: fileSave
            imgSource: "qrc:/filesave.svg"
            entryText: qsTr("Save project")
            onEntered: submenuItem = null

            onClicked:
            {
                menuRoot.close()

                if (qlcplus.fileName())
                    qlcplus.saveWorkspace(qlcplus.fileName())
                else
                    saveDialog.open()
            }
        }
        ContextMenuEntry
        {
            id: fileSaveAs
            imgSource: "qrc:/filesaveas.svg"
            entryText: qsTr("Save project as...")
            onEntered: submenuItem = null

            onClicked:
            {
                menuRoot.close()
                saveDialog.open()
            }
        }
        ContextMenuEntry
        {
            id: undo
            imgSource: "qrc:/undo.svg"
            entryText: qsTr("Undo")
            onEntered: submenuItem = null

            onClicked:
            {
                menuRoot.close()
                tardis.undoAction()
            }
        }
        ContextMenuEntry
        {
            imgSource: "qrc:/network.svg"
            entryText: qsTr("Network")
            onEntered: submenuItem = networkMenu

            onClicked:
            {
                if (Qt.platform.os === "android")
                    submenuItem = networkMenu
            }

            Rectangle
            {
                id: networkMenu
                x: menuRoot.width
                width: networkColumn.width
                height: networkColumn.height
                color: UISettings.bgStrong
                visible: submenuItem === networkMenu

                Column
                {
                    id: networkColumn

                    ContextMenuEntry
                    {
                        id: startServer
                        entryText: qsTr("Server setup")

                        onClicked:
                        {
                            menuRoot.close()
                            pNetServer.open()
                        }

                        PopupNetworkServer
                        {
                            id: pNetServer
                            implicitWidth: Math.min(UISettings.bigItemHeight * 4, mainView.width / 3)
                        }
                    }

                    ContextMenuEntry
                    {
                        id: connectToServer
                        entryText: qsTr("Client setup")

                        onClicked:
                        {
                            menuRoot.close()
                            pNetClient.open()
                        }

                        PopupNetworkClient
                        {
                            id: pNetClient
                            implicitWidth: Math.min(UISettings.bigItemHeight * 4, mainView.width / 3)
                        }
                    }
                }
            }
        }

        ContextMenuEntry
        {
            imgSource: "qrc:/diptool.svg"
            entryText: qsTr("Address tool")
            onEntered: submenuItem = null
            onClicked:
            {
                close()
                addrToolDialog.open()
            }

            CustomPopupDialog
            {
                id: addrToolDialog
                title: qsTr("DMX Address tool")
                standardButtons: Dialog.Close

                contentItem:
                    DMXAddressTool { }
            }

        }

        ContextMenuEntry
        {
            id: fullScreen
            imgSource: "qrc:/fullscreen.svg"
            entryText: qsTr("Toggle fullscreen")
            onEntered: submenuItem = null
            onClicked:
            {
                menuRoot.close()
                qlcplus.toggleFullscreen()
            }
        }
    }
}


