/*
  Q Light Controller Plus
  ColorToolFilters.qml

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
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.1

import org.qlcplus.classes 1.0
import "GenericHelpers.js" as Helpers
import "."

Rectangle
{
    id: rootBox
    width: 330
    height: 370
    color: "#444"
    border.color: "#222"
    border.width: 2

    property ColorFilters cfRef: fixtureManager.selectedFilters
    property int colorsMask: 0
    property bool isUpdating: false

    signal colorChanged(real r, real g, real b, int w, int a, int uv)

    function updateFilter()
    {
        if (!cfRef)
            return

        cfRef.changeFilterAt(filtersList.currentIndex,
                             rSpin.value, gSpin.value, bSpin.value,
                             wSpin.value, aSpin.value, uvSpin.value)
    }

    ColumnLayout
    {
        anchors.fill: parent

        spacing: 3

        RowLayout
        {
            width: parent.width
            z: 2
            spacing: 3

            CustomComboBox
            {
                z: 2
                Layout.fillWidth: true
                model: fixtureManager.colorFiltersList
                currentIndex: fixtureManager.colorFilterIndex
                onCurrentIndexChanged: fixtureManager.colorFilterIndex = currentIndex
            }

            IconButton
            {
                id: actionsButton
                height: UISettings.listItemHeight
                width: height
                faSource: FontAwesome.fa_bars
                faColor: "white"
                tooltip: qsTr("Open filters menu")
                onClicked: actionsMenu.open()
            }
        }

        ListView
        {
            id: filtersList
            z: 1
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            boundsBehavior: Flickable.StopAtBounds

            model: cfRef ? cfRef.filtersList : null

            highlightFollowsCurrentItem: false
            highlight:
                Rectangle
                {
                    y: filtersList.currentItem.y
                    height: UISettings.listItemHeight
                    width: filtersList.width
                    color: UISettings.highlight
                }

            delegate:
                Rectangle
                {
                    height: UISettings.listItemHeight
                    width: filtersList.width
                    color: "transparent"

                    property color rgbCol: modelData.rgb
                    property color wauvCol: modelData.wauv

                    Rectangle
                    {
                        id: colorDisp
                        y: 1
                        width: UISettings.bigItemHeight
                        height: parent.height - 2
                        color: rgbCol
                    }
                    RobotoText
                    {
                        x: UISettings.bigItemHeight + 3
                        height: parent.height
                        label: modelData.name
                    }
                    MouseArea
                    {
                        anchors.fill: parent
                        onClicked:
                        {
                            rootBox.colorChanged(colorDisp.color.r, colorDisp.color.g, colorDisp.color.b, 0, 0, 0)
                            isUpdating = true
                            rSpin.value = rgbCol.r * 255
                            gSpin.value = rgbCol.g * 255
                            bSpin.value = rgbCol.b * 255
                            wSpin.value = wauvCol.r * 255
                            aSpin.value = wauvCol.g * 255
                            uvSpin.value = wauvCol.b * 255
                            filtersList.currentIndex = index
                            isUpdating = false
                        }
                    }
                }

            CustomScrollBar { flickable: filtersList }
        }

        Grid
        {
            z: 1
            width: parent.width

            columns: 4
            columnSpacing: 5

            // row 1
            RobotoText
            {
                height: UISettings.listItemHeight
                label: cmyCheck.checked ? qsTr("Cyan") : qsTr("Red")
            }

            CustomSpinBox
            {
                id: rSpin
                //Layout.fillWidth: true
                height: UISettings.listItemHeight
                from: 0
                to: 255
                onValueChanged:
                {
                    if (isUpdating)
                        return
                    htmlText.inputText = Helpers.getHTMLColor(rSpin.value, gSpin.value, bSpin.value)
                    rootBox.updateFilter()
                }
            }

            RobotoText
            {
                height: UISettings.listItemHeight
                label: qsTr("White")
            }

            CustomSpinBox
            {
                id: wSpin
                //Layout.fillWidth: true
                height: UISettings.listItemHeight
                from: 0
                to: 255
                onValueChanged:
                {
                    if (isUpdating)
                        return
                    rootBox.updateFilter()
                }
            }

            // row 2
            RobotoText
            {
                height: UISettings.listItemHeight
                label: cmyCheck.checked ? qsTr("Magenta") : qsTr("Green")
            }

            CustomSpinBox
            {
                id: gSpin
                //Layout.fillWidth: true
                height: UISettings.listItemHeight
                from: 0
                to: 255
                onValueChanged:
                {
                    if (isUpdating)
                        return
                    htmlText.inputText = Helpers.getHTMLColor(rSpin.value, gSpin.value, bSpin.value)
                    rootBox.updateFilter()
                }
            }

            RobotoText
            {
                height: UISettings.listItemHeight
                label: qsTr("Amber")
            }

            CustomSpinBox
            {
                id: aSpin
                //Layout.fillWidth: true
                height: UISettings.listItemHeight
                from: 0
                to: 255
                onValueChanged:
                {
                    if (isUpdating)
                        return
                    rootBox.updateFilter()
                }
            }

            // row 3
            RobotoText
            {
                height: UISettings.listItemHeight
                label: cmyCheck.checked ? qsTr("Yellow") : qsTr("Blue")
            }

            CustomSpinBox
            {
                id: bSpin
                //Layout.fillWidth: true
                height: UISettings.listItemHeight
                from: 0
                to: 255
                onValueChanged:
                {
                    if (isUpdating)
                        return
                    htmlText.inputText = Helpers.getHTMLColor(rSpin.value, gSpin.value, bSpin.value)
                    rootBox.updateFilter()
                }
            }

            RobotoText
            {
                height: UISettings.listItemHeight
                label: qsTr("UV")
            }

            CustomSpinBox
            {
                id: uvSpin
                //Layout.fillWidth: true
                height: UISettings.listItemHeight
                from: 0
                to: 255
                onValueChanged:
                {
                    if (isUpdating)
                        return
                    rootBox.updateFilter()
                }
            }

            // row 4
            RobotoText
            {
                height: UISettings.listItemHeight
                label: qsTr("CMY")
            }

            CustomCheckBox
            {
                id: cmyCheck
                implicitHeight: UISettings.listItemHeight
                implicitWidth: implicitHeight
                autoExclusive: false
            }

            RobotoText
            {
                height: UISettings.listItemHeight
                label: "HTML"
            }

            CustomTextEdit
            {
                id: htmlText
                width: UISettings.bigItemHeight * 0.7
                height: UISettings.listItemHeight
            }
        } // GridLayout

    } // ColumnLayout

    Popup
    {
        id: actionsMenu
        x: actionsButton.x +  actionsButton.width
        padding: 0

        background:
            Rectangle
            {
                color: UISettings.bgStrong
                border.color: UISettings.bgStronger
            }


        Column
        {
            ContextMenuEntry
            {
                height: UISettings.listItemHeight
                imgSource: "qrc:/filenew.svg"
                imgSize: UISettings.listItemHeight
                entryText: qsTr("Add a new color filters file")
                onClicked: fixtureManager.createColorFilters()
            }

            ContextMenuEntry
            {
                height: UISettings.listItemHeight
                imgSource: "qrc:/rename.svg"
                imgSize: UISettings.listItemHeight
                entryText: qsTr("Rename the current color filters")
                enabled: cfRef && cfRef.isUser ? true : false
            }

            ContextMenuEntry
            {
                height: UISettings.listItemHeight
                imgSource: "qrc:/filesave.svg"
                imgSize: UISettings.listItemHeight
                entryText: qsTr("Save the current color filters")
                enabled: cfRef && cfRef.isUser ? true : false
                onClicked: if (cfRef) cfRef.save()
            }

            ContextMenuEntry
            {
                height: UISettings.listItemHeight
                imgSource: "qrc:/add.svg"
                imgSize: UISettings.listItemHeight
                entryText: qsTr("Add a new filter")
                enabled: cfRef && cfRef.isUser ? true : false
                onClicked:
                {
                    if (!cfRef)
                        return

                    cfRef.addFilter("New filter " + (filtersList.currentIndex + 1),
                                    rSpin.value, gSpin.value, bSpin.value,
                                    wSpin.value, aSpin.value, uvSpin.value)
                }
            }

            ContextMenuEntry
            {
                height: UISettings.listItemHeight
                imgSource: "qrc:/remove.svg"
                imgSize: UISettings.listItemHeight
                entryText: qsTr("Delete the selected filter")
                enabled: cfRef && cfRef.isUser ? true : false
                onClicked: if (cfRef) cfRef.removeFilterAt(filtersList.currentIndex)
            }
        }
    }
}
