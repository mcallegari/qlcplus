/*
  Q Light Controller Plus
  MainView.qml

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
import QtQuick.Controls 1.2

import "DetachWindow.js" as WinLoader

Rectangle
{
    id: mainView
    visible: true
    width: 800
    height: 600
    anchors.fill: parent

    FontLoader
    {
        source: "qrc:RobotoCondensed-Regular.ttf"
    }

    Rectangle
    {
        id: mainToolbar
        width: parent.width
        // this can be read from an external variable (QSettings ?) and will still work !
        height: 40
        z: 50
        gradient: Gradient
        {
            id: bgGradient
            GradientStop { position: 0 ; color: "#1a1a1a" }
            GradientStop { position: 1 ; color: "#111" }
        }

        RowLayout
        {
            spacing: 5
            anchors.fill: parent

            ExclusiveGroup { id: menuBarGroup }
            MenuBarEntry
            {
                id: actEntry
                imgSource: "qrc:/qlcplus.png"
                entryText: qsTr("Actions")
                onClicked:
                {
                    actionsMenu.visible = true
                    contextMenuArea.enabled = true
                    contextMenuArea.z = 98
                }
            }
            MenuBarEntry
            {
                id: edEntry
                imgSource: "editor.svg"
                entryText: qsTr("Fixtures & Functions")
                checkable: true
                checked: true
                exclusiveGroup: menuBarGroup
                onCheckedChanged:
                {
                    if (checked == true)
                        viewLoader.source = "qrc:/FixturesAndFunctions.qml"
                }
                /*
                onRightClicked:
                {
                    WinLoader.createWindow("qrc:/FixturesAndFunctions.qml")
                }
                */
            }
            MenuBarEntry
            {
                id: vcEntry
                imgSource: "virtualconsole.svg"
                entryText: qsTr("Virtual Console")
                checkable: true
                exclusiveGroup: menuBarGroup
                onCheckedChanged:
                {
                    if (checked == true)
                        viewLoader.source = "qrc:/VirtualConsole.qml"
                }
                onRightClicked:
                {
                    WinLoader.createWindow("qrc:/VirtualConsole.qml")
                }
            }
            MenuBarEntry
            {
                id: sdEntry
                imgSource: "simpledesk.svg"
                entryText: qsTr("Simple Desk")
                checkable: true
                exclusiveGroup: menuBarGroup
                onCheckedChanged:
                {
                    if (checked == true)
                        viewLoader.source = "qrc:/SimpleDesk.qml"
                }
                onRightClicked:
                {
                    WinLoader.createWindow("qrc:/SimpleDesk.qml")
                }
            }
            MenuBarEntry
            {
                id: smEntry
                imgSource: "showmanager.svg"
                entryText: qsTr("Show Manager")
                checkable: true
                exclusiveGroup: menuBarGroup
                onCheckedChanged:
                {
                    if (checked == true)
                        viewLoader.source = "qrc:/ShowManager.qml"
                }
                onRightClicked:
                {
                    WinLoader.createWindow("qrc:/ShowManager.qml")
                }
            }
            MenuBarEntry
            {
                id: ioEntry
                imgSource: "inputoutput.svg"
                entryText: qsTr("Input/Output")
                checkable: true
                exclusiveGroup: menuBarGroup
                onCheckedChanged:
                {
                    if (checked == true)
                        viewLoader.source = "qrc:/InputOutputManager.qml"
                }
                onRightClicked:
                {
                    WinLoader.createWindow("qrc:/InputOutputManager.qml")
                }
            }
            Rectangle
            {
                // acts like an horizontal spacer
                Layout.fillWidth: true
            }
        }
    }

    /** Menu to open/load/save a project */
    ActionsMenu
    {
        id: actionsMenu
    }

    /** Mouse area enabled when actionsMenu is visible
     *  It fills the whole application window to grab
     *  a click outside the menu and close it
     */
    MouseArea
    {
        id: contextMenuArea
        z: 0
        enabled: false
        anchors.fill: parent
        onClicked:
        {
            console.log("Root clicked")
            if (actionsMenu.visible == true)
            {
                contextMenuArea.enabled = false
                contextMenuArea.z = 0;
                actionsMenu.visible = false
            }
        }
    }

    Rectangle
    {
        id: mainViewArea
        width: parent.width
        height: parent.height - mainToolbar.height
        y: mainToolbar.height
        color: "#303030"

        Loader
        {
            id: viewLoader
            anchors.fill: parent
            source: "qrc:/FixturesAndFunctions.qml"
        }
    }
}
