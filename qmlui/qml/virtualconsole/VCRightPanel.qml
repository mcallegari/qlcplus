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

import QtQuick 2.0
import QtQuick.Controls 2.1

import org.qlcplus.classes 1.0
import "."

SidePanel
{
    id: vcRightPanel

    onContentLoaded: if (item.functionID) item.functionID = itemID

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
                imgSource: "qrc:/add.svg"
                checkable: true
                ButtonGroup.group: vcButtonsGroup
                tooltip: qsTr("Add a new widget to the console")
                onToggled:
                {
                    if (checked == true)
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
                checkable: true
                ButtonGroup.group: vcButtonsGroup
                tooltip: qsTr("Enable/Disable the widgets edit mode")

                onCheckedChanged:
                {
                    virtualConsole.editMode = checked
                    if (checked == true)
                        loaderSource = "qrc:/VCWidgetProperties.qml"
                    else
                        border.color = "#1D1D1D"
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
                checkable: true
                ButtonGroup.group: vcButtonsGroup
                onToggled:
                {
                    if (checked == true)
                        loaderSource = "qrc:/FunctionManager.qml"
                    animatePanel(checked);
                }
            }

            IconButton
            {
                id: removeButton
                z: 2
                width: iconSize
                height: iconSize
                imgSource: "qrc:/remove.svg"
                tooltip: qsTr("Remove the selected widgets")
                counter: virtualConsole.selectedWidgetsCount
                onClicked:
                {
                    var selNames = virtualConsole.selectedWidgetNames()
                    //console.log(selNames)
                    deleteWidgetsPopup.message = qsTr("Are you sure you want to remove the following widgets ?") + "\n" + selNames
                    deleteWidgetsPopup.open()
                }

                CustomPopupDialog
                {
                    id: deleteWidgetsPopup
                    title: qsTr("Delete functions")
                    onAccepted: virtualConsole.deleteVCWidgets(virtualConsole.selectedWidgetIDs())
                }
            }
        }
    }
}

