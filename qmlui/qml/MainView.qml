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
import QtQuick.Controls 2.1

import org.qlcplus.classes 1.0
import "."

Rectangle
{
    id: mainView
    visible: true
    width: 800
    height: 600
    anchors.fill: parent
    color: UISettings.bgMedium

    property string currentContext: ""

    Component.onCompleted: UISettings.sidePanelWidth = Math.min(width / 3, UISettings.bigItemHeight * 5)
    onWidthChanged: UISettings.sidePanelWidth = Math.min(width / 3, UISettings.bigItemHeight * 5)

    function enableContext(ctx, setChecked)
    {
        var item = null

        if (ctx === "FIXANDFUNC")
            item = fnfEntry
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
            return true
        }
        return false
    }

    function switchToContext(ctx, qmlRes)
    {
        if (currentContext === ctx)
            return

        if (enableContext(ctx, true) === true)
        {
            currentContext = ctx
            mainToolbar.visible = true
        }
        else
        {
            mainToolbar.visible = false
            currentContext = ""
        }

        if (qmlRes)
            mainViewLoader.source = qmlRes
    }

    function setDimScreen(enable)
    {
        dimScreen.visible = enable
    }

    function openAccessRequest(clientName)
    {
        clientAccessPopup.clientName = clientName
        clientAccessPopup.open()
    }

    function saveProject()
    {
        actionsMenu.handleSaveAction()
    }

    function saveBeforeExit()
    {
        //actionsMenu.open()
        actionsMenu.saveBeforeExit()
    }

    function loadResource(qmlRes)
    {
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
        visible: qlcplus.accessMask !== App.AC_VCControl
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
                id: actEntry
                imgSource: "qrc:/qlcplus.svg"
                entryText: qsTr("Actions")
                onPressed: actionsMenu.open()
                autoExclusive: false
                checkable: false

                Image
                {
                    visible: qlcplus.docModified
                    source: "qrc:/filesave.svg"
                    x: 1
                    y: parent.height - height - 1
                    height: parent.height / 3
                    width: height
                    sourceSize: Qt.size(width, height)
                }
            }
            MenuBarEntry
            {
                id: fnfEntry
                property string ctxName: "FIXANDFUNC"
                property string ctxRes: "qrc:/FixturesAndFunctions.qml"

                imgSource: "qrc:/editor.svg"
                entryText: qsTr("Fixtures & Functions")
                checked: false
                ButtonGroup.group: menuBarGroup
                onCheckedChanged:
                {
                    if (checked == true)
                        switchToContext(fnfEntry.ctxName, fnfEntry.ctxRes)
                }
            }
            MenuBarEntry
            {
                id: vcEntry
                property string ctxName: "VC"
                property string ctxRes: "qrc:/VirtualConsole.qml"

                visible: qlcplus.accessMask & App.AC_VCControl
                imgSource: "qrc:/virtualconsole.svg"
                entryText: qsTr("Virtual Console")
                ButtonGroup.group: menuBarGroup
                onCheckedChanged:
                {
                    if (checked == true)
                        switchToContext(vcEntry.ctxName, vcEntry.ctxRes)
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
                property string ctxName: "SDESK"
                property string ctxRes: "qrc:/SimpleDesk.qml"

                visible: qlcplus.accessMask & App.AC_SimpleDesk
                imgSource: "qrc:/simpledesk.svg"
                entryText: qsTr("Simple Desk")
                ButtonGroup.group: menuBarGroup
                onCheckedChanged:
                {
                    if (checked == true)
                        switchToContext(sdEntry.ctxName, sdEntry.ctxRes)
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
                property string ctxName: "SHOWMGR"
                property string ctxRes: "qrc:/ShowManager.qml"

                visible: qlcplus.accessMask & App.AC_ShowManager
                imgSource: "qrc:/showmanager.svg"
                entryText: qsTr("Show Manager")
                ButtonGroup.group: menuBarGroup
                onCheckedChanged:
                {
                    if (checked == true)
                        switchToContext(smEntry.ctxName, smEntry.ctxRes)
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
                property string ctxName: "IOMGR"
                property string ctxRes: "qrc:/InputOutputManager.qml"

                visible: qlcplus.accessMask & App.AC_InputOutput
                imgSource: "qrc:/inputoutput.svg"
                entryText: qsTr("Input/Output")
                ButtonGroup.group: menuBarGroup
                onCheckedChanged:
                {
                    if (checked == true)
                        switchToContext(ioEntry.ctxName, ioEntry.ctxRes)
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
                height: parent.height
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
                color: UISettings.fgMedium

                ColorAnimation on color
                {
                    id: cAnim
                    from: "#00FF00"
                    to: UISettings.fgMedium
                    // half the duration of the current BPM
                    duration: ioManager.bpmNumber ? 30000 / ioManager.bpmNumber : 200
                    running: false
                }

                Connections
                {
                    id: beatSignal
                    target: ioManager
                    function onBeat()
                    {
                        cAnim.restart()
                    }
                }
            }
            IconButton
            {
                id: stopAllButton
                width: UISettings.iconSizeDefault
                height: UISettings.iconSizeDefault
                enabled: runningCount ? true : false
                bgColor: "transparent"
                imgSource: "qrc:/stop.svg"
                tooltip: qsTr("Stop all the running functions")
                onClicked: qlcplus.stopAllFunctions()

                property int runningCount: qlcplus.runningFunctionsCount

                onRunningCountChanged: console.log("Functions running: " + runningCount)

                Rectangle
                {
                    x: parent.width / 2
                    y: parent.height / 2
                    width: parent.width * 0.4
                    height: width
                    color: UISettings.highlight
                    border.width: 1
                    border.color: UISettings.fgMain
                    radius: 3
                    clip: true
                    visible: stopAllButton.runningCount

                    RobotoText
                    {
                        anchors.centerIn: parent
                        height: parent.height * 0.7
                        label: stopAllButton.runningCount
                        fontSize: height
                    }
                }
            }

        } // end of RowLayout
    } // end of mainToolbar

    Loader
    {
        id: mainViewLoader
        width: parent.width
        height: parent.height - (mainToolbar.visible ? mainToolbar.height : 0)
        y: mainToolbar.visible ? mainToolbar.height : 0

        Component.onCompleted:
        {
            var ctx = "FIXANDFUNC"
            // handle Kiosk mode on startup
            if (qlcplus.accessMask === App.AC_VCControl)
                ctx = "VC"
            enableContext(ctx, true)
        }
    }

    PopupNetworkConnect { id: clientAccessPopup }

    /** Menu to open/load/save a project */
    ActionsMenu
    {
        id: actionsMenu
        x: 1
        y: actEntry.height + 1
        visible: false
        z: visible ? 99 : 0
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

    PopupDisclaimer { }
}
