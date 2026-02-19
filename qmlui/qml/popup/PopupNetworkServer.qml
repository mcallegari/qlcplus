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

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import org.qlcplus.classes 1.0
import "."

CustomPopupDialog
{
    id: popupRoot
    width: mainView.width / 3
    title: qsTr("QLC+ server setup")
    property bool nativeServer: networkManager.serverType === NetworkManager.NativeServer

    contentItem:
        GridLayout
        {
            columns: 3
            rowSpacing: 5
            columnSpacing: 5

            ButtonGroup { id: serverTypeGroup }

            // Row 1
            RobotoText
            {
                height: UISettings.listItemHeight
                label: qsTr("Server type")
            }

            RowLayout
            {
                Layout.columnSpan: 2
                Layout.fillWidth: true
                spacing: 20

                Row
                {
                    spacing: 5
                    CustomCheckBox
                    {
                        id: webServerCheck
                        implicitHeight: UISettings.listItemHeight
                        implicitWidth: height
                        checked: networkManager.serverType === NetworkManager.WebServer
                        ButtonGroup.group: serverTypeGroup
                        onClicked: if (checked) networkManager.serverType = NetworkManager.WebServer
                    }
                    RobotoText
                    {
                        height: UISettings.listItemHeight
                        label: qsTr("Web server")
                    }
                }

                Row
                {
                    spacing: 5
                    CustomCheckBox
                    {
                        id: nativeServerCheck
                        implicitHeight: UISettings.listItemHeight
                        implicitWidth: height
                        checked: networkManager.serverType === NetworkManager.NativeServer
                        ButtonGroup.group: serverTypeGroup
                        onClicked: if (checked) networkManager.serverType = NetworkManager.NativeServer
                    }
                    RobotoText
                    {
                        height: UISettings.listItemHeight
                        label: qsTr("Native server")
                    }
                }
            }

            // Row 2
            RobotoText
            {
                height: UISettings.listItemHeight
                label: qsTr("Server name")
            }

            CustomTextEdit
            {
                id: nameEdit
                Layout.columnSpan: 2
                Layout.fillWidth: true
                enabled: popupRoot.nativeServer
                text: networkManager.hostName
                onTextEdited: networkManager.hostName = text
            }

            // Row 3
            RobotoText
            {
                height: UISettings.listItemHeight
                label: qsTr("Encryption key")
            }

            CustomTextEdit
            {
                id: keyEdit
                Layout.fillWidth: true
                enabled: popupRoot.nativeServer
                echoMode: TextInput.Password
                maximumLength: 8
                text: networkManager.serverPassword
                onTextEdited: networkManager.serverPassword = text
            }

            IconButton
            {
                width: UISettings.listItemHeight
                height: width
                enabled: popupRoot.nativeServer
                faSource: FontAwesome.fa_gear
                faColor: UISettings.fgMain
            }

            // Row 4
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
                checked: networkManager.startAutomatically
                onClicked: networkManager.startAutomatically = checked
            }

            // Row 5
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

            // Row 6
            RobotoText
            {
                height: UISettings.listItemHeight
                label: qsTr("Clients connected")
            }

            RobotoText
            {
                height: UISettings.listItemHeight
                Layout.columnSpan: 2
                label: popupRoot.nativeServer ? networkManager.connectionsCount : "-"
            }

            // Row 7
            Row
            {
                Layout.columnSpan: 3
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
