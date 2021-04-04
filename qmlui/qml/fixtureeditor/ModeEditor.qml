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

    property EditorRef fixtureEditor: null
    property ModeEdit mode: null

    function setItemName(name)
    {
        mode = fixtureEditor.requestModeEditor(name)
        nameEdit.selectAndFocus()
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
                height: UISettings.listItemHeight

                RobotoText { label: qsTr("Name") }
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
                    Column
                    {
                        width: channelSection.width
                        //height: channelList.height

                        ListView
                        {
                            id: channelList
                            width: channelSection.width
                            height: UISettings.listItemHeight * count
                            boundsBehavior: Flickable.StopAtBounds
                            currentIndex: -1

                            property bool dragActive: false

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
                                        id: delegateRoot
                                        width: channelList.width
                                        height: parent.height

                                        property bool dragActive: drag.active

                                        //drag.target: cDragItem
                                        //drag.threshold: height / 2

                                        onDragActiveChanged: channelList.dragActive = dragActive
                                        onPressed: channelList.currentIndex = index

                                        Rectangle
                                        {
                                            anchors.fill: parent
                                            radius: 3
                                            color: UISettings.highlight
                                            visible: channelList.currentIndex === index
                                        }

                                        RowLayout
                                        {
                                            width: channelList.width
                                            height: UISettings.listItemHeight

                                            IconTextEntry
                                            {
                                                Layout.fillWidth: true
                                                height: UISettings.listItemHeight
                                                tLabel: (index + 1) + ": " + modelData.mLabel
                                                iSrc: modelData.mIcon
                                            }
                                            Rectangle { width: 1; height: UISettings.listItemHeight }
                                            CustomComboBox
                                            {
                                                implicitWidth: UISettings.bigItemHeight * 2
                                                height: UISettings.listItemHeight
                                            }
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
                    }
            }

            SectionBox
            {
                id: headSection
                width: parent.width
                sectionLabel: qsTr("Heads")

                sectionContents:
                    GridLayout
                    {

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
                                     (fixtureEditor ? fixtureEditor.globalPhysical : null) :
                                     (mode ? mode.physical : null)
                            enabled: overridePhyCheck.checked
                        }
                    }
            }
        }

        ScrollBar.vertical: CustomScrollBar { id: sbar }
    } // Flickable
}
