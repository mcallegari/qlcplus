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

    function saveBeforeExit()
    {
        saveFirstPopup.action = "#EXIT"
        saveFirstPopup.open()
    }

    FileDialog
    {
        id: openDialog
        visible: false
        title: qsTr("Open a project")
        folder: "file://" + qlcplus.workingPath
        nameFilters: [ qsTr("Project files") + " (*.qxw)", qsTr("All files") + " (*)" ]

        onAccepted:
        {
            console.log("You chose: " + fileUrl)
            qlcplus.loadWorkspace(fileUrl)
            console.log("Folder: " + folder.toString())
            qlcplus.workingPath = folder.toString()
        }
    }

    FileDialog
    {
        id: importDialog
        visible: false
        title: qsTr("Import from project")
        folder: "file://" + qlcplus.workingPath
        nameFilters: [ qsTr("Project files") + " (*.qxw)", qsTr("All files") + " (*)" ]

        onAccepted:
        {
            if (qlcplus.loadImportWorkspace(fileUrl) === true)
                importLoader.source = "qrc:/PopupImportProject.qml"
        }
    }

    FileDialog
    {
        id: saveDialog
        visible: false
        title: qsTr("Save project as...")
        selectExisting: false
        nameFilters: [ qsTr("Project files") + " (*.qxw)", qsTr("All files") + " (*)" ]

        onAccepted:
        {
            console.log("You chose: " + fileUrl)
            qlcplus.saveWorkspace(fileUrl)

            if (saveFirstPopup.action == "#EXIT")
                qlcplus.exit()
        }
    }

    CustomPopupDialog
    {
        id: saveFirstPopup
        title: qsTr("Your project has changes")
        message: qsTr("Do you wish to save the current project first?\nChanges will be lost if you don't save them.")
        standardButtons: Dialog.Yes | Dialog.No | Dialog.Cancel

        property string action: ""

        onClicked:
        {
            if (role === Dialog.Yes)
            {
                if (qlcplus.fileName())
                {
                    qlcplus.saveWorkspace(qlcplus.fileName())
                    if (action == "#EXIT")
                        qlcplus.exit()
                }
                else
                {
                    saveDialog.open()
                    if (action == "#EXIT")
                        return
                }
            }
            else if (role === Dialog.No)
            {
                if (action == "#OPEN")
                    openDialog.open()
                else if (action == "#NEW")
                    qlcplus.newWorkspace()
                else if (action == "#EXIT")
                    qlcplus.exit()
                else
                    qlcplus.loadWorkspace(action)
            }
            else if (role === Dialog.Cancel)
            {
                console.log("Cancel clicked")
            }

            action = ""
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
                if (qlcplus.docModified)
                {
                    saveFirstPopup.action = "#NEW"
                    saveFirstPopup.open()
                }
                else
                    qlcplus.newWorkspace()

                menuRoot.close()
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
                if (qlcplus.docModified)
                {
                    saveFirstPopup.action = "#OPEN"
                    saveFirstPopup.open()
                }
                else
                    openDialog.open()

                menuRoot.close()
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
                                    if (qlcplus.docModified)
                                    {
                                        saveFirstPopup.open()
                                        saveFirstPopup.action = entryText
                                    }
                                    else
                                    {
                                        menuRoot.close()
                                        qlcplus.loadWorkspace(entryText)
                                    }
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
                if (qlcplus.fileName())
                    qlcplus.saveWorkspace(qlcplus.fileName())
                else
                    saveDialog.open()

                menuRoot.close()
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
                saveDialog.open()
                menuRoot.close()
            }
        }

        ContextMenuEntry
        {
            id: fileImport
            imgSource: "qrc:/import.svg"
            entryText: qsTr("Import from project")
            onEntered: submenuItem = null

            onClicked:
            {
                importDialog.open()
                menuRoot.close()
            }

            Loader
            {
                id: importLoader
                onLoaded: item.open()

                Connections
                {
                    target: importLoader.item
                    onClose: importLoader.source = ""
                }
            }
        }

        Row
        {
            height: UISettings.iconSizeDefault

            ContextMenuEntry
            {
                width: actionsMenuEntries.width / 2
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
                width: actionsMenuEntries.width / 2
                imgSource: "qrc:/redo.svg"
                entryText: qsTr("Redo")
                onEntered: submenuItem = null

                onClicked:
                {
                    menuRoot.close()
                    tardis.redoAction()
                }
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

        ContextMenuEntry
        {
            imgSource: "qrc:/global.svg"
            entryText: qsTr("Language")
            onEntered: submenuItem = languageMenu

            onClicked:
            {
                if (Qt.platform.os === "android")
                    submenuItem = languageMenu
            }

            Rectangle
            {
                id: languageMenu
                x: menuRoot.width
                y: -height + parent.height
                width: languageColumn.width
                height: languageColumn.height
                color: UISettings.bgStrong
                visible: submenuItem === languageMenu

                Column
                {
                    id: languageColumn

                    ContextMenuEntry
                    {
                        imgSource: "qrc:/flag_ca.svg"
                        iconWidth: UISettings.iconSizeDefault * 1.5
                        entryText: qsTr("Catalan")
                        onClicked: { qlcplus.setLanguage("ca_ES"); menuRoot.close() }
                    }
                    ContextMenuEntry
                    {
                        imgSource: "qrc:/flag_nl.svg"
                        iconWidth: UISettings.iconSizeDefault * 1.5
                        entryText: qsTr("Dutch")
                        onClicked: { qlcplus.setLanguage("nl_NL"); menuRoot.close() }
                    }
                    ContextMenuEntry
                    {
                        imgSource: "qrc:/flag_uk_us.svg"
                        iconWidth: UISettings.iconSizeDefault * 1.5
                        entryText: qsTr("English")
                        onClicked: { qlcplus.setLanguage("en_EN"); menuRoot.close() }
                    }
                    ContextMenuEntry
                    {
                        imgSource: "qrc:/flag_fr.svg"
                        iconWidth: UISettings.iconSizeDefault * 1.5
                        entryText: qsTr("French")
                        onClicked: { qlcplus.setLanguage("fr_FR"); menuRoot.close() }
                    }
                    ContextMenuEntry
                    {
                        imgSource: "qrc:/flag_de.svg"
                        iconWidth: UISettings.iconSizeDefault * 1.5
                        entryText: qsTr("German")
                        onClicked: { qlcplus.setLanguage("de_DE"); menuRoot.close() }
                    }
                    ContextMenuEntry
                    {
                        imgSource: "qrc:/flag_it.svg"
                        iconWidth: UISettings.iconSizeDefault * 1.5
                        entryText: qsTr("Italian")
                        onClicked: { qlcplus.setLanguage("it_IT"); menuRoot.close() }
                    }
                    ContextMenuEntry
                    {
                        imgSource: "qrc:/flag_jp.svg"
                        iconWidth: UISettings.iconSizeDefault * 1.5
                        entryText: qsTr("Japanese")
                        onClicked: { qlcplus.setLanguage("ja_JP"); menuRoot.close() }
                    }
                    ContextMenuEntry
                    {
                        imgSource: "qrc:/flag_ru.svg"
                        iconWidth: UISettings.iconSizeDefault * 1.5
                        entryText: qsTr("Russian")
                        onClicked: { qlcplus.setLanguage("ru_RU"); menuRoot.close() }
                    }
                    ContextMenuEntry
                    {
                        imgSource: "qrc:/flag_es.svg"
                        iconWidth: UISettings.iconSizeDefault * 1.5
                        entryText: qsTr("Spanish")
                        onClicked: { qlcplus.setLanguage("es_ES"); menuRoot.close() }
                    }
                }
            }
        }

        ContextMenuEntry
        {
            id: info
            imgSource: "qrc:/info.svg"
            entryText: qsTr("About")
            onEntered: submenuItem = null
            onClicked:
            {
                menuRoot.close()
                infoPopup.open()
            }

            PopupAbout
            {
                id: infoPopup
            }
        }
    }
}


