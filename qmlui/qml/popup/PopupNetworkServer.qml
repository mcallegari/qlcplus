/*
  Q Light Controller Plus
  PopupNetworkServer.qml

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
import QtQuick.Layouts 1.1

import org.qlcplus.classes 1.0
import "."

CustomPopupDialog
{
    id: popupRoot
    width: mainView.width / 3
    title: qsTr("QLC+ server setup")

    contentItem:
        GridLayout
        {
            columns: 3
            rowSpacing: 5
            columnSpacing: 5

            // Row 1
            RobotoText
            {
                height: UISettings.listItemHeight
                label: qsTr("Server name")
            }

            CustomTextEdit
            {
                property string hostname: networkManager.hostName

                id: nameEdit
                Layout.columnSpan: 2
                Layout.fillWidth: true
                KeyNavigation.tab: keyEdit
                KeyNavigation.backtab: startCheckBox
                text: hostname
                onTextChanged: networkManager.hostName = text
            }

            // Row 2
            RobotoText
            {
                height: UISettings.listItemHeight
                label: qsTr("Encryption key")
            }

            CustomTextEdit
            {
                id: keyEdit
                Layout.fillWidth: true
                echoMode: TextInput.Password
                maximumLength: 8
                KeyNavigation.tab: startCheckBox
                KeyNavigation.backtab: nameEdit

                onTextChanged:
                {
                }
            }

            IconButton
            {
                width: UISettings.listItemHeight
                height: width
                faSource: FontAwesome.fa_gear
                faColor: UISettings.fgMain
            }

            // Row 3
            RobotoText
            {
                height: UISettings.listItemHeight
                label: qsTr("Start automatically")
            }
            CustomCheckBox
            {
                id: startCheckBox
                implicitHeight: UISettings.listItemHeight
                implicitWidth: height
                Layout.columnSpan: 2
                KeyNavigation.tab: nameEdit
                KeyNavigation.backtab: keyEdit
            }

            // Row 4
            RobotoText
            {
                height: UISettings.listItemHeight
                label: qsTr("Server status")
            }

            RobotoText
            {
                height: UISettings.listItemHeight
                Layout.columnSpan: 2
                label: networkManager.serverStarted ? qsTr("Running") : qsTr("Stopped")
                labelColor: networkManager.serverStarted ? "green" : "red"
            }

            // Row 5
            RobotoText
            {
                height: UISettings.listItemHeight
                label: qsTr("Clients connected")
            }

            RobotoText
            {
                height: UISettings.listItemHeight
                Layout.columnSpan: 2
                label: networkManager.connectionsCount
            }

            // Row 6
            Row
            {
                Layout.columnSpan: 2
                Layout.fillWidth: true

                GenericButton
                {
                    width: contentItem.width / 2
                    label: qsTr("Close")
                    onClicked: popupRoot.close()
                }

                GenericButton
                {
                    width: contentItem.width / 2
                    label: networkManager.serverStarted ? qsTr("Stop server") : qsTr("Start server")
                    onClicked: networkManager.serverStarted ? networkManager.stopServer() : networkManager.startServer()
                }
            }
        }

    footer: null
}
