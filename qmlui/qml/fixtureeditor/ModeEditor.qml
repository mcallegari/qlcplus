/*
  Q Light Controller Plus
  ModeEditor.qml

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

import QtQuick 2.14
import QtQuick.Layouts 1.14
import QtQuick.Controls 2.14

import org.qlcplus.classes 1.0
import "."

Rectangle
{
    id: editorRoot
    color: "transparent"

    property EditorRef editorView: null
    property ModeEdit mode: null

    function setItemName(name)
    {
        mode = editorView.requestModeEditor(name)
        nameEdit.selectAndFocus()
    }

    ModelSelector
    {
        id: modeChanSelector
    }
    ModelSelector
    {
        id: modeHeadSelector
    }

    Flickable
    {
        id: editorFlickable
        width: parent.width - (sbar.visible ? sbar.width : 0)
        height: parent.height

        contentHeight: editorColumn.height
        boundsBehavior: Flickable.StopAtBounds

        Column
        {
            id: editorColumn
            width: parent.width - (sbar.visible ? sbar.width : 0)
            spacing: 3

            RowLayout
            {
                width: parent.width
                height: nameEdit.height

                RobotoText
                {
                    height: UISettings.listItemHeight
                    label: qsTr("Name")
                }
                CustomTextEdit
                {
                    id: nameEdit
                    Layout.fillWidth: true
                    Layout.columnSpan: 3
                    text: mode ? mode.name : ""
                    onTextChanged: if (mode) mode.name = text
                }
            }

            SectionBox
            {
                id: channelSection
                width: parent.width
                sectionLabel: qsTr("Channels")

                sectionContents:
                    Rectangle
                    {
                        width: channelSection.width
                        height: chEditToolbar.height + Math.max(channelList.height + UISettings.listItemHeight, UISettings.bigItemHeight)
                        color: "transparent"

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
                                    faSource: FontAwesome.fa_certificate
                                    faColor: UISettings.fgMain
                                    tooltip: qsTr("Create a new emitter")
                                    enabled: modeChanSelector.itemsCount

                                    onClicked:
                                    {
                                        mode.addHead(modeChanSelector.itemsList())
                                    }

                                    Image
                                    {
                                        x: parent.width - width - 2
                                        y: 2
                                        width: parent.height / 2
                                        height: width
                                        source: "qrc:/add.svg"
                                        sourceSize: Qt.size(width, height)
                                    }
                                }

                                IconButton
                                {
                                    id: delChButton
                                    imgSource: "qrc:/remove.svg"
                                    tooltip: qsTr("Remove the selected channel(s)")
                                    enabled: modeChanSelector.itemsCount

                                    onClicked:
                                    {
                                        for (var i = 0; i < mcDragItem.itemsList.length; i++)
                                            mode.deleteChannel(mcDragItem.itemsList[i].cRef)

                                        mcDragItem.itemsList = []
                                    }
                                }

                                Rectangle
                                {
                                    Layout.fillWidth: true
                                    color: "transparent"
                                }
                            }
                        } // Rectangle - toolbar

                        Rectangle
                        {
                            visible: channelList.count ? false : true
                            y: chEditToolbar.height
                            width: channelSection.width
                            height: UISettings.bigItemHeight
                            color: "transparent"
                            radius: height * 0.2
                            border.width: 1
                            border.color: UISettings.fgMedium

                            RobotoText
                            {
                                anchors.centerIn: parent
                                label: qsTr("Drop channels here")
                            }
                        }

                        ListView
                        {
                            id: channelList
                            visible: count ? true : false
                            y: chEditToolbar.height
                            width: channelSection.width
                            height: UISettings.listItemHeight * (count + 1)
                            boundsBehavior: Flickable.StopAtBounds
                            currentIndex: -1
                            interactive: false

                            property bool dragActive: false
                            property int dragInsertIndex: -1

                            model: mode ? mode.channels : null

                            header:
                                RowLayout
                                {
                                    z: 2
                                    width: channelList.width
                                    height: UISettings.listItemHeight

                                    RobotoText
                                    {
                                        Layout.fillWidth: true
                                        height: UISettings.listItemHeight
                                        label: qsTr("Channels")
                                        color: UISettings.sectionHeader
                                    }
                                    Rectangle { width: 1; height: UISettings.listItemHeight }

                                    RobotoText
                                    {
                                        width: UISettings.bigItemHeight * 2
                                        height: UISettings.listItemHeight
                                        label: qsTr("Acts on")
                                        color: UISettings.sectionHeader
                                    }
                                }

                            delegate:
                                Item
                                {
                                    width: channelList.width
                                    height: UISettings.listItemHeight

                                    MouseArea
                                    {
                                        id: mcDelegate
                                        width: channelList.width
                                        height: parent.height

                                        property bool dragActive: drag.active
                                        property QLCChannel cRef: model.cRef

                                        drag.target: mcDragItem
                                        drag.threshold: height / 2

                                        onPressed:
                                        {
                                            var posnInWindow = mcDelegate.mapToItem(channelList, mcDelegate.x, mcDelegate.y)
                                            mcDragItem.parent = channelList
                                            mcDragItem.x = posnInWindow.x - (mcDragItem.width / 4)
                                            mcDragItem.y = posnInWindow.y - (mcDragItem.height / 4)
                                            mcDragItem.z = 10

                                            if (model.isSelected)
                                                return

                                            modeChanSelector.selectItem(index, channelList.model, mouse.modifiers)

                                            if ((mouse.modifiers & Qt.ControlModifier) == 0)
                                                mcDragItem.itemsList = []

                                            mcDragItem.itemsList.push(mcDelegate)
                                        }

                                        onDragActiveChanged:
                                        {
                                            if (dragActive)
                                            {
                                                mcDragItem.itemLabel = mcEntryItem.tLabel
                                                mcDragItem.itemIcon = mcEntryItem.iSrc
                                                channelList.dragActive = true
                                            }
                                            else
                                            {
                                                mcDragItem.Drag.drop()
                                                mcDragItem.parent = channelList
                                                mcDragItem.x = 0
                                                mcDragItem.y = 0
                                                channelList.dragActive = false
                                            }
                                        }

                                        Rectangle
                                        {
                                            anchors.fill: parent
                                            radius: 3
                                            color: UISettings.highlight
                                            visible: model.isSelected
                                        }

                                        RowLayout
                                        {
                                            width: channelList.width
                                            height: UISettings.listItemHeight

                                            IconTextEntry
                                            {
                                                id: mcEntryItem
                                                Layout.fillWidth: true
                                                height: UISettings.listItemHeight
                                                tLabel: mcDelegate.cRef ? (index + 1) + ": " + mcDelegate.cRef.name : ""
                                                iSrc: mcDelegate.cRef ? mcDelegate.cRef.getIconNameFromGroup(mcDelegate.cRef.group, true) : ""
                                            }
                                            Rectangle { width: 1; height: UISettings.listItemHeight }

                                            CustomComboBox
                                            {
                                                implicitWidth: UISettings.bigItemHeight * 2
                                                height: UISettings.listItemHeight
                                                model: mode ? mode.actsOnChannels : null
                                                textRole: ""
                                                currentIndex: mode ? mode.actsOnChannel(index) : -1
                                                onCurrentIndexChanged: if (mode) mode.setActsOnChannel(index, currentIndex)
                                            }
                                        }

                                        // bottom divider line
                                        Rectangle
                                        {
                                            width: parent.width
                                            height: 1
                                            y: parent.height - 1
                                            color: UISettings.fgMedium
                                        }

                                        // top line drag highlight
                                        Rectangle
                                        {
                                            visible: channelList.dragInsertIndex == index
                                            width: parent.width
                                            height: 2
                                            z: 1
                                            color: UISettings.selection
                                        }
                                    }
                                }

                            GenericMultiDragItem
                            {
                                id: mcDragItem

                                visible: channelList.dragActive

                                Drag.active: channelList.dragActive
                                Drag.source: mcDragItem
                                Drag.keys: [ "channel" ]
                            }
                        } // ListView

                        DropArea
                        {
                            id: clDropArea
                            anchors.fill: parent

                            // accept only channels
                            keys: [ "channel" ]

                            onDropped:
                            {
                                var idx = channelList.dragInsertIndex
                                if (idx === -1)
                                    idx = 0;

                                for (var i = 0; i < drag.source.itemsList.length; i++)
                                {
                                    if (drag.source.hasOwnProperty("fromMainEditor"))
                                    {
                                        console.log("Adding channel: " + drag.source.itemsList[i].cRef.name)
                                        mode.addChannel(drag.source.itemsList[i].cRef, idx + i)
                                    }
                                    else
                                    {
                                        mode.moveChannel(drag.source.itemsList[i].cRef, idx + i)
                                    }
                                }
                                channelList.dragInsertIndex = -1
                            }
                            onPositionChanged:
                            {
                                var yInList = drag.y - chEditToolbar.height - UISettings.listItemHeight
                                var idx = channelList.indexAt(drag.x, yInList)
                                var item = channelList.itemAt(drag.x, yInList)
                                if (item === null)
                                    return
                                var itemY = item.mapToItem(channelList, 0, 0).y

                                //console.log("Item index:" + idx)
                                if (drag.y < (itemY + item.height) / 2)
                                    channelList.dragInsertIndex = idx
                                else
                                    channelList.dragInsertIndex = idx + 1
                            }
                        }
                    }
            }

            SectionBox
            {
                id: headSection
                width: parent.width
                sectionLabel: qsTr("Emitters")

                sectionContents:
                    Column
                    {
                        width: parent.width
                        spacing: 0

                        Rectangle
                        {
                            id: headEditToolbar
                            width: headSection.width
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
                                    id: delHeadButton
                                    imgSource: "qrc:/remove.svg"
                                    tooltip: qsTr("Remove the selected emitter(s)")
                                    enabled: modeHeadSelector.itemsCount

                                    onClicked:
                                    {
                                        mode.deleteHeads(modeHeadSelector.itemsList())
                                    }
                                }

                                Rectangle
                                {
                                    Layout.fillWidth: true
                                    color: "transparent"
                                }
                            }
                        } // Rectangle - toolbar

                        Repeater
                        {
                            id: headList
                            width: headSection.width
                            //height: UISettings.listItemHeight * count

                            model: mode.heads
                            delegate:
                                Rectangle
                                {
                                    width: headSection.width
                                    height: hcRepeater.count * UISettings.listItemHeight
                                    color: "transparent"
                                    border.width: 1
                                    border.color: UISettings.bgLight

                                    property var chList: model.channelList

                                    // Head block selection
                                    Rectangle
                                    {
                                        anchors.fill: parent
                                        radius: 3
                                        color: UISettings.highlight
                                        visible: model.isSelected
                                    }

                                    // Head number side block
                                    RobotoText
                                    {
                                        width: UISettings.bigItemHeight
                                        height: parent.height
                                        textHAlign: Text.AlignHCenter
                                        label: "#" + (index + 1)
                                    }

                                    MouseArea
                                    {
                                        anchors.fill: parent
                                        onClicked:
                                        {
                                            modeHeadSelector.selectItem(index, headList.model, mouse.modifiers)
                                        }
                                    }

                                    ColumnLayout
                                    {
                                        id: headChanList
                                        x: UISettings.bigItemHeight
                                        width: headSection.width - x
                                        height: hcRepeater.count * UISettings.listItemHeight
                                        spacing: 0

                                        Repeater
                                        {
                                            id: hcRepeater
                                            width: headChanList.width
                                            model: chList
                                            delegate:
                                                Rectangle
                                                {
                                                    id: cDelegate
                                                    width: headChanList.width
                                                    height: UISettings.listItemHeight
                                                    color: "transparent"

                                                    property int chIndex: modelData
                                                    property QLCChannel cRef: mode.channelFromIndex(chIndex)

                                                    IconTextEntry
                                                    {
                                                        width: headChanList.width
                                                        height: UISettings.listItemHeight
                                                        tLabel: "" + (cDelegate.chIndex + 1) + ": " + (cDelegate.cRef ? cDelegate.cRef.name : "")
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
                                        } // Repeater
                                    } // ColumnLayout
                                }
                        }
                    }
            }

            SectionBox
            {
                id: physicalSection
                width: parent.width
                sectionLabel: qsTr("Physical")

                sectionContents:
                    ColumnLayout
                    {
                        width: parent.width

                        RowLayout
                        {
                            width: parent.width
                            height: UISettings.listItemHeight

                            CustomCheckBox
                            {
                                id: globalPhyCheck
                                checked: mode.useGlobalPhysical
                                autoExclusive: true
                            }
                            RobotoText { label: qsTr("Use global settings") }

                            CustomCheckBox
                            {
                                id: overridePhyCheck
                                checked: !mode.useGlobalPhysical
                                autoExclusive: true
                            }
                            RobotoText { label: qsTr("Override global settings") }
                        }

                        PhysicalProperties
                        {
                            width: Math.min(editorRoot.width / 2, parent.width)
                            phy: globalPhyCheck.checked ?
                                     (editorView ? editorView.globalPhysical : null) :
                                     (mode ? mode.physical : null)
                            enabled: overridePhyCheck.checked
                        }
                    }
            }
        }

        ScrollBar.vertical: CustomScrollBar { id: sbar }
    } // Flickable
}
