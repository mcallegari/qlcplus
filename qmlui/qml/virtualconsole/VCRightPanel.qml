/*
  Q Light Controller Plus
  VCLeftPanel.qml

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

import org.qlcplus.classes 1.0
import "."

SidePanel
{
    onContentLoaded: (item, ID) =>
    {
        if (item.functionID)
            item.functionID = ID
    }

    Rectangle
    {
        id: sideBar
        width: collapseWidth
        height: parent.height
        color: "transparent"
        z: 2

        Column
        {
            anchors.horizontalCenter: parent.horizontalCenter
            width: iconSize
            spacing: 3

            ButtonGroup { id: vcButtonsGroup }

            IconButton
            {
                id: addWidgetButton
                z: 2
                width: iconSize
                height: iconSize
                faSource: FontAwesome.fa_plus
                faColor: "limegreen"
                ButtonGroup.group: vcButtonsGroup
                autoExclusive: false
                tooltip: qsTr("Add a new widget to the console")
                onClicked:
                {
                    checked = !checked
                    virtualConsole.editMode = false
                    if (checked)
                        loaderSource = "qrc:/WidgetsList.qml"
                    animatePanel(checked)
                }
            }

            IconButton
            {
                id: editModeButton
                z: 2
                width: iconSize
                height: iconSize
                imgSource: "qrc:/edit.svg"
                border.color: checked ? "red" : "#1D1D1D"
                checked: virtualConsole.editMode
                ButtonGroup.group: vcButtonsGroup
                autoExclusive: false
                tooltip: qsTr("Enable/Disable the widgets edit mode")

                onClicked:
                {
                    checked = !checked
                    virtualConsole.editMode = checked
                    if (checked)
                        loaderSource = "qrc:/VCWidgetProperties.qml"
                    animatePanel(checked)
                }

                SequentialAnimation on border.color
                {
                    PropertyAnimation { to: "red"; duration: 1000 }
                    PropertyAnimation { to: "white"; duration: 1000 }
                    running: editModeButton.checked
                    loops: Animation.Infinite
                }
            }
            IconButton
            {
                id: funcEditor
                z: 2
                width: iconSize
                height: iconSize
                imgSource: "qrc:/functions.svg"
                tooltip: qsTr("Function Manager")
                ButtonGroup.group: vcButtonsGroup
                autoExclusive: false
                onClicked:
                {
                    checked = !checked
                    virtualConsole.editMode = false
                    if (checked)
                        loaderSource = "qrc:/FunctionManager.qml"
                    animatePanel(checked)
                }
            }

            IconButton
            {
                id: removeButton
                z: 2
                width: iconSize
                height: iconSize
                faSource: FontAwesome.fa_minus
                faColor: "crimson"
                tooltip: qsTr("Remove the selected widgets")
                counter: virtualConsole.selectedWidgetsCount
                onClicked:
                {
                    var selNames = virtualConsole.selectedWidgetNames().join(", ")
                    deleteWidgetsPopup.message = qsTr("Are you sure you want to remove the following widgets?") + "\n" + selNames
                    deleteWidgetsPopup.open()
                }

                CustomPopupDialog
                {
                    id: deleteWidgetsPopup
                    title: qsTr("Delete selected widgets")
                    onAccepted: virtualConsole.deleteVCWidgets(virtualConsole.selectedWidgetIDs())
                }
            }

            IconButton
            {
                id: copyButton
                z: 2
                width: iconSize
                height: iconSize
                faSource: FontAwesome.fa_copy
                faColor: UISettings.fgMain
                tooltip: qsTr("Copy the selected widgets to clipboard")
                counter: virtualConsole.selectedWidgetsCount
                onClicked: virtualConsole.copyToClipboard()
            }

            IconButton
            {
                id: pasteButton
                z: 2
                width: iconSize
                height: iconSize
                faSource: FontAwesome.fa_paste
                faColor: UISettings.fgMain
                tooltip: qsTr("Paste widgets from clipboard")
                counter: virtualConsole.clipboardItemsCount
                onClicked: virtualConsole.pasteFromClipboard()

                Rectangle
                {
                    x: -3
                    y: -3
                    width: pasteButton.width * 0.4
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
                        label: virtualConsole.clipboardItemsCount
                        fontSize: height
                    }
                }

                MouseArea
                {
                    id: dumpDragArea
                    anchors.fill: parent
                    propagateComposedEvents: true
                    drag.target: pasteDragItem
                    drag.threshold: 10
                    onClicked: (mouse) => mouse.accepted = false

                    property bool dragActive: drag.active

                    onDragActiveChanged:
                    {
                        console.log("Drag active changed: " + dragActive)
                        if (dragActive == false)
                        {
                            pasteDragItem.Drag.drop()
                            pasteDragItem.parent = pasteButton
                            pasteDragItem.x = 0
                            pasteDragItem.y = 0
                        }
                        pasteDragItem.Drag.active = dragActive
                    }
                }

                Item
                {
                    id: pasteDragItem
                    visible: dumpDragArea.drag.active

                    Drag.source: pasteDragItem
                    Drag.keys: [ "pasteWidgets" ]

                    function itemDropped(id, name)
                    {
                        console.log("Paste widgets dropped on " + id)
                        /*dmxDumpDialog.sceneID = id
                        dmxDumpDialog.sceneName = name
                        dmxDumpDialog.open()
                        dmxDumpDialog.focusEditItem()
                        */
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
                            label: virtualConsole.clipboardItemsCount
                        }
                    }
                }
            }
        }
    }
}

