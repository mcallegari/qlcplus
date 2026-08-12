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

    /** Flag of the native server being enabled. Settings that apply
     *  only to that server type are disabled when it is off */
    property bool nativeServer: (networkManager.serverType & NetworkManager.NativeServer) !== 0

    /** Flag raised when the encryption key has been edited but not
     *  applied yet. It enables the key "apply" button */
    property bool keyModified: false

    // discard a key edited but never applied on the previous run
    onOpened:
    {
        keyEdit.text = networkManager.serverPassword
        keyModified = false
    }

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
                label: qsTr("Web server")
            }

            RobotoText
            {
                Layout.fillWidth: true
                height: UISettings.listItemHeight
                label: networkManager.webServerStarted ? qsTr("Started") : qsTr("Stopped")
                labelColor: networkManager.webServerStarted ? "green" : "red"
            }

            IconButton
            {
                width: UISettings.listItemHeight
                height: width
                tooltip: networkManager.webServerStarted ? qsTr("Stop the web server") : qsTr("Start the web server")
                faSource: networkManager.webServerStarted ? FontAwesome.fa_stop : FontAwesome.fa_play
                faColor: networkManager.webServerStarted ? "red" : UISettings.fgMain
                onClicked: networkManager.toggleServerType(NetworkManager.WebServer)
            }

            // Row 2
            RobotoText
            {
                height: UISettings.listItemHeight
                label: qsTr("Native server")
            }

            RobotoText
            {
                Layout.fillWidth: true
                height: UISettings.listItemHeight
                label: networkManager.nativeServerStarted ? qsTr("Started") : qsTr("Stopped")
                labelColor: networkManager.nativeServerStarted ? "green" : "red"
            }

            IconButton
            {
                width: UISettings.listItemHeight
                height: width
                tooltip: networkManager.nativeServerStarted ? qsTr("Stop the native server") : qsTr("Start the native server")
                faSource: networkManager.nativeServerStarted ? FontAwesome.fa_stop : FontAwesome.fa_play
                faColor: networkManager.nativeServerStarted ? "red" : UISettings.fgMain
                onClicked: networkManager.toggleServerType(NetworkManager.NativeServer)
            }

            // Row 3
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

            // Row 4
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
                // not bound: the text is loaded on open and pushed
                // to the NetworkManager only when applied
                onTextEdited: popupRoot.keyModified = true
            }

            IconButton
            {
                width: UISettings.listItemHeight
                height: width
                enabled: popupRoot.nativeServer && popupRoot.keyModified
                tooltip: qsTr("Apply and save the encryption key")
                faSource: FontAwesome.fa_upload
                faColor: UISettings.fgMain
                onClicked:
                {
                    networkManager.saveEncryptionKey(keyEdit.text)
                    popupRoot.keyModified = false
                }
            }

            // Row 5
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

            // Row 6
            RobotoText
            {
                height: UISettings.listItemHeight
                label: qsTr("Clients connected")
            }

            RobotoText
            {
                height: UISettings.listItemHeight
                Layout.fillWidth: true
                Layout.columnSpan: 2
                label: networkManager.nativeServerStarted ? networkManager.connectionsCount : "-"
            }

            // Row 7
            GenericButton
            {
                Layout.columnSpan: 3
                Layout.fillWidth: true
                label: qsTr("Close")
                onClicked: popupRoot.close()
            }
        }

    footer: null
}
