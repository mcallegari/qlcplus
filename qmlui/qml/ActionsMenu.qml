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
import QtQuick.Dialogs 1.2

import "."

Rectangle
{
    id: menuRoot
    radius: 2
    border.width: 1
    border.color: UISettings.bgStronger
    color: UISettings.bgStrong
    width: actionsMenuEntries.width
    height: actionsMenuEntries.height

    FileDialog
    {
        id: fileDialog
        visible: false
        folder: "file://" + qlcplus.workingPath

        onAccepted:
        {
            console.log("You chose: " + fileDialog.fileUrl)
            qlcplus.loadWorkspace(fileDialog.fileUrl)
            console.log("Folder: " + folder.toString())
            qlcplus.workingPath = folder.toString()
        }
        onRejected:
        {
            console.log("Canceled")
        }
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
                qlcplus.newWorkspace()
                menuRoot.visible = false
            }
            onEntered: recentMenu.visible = false
        }
        ContextMenuEntry
        {
            id: fileOpen
            imgSource: "qrc:/fileopen.svg"
            entryText: qsTr("Open project")
            onClicked:
            {
                fileDialog.title = qsTr("Open a workspace")
                fileDialog.nameFilters = [ qsTr("Workspace files") + " (*.qxw)", qsTr("All files") + " (*)" ]
                fileDialog.visible = true
                menuRoot.visible = false
                fileDialog.open()
            }
            onEntered: recentMenu.visible = true
            //onExited: recentMenu.visible = false

            Rectangle
            {
                id: recentMenu
                x: menuRoot.width
                width: recentColumn.width
                height: recentColumn.height
                color: UISettings.bgStrong
                visible: false

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
                                    recentMenu.visible = false
                                    menuRoot.visible = false
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
            onClicked: { }
            onEntered: recentMenu.visible = false
        }
        ContextMenuEntry
        {
            id: fileSaveAs
            imgSource: "qrc:/filesaveas.svg"
            entryText: qsTr("Save project as...")
            onClicked: { }
            onEntered: recentMenu.visible = false
        }
    }
}


