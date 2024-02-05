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
import QtQuick.Controls 2.2

import org.qlcplus.classes 1.0
import "."

CustomPopupDialog
{
    id: popupRoot
    width: mainView.width / 3
    title: qsTr("QLC+ client setup")

    property string serverAddress: "192.168.0.1"
    property int clientStatus: networkManager.clientStatus
    property int selectedIndex

    onOpened:
    {
        selectedIndex = -1
        networkManager.initializeClient()
        setClientStatus(networkManager.clientStatus)
    }

    onClientStatusChanged: setClientStatus(clientStatus)

    function setClientStatus(clientStatus)
    {
        if (clientStatus === NetworkManager.Disconnected)
        {
            statusText.label = qsTr("Disconnected")
            statusText.labelColor = "red"
            statusProgress.visible = false
        }
        else if (clientStatus === NetworkManager.WaitAuthentication)
        {
            statusText.label = qsTr("Waiting for access")
            statusText.labelColor = "yellow"
            statusProgress.indeterminate = true
            statusProgress.visible = true
        }
        else if (clientStatus === NetworkManager.DownloadingProject)
        {
            statusText.label = qsTr("Downloading project")
            statusText.labelColor = "yellow"
            statusProgress.visible = true
        }
        else if (clientStatus === NetworkManager.Connected)
        {
            statusText.label = qsTr("Connected")
            statusText.labelColor = "green"
            statusProgress.visible = false
        }
    }

    contentItem:
        GridLayout
        {
            columns: 2
            rowSpacing: 5
            columnSpacing: 5

            ButtonGroup { id: checkGroup }

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
                text: hostname
                onTextChanged: networkManager.hostName = text
            }

            // row 2
            Row
            {
                Layout.columnSpan: 2
                Layout.fillWidth: true
                spacing: 5

                CustomCheckBox
                {
                    id: autoServerCheck
                    implicitHeight: UISettings.listItemHeight
                    implicitWidth: height
                    checked: true
                    ButtonGroup.group: checkGroup
                }
                RobotoText
                {
                    height: UISettings.listItemHeight
                    Layout.fillWidth: true
                    label: qsTr("Detected servers")
                }
            }

            // row 3
            ListView
            {
                id: serverRepeater
                Layout.fillWidth: true
                Layout.columnSpan: 2
                implicitHeight: UISettings.listItemHeight * 3
                boundsBehavior: Flickable.StopAtBounds

                model: networkManager.serverList

                delegate:
                    Rectangle
                    {
                        id: serverDelegate
                        Layout.columnSpan: 2
                        width: parent.width
                        height: UISettings.listItemHeight
                        color: index === selectedIndex ? UISettings.highlight : UISettings.bgLight

                        IconTextEntry
                        {
                            id: serverName
                            height: UISettings.listItemHeight
                            tLabel: modelData.name + " (" + modelData.address + ")"
                            faSource: FontAwesome.fa_server
                            faColor: UISettings.fgMain
                        }

                        MouseArea
                        {
                            anchors.fill: parent
                            onClicked:
                            {
                                selectedIndex = index
                                popupRoot.serverAddress = modelData.address
                            }
                        }
                        Rectangle
                        {
                            y: parent.height - 1
                            width: parent.width
                            height: 1
                            color: UISettings.bgLighter
                        }
                    }
            }

            // row 4
            Row
            {
                Layout.columnSpan: 2
                Layout.fillWidth: true
                spacing: 5

                CustomCheckBox
                {
                    id: manualServerCheck
                    implicitHeight: UISettings.listItemHeight
                    implicitWidth: height
                    ButtonGroup.group: checkGroup
                }
                RobotoText
                {
                    height: UISettings.listItemHeight
                    Layout.fillWidth: true
                    label: qsTr("Manual server")
                }
            }

            // row 5
            CustomTextEdit
            {
                Layout.columnSpan: 2
                Layout.fillWidth: true
                enabled: manualServerCheck.checked
                text: popupRoot.serverAddress
                onTextChanged: popupRoot.serverAddress = text
            }

            Row
            {
                Layout.columnSpan: 2
                Layout.fillWidth: true
                spacing: 5

                RobotoText
                {
                    height: UISettings.listItemHeight
                    label: qsTr("Status:")
                }

                RobotoText
                {
                    id: statusText
                    height: UISettings.listItemHeight
                }
            }

            // row 6
            ProgressBar
            {
                id: statusProgress
                Layout.columnSpan: 2
                Layout.fillWidth: true
            }

            // row 7
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
                    enabled: manualServerCheck.checked || selectedIndex != -1
                    width: contentItem.width / 2
                    label: popupRoot.clientStatus === NetworkManager.Connected ? qsTr("Disconnect") : qsTr("Connect")
                    onClicked:
                    {
                        if (popupRoot.clientStatus === NetworkManager.Connected)
                            networkManager.disconnectClient()
                        else
                            networkManager.connectClient(popupRoot.serverAddress)
                    }
                }
            }
        }

    footer: null
}
