/*
  Q Light Controller Plus
  PopupNetworkConnect.qml

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
import QtQuick.Layouts

import org.qlcplus.classes 1.0
import "."

CustomPopupDialog
{
    id: popupRoot
    width: mainView.width / 3
    title: qsTr("Client access request")

    property string sessionId: ""
    property string clientName: ""
    property string peerAddress: ""
    property int peerPort: 0
    property bool deciding: false

    onClosed:
    {
        if (!deciding && sessionId !== "")
            networkManager.setClientAccess(sessionId, false, 0)
    }

    contentItem:
        GridLayout
        {
            columns: 4
            rowSpacing: 5
            columnSpacing: 5

            // row 1
            RobotoText
            {
                Layout.columnSpan: 4
                Layout.fillWidth: true
                //wrapText: true
                height: UISettings.listItemHeight * 5
                label: qsTr("A client is requesting access to this session.") +
                       "<br><b>" + qsTr("Name: ") + "</b>" + clientName +
                       "<br><b>" + qsTr("Address: ") + "</b>" + peerAddress + ":" + peerPort +
                       "<br><b>" + qsTr("Session ID: ") + "</b>" + sessionId +
                       "<br><br>" + qsTr("Access level:")
            }

            // row 2
            CustomCheckBox
            {
                id: fixtureEditCheck
                implicitHeight: UISettings.listItemHeight
                implicitWidth: height
                checked: true
                autoExclusive: false

            }
            RobotoText
            {
                height: UISettings.listItemHeight
                label: qsTr("Fixture/Group editing")
            }

            CustomCheckBox
            {
                id: funcEditCheck
                implicitHeight: UISettings.listItemHeight
                implicitWidth: height
                checked: true
                autoExclusive: false
            }
            RobotoText
            {
                height: UISettings.listItemHeight
                label: qsTr("Function editing")
            }

            // row 3
            CustomCheckBox
            {
                id: vcControlCheck
                implicitHeight: UISettings.listItemHeight
                implicitWidth: height
                checked: true
                autoExclusive: false
            }
            RobotoText
            {
                height: UISettings.listItemHeight
                label: qsTr("Virtual console control")
            }

            CustomCheckBox
            {
                id: vcEditCheck
                implicitHeight: UISettings.listItemHeight
                implicitWidth: height
                checked: true
                autoExclusive: false
            }
            RobotoText
            {
                height: UISettings.listItemHeight
                label: qsTr("Virtual console editing")
            }

            // row 4
            CustomCheckBox
            {
                id: sdeskCheck
                implicitHeight: UISettings.listItemHeight
                implicitWidth: height
                checked: true
                autoExclusive: false
            }
            RobotoText
            {
                height: UISettings.listItemHeight
                label: qsTr("Simple Desk")
            }

            CustomCheckBox
            {
                id: showMgrCheck
                implicitHeight: UISettings.listItemHeight
                implicitWidth: height
                checked: true
                autoExclusive: false
            }
            RobotoText
            {
                height: UISettings.listItemHeight
                label: qsTr("Show Manager")
            }

            // row 5
            CustomCheckBox
            {
                id: ioCheck
                implicitHeight: UISettings.listItemHeight
                implicitWidth: height
                checked: true
                autoExclusive: false
            }
            RobotoText
            {
                Layout.columnSpan: 3
                height: UISettings.listItemHeight
                label: qsTr("Input/Output")
            }

            // row 6
            CustomCheckBox
            {
                id: alwaysCheck
                implicitHeight: UISettings.listItemHeight
                implicitWidth: height
                checked: true
                autoExclusive: false
            }

            RobotoText
            {
                Layout.columnSpan: 3
                height: UISettings.listItemHeight
                label: qsTr("Always allow this client")
            }

            // row 7
            Row
            {
                Layout.columnSpan: 2
                Layout.fillWidth: true

                GenericButton
                {
                    width: contentItem.width / 2
                    label: qsTr("Deny")
                    onClicked:
                    {
                        var deniedSession = sessionId
                        deciding = true
                        popupRoot.close()
                        networkManager.setClientAccess(deniedSession, false, 0)
                    }
                }

                GenericButton
                {
                    width: contentItem.width / 2
                    label: qsTr("Allow")
                    onClicked:
                    {
                        var access = 0
                        if (fixtureEditCheck.checked)
                            access |= App.AC_FixtureEditing
                        if (funcEditCheck.checked)
                            access |= App.AC_FunctionEditing
                        if (vcControlCheck.checked)
                            access |= App.AC_VCControl
                        if (vcEditCheck.checked)
                            access |= App.AC_VCEditing
                        if (sdeskCheck.checked)
                            access |= App.AC_SimpleDesk
                        if (showMgrCheck.checked)
                            access |= App.AC_ShowManager
                        if (ioCheck.checked)
                            access |= App.AC_InputOutput

                        var allowedSession = sessionId
                        deciding = true
                        popupRoot.close()
                        if (networkManager.setClientAccess(allowedSession, true, access, alwaysCheck.checked))
                            networkManager.sendWorkspaceToClient(allowedSession, qlcplus.fileName())
                    }
                }
            }
        }

    footer.visible: false
}
