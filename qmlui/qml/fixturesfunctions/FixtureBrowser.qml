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
import QtQuick.Controls 1.1

import org.qlcplus.classes 1.0
import "."

Rectangle
{
    id: fxBrowserBox
    anchors.fill: parent
    color: "transparent"

    property int manufacturerIndex: fixtureBrowser.manufacturerIndex
    property string selectedModel

    Rectangle
    {
        id: searchBox
        z: 1
        height: UISettings.iconSizeMedium
        anchors.right: parent.right
        anchors.rightMargin: 8
        anchors.left: parent.left
        anchors.leftMargin: 8
        color: UISettings.bgMain
        radius: 5
        border.width: 2
        border.color: "#111"

        Text
        {
            id: searchIcon
            x: 3
            y: 3
            width: height
            height: parent.height - 6
            color: "gray"
            font.family: "FontAwesome"
            font.pixelSize: height
            text: FontAwesome.fa_search
        }

        TextInput
        {
            id: textEdit1
            x: searchIcon.width + 10
            y: 3
            height: parent.height - 6
            width: searchBox.width - searchIcon.width - 10
            color: UISettings.fgMain
            text: fixtureBrowser.searchFilter
            font.family: "Roboto Condensed"
            font.pixelSize: height
            selectionColor: UISettings.highlightPressed
            selectByMouse: true

            onTextChanged: fixtureBrowser.searchFilter = text
        }
    }

    ListView
    {
        id: manufacturerList
        x: 8
        z: 0
        visible: fixtureBrowser.selectedManufacturer.length == 0 && fixtureBrowser.searchFilter.length < 3
        anchors.top: searchBox.bottom
        anchors.topMargin: 6
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 6
        anchors.right: parent.right
        anchors.rightMargin: 8
        anchors.left: parent.left
        anchors.leftMargin: 8
        focus: true

        boundsBehavior: Flickable.StopAtBounds
        currentIndex: manufacturerIndex

        highlight: Component
        {
            Rectangle
            {
                y: manufacturerList.currentItem.y
                width: parent.width - 30
                height: UISettings.listItemHeight
                color: "#0978FF"
                radius: 5
            }
        }
        highlightFollowsCurrentItem: false

        model: fixtureBrowser.manufacturers
        delegate: FixtureBrowserDelegate
        {
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

        CustomScrollBar { flickable: manufacturerList }
    }

    Rectangle
    {
        id: fixtureArea
        visible: fixtureBrowser.selectedManufacturer.length && fixtureBrowser.searchFilter.length < 3
        color: "transparent"

        width: parent.width
        height: parent.height - searchBox.height - (fxPropsRect.visible ? fxPropsRect.height : 0)
        anchors.top: searchBox.bottom
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
                }
            }
        }

        ListView
        {
            id: modelsList
            z: 0

            width: parent.width
            height: parent.height - manufBackLink.height - 12
            anchors.top: manufBackLink.bottom
            anchors.margins: 6

            focus: true
            boundsBehavior: Flickable.StopAtBounds
            highlight:
                Rectangle
                {
                    width: modelsList.width - 30
                    height: UISettings.listItemHeight - 2
                    color: UISettings.highlight
                    radius: 5
                    y: modelsList.currentItem ? modelsList.currentItem.y + 1 : 0
                }
            highlightFollowsCurrentItem: false

            model: fixtureBrowser.modelsList
            delegate: FixtureBrowserDelegate
            {
                id: dlg
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
                    }
                }
            }
            CustomScrollBar { flickable: modelsList }
        }
    }

    Flickable
    {
        id: searchRect
        clip: true
        visible: fixtureBrowser.searchFilter.length >= 3 ? true : false

        contentHeight: searchColumn.height

        width: parent.width
        height: parent.height - searchBox.height - (fxPropsRect.visible ? fxPropsRect.height : 0) - 12
        anchors.top: searchBox.bottom
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
                                onMouseEvent:
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
    } // end of Flickable
    CustomScrollBar { flickable: searchRect }

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
