/*
  Q Light Controller Plus
  FixtureSubstitutionPanel.qml

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

Rectangle
{
    id: subPanelRoot
    anchors.fill: parent
    color: UISettings.bgStrong

    signal closed()

    Component.onCompleted:
    {
        fixtureBrowser.selectedManufacturer = ""
        fixtureBrowser.selectedModel = ""
        fixtureBrowser.selectedMode = ""
        fixtureBrowser.searchFilter = ""
    }

    ColumnLayout
    {
        anchors.fill: parent
        anchors.margins: 2
        spacing: 2

        Rectangle
        {
            height: UISettings.listItemHeight
            Layout.fillWidth: true
            color: UISettings.sectionHeader

            RobotoText
            {
                anchors.centerIn: parent
                label: qsTr("Substitute Profile")
            }
        }

        Item
        {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true

            FixtureBrowser
            {
                id: fxBrowser
                // O FixtureBrowser.qml tem anchors.fill: parent,
                // então ele vai preencher este Item.
            }
        }

        RowLayout
        {
            Layout.fillWidth: true
            Layout.preferredHeight: UISettings.listItemHeight
            Layout.margins: 4
            spacing: 5

            GenericButton
            {
                Layout.fillWidth: true
                Layout.preferredHeight: UISettings.listItemHeight
                label: qsTr("Cancel")
                onClicked: subPanelRoot.closed()
            }

            GenericButton
            {
                Layout.fillWidth: true
                Layout.preferredHeight: UISettings.listItemHeight
                label: qsTr("Confirm")
                // Usando a instância global fixtureBrowser para garantir a leitura dos dados
                enabled: fixtureBrowser.selectedModel !== "" && fixtureBrowser.selectedMode !== ""
                onClicked:
                {
                    console.log("Substituindo para: " + fixtureBrowser.selectedManufacturer + " " + fixtureBrowser.selectedModel + " (" + fixtureBrowser.selectedMode + ")")
                    var selectedIDs = contextManager.selectedFixtureIDVariantList()
                    if (fixtureManager.replaceFixturesProfile(selectedIDs,
                                                             fixtureBrowser.selectedManufacturer,
                                                             fixtureBrowser.selectedModel,
                                                             fixtureBrowser.selectedMode))
                    {
                        subPanelRoot.closed()
                    }
                }
            }
        }
    }
}
