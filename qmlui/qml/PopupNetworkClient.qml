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
    property int selectedIndex

    onOpened:
    {
        selectedIndex = -1
        networkManager.initializeClient()
    }

    title: qsTr("QLC+ client setup")

    contentItem:
        GridLayout
        {
            columns: 2
            rowSpacing: 5
            columnSpacing: 5

            // row 1
            RobotoText
            {
                height: UISettings.listItemHeight
                label: qsTr("Client name")
            }

            CustomTextEdit
            {
                property string hostname: networkManager.hostName

                Layout.fillWidth: true
                inputText: hostname
                onTextChanged: networkManager.hostName = text
            }

            // row 2
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

            // row 3
            ListView
            {
                id: serverRepeater
                Layout.fillWidth: true
                Layout.columnSpan: 2
                implicitHeight: UISettings.listItemHeight * count
                boundsBehavior: Flickable.StopAtBounds

                model: networkManager.serverList

                delegate:
                    Rectangle
                    {
                        id: serverDelegate
                        Layout.columnSpan: 2
                        width: serverName.width
                        height: UISettings.listItemHeight
                        color: index === selectedIndex ? UISettings.highlight : UISettings.bgMedium

                        RobotoText
                        {
                            id: serverName
                            anchors.verticalCenter: parent.verticalCenter
                            label: modelData.name + " (" + modelData.address + ")"

                            MouseArea
                            {
                                anchors.fill: parent
                                onClicked:
                                {
                                    selectedIndex = index
                                    popupRoot.serverAddress = modelData.address
                                }
                            }
                        }
                    }
            }

            // row 4
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

            // row 5
            CustomTextEdit
            {
                Layout.columnSpan: 2
                Layout.fillWidth: true
                enabled: manualServerCheck.checked
                inputText: popupRoot.serverAddress
                onTextChanged: popupRoot.serverAddress = text
            }

            // row 6
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
                    label: networkManager.clientConnected ? qsTr("Disconnect") : qsTr("Connect")
                    onClicked: networkManager.clientConnected ?
                                   networkManager.disconnectClient() :
                                   networkManager.connectClient(popupRoot.serverAddress)
                }
            }
        }

    footer: null
}
