/*
  Q Light Controller Plus
  WizardFixtureBrowser.qml

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
import QtQuick.Controls.Basic
import "."

/*
 * Self-contained fixture browser for the Show Wizard.
 * Uses fixtureBrowser (C++ context property) for the model/manufacturer data
 * and fixtureManager.addFixture() to patch fixtures — no FixtureDrag.js,
 * no references to IDs outside this component.
 */
Item
{
    id: root
    anchors.fill: parent

    // ── Search bar ─────────────────────────────────────────────────────────
    Rectangle
    {
        id: searchBar
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: UISettings.listItemHeight
        color: UISettings.bgMedium
        radius: 4
        border.color: UISettings.borderColorDark
        border.width: 1

        Row
        {
            anchors.fill: parent
            anchors.leftMargin: 6
            anchors.rightMargin: 4
            spacing: 4

            Text
            {
                anchors.verticalCenter: parent.verticalCenter
                color: "#777799"
                font.family: UISettings.fontAwesomeFontName
                font.pixelSize: UISettings.textSizeDefault
                text: FontAwesome.fa_magnifying_glass
            }

            TextInput
            {
                id: searchInput
                width: parent.width - 30
                height: parent.height - 4
                anchors.verticalCenter: parent.verticalCenter
                color: UISettings.fgMain
                font.family: UISettings.robotoFontName
                font.pixelSize: UISettings.textSizeDefault
                selectionColor: UISettings.highlightPressed
                selectByMouse: true
                onTextEdited: fixtureBrowser.searchFilter = text
            }
        }
    }

    // ── Manufacturer list ──────────────────────────────────────────────────
    ListView
    {
        id: manufList
        anchors.top: searchBar.bottom
        anchors.topMargin: 4
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        clip: true
        visible: fixtureBrowser.selectedManufacturer.length === 0 &&
                 fixtureBrowser.searchFilter.length < 3
        model: fixtureBrowser.manufacturers
        boundsBehavior: Flickable.StopAtBounds
        ScrollBar.vertical: CustomScrollBar {}

        delegate: Rectangle
        {
            width: manufList.width
            height: UISettings.listItemHeight
            color: mfMouse.containsMouse ? UISettings.highlight : (index % 2 ? UISettings.bgMedium : "transparent")

            Row
            {
                anchors.fill: parent
                anchors.leftMargin: 8
                spacing: 6

                RobotoText
                {
                    anchors.verticalCenter: parent.verticalCenter
                    label: modelData
                    fontSize: UISettings.textSizeDefault
                    labelColor: UISettings.fgMain
                }
            }

            MouseArea
            {
                id: mfMouse
                anchors.fill: parent
                hoverEnabled: true
                onClicked:
                {
                    fixtureBrowser.selectedManufacturer = modelData
                    fixtureBrowser.manufacturerIndex = index
                    modelList.currentIndex = -1
                }
            }
        }
    }

    // ── Model list (after manufacturer selected) ───────────────────────────
    Item
    {
        anchors.top: searchBar.bottom
        anchors.topMargin: 4
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        visible: fixtureBrowser.selectedManufacturer.length > 0 &&
                 fixtureBrowser.searchFilter.length < 3

        // Back link
        Rectangle
        {
            id: backBar
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            height: UISettings.listItemHeight
            color: backMouse.containsMouse ? UISettings.bgLight : UISettings.bgMedium

            Row
            {
                anchors.fill: parent
                anchors.leftMargin: 8
                spacing: 6

                Text
                {
                    anchors.verticalCenter: parent.verticalCenter
                    color: UISettings.fgLight
                    font.family: UISettings.fontAwesomeFontName
                    font.pixelSize: UISettings.textSizeDefault
                    text: FontAwesome.fa_chevron_left
                }
                RobotoText
                {
                    anchors.verticalCenter: parent.verticalCenter
                    label: fixtureBrowser.selectedManufacturer
                    fontSize: UISettings.textSizeDefault
                    fontBold: true
                    labelColor: UISettings.fgMedium
                }
            }

            MouseArea
            {
                id: backMouse
                anchors.fill: parent
                hoverEnabled: true
                onClicked:
                {
                    fixtureBrowser.selectedManufacturer = ""
                    fixtureBrowser.selectedModel = ""
                    propsPanel.visible = false
                }
            }
        }

        ListView
        {
            id: modelList
            anchors.top: backBar.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: propsPanel.visible ? propsPanel.top : addBtn.top
            anchors.bottomMargin: 4
            clip: true
            model: fixtureBrowser.modelsList
            boundsBehavior: Flickable.StopAtBounds
            ScrollBar.vertical: CustomScrollBar {}

            delegate: Rectangle
            {
                width: modelList.width
                height: UISettings.listItemHeight
                color: modelList.currentIndex === index ? UISettings.highlight
                     : (mdMouse.containsMouse ? UISettings.bgLight : (index % 2 ? UISettings.bgMedium : "transparent"))

                RobotoText
                {
                    anchors.fill: parent
                    anchors.leftMargin: 12
                    label: modelData
                    fontSize: UISettings.textSizeDefault
                    labelColor: UISettings.fgMain
                }

                MouseArea
                {
                    id: mdMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    onClicked:
                    {
                        modelList.currentIndex = index
                        fixtureBrowser.selectedModel = modelData
                        propsPanel.visible = true
                    }
                }
            }
        }

        // Properties panel (universe, address, mode, quantity)
        FixtureProperties
        {
            id: propsPanel
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: addBtn.top
            anchors.bottomMargin: 4
            visible: false
        }

        // Add Fixture button
        GenericButton
        {
            id: addBtn
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 4
            height: UISettings.listItemHeight
            label: qsTr("+ Add Fixture to Project")
            bgColor: "#0550AA"
            hoverColor: "#0978FF"
            fgColor: "white"
            enabled: fixtureBrowser.selectedModel.length > 0
            onClicked:
            {
                fixtureManager.addFixture(
                    fixtureBrowser.selectedManufacturer,
                    fixtureBrowser.selectedModel,
                    fixtureBrowser.selectedMode,
                    fixtureBrowser.fixtureName,
                    propsPanel.fxUniverseIndex,
                    propsPanel.fxAddress - 1,
                    propsPanel.fxChannels,
                    propsPanel.fxQuantity,
                    propsPanel.fxGap,
                    0, 0)
            }
        }
    }

    // ── Search results ─────────────────────────────────────────────────────
    ListView
    {
        anchors.top: searchBar.bottom
        anchors.topMargin: 4
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        clip: true
        visible: fixtureBrowser.searchFilter.length >= 3
        model: fixtureBrowser.manufacturers
        boundsBehavior: Flickable.StopAtBounds
        ScrollBar.vertical: CustomScrollBar {}

        delegate: Rectangle
        {
            width: parent ? parent.width : 0
            height: UISettings.listItemHeight
            color: srMouse.containsMouse ? UISettings.highlight : (index % 2 ? UISettings.bgMedium : "transparent")

            RobotoText
            {
                anchors.fill: parent
                anchors.leftMargin: 8
                label: modelData
                fontSize: UISettings.textSizeDefault
                labelColor: UISettings.fgMain
            }

            MouseArea
            {
                id: srMouse
                anchors.fill: parent
                hoverEnabled: true
                onClicked:
                {
                    fixtureBrowser.selectedManufacturer = modelData
                    fixtureBrowser.searchFilter = ""
                    searchInput.text = ""
                }
            }
        }
    }
}
