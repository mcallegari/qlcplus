/*
  Q Light Controller Plus
  PopupNetworkClient.qml

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

    property string serverAddress: "192.168.0.1"

    onOpened: networkManager.initializeClient()

    contentItem:
        GridLayout
        {
            columns: 2
            rowSpacing: 5
            columnSpacing: 5

            // Row 1
            CustomCheckBox
            {
                id: autoServerCheck
                implicitHeight: UISettings.listItemHeight
                implicitWidth: height
                checked: true
            }
            RobotoText
            {
                height: UISettings.listItemHeight
                Layout.fillWidth: true
                label: qsTr("Detected servers")
            }

            CustomCheckBox
            {
                id: manualServerCheck
                implicitHeight: UISettings.listItemHeight
                implicitWidth: height
            }
            RobotoText
            {
                height: UISettings.listItemHeight
                Layout.fillWidth: true
                label: qsTr("Manual server")
            }
            CustomTextEdit
            {
                id: nameEdit
                Layout.columnSpan: 2
                Layout.fillWidth: true
                enabled: manualServerCheck.checked
                inputText: popupRoot.serverAddress
                onTextChanged: popupRoot.serverAddress = text
            }

            Row
            {
                Layout.columnSpan: 2

                // last row
                GenericButton
                {
                    label: qsTr("Close")
                    onClicked: popupRoot.close()
                }

                GenericButton
                {
                    label: networkManager.clientConnected ? qsTr("Disconnect") : qsTr("Connect")
                    onClicked: networkManager.clientConnected ?
                                   networkManager.disconnectClient() :
                                   networkManager.connectClient(popupRoot.serverAddress)
                }
            }
        }

    footer: null
}
