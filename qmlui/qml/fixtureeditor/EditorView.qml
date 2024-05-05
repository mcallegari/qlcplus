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
    property EditorRef editorView: null

    color: "transparent"

    function initialize()
    {
        if (editorView.isUser === false)
        {
            messagePopup.message = qsTr("You are trying to edit a bundled fixture definition.<br>" +
                                        "If you modify and save it, a new file will be stored in<br><i>" +
                                        fixtureEditor.userFolder + "</i><br>and will override the bundled file.")
            messagePopup.open()
        }
    }

    function save(path)
    {
        //console.log("MANUFACTURER: " + editorView.manufacturer + ", MODEL: " + editorView.model)

        if (editorView.manufacturer === "" || editorView.model === "")
        {
            messagePopup.message = qsTr("Manufacturer or model cannot be empty!")
            messagePopup.open()
            return
        }

        var errors = ""

        if (path)
            errors = editorView.saveAs(path)
        else
            errors = editorView.save()

        if (errors != "")
        {
            messagePopup.message = qsTr("The following errors have been detected:") + "<br><ul>" + errors + "</ul>"
            messagePopup.open()
        }
    }

    CustomPopupDialog
    {
        id: messagePopup
        width: mainView.width / 2
        standardButtons: Dialog.Ok
        title: qsTr("!! Warning !!")
        onAccepted: close()
    }

    SplitView
    {
        anchors.fill: parent

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
                            columns: 3

                            // row 2
                            RobotoText { label: qsTr("Manufacturer") }
                            CustomTextEdit
                            {
                                id: manufacturerEdit
                                Layout.fillWidth: true
                                text: editorView ? editorView.manufacturer : ""
                                onTextChanged: if (editorView) editorView.manufacturer = text
                                KeyNavigation.tab: modelEdit
                            }

                            // row 1
                            GroupBox
                            {
                                title: qsTr("Type")
                                Layout.rowSpan: 3
                                implicitWidth: UISettings.bigItemHeight + rightPadding * 2
                                font.family: UISettings.robotoFontName
                                font.pixelSize: UISettings.textSizeDefault
                                palette.windowText: UISettings.fgMain

                                IconPopupButton
                                {
                                    ListModel
                                    {
                                        id: typeModel
                                        ListElement { mLabel: "Color Changer"; mIcon: "qrc:/fixture.svg"; mValue: 0 }
                                        ListElement { mLabel: "Dimmer"; mIcon: "qrc:/dimmer.svg"; mValue: 1 }
                                        ListElement { mLabel: "Effect"; mIcon: "qrc:/effect.svg"; mValue: 2 }
                                        ListElement { mLabel: "Fan"; mIcon: "qrc:/fan.svg"; mValue: 3 }
                                        ListElement { mLabel: "Flower"; mIcon: "qrc:/flower.svg"; mValue: 4 }
                                        ListElement { mLabel: "Hazer"; mIcon: "qrc:/hazer.svg"; mValue: 5 }
                                        ListElement { mLabel: "Laser"; mIcon: "qrc:/laser.svg"; mValue: 6 }
                                        ListElement { mLabel: "LED Bar (Beams)"; mIcon: "qrc:/ledbar_beams.svg"; mValue: 7 }
                                        ListElement { mLabel: "LED Bar (Pixels)"; mIcon: "qrc:/ledbar_pixels.svg"; mValue: 8 }
                                        ListElement { mLabel: "Moving Head"; mIcon: "qrc:/movinghead.svg"; mValue: 9 }
                                        ListElement { mLabel: "Other"; mIcon: "qrc:/other.svg"; mValue: 10 }
                                        ListElement { mLabel: "Scanner"; mIcon: "qrc:/scanner.svg"; mValue: 11 }
                                        ListElement { mLabel: "Smoke"; mIcon: "qrc:/smoke.svg"; mValue: 12 }
                                        ListElement { mLabel: "Strobe"; mIcon: "qrc:/strobe.svg"; mValue: 13 }
                                    }

                                    implicitHeight: UISettings.bigItemHeight
                                    delegateHeight: UISettings.listItemHeight
                                    width: UISettings.bigItemHeight
                                    model: typeModel
                                    currValue: editorView ? editorView.productType : 0
                                    onValueChanged: if (editorView) editorView.productType = value
                                }
                            }

                            // row 2
                            RobotoText { label: qsTr("Model") }
                            CustomTextEdit
                            {
                                id: modelEdit
                                Layout.fillWidth: true
                                text: editorView ? editorView.model : ""
                                onTextChanged: if (editorView) editorView.model = text
                                KeyNavigation.tab: authorEdit
                            }

                            // row 4
                            RobotoText { label: qsTr("Author") }
                            CustomTextEdit
                            {
                                id: authorEdit
                                Layout.fillWidth: true
                                text: editorView ? editorView.author : ""
                                onTextChanged: if (editorView) editorView.author = text
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
                            phy: editorView ? editorView.globalPhysical : null
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
                                        onClicked:
                                        {
                                            sideEditor.active = false
                                            sideEditor.itemName = ""
                                            sideEditor.source = "qrc:/ChannelEditor.qml"
                                            sideEditor.active = true
                                        }
                                    }

                                    IconButton
                                    {
                                        id: delChButton
                                        imgSource: "qrc:/remove.svg"
                                        tooltip: qsTr("Remove the selected channel(s)")
                                        enabled: chanSelector.itemsCount
                                        onClicked:
                                        {
                                            // retrieve selected indices from model selector and
                                            // channel references from the ListView items
                                            var refsArray = []
                                            var selItems = chanSelector.itemsList()

                                            for (var i = 0; i < selItems.length; i++)
                                                refsArray.push(channelList.itemAtIndex(selItems[i]).cRef)

                                            editorView.deleteChannels(refsArray)
                                            cDragItem.itemsList = []
                                        }
                                    }

                                    Rectangle
                                    {
                                        Layout.fillWidth: true
                                        color: "transparent"
                                    }

                                    IconButton
                                    {
                                        id: chWizButton
                                        imgSource: "qrc:/wizard.svg"
                                        tooltip: qsTr("Channel wizard")
                                        onClicked: wizardPopup.open()

                                        PopupChannelWizard
                                        {
                                            id: wizardPopup
                                            editorView: editorRoot.editorView
                                        }
                                    }
                                }
                            } // Rectangle - toolbar

                            ListView
                            {
                                id: channelList
                                width: channelSection.width
                                height: UISettings.listItemHeight * count
                                boundsBehavior: Flickable.StopAtBounds
                                interactive: false

                                property bool dragActive: false

                                model: editorView ? editorView.channels : null
                                delegate:
                                    Item
                                    {
                                        id: itemRoot
                                        width: channelList.width
                                        height: UISettings.listItemHeight

                                        property QLCChannel cRef: model.cRef
                                        property alias chanDelegate: delegateRoot.channelDelegate

                                        MouseArea
                                        {
                                            id: delegateRoot
                                            width: channelList.width
                                            height: parent.height
                                            propagateComposedEvents: true

                                            property bool dragActive: drag.active
                                            property Item channelDelegate: cDelegate

                                            drag.target: cDragItem
                                            drag.threshold: height / 2

                                            onPressed:
                                            {
                                                var posnInWindow = cDelegate.mapToItem(mainView, cDelegate.x, cDelegate.y)
                                                cDragItem.parent = mainView
                                                cDragItem.x = posnInWindow.x - (cDragItem.width / 4)
                                                cDragItem.y = posnInWindow.y - (cDragItem.height / 4)
                                                cDragItem.z = 10
                                            }

                                            onClicked:
                                            {
                                                chanSelector.selectItem(index, channelList.model, mouse.modifiers)
                                            }

                                            onDoubleClicked:
                                            {
                                                sideEditor.active = false
                                                sideEditor.itemName = itemRoot.cRef.name
                                                sideEditor.source = "qrc:/ChannelEditor.qml"
                                                sideEditor.active = true
                                            }

                                            onDragActiveChanged:
                                            {
                                                if (dragActive)
                                                {
                                                    cDragItem.itemLabel = cEntryItem.tLabel
                                                    cDragItem.itemIcon = cEntryItem.iSrc
                                                    channelList.dragActive = true
                                                }
                                                else
                                                {
                                                    cDragItem.Drag.drop()
                                                    cDragItem.parent = channelList
                                                    cDragItem.x = 0
                                                    cDragItem.y = 0
                                                    channelList.dragActive = false
                                                }
                                            }

                                            Rectangle
                                            {
                                                id: cDelegate
                                                width: channelList.width
                                                height: UISettings.listItemHeight
                                                color: "transparent"

                                                //property int itemType: App.ChannelDragItem
                                                property QLCChannel cRef: itemRoot.cRef

                                                Rectangle
                                                {
                                                    anchors.fill: parent
                                                    radius: 3
                                                    color: UISettings.highlight
                                                    visible: model.isSelected
                                                }

                                                IconTextEntry
                                                {
                                                    id: cEntryItem
                                                    width: channelList.width
                                                    height: UISettings.listItemHeight
                                                    tLabel: cDelegate.cRef ? cDelegate.cRef.name : ""
                                                    iSrc: cDelegate.cRef ? cDelegate.cRef.getIconNameFromGroup(cDelegate.cRef.group, true) : ""
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
                                    }

                                ModelSelector
                                {
                                    id: chanSelector
                                    onItemsCountChanged:
                                    {
                                        cDragItem.itemsList = []
                                        var selItems = itemsList()

                                        for (var i = 0; i < selItems.length; i++)
                                        {
                                            var item = channelList.itemAtIndex(selItems[i])
                                            cDragItem.itemsList.push(item.chanDelegate)
                                        }
                                    }
                                }

                                GenericMultiDragItem
                                {
                                    id: cDragItem
                                    visible: channelList.dragActive

                                    property bool fromMainEditor: true

                                    Drag.active: channelList.dragActive
                                    Drag.source: cDragItem
                                    Drag.keys: [ "channel" ]
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
                                        onClicked:
                                        {
                                            sideEditor.active = false
                                            sideEditor.itemName = ""
                                            sideEditor.source = "qrc:/ModeEditor.qml"
                                            sideEditor.active = true
                                        }
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
                                interactive: false

                                model: editorView ? editorView.modes : null
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
                                                sideEditor.itemName = model.name
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
                                                tLabel: model.name
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
            asynchronous: false

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

                item.editorView = editorRoot.editorView
                item.setItemName(itemName)
            }
        }
    }
}
