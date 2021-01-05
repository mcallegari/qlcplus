/*
  Q Light Controller Plus
  EditorView.qml

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

import QtQuick 2.2
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.13

import org.qlcplus.classes 1.0
import "."

Rectangle
{
    id: editorRoot

    property int editorId
    property EditorRef editor: null

    color: "transparent"

    SplitView
    {
        anchors.fill: parent
        orientation: Qt.Horizontal

        // left view: definition sections
        Flickable
        {
            id: editorFlickable
            Layout.fillWidth: true
            height: parent.height

            contentHeight: editorColumn.height
            boundsBehavior: Flickable.StopAtBounds

            SplitView.minimumWidth: editorRoot.width * 0.2
            SplitView.preferredWidth: editorRoot.width * 0.5
            SplitView.maximumWidth: editorRoot.width * 0.8

            Column
            {
                id: editorColumn
                width: parent.width - (sbar.visible ? sbar.width : 0)

                SectionBox
                {
                    id: generalSection
                    width: parent.width
                    sectionLabel: qsTr("General")

                    sectionContents:
                        GridLayout
                        {
                            width: Math.min(editorRoot.width / 2, parent.width)
                            columns: 2

                            // row 1
                            RobotoText { label: qsTr("Manufacturer") }
                            CustomTextEdit
                            {
                                Layout.fillWidth: true
                                text: editor ? editor.manufacturer : ""
                                onTextChanged: if (editor) editor.manufacturer = text
                            }

                            // row 2
                            RobotoText { label: qsTr("Model") }
                            CustomTextEdit
                            {
                                Layout.fillWidth: true
                                text: editor ? editor.model : ""
                                onTextChanged: if (editor) editor.model = text
                            }

                            // row 3
                            RobotoText { label: qsTr("Type") }
                            CustomComboBox
                            {
                                ListModel
                                {
                                    id: typeModel
                                    ListElement { mLabel: "Color Changer"; mIcon: "qrc:/fixture.svg" }
                                    ListElement { mLabel: "Dimmer"; mIcon: "qrc:/dimmer.svg" }
                                    ListElement { mLabel: "Effect"; mIcon: "qrc:/effect.svg" }
                                    ListElement { mLabel: "Fan"; mIcon: "qrc:/fan.svg" }
                                    ListElement { mLabel: "Flower"; mIcon: "qrc:/flower.svg" }
                                    ListElement { mLabel: "Hazer"; mIcon: "qrc:/hazer.svg" }
                                    ListElement { mLabel: "Laser"; mIcon: "qrc:/laser.svg" }
                                    ListElement { mLabel: "LED Bar (Beams)"; mIcon: "qrc:/ledbar_beams.svg" }
                                    ListElement { mLabel: "LED Bar (Pixels)"; mIcon: "qrc:/ledbar_pixels.svg" }
                                    ListElement { mLabel: "Moving Head"; mIcon: "qrc:/movinghead.svg" }
                                    ListElement { mLabel: "Other"; mIcon: "qrc:/other.svg" }
                                    ListElement { mLabel: "Scanner"; mIcon: "qrc:/scanner.svg" }
                                    ListElement { mLabel: "Smoke"; mIcon: "qrc:/smoke.svg" }
                                    ListElement { mLabel: "Strobe"; mIcon: "qrc:/strobe.svg" }
                                }

                                Layout.fillWidth: true
                                implicitHeight: UISettings.iconSizeDefault
                                delegateHeight: UISettings.iconSizeDefault
                                model: typeModel
                            }

                            // row 4
                            RobotoText { label: qsTr("Author") }
                            CustomTextEdit
                            {
                                Layout.fillWidth: true
                                text: editor ? editor.author : ""
                                onTextChanged: if (editor) editor.author = text
                            }
                        }
                } // SectionBox - General

                SectionBox
                {
                    id: phySection
                    width: parent.width
                    sectionLabel: qsTr("Physical properties")

                    sectionContents:
                        PhysicalProperties
                        {
                            width: Math.min(editorRoot.width / 2, parent.width)
                            phy: editor ? editor.globalPhysical : null
                        }
                } // SectionBox - Physical

                SectionBox
                {
                    id: channelSection
                    width: parent.width
                    sectionLabel: qsTr("Channels")

                    sectionContents:
                        Column
                        {
                            width: channelSection.width
                            //height: chEditToolbar.height + channelList.height

                            Rectangle
                            {
                                id: chEditToolbar
                                width: channelSection.width
                                height: UISettings.iconSizeDefault
                                gradient: Gradient
                                {
                                    GradientStop { position: 0; color: UISettings.toolbarStartSub }
                                    GradientStop { position: 1; color: UISettings.toolbarEnd }
                                }

                                RowLayout
                                {
                                    anchors.fill: parent

                                    IconButton
                                    {
                                        id: newChButton
                                        imgSource: "qrc:/add.svg"
                                        tooltip: qsTr("Add a new channel")
                                        onClicked: { /* TODO */ }
                                    }

                                    IconButton
                                    {
                                        id: delChButton
                                        imgSource: "qrc:/remove.svg"
                                        tooltip: qsTr("Remove the selected channel(s)")
                                        onClicked: { /* TODO */ }
                                    }

                                    Rectangle
                                    {
                                        Layout.fillWidth: true
                                        color: "transparent"
                                    }
                                }
                            } // Rectangle - toolbar

                            ListView
                            {
                                id: channelList
                                width: channelSection.width
                                height: UISettings.listItemHeight * count
                                boundsBehavior: Flickable.StopAtBounds
                                currentIndex: -1

                                property bool dragActive: false

                                model: editor ? editor.channels : null
                                delegate:
                                    Item
                                    {
                                        width: channelList.width
                                        height: UISettings.listItemHeight

                                        MouseArea
                                        {
                                            id: delegateRoot
                                            width: channelList.width
                                            height: parent.height

                                            property bool dragActive: drag.active

                                            drag.target: cDragItem
                                            drag.threshold: height / 2

                                            onDragActiveChanged: channelList.dragActive = dragActive
                                            onPressed: channelList.currentIndex = index
                                            onDoubleClicked:
                                            {
                                                sideEditor.source = ""
                                                sideEditor.itemName = modelData.mLabel
                                                sideEditor.source = "qrc:/ChannelEditor.qml"
                                            }

                                            Rectangle
                                            {
                                                anchors.fill: parent
                                                radius: 3
                                                color: UISettings.highlight
                                                visible: channelList.currentIndex === index
                                            }

                                            IconTextEntry
                                            {
                                                width: channelList.width
                                                height: UISettings.listItemHeight
                                                tLabel: modelData.mLabel
                                                iSrc: modelData.mIcon
                                            }

                                            Rectangle
                                            {
                                                width: parent.width
                                                height: 1
                                                y: parent.height - 1
                                                color: UISettings.fgMedium
                                            }
                                        }
                                    }

                                GenericMultiDragItem
                                {
                                    id: cDragItem

                                    property bool fromFunctionManager: true

                                    visible: channelList.dragActive

                                    Drag.active: channelList.dragActive
                                    Drag.source: cDragItem
                                    Drag.keys: [ "channel" ]

                                    function itemDropped(id, name)
                                    {
                                        //var path = functionManager.functionPath(id)
                                        //functionManager.moveFunctions(path)
                                    }

                                    onItemsListChanged:
                                    {
                                        console.log("Items in list: " + itemsList.length)
                                        /*
                                        if (itemsList.length)
                                        {
                                            var funcRef = functionManager.getFunction(itemsList[0])
                                            itemLabel = funcRef.name
                                            itemIcon = functionManager.functionIcon(funcRef.type)
                                            //multipleItems = itemsList.length > 1 ? true : false
                                        }
                                        */
                                    }
                                }
                            } // ListView
                        } // Column
                } // SectionBox - Channels

                SectionBox
                {
                    id: modeSection
                    width: parent.width
                    sectionLabel: qsTr("Modes")

                    sectionContents:
                        Column
                        {
                            width: modeSection.width
                            //height: modeEditToolbar.height + modeList.height

                            Rectangle
                            {
                                id: modeEditToolbar
                                width: modeSection.width
                                height: UISettings.iconSizeDefault
                                gradient: Gradient
                                {
                                    GradientStop { position: 0; color: UISettings.toolbarStartSub }
                                    GradientStop { position: 1; color: UISettings.toolbarEnd }
                                }

                                RowLayout
                                {
                                    anchors.fill: parent

                                    IconButton
                                    {
                                        id: newModeButton
                                        imgSource: "qrc:/add.svg"
                                        tooltip: qsTr("Add a new mode")
                                        onClicked: { /* TODO */ }
                                    }

                                    IconButton
                                    {
                                        id: delModeButton
                                        imgSource: "qrc:/remove.svg"
                                        tooltip: qsTr("Remove the selected mode(s)")
                                        onClicked: { /* TODO */ }
                                    }

                                    Rectangle
                                    {
                                        Layout.fillWidth: true
                                        color: "transparent"
                                    }
                                }
                            } // Rectangle - toolbar

                            ListView
                            {
                                id: modeList
                                width: modeSection.width
                                height: UISettings.listItemHeight * count
                                boundsBehavior: Flickable.StopAtBounds
                                currentIndex: -1

                                model: editor ? editor.modes : null
                                delegate:
                                    Item
                                    {
                                        width: modeList.width
                                        height: UISettings.listItemHeight

                                        MouseArea
                                        {
                                            width: modeList.width
                                            height: parent.height

                                            onPressed: modeList.currentIndex = index
                                            onDoubleClicked:
                                            {
                                                sideEditor.source = ""
                                                sideEditor.itemName = modelData.mLabel
                                                sideEditor.source = "qrc:/ModeEditor.qml"
                                            }

                                            Rectangle
                                            {
                                                anchors.fill: parent
                                                radius: 3
                                                color: UISettings.highlight
                                                visible: modeList.currentIndex === index
                                            }

                                            IconTextEntry
                                            {
                                                width: modeList.width
                                                height: UISettings.listItemHeight
                                                tLabel: modelData.mLabel
                                                faSource: FontAwesome.fa_list
                                                faColor: UISettings.fgMain
                                            }

                                            Rectangle
                                            {
                                                width: parent.width
                                                height: 1
                                                y: parent.height - 1
                                                color: UISettings.fgMedium
                                            }
                                        }
                                    }
                            } // ListView
                        } // Column
                } // SectionBox - Modes

                SectionBox
                {
                    id: aliasSection
                    width: parent.width
                    sectionLabel: qsTr("Aliases")

                    sectionContents: null
                } // SectionBox - Alias

            } // Column
            ScrollBar.vertical: CustomScrollBar { id: sbar }
        } // Flickable

        // right view: editors
        Loader
        {
            id: sideEditor

            SplitView.minimumWidth: editorRoot.width * 0.2
            SplitView.preferredWidth: editorRoot.width * 0.5
            SplitView.maximumWidth: editorRoot.width * 0.8

            property string itemName: ""

            onLoaded:
            {
                item.x = 10
                item.y = 10
                item.width = Qt.binding(function() { return sideEditor.width - 20 })
                item.height = Qt.binding(function() { return sideEditor.height - 20 })

                item.editor = editorRoot.editor
                item.itemName = itemName
            }
        }
    }
}
