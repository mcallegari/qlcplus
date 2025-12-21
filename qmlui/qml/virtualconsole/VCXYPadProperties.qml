/*
  Q Light Controller Plus
  VCXYPadProperties.qml

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
    id: propsRoot
    color: "transparent"
    height: xyPadPropsColumn.height

    property VCXYPad widgetRef: null

    property int gridItemsHeight: UISettings.listItemHeight

    ModelSelector
    {
        id: fxSelector
    }

    Column
    {
        id: xyPadPropsColumn
        width: parent.width
        spacing: 5

        SectionBox
        {
            sectionLabel: qsTr("Display Properties")

            sectionContents:
              GridLayout
              {
                width: parent.width
                columns: 2
                columnSpacing: 5
                rowSpacing: 4

                // row 1
                RobotoText
                {
                    height: gridItemsHeight
                    Layout.fillWidth: true
                    label: qsTr("Inverted Y-Axis")
                }

                CustomCheckBox
                {
                    implicitWidth: UISettings.iconSizeMedium
                    implicitHeight: implicitWidth
                    checked: widgetRef ? widgetRef.invertedAppearance : false
                    onClicked: if (widgetRef) widgetRef.invertedAppearance = checked
                }
              } // GridLayout
        } // SectionBox

        SectionBox
        {
            sectionLabel: qsTr("Range Display Mode")

            sectionContents:
              GridLayout
              {
                width: parent.width
                columns: 6
                columnSpacing: 5
                rowSpacing: 4

                ButtonGroup { id: rangeModeGroup }

                // row 1
                CustomCheckBox
                {
                    implicitWidth: UISettings.iconSizeMedium
                    implicitHeight: implicitWidth
                    ButtonGroup.group: rangeModeGroup
                    checked: widgetRef ? widgetRef.displayMode === VCXYPad.Degrees : true
                    onClicked: if (checked && widgetRef) widgetRef.displayMode = VCXYPad.Degrees
                }

                RobotoText
                {
                    height: gridItemsHeight
                    Layout.fillWidth: true
                    label: qsTr("Degrees")
                }

                CustomCheckBox
                {
                    implicitWidth: UISettings.iconSizeMedium
                    implicitHeight: implicitWidth
                    ButtonGroup.group: rangeModeGroup
                    checked: widgetRef ? widgetRef.displayMode === VCXYPad.Percentage : true
                    onClicked: if (checked && widgetRef) widgetRef.displayMode = VCXYPad.Percentage
                }

                RobotoText
                {
                    height: gridItemsHeight
                    Layout.fillWidth: true
                    label: qsTr("Percentage")
                }

                CustomCheckBox
                {
                    implicitWidth: UISettings.iconSizeMedium
                    implicitHeight: implicitWidth
                    ButtonGroup.group: rangeModeGroup
                    checked: widgetRef ? widgetRef.displayMode === VCXYPad.DMX : true
                    onClicked: if (checked && widgetRef) widgetRef.displayMode = VCXYPad.DMX
                }

                RobotoText
                {
                    height: gridItemsHeight
                    Layout.fillWidth: true
                    label: qsTr("DMX")
                }

              } // GridLayout
        } // SectionBox

        SectionBox
        {
            id: fixtureSection
            sectionLabel: qsTr("Fixtures")

            sectionContents:
                GridLayout
                {
                    width: parent.width
                    columns: 3
                    columnSpacing: 0
                    rowSpacing: 0

                    // row 1 - toolbar
                    Rectangle
                    {
                        Layout.fillWidth: true
                        Layout.columnSpan: 3
                        color: UISettings.bgMedium
                        height: UISettings.listItemHeight

                        IconButton
                        {
                            id: addFixture
                            anchors.top: parent.top
                            anchors.right: removeFixture.left

                            width: height
                            height: parent.height
                            faSource: FontAwesome.fa_plus
                            faColor: "limegreen"
                            checkable: true
                            tooltip: qsTr("Add a fixture/head")
                            onCheckedChanged:
                            {
                                if (checked)
                                {
                                    if (!sideLoader.visible)
                                        rightSidePanel.width += UISettings.sidePanelWidth
                                    sideLoader.visible = true
                                    sideLoader.modelProvider = widgetRef
                                    sideLoader.source = "qrc:/FixtureGroupManager.qml"
                                }
                                else
                                {
                                    rightSidePanel.width -= sideLoader.width
                                    sideLoader.source = ""
                                    sideLoader.visible = false
                                }
                            }
                        }
                        IconButton
                        {
                            id: removeFixture
                            anchors.top: parent.top
                            anchors.right: parent.right
                            width: height
                            height: parent.height
                            faSource: FontAwesome.fa_minus
                            faColor: "crimson"
                            tooltip: qsTr("Remove the selected fixture head(s)")
                            //onClicked: widgetRef.removeHeads(fxSelector.itemsList())
                        }
                    }

                    // row 2 - header
                    RobotoText
                    {
                        id: fxNameCol
                        Layout.fillWidth: true
                        height: UISettings.listItemHeight
                        label: qsTr("Fixture")

                        Rectangle
                        {
                            height: UISettings.listItemHeight
                            width: 1
                            anchors.right: parent.right
                            color: UISettings.fgMedium
                        }
                    }

                    RobotoText
                    {
                        id: xRangeCol
                        height: UISettings.listItemHeight
                        width: UISettings.bigItemHeight
                        label: qsTr("X-Axis Range")

                        Rectangle
                        {
                            height: UISettings.listItemHeight
                            width: 1
                            anchors.right: parent.right
                            color: UISettings.fgMedium
                        }
                    }

                    RobotoText
                    {
                        id: yRangeCol
                        height: UISettings.listItemHeight
                        label: qsTr("Y-Axis Range")

                        Rectangle
                        {
                            height: UISettings.listItemHeight
                            width: 1
                            anchors.right: parent.right
                            color: UISettings.fgMedium
                        }
                    }

                    // row 3
                    ListView
                    {
                        id: fixtureListView
                        Layout.fillWidth: true
                        Layout.columnSpan: 3
                        model: widgetRef ? widgetRef.fixtureList : null
                        implicitHeight: count * UISettings.listItemHeight

                        delegate:
                            Rectangle
                            {
                                width: fixtureListView.width
                                height: UISettings.listItemHeight
                                color: "transparent"

                                // Highlight rectangle
                                Rectangle
                                {
                                    anchors.fill: parent
                                    //z: 5
                                    //opacity: 0.3
                                    radius: 3
                                    color: UISettings.highlight
                                    visible: model.isSelected
                                }

                                MouseArea
                                {
                                    anchors.fill: parent
                                    onClicked:
                                    {
                                        fxSelector.selectItem(index, fixtureListView.model, mouse.modifiers)
                                    }
                                }

                                Row
                                {
                                    //width: fixtureListView.width
                                    height: UISettings.listItemHeight

                                    /* Head name */
                                    RobotoText
                                    {
                                        width: fxNameCol.width
                                        height: UISettings.listItemHeight
                                        label: model.name

                                        Rectangle
                                        {
                                            height: UISettings.listItemHeight
                                            width: 1
                                            anchors.right: parent.right
                                            color: UISettings.fgMedium
                                        }
                                    }

                                    /* X-Axis Range */
                                    RobotoText
                                    {
                                        width: xRangeCol.width
                                        height: UISettings.listItemHeight
                                        label: model.xRange

                                        Rectangle
                                        {
                                            height: UISettings.listItemHeight
                                            width: 1
                                            anchors.right: parent.right
                                            color: UISettings.fgMedium
                                        }
                                    }

                                    /* Y-Axis Range */
                                    RobotoText
                                    {
                                        width: yRangeCol.width
                                        height: UISettings.listItemHeight
                                        label: model.yRange

                                        Rectangle
                                        {
                                            height: UISettings.listItemHeight
                                            width: 1
                                            anchors.right: parent.right
                                            color: UISettings.fgMedium
                                        }
                                    }

                                } // Row
                            } // Rectangle
                    } // ListView

                    Rectangle
                    {
                        id: newFixtureBox
                        Layout.fillWidth: true
                        Layout.columnSpan: 5
                        height: UISettings.bigItemHeight * 0.6
                        color: "transparent"
                        radius: 10
                        visible: addFixture.checked

                        RobotoText
                        {
                            id: ntText
                            visible: false
                            anchors.centerIn: parent
                            label: qsTr("Add a new fixture")
                        }

                        DropArea
                        {
                            id: newFixtureDrop
                            anchors.fill: parent

                            keys: [ "fixture" ]

                            states: [
                                State
                                {
                                    when: newFixtureDrop.containsDrag
                                    PropertyChanges
                                    {
                                        target: newFixtureBox
                                        color: "#3F00FF00"
                                    }
                                    PropertyChanges
                                    {
                                        target: ntText
                                        visible: true
                                    }
                                }
                            ]

                            onDropped:
                            {
                                console.log("Item(s) dropped here. x: " + drag.x + " y: " + drag.y)

                                for (var i = 0; i < drag.source.itemsList.length; i++)
                                {
                                    console.log("Item #" + i + " type: " + drag.source.itemsList[i].itemType)
                                    switch(drag.source.itemsList[i].itemType)
                                    {
                                        case App.UniverseDragItem:
                                        case App.FixtureGroupDragItem:
                                            efxEditor.addGroup(drag.source.itemsList[i].cRef)
                                        break;
                                        case App.FixtureDragItem:
                                            efxEditor.addFixture(drag.source.itemsList[i].cRef)
                                        break;
                                        case App.HeadDragItem:
                                            efxEditor.addHead(drag.source.itemsList[i].fixtureID, drag.source.itemsList[i].headIndex)
                                        break;
                                    }
                                }
                            }
                        }
                    }
                } // GridLayout
        } // SectionBox
    } // Column
}
