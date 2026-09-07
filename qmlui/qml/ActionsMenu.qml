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

import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts

import org.qlcplus.classes 1.0
import "."

Popup
{
    id: menuRoot
    padding: 0
    contentWidth: requiredMenuWidth()
    contentHeight: actionsMenuEntries.implicitHeight
    width: contentWidth
    height: contentHeight

    property var submenuItem: null
    property int flagSize: UISettings.iconSizeDefault * 1.5

    onSubmenuItemChanged:
    {
        if (submenuItem !== recentFilesPopup)
            recentFilesPopup.close()
        if (submenuItem !== networkMenuPopup)
            networkMenuPopup.close()
        if (submenuItem !== languageMenuPopup)
            languageMenuPopup.close()
    }

    onClosed:
    {
        submenuItem = null
        closeSubmenus()
        openFileCoordinator.completeDismissal()
        recentFileCoordinator.completeDismissal()
    }

    function showSubmenu(popup)
    {
        submenuItem = popup
        if (!popup.opened)
            popup.open()
    }

    function closeSubmenus()
    {
        recentFilesPopup.close()
        networkMenuPopup.close()
        languageMenuPopup.close()
    }

    function requiredMenuWidth()
    {
        let requiredWidth = 0
        for (let index = 0; index < actionsMenuEntries.children.length; ++index)
            requiredWidth = Math.max(requiredWidth,
                                     actionsMenuEntries.children[index].implicitWidth)
        return requiredWidth
    }

    function handleSaveAction()
    {
        if (qlcplus.fileName())
        {
            if (qlcplus.saveWorkspace(qlcplus.fileName()))
                pendingWorkspaceAction.dispatch(false)
            else
                pendingWorkspaceAction.cancel()
        }
        else
            openDialog(App.SaveAsMode)
    }

    function handleOpenAction()
    {
        if (qlcplus.docModified)
        {
            pendingWorkspaceAction.action = "#OPEN"
            saveFirstPopup.open()
        }
        else
            openDialog(App.OpenMode)
    }

    function saveBeforeExit()
    {
        pendingWorkspaceAction.action = "#EXIT"
        saveFirstPopup.open()
    }

    function setLanguage(lang)
    {
        qlcplus.setLanguage(lang)
        menuRoot.close()
    }

    property string dialogTitle
    property url dialogCurrentFolder: qlcplus.workingPath
    property url dialogSelectedFile
    property var dialogNameFilters: [ qsTr("QLC+ files") + " (*.qxw *.qxf)", qsTr("All files") + " (*)" ]
    property int dialogFileMode: FileDialog.OpenFile
    property int dialogOpMode: App.OpenMode

    function openDialog(opMode)
    {
        dialogOpMode = opMode
        switch (dialogOpMode)
        {
            case App.OpenMode:
                dialogTitle = qsTr("Open a file")
                dialogFileMode = FileDialog.OpenFile
            break
            case App.SaveMode:
            case App.SaveAsMode:
                dialogTitle = qsTr("Save project as...")
                dialogFileMode = FileDialog.SaveFile
            break
            case App.ImportMode:
                dialogTitle = qsTr("Import from project")
            break
        }

        if (Qt.platform.os === "linux")
            customDialogLoader.item.open()
        else
            nativeDialog.open()
    }

    function handleAccept()
    {
        console.log("Selected file: " + dialogSelectedFile)

        let dispatchPendingActionOnClose = false

        switch (dialogOpMode)
        {
            case App.OpenMode:
            {
                if (dialogSelectedFile.toString().endsWith("qxf") ||
                    dialogSelectedFile.toString().endsWith("d4"))
                    qlcplus.loadFixture(dialogSelectedFile)
                else
                    qlcplus.loadWorkspace(dialogSelectedFile)
                qlcplus.workingPath = dialogCurrentFolder.toString()
            }
            break
            case App.SaveMode:
            case App.SaveAsMode:
            {
                if (qlcplus.saveWorkspace(dialogSelectedFile))
                    dispatchPendingActionOnClose = pendingWorkspaceAction.hasAction
                else
                    pendingWorkspaceAction.cancel()
            }
            break
            case App.ImportMode:
            {
                if (qlcplus.loadImportWorkspace(dialogSelectedFile) === true)
                {
                    importLoader.source = ""
                    importLoader.source = "qrc:/PopupImportProject.qml"
                }
            }
            break
        }

        return dispatchPendingActionOnClose
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
            if (handleAccept())
            {
                nativeDialogSaveCoordinator.trigger()
                if (!visible)
                    nativeDialogSaveCoordinator.completeDismissal()
            }
        }

        onRejected: pendingWorkspaceAction.cancel()
        onVisibleChanged:
        {
            if (!visible)
                nativeDialogSaveCoordinator.completeDismissal()
        }
    }

    PlatformPopupLoader
    {
        id: customDialogLoader

        sourceComponent: Component
        {
            PopupFolderBrowser
            {
                title: dialogTitle
                currentFolder: dialogCurrentFolder
                nameFilters: dialogNameFilters
                standardButtons: Dialog.Cancel |
                    ((dialogOpMode === App.SaveMode | dialogOpMode === App.SaveAsMode) ? Dialog.Save : Dialog.Open)

                onAccepted:
                {
                    dialogSelectedFile = currentFolder + folderSeparator() + selectedFile
                    dialogCurrentFolder = currentFolder
                    if (handleAccept())
                        customDialogSaveCoordinator.trigger()
                }

                onRejected: pendingWorkspaceAction.cancel()

                onClosed:
                {
                    customDialogSaveCoordinator.completeDismissal()
                }
            }
        }
    }

    DeferredPopupAction
    {
        id: openFileCoordinator

        onDismissRequested:
        {
            submenuItem = null
            menuRoot.close()
        }
        onActionRequested: handleOpenAction()
    }

    DeferredPopupAction
    {
        id: recentFileCoordinator

        property string filePath: ""

        onDismissRequested:
        {
            submenuItem = null
            recentFilesPopup.close()
            menuRoot.close()
        }
        onActionRequested:
        {
            const selectedFilePath = filePath
            filePath = ""

            if (qlcplus.docModified)
            {
                pendingWorkspaceAction.action = selectedFilePath
                saveFirstPopup.open()
            }
            else
                qlcplus.loadWorkspace(selectedFilePath)
        }
    }

    PendingWorkspaceAction
    {
        id: pendingWorkspaceAction

        onOpenRequested: openDialog(App.OpenMode)
        onNewRequested: qlcplus.newWorkspace()
        onExitRequested: function(discardChanges) {
            qlcplus.exit(discardChanges)
        }
        onRecentRequested: function(filePath) {
            qlcplus.loadWorkspace(filePath)
        }
    }

    DeferredPopupAction
    {
        id: saveFirstCoordinator

        property int selectedRole: Dialog.Cancel

        onDismissRequested: saveFirstPopup.close()
        onActionRequested:
        {
            if (selectedRole === Dialog.Yes)
                handleSaveAction()
            else if (selectedRole === Dialog.No)
                pendingWorkspaceAction.dispatch(true)
            else
                pendingWorkspaceAction.cancel()

            selectedRole = Dialog.Cancel
        }
    }

    DeferredPopupAction
    {
        id: customDialogSaveCoordinator

        onActionRequested: pendingWorkspaceAction.dispatch(false)
    }

    DeferredPopupAction
    {
        id: nativeDialogSaveCoordinator

        onActionRequested: pendingWorkspaceAction.dispatch(false)
    }

    ActionsMenuGeometry
    {
        id: recentMenuGeometry
        mainMenuWidth: menuRoot.width
        requestedSubmenuWidth: Math.max(mainMenuWidth, mainView.width * 0.55)
        windowWidth: mainView.width
        popupLeft: menuRoot.x
        margin: 8
    }

    ActionsMenuGeometry
    {
        id: networkMenuGeometry
        mainMenuWidth: menuRoot.width
        requestedSubmenuWidth: Math.max(mainMenuWidth,
                                        networkColumn.implicitWidth)
        windowWidth: mainView.width
        popupLeft: menuRoot.x
        margin: 8
    }

    ActionsMenuGeometry
    {
        id: languageMenuGeometry
        mainMenuWidth: menuRoot.width
        requestedSubmenuWidth: Math.max(mainMenuWidth,
                                        languageColumn.implicitWidth)
        windowWidth: mainView.width
        popupLeft: menuRoot.x
        margin: 8
    }

    RecentFilesPopup
    {
        id: recentFilesPopup
        parent: menuRoot.contentItem
        x: recentMenuGeometry.submenuX
        y: fileOpen.y
        width: recentMenuGeometry.submenuWidth
        recentFiles: qlcplus.recentFiles

        onFileSelected: function(filePath) {
            recentFileCoordinator.filePath = filePath
            recentFileCoordinator.trigger()
        }

        onClosed:
        {
            if (submenuItem === recentFilesPopup)
                submenuItem = null
        }
    }

    ActionsSubmenuPopup
    {
        id: networkMenuPopup
        parent: menuRoot.contentItem
        x: networkMenuGeometry.submenuX
        y: networkEntry.y
        width: networkMenuGeometry.submenuWidth
        height: networkColumn.implicitHeight

        contentItem:
            Column
            {
                id: networkColumn
                width: networkMenuPopup.width

                ContextMenuEntry
                {
                    id: startServer
                    objectName: "networkServerEntry"
                    entryText: qsTr("Server setup")

                    onClicked:
                    {
                        submenuItem = null
                        menuRoot.close()
                        pNetServer.open()
                    }
                }

                ContextMenuEntry
                {
                    id: connectToServer
                    objectName: "networkClientEntry"
                    entryText: qsTr("Client setup")

                    onClicked:
                    {
                        submenuItem = null
                        menuRoot.close()
                        pNetClient.open()
                    }
                }
            }

        onClosed:
        {
            if (submenuItem === networkMenuPopup)
                submenuItem = null
        }
    }

    PopupNetworkServer
    {
        id: pNetServer
        implicitWidth: Math.min(UISettings.bigItemHeight * 4,
                                mainView.width / 3)
    }

    PopupNetworkClient
    {
        id: pNetClient
        implicitWidth: Math.min(UISettings.bigItemHeight * 4,
                                mainView.width / 3)
    }

    ActionsSubmenuPopup
    {
        id: languageMenuPopup
        parent: menuRoot.contentItem
        x: languageMenuGeometry.submenuX
        y: Math.max(0, Math.min(languageEntry.y + languageEntry.height - height,
                                mainView.height - menuRoot.y - height - 8))
        width: languageMenuGeometry.submenuWidth
        height: languageColumn.implicitHeight

        contentItem:
            GridLayout
            {
                id: languageColumn
                width: languageMenuPopup.width
                columns: 2
                columnSpacing: 0
                rowSpacing: 0

                ContextMenuEntry
                {
                    objectName: "languageEntry_ca_ES"
                    Layout.fillWidth: true
                    imgSource: "qrc:/flag_ca.svg"
                    iconWidth: flagSize
                    entryText: qsTr("Catalan")
                    onClicked: setLanguage("ca_ES")
                }
                ContextMenuEntry
                {
                    objectName: "languageEntry_nl_NL"
                    Layout.fillWidth: true
                    imgSource: "qrc:/flag_nl.svg"
                    iconWidth: flagSize
                    entryText: qsTr("Dutch")
                    onClicked: setLanguage("nl_NL")
                }
                ContextMenuEntry
                {
                    objectName: "languageEntry_en_EN"
                    Layout.fillWidth: true
                    imgSource: "qrc:/flag_uk_us.svg"
                    iconWidth: flagSize
                    entryText: qsTr("English")
                    onClicked: setLanguage("en_EN")
                }
                ContextMenuEntry
                {
                    objectName: "languageEntry_fr_FR"
                    Layout.fillWidth: true
                    imgSource: "qrc:/flag_fr.svg"
                    iconWidth: flagSize
                    entryText: qsTr("French")
                    onClicked: setLanguage("fr_FR")
                }
                ContextMenuEntry
                {
                    objectName: "languageEntry_de_DE"
                    Layout.fillWidth: true
                    imgSource: "qrc:/flag_de.svg"
                    iconWidth: flagSize
                    entryText: qsTr("German")
                    onClicked: setLanguage("de_DE")
                }
                ContextMenuEntry
                {
                    objectName: "languageEntry_it_IT"
                    Layout.fillWidth: true
                    imgSource: "qrc:/flag_it.svg"
                    iconWidth: flagSize
                    entryText: qsTr("Italian")
                    onClicked: setLanguage("it_IT")
                }
                ContextMenuEntry
                {
                    objectName: "languageEntry_ja_JP"
                    Layout.fillWidth: true
                    imgSource: "qrc:/flag_jp.svg"
                    iconWidth: flagSize
                    entryText: qsTr("Japanese")
                    onClicked: setLanguage("ja_JP")
                }
                ContextMenuEntry
                {
                    objectName: "languageEntry_pl_PL"
                    Layout.fillWidth: true
                    imgSource: "qrc:/flag_pl.svg"
                    iconWidth: flagSize
                    entryText: qsTr("Polish")
                    onClicked: setLanguage("pl_PL")
                }
                ContextMenuEntry
                {
                    objectName: "languageEntry_ru_RU"
                    Layout.fillWidth: true
                    imgSource: "qrc:/flag_ru.svg"
                    iconWidth: flagSize
                    entryText: qsTr("Russian")
                    onClicked: setLanguage("ru_RU")
                }
                ContextMenuEntry
                {
                    objectName: "languageEntry_es_ES"
                    Layout.fillWidth: true
                    imgSource: "qrc:/flag_es.svg"
                    iconWidth: flagSize
                    entryText: qsTr("Spanish")
                    onClicked: setLanguage("es_ES")
                }
                ContextMenuEntry
                {
                    objectName: "languageEntry_uk_UA"
                    Layout.fillWidth: true
                    imgSource: "qrc:/flag_ua.svg"
                    iconWidth: flagSize
                    entryText: qsTr("Ukrainian")
                    onClicked: setLanguage("uk_UA")
                }
            }

        onClosed:
        {
            if (submenuItem === languageMenuPopup)
                submenuItem = null
        }
    }

    CustomPopupDialog
    {
        id: saveFirstPopup
        width: mainView.width / 2
        height: mainView.height / 3
        title: qsTr("Your project has changes")
        message: qsTr("Do you wish to save the current project first?\nChanges will be lost if you don't save them.")
        standardButtons: Dialog.Yes | Dialog.No | Dialog.Cancel

        onClosed:
        {
            if (saveFirstCoordinator.pending)
                saveFirstCoordinator.completeDismissal()
            else
                pendingWorkspaceAction.cancel()
        }

        onClicked: function(role)
        {
            saveFirstCoordinator.selectedRole = role
            saveFirstCoordinator.trigger()
        }
    }

    background:
        Rectangle
        {
            //radius: 2
            anchors.fill: parent
            border.width: 1
            border.color: UISettings.bgStronger
            color: UISettings.bgStrong
        }

    Column
    {
        id: actionsMenuEntries
        width: menuRoot.contentWidth

        ContextMenuEntry
        {
            id: fileNew
            imgSource: "qrc:/filenew.svg"
            entryText: qsTr("New project")
            onClicked:
            {
                if (qlcplus.docModified)
                {
                    pendingWorkspaceAction.action = "#NEW"
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
            entryText: qsTr("Open file")
            onClicked: openFileCoordinator.trigger()
            onEntered: showSubmenu(recentFilesPopup)
        }

        ContextMenuEntry
        {
            id: fileSave
            imgSource: "qrc:/filesave.svg"
            entryText: qsTr("Save project")
            onEntered: submenuItem = null

            onClicked:
            {
                handleSaveAction()
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
                openDialog(App.SaveMode)
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
                openDialog(App.ImportMode)
                menuRoot.close()
            }

            Loader
            {
                id: importLoader
                onLoaded: item.open()

                Connections
                {
                    target: importLoader.item
                    function onClose()
                    {
                        importLoader.source = ""
                    }
                }
            }
        }

        RowLayout
        {
            height: UISettings.iconSizeDefault
            width: parent.width
            spacing: 0

            ContextMenuEntry
            {
                Layout.fillWidth: true
                Layout.fillHeight: true
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
                Layout.fillWidth: true
                Layout.fillHeight: true
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
            id: networkEntry
            imgSource: "qrc:/network.svg"
            //faSource: FontAwesome.fa_network_wired
            //faColor: "darkseagreen"
            entryText: qsTr("Network")
            onEntered: showSubmenu(networkMenuPopup)
            onClicked: showSubmenu(networkMenuPopup)
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
                width: mainView.width / 3.5
                title: qsTr("DMX Address tool")
                standardButtons: Dialog.Close

                contentItem:
                    DMXAddressTool { }
            }
        }

        ContextMenuEntry
        {
            id: uiConfig
            imgSource: "qrc:/configure.svg"
            entryText: qsTr("UI Settings")
            onEntered: submenuItem = null
            onClicked:
            {
                menuRoot.close()
                mainView.loadResource("qrc:/UISettingsEditor.qml")
            }
        }

        ContextMenuEntry
        {
            id: fullScreen
            faSource: FontAwesome.fa_maximize
            faColor: UISettings.fgLight
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
            id: languageEntry
            faSource: FontAwesome.fa_earth_europe
            faColor: "deepskyblue"
            entryText: qsTr("Language")
            onEntered: showSubmenu(languageMenuPopup)
            onClicked: showSubmenu(languageMenuPopup)
        }

        ContextMenuEntry
        {
            id: info
            faSource: FontAwesome.fa_circle_info
            faColor: "skyblue"
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
                width: mainView.width / 2
            }
        }
    }
}
