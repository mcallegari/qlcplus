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

import QtQuick.Window 2.0

import "."

Rectangle
{
    id: mainView
    visible: true
    width: 800
    height: 600
    anchors.fill: parent

    property string currentContext: "FIXANDFUNC"

    function enableContext(ctx, setChecked)
    {
        var item = null

        if (ctx === "FIXANDFUNC")
            item = edEntry
        else if (ctx === "VC")
            item = vcEntry
        else if (ctx === "SDESK")
            item = sdEntry
        else if (ctx === "SHOWMGR")
            item = smEntry
        else if (ctx === "IOMGR")
            item = ioEntry

        if (item)
        {
            item.visible = true
            if (setChecked)
                item.checked = true
        }
    }

    function switchToContext(ctx, qmlRes)
    {
        if (currentContext === ctx)
            return

        enableContext(ctx, true)
        currentContext = ctx
        mainViewLoader.source = qmlRes
    }

    FontLoader
    {
        source: "qrc:/RobotoCondensed-Regular.ttf"
    }

    // Load the "FontAwesome" font for the monochrome icons
    FontLoader
    {
        source: "qrc:/FontAwesome.otf"
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

        Component.onCompleted:
        {
            console.log("density: " + Screen.pixelDensity + ", ratio: " + Screen.devicePixelRatio)
        }

        RowLayout
        {
            spacing: 5
            anchors.fill: parent

            ExclusiveGroup { id: menuBarGroup }
            MenuBarEntry
            {
                id: actEntry
                imgSource: "qrc:/qlcplus.svg"
                entryText: qsTr("Actions")
                onClicked: actionsMenu.visible = true
            }
            MenuBarEntry
            {
                id: edEntry
                imgSource: "qrc:/editor.svg"
                entryText: qsTr("Fixtures & Functions")
                checkable: true
                checked: true
                exclusiveGroup: menuBarGroup
                onCheckedChanged:
                {
                    if (checked == true)
                        switchToContext("FIXANDFUNC", "qrc:/FixturesAndFunctions.qml")
                }
            }
            MenuBarEntry
            {
                id: vcEntry
                imgSource: "qrc:/virtualconsole.svg"
                entryText: qsTr("Virtual Console")
                checkable: true
                exclusiveGroup: menuBarGroup
                onCheckedChanged:
                {
                    if (checked == true)
                        switchToContext("VC", "qrc:/VirtualConsole.qml")
                }
                onRightClicked:
                {
                    vcEntry.visible = false
                    contextManager.detachContext("VC")
                }
            }
            MenuBarEntry
            {
                id: sdEntry
                imgSource: "qrc:/simpledesk.svg"
                entryText: qsTr("Simple Desk")
                checkable: true
                exclusiveGroup: menuBarGroup
                onCheckedChanged:
                {
                    if (checked == true)
                        switchToContext("SDESK", "qrc:/SimpleDesk.qml")
                }
                onRightClicked:
                {
                    sdEntry.visible = false
                    contextManager.detachContext("SDESK")
                }
            }
            MenuBarEntry
            {
                id: smEntry
                imgSource: "qrc:/showmanager.svg"
                entryText: qsTr("Show Manager")
                checkable: true
                exclusiveGroup: menuBarGroup
                onCheckedChanged:
                {
                    if (checked == true)
                        switchToContext("SHOWMGR", "qrc:/ShowManager.qml")
                }
                onRightClicked:
                {
                    smEntry.visible = false
                    contextManager.detachContext("SHOWMGR")
                }
            }
            MenuBarEntry
            {
                id: ioEntry
                imgSource: "qrc:/inputoutput.svg"
                entryText: qsTr("Input/Output")
                checkable: true
                exclusiveGroup: menuBarGroup
                onCheckedChanged:
                {
                    if (checked == true)
                        switchToContext("IOMGR", "qrc:/InputOutputManager.qml")
                }
                onRightClicked:
                {
                    ioEntry.visible = false
                    contextManager.detachContext("IOMGR")
                }
            }
            Rectangle
            {
                // acts like an horizontal spacer
                Layout.fillWidth: true
                color: "transparent"
            }
            RobotoText
            {
                label: "BPM: " + (ioManager.bpmNumber > 0 ? ioManager.bpmNumber : qsTr("Off"))
                color: gsMouseArea.containsMouse ? UISettings.bgLight : "transparent"
                fontSize: UISettings.textSizeDefault

                MouseArea
                {
                    id: gsMouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    onClicked: beatSelectionPanel.visible = !beatSelectionPanel.visible
                }
                BeatGeneratorsPanel
                {
                    id: beatSelectionPanel
                    parent: mainView
                    y: mainToolbar.height
                    x: beatIndicator.x - width
                    z: 51
                    visible: false
                }
            }
            Rectangle
            {
                id: beatIndicator
                width: height
                height: parent.height * 0.5
                radius: height / 2
                border.width: 2
                border.color: "#333"
                color: "#666"

                ColorAnimation on color
                {
                    id: cAnim
                    from: "#00FF00"
                    to: "#666"
                    // half the duration of the current BPM
                    duration: ioManager.bpmNumber ? 30000 / ioManager.bpmNumber : 200
                    running: false
                }

                Connections
                {
                    id: beatSignal
                    target: ioManager
                    onBeat: cAnim.restart()
                }
            }

        } // end of RowLayout
    } // end of mainToolbar

    /** Menu to open/load/save a project */
    ActionsMenu
    {
        id: actionsMenu
        x: 1
        y: actEntry.height + 1
        visible: false
        z: visible ? 99 : 0
    }

    /** Mouse area enabled when actionsMenu is visible
     *  It fills the whole application window to grab
     *  a click outside the menu and close it
     */
    MouseArea
    {
        id: contextMenuArea
        z: actionsMenu.visible ? 98 : 0
        enabled: actionsMenu.visible
        anchors.fill: parent
        onClicked: actionsMenu.visible = false
    }

    Rectangle
    {
        id: mainViewArea
        width: parent.width
        height: parent.height - mainToolbar.height
        y: mainToolbar.height
        color: UISettings.bgMain

        Loader
        {
            id: mainViewLoader
            anchors.fill: parent
            source: "qrc:/FixturesAndFunctions.qml"
        }
    }
    PopupBox
    {
        anchors.fill: parent
    }
}
