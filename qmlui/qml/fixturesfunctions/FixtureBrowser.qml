/*
  Q Light Controller Plus
  FixtureBrowser.qml

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
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.1

import org.qlcplus.classes 1.0
import "."

Rectangle
{
    anchors.fill: parent
    color: "transparent"

    property int manufacturerIndex: fixtureBrowser.manufacturerIndex
    property string selectedModel

    RowLayout
    {
        id: toolBar
        z: 1
        width: parent.width
        height: UISettings.iconSizeMedium

        Rectangle
        {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: UISettings.bgMedium
            radius: 5
            border.width: 2
            border.color: UISettings.borderColorDark

            Text
            {
                id: searchIcon
                x: 6
                width: height
                height: parent.height - 6
                anchors.verticalCenter: parent.verticalCenter
                color: "gray"
                font.family: "FontAwesome"
                font.pixelSize: height - 6
                text: FontAwesome.fa_search
            }

            TextInput
            {
                x: searchIcon.width + 14
                y: 3
                height: parent.height - 6
                width: parent.width - x
                color: UISettings.fgMain
                text: fixtureBrowser.searchFilter
                font.family: "Roboto Condensed"
                font.pixelSize: height - 6
                selectionColor: UISettings.highlightPressed
                selectByMouse: true

                onTextChanged: fixtureBrowser.searchFilter = text
            }
        }

        IconButton
        {
            width: height
            height: toolBar.height - 2
            imgSource: "qrc:/add.svg"
            tooltip: qsTr("Create a new fixture definition")
            onClicked: qlcplus.createFixture()
        }

        IconButton
        {
            id: editButton
            enabled: fixtureBrowser.selectedModel.length ? true : false
            width: height
            height: toolBar.height - 2
            imgSource: "qrc:/edit.svg"
            tooltip: qsTr("Edit the selected fixture definition")
            onClicked: qlcplus.editFixture(fixtureBrowser.selectedManufacturer, fixtureBrowser.selectedModel)
        }
    }

    ListView
    {
        id: manufacturerList
        visible: fixtureBrowser.selectedManufacturer.length === 0 && fixtureBrowser.searchFilter.length < 3
        z: 0
        width: parent.width - 12
        height: parent.height - toolBar.height - 12
        anchors.top: toolBar.bottom
        anchors.margins: 6
        focus: true

        boundsBehavior: Flickable.StopAtBounds
        currentIndex: manufacturerIndex

        highlight: Component
        {
            Rectangle
            {
                y: manufacturerList.currentItem.y
                width: parent.width
                height: UISettings.listItemHeight
                color: UISettings.highlight
            }
        }
        highlightFollowsCurrentItem: false

        model: fixtureBrowser.manufacturers
        delegate:
            FixtureBrowserDelegate
            {
                width: modelsList.width - (manufScroll.visible ? manufScroll.width : 0)
                isManufacturer: true
                textLabel: modelData
                onMouseEvent:
                {
                    if (type == App.Clicked)
                    {
                        mfText.label = modelData
                        fixtureBrowser.manufacturerIndex = index
                        fixtureBrowser.selectedManufacturer = modelData
                        modelsList.currentIndex = -1
                    }
                }
            }

        Component.onCompleted: manufacturerList.positionViewAtIndex(manufacturerIndex, ListView.Center)

        ScrollBar.vertical: CustomScrollBar { id: manufScroll }
    }

    Rectangle
    {
        id: fixtureArea
        visible: fixtureBrowser.selectedManufacturer.length && fixtureBrowser.searchFilter.length < 3
        color: "transparent"

        width: parent.width
        height: parent.height - toolBar.height - (fxPropsRect.visible ? fxPropsRect.height : 0)
        anchors.top: toolBar.bottom
        anchors.margins: 6

        Rectangle
        {
            id: manufBackLink
            height: UISettings.iconSizeMedium
            z: 1
            anchors.right: parent.right
            anchors.left: parent.left
            color: blMouseArea.pressed ? UISettings.bgLight : UISettings.bgMedium

            Image
            {
                id: leftArrow
                rotation: 180
                anchors.left: parent.left
                anchors.leftMargin: 5
                anchors.verticalCenter: parent.verticalCenter
                source: "qrc:/arrow-right.svg"
                sourceSize: Qt.size(width, height)
                height: parent.height
                width: height * 0.8
            }

            RobotoText
            {
                id: mfText
                anchors.left: leftArrow.right
                anchors.leftMargin: 10
                anchors.verticalCenter: parent.verticalCenter
                fontSize: UISettings.textSizeDefault
                fontBold: true
                labelColor: UISettings.fgMedium
            }
            MouseArea
            {
                id: blMouseArea
                anchors.fill: parent
                hoverEnabled: true

                onClicked:
                {
                    fxPropsRect.visible = false
                    panelPropsRect.visible = false
                    fixtureBrowser.selectedManufacturer = ""
                    editButton.enabled = false
                }
            }
        }

        ListView
        {
            id: modelsList
            z: 0

            width: parent.width - 12
            height: parent.height - manufBackLink.height - 12
            anchors.top: manufBackLink.bottom
            anchors.margins: 6

            focus: true
            boundsBehavior: Flickable.StopAtBounds
            highlight:
                Rectangle
                {
                    width: modelsList.width
                    height: UISettings.listItemHeight - 2
                    color: UISettings.highlight
                    y: modelsList.currentItem ? modelsList.currentItem.y + 1 : 0
                }
            highlightFollowsCurrentItem: false

            model: fixtureBrowser.modelsList
            delegate:
                FixtureBrowserDelegate
                {
                    id: dlg
                    width: modelsList.width - (modelsScroll.visible ? modelsScroll.width : 0)
                    manufacturer: fixtureBrowser.selectedManufacturer
                    textLabel: modelData

                    onMouseEvent:
                    {
                        if (type == App.Clicked)
                        {
                            modelsList.currentIndex = index
                            fixtureBrowser.selectedModel = modelData
                            if (modelData == "Generic RGB Panel")
                            {
                                fxPropsRect.visible = false
                                panelPropsRect.visible = true
                            }
                            else
                            {
                                panelPropsRect.visible = false
                                fxPropsRect.visible = true
                            }
                            editButton.enabled = true
                        }
                    }
                }
            ScrollBar.vertical: CustomScrollBar { id: modelsScroll }
        }
    }

    Flickable
    {
        id: searchRect
        clip: true
        visible: fixtureBrowser.searchFilter.length >= 3 ? true : false
        boundsBehavior: Flickable.StopAtBounds

        contentHeight: searchColumn.height

        width: parent.width
        height: parent.height - toolBar.height - (fxPropsRect.visible ? fxPropsRect.height : 0) - 12
        anchors.top: toolBar.bottom
        anchors.margins: 6

        Column
        {
            id: searchColumn
            width: parent.width

            Repeater
            {
                id: searchListView
                anchors.fill: parent
                z: 4
                model: fixtureBrowser.searchTreeModel
                delegate:
                    Component
                    {
                        Loader
                        {
                            width: searchListView.width
                            source: hasChildren ? "qrc:/TreeNodeDelegate.qml" : "qrc:/FixtureBrowserDelegate.qml"
                            onLoaded:
                            {
                                if (hasChildren)
                                {
                                    item.textLabel = label
                                    item.nodePath = path
                                    item.itemIcon = ""
                                    item.isExpanded = true
                                    item.childrenDelegate = "qrc:/FixtureBrowserDelegate.qml"
                                    item.nodeChildren = childrenModel
                                }
                            }
                            Connections
                            {
                                target: item
                                function onMouseEvent(type, iID, iType, qItem, mouseMods)
                                {
                                    if (type === App.Clicked)
                                    {
                                        console.log("Item clicked with path: " + item.nodePath + "/" + qItem.textLabel)
                                        model.isSelected = 2
                                        qItem.manufacturer = item.nodePath
                                        fixtureBrowser.selectedManufacturer = qItem.manufacturer
                                        fixtureBrowser.selectedModel = qItem.textLabel
                                        fxPropsRect.visible = true
                                    }
                                }
                            }
                        }
                    }
            } // end of Repeater
        } // end of Column

        ScrollBar.vertical: CustomScrollBar { }
    } // end of Flickable

    FixtureProperties
    {
        id: fxPropsRect
        anchors.right: parent.right
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 8
        visible: false
    }

    RGBPanelProperties
    {
        objectName: "RGBPanelProps"
        id: panelPropsRect
        anchors.right: parent.right
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 8
        visible: false
    }
}
