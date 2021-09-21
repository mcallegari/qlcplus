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
    color: UISettings.bgMedium
    border.color: "#222"
    border.width: 2

    property ColorFilters cfRef: fixtureManager.selectedFilters
    property int colorsMask: 0
    property bool isUpdating: false
    property color currentRGB
    property color currentWAUV

    property color filterRGB
    property color filterWAUV

    property int currentFilterIndex: -1

    signal colorChanged(real r, real g, real b, real w, real a, real uv)

    onFilterRGBChanged:
    {
        rSpin.value = filterRGB.r * 255
        gSpin.value = filterRGB.g * 255
        bSpin.value = filterRGB.b * 255
        htmlText.text = Helpers.getHTMLColor(filterRGB.r * 255, filterRGB.g * 255, filterRGB.b * 255)
        emitCurrentColor()
    }
    onFilterWAUVChanged:
    {
        wSpin.value = filterWAUV.r * 255
        aSpin.value = filterWAUV.g * 255
        uvSpin.value = filterWAUV.b * 255
        emitCurrentColor()
    }

    function emitCurrentColor()
    {
        if (isUpdating)
            return
        colorChanged(filterRGB.r, filterRGB.g, filterRGB.b, filterWAUV.r, filterWAUV.g, filterWAUV.b)
    }

    function updateFilter()
    {
        if (!cfRef || !cfRef.isUser)
            return

        var idx = rootBox.currentFilterIndex
        cfRef.changeFilterAt(idx, filterRGB.r * 255, filterRGB.g * 255, filterRGB.b * 255,
                                  filterWAUV.r * 255, filterWAUV.g * 255, filterWAUV.b * 255)
        rootBox.currentFilterIndex = idx
    }

    function setColors(rgb, wauv, index)
    {
        rootBox.currentFilterIndex = index
        isUpdating = true
        filterRGB = rgb
        filterWAUV = wauv
        isUpdating = false
        emitCurrentColor()
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
                textRole: ""
                model: fixtureManager.colorFiltersFileList
                currentIndex: fixtureManager.colorFilterFileIndex
                onCurrentIndexChanged:
                {
                    fixtureManager.colorFilterFileIndex = currentIndex
                    rootBox.currentFilterIndex = 0
                }
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
            currentIndex: rootBox.currentFilterIndex

            delegate:
                Rectangle
                {
                    height: UISettings.listItemHeight
                    width: filtersList.width
                    color: index === rootBox.currentFilterIndex ? UISettings.highlight : "transparent"

                    property color rgbCol: modelData.rgb
                    property color wauvCol: modelData.wauv

                    Rectangle
                    {
                        id: colorDisp
                        y: 1
                        width: UISettings.bigItemHeight * 0.75
                        height: parent.height - 2
                        color: rgbCol

                        Rectangle
                        {
                            anchors.fill: parent
                            color: "white"
                            opacity: wauvCol.r
                        }
                        Rectangle
                        {
                            anchors.fill: parent
                            color: "#FF7E00"
                            opacity: wauvCol.g
                        }
                        Rectangle
                        {
                            anchors.fill: parent
                            color: "#9400D3"
                            opacity: wauvCol.b
                        }
                    }
                    RobotoText
                    {
                        x: UISettings.bigItemHeight * 0.75 + 3
                        height: parent.height
                        label: modelData.name
                    }
                    MouseArea
                    {
                        anchors.fill: parent
                        onClicked: rootBox.setColors(rgbCol, wauvCol, index)
                    }
                }

            ScrollBar.vertical: CustomScrollBar { }
        } // ListView

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
                width: UISettings.bigItemHeight * 0.75
                from: 0
                to: 255
                onValueChanged:
                {
                    filterRGB = Qt.rgba(value / 255, filterRGB.g, filterRGB.b, 1.0)
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
                width: UISettings.bigItemHeight * 0.75
                from: 0
                to: 255
                onValueChanged:
                {
                    filterWAUV = Qt.rgba(value / 255, filterWAUV.g, filterWAUV.b, 1.0)
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
                width: UISettings.bigItemHeight * 0.75
                from: 0
                to: 255
                onValueChanged:
                {
                    filterRGB = Qt.rgba(filterRGB.r, value / 255, filterRGB.b, 1.0)
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
                width: UISettings.bigItemHeight * 0.75
                from: 0
                to: 255
                onValueChanged:
                {
                    filterWAUV = Qt.rgba(filterWAUV.r, value / 255, filterWAUV.b, 1.0)
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
                width: UISettings.bigItemHeight * 0.75
                from: 0
                to: 255
                onValueChanged:
                {
                    filterRGB = Qt.rgba(filterRGB.r, filterRGB.g, value / 255, 1.0)
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
                width: UISettings.bigItemHeight * 0.75
                from: 0
                to: 255
                onValueChanged:
                {
                    filterWAUV = Qt.rgba(filterWAUV.r, filterWAUV.g, value / 255, 1.0)
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
                width: UISettings.bigItemHeight * 0.75
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
                iconHeight: UISettings.listItemHeight
                entryText: qsTr("Add a new color filters file")
                onClicked:
                {
                    fixtureManager.addColorFiltersFile()
                    rootBox.currentFilterIndex = -1
                    actionsMenu.close()
                }
            }

            ContextMenuEntry
            {
                height: UISettings.listItemHeight
                imgSource: "qrc:/rename.svg"
                iconHeight: UISettings.listItemHeight
                entryText: qsTr("Rename the current color filters file")
                enabled: cfRef && cfRef.isUser ? true : false
            }

            ContextMenuEntry
            {
                height: UISettings.listItemHeight
                imgSource: "qrc:/filesave.svg"
                iconHeight: UISettings.listItemHeight
                entryText: qsTr("Save the current color filters file")
                enabled: cfRef && cfRef.isUser ? true : false
                onClicked:
                {
                    if (!cfRef || !cfRef.isUser)
                        return
                    cfRef.save()
                    actionsMenu.close()
                }
            }

            ContextMenuEntry
            {
                height: UISettings.listItemHeight
                imgSource: "qrc:/add.svg"
                iconHeight: UISettings.listItemHeight
                entryText: qsTr("Add a new filter")
                enabled: cfRef && cfRef.isUser ? true : false
                onClicked:
                {
                    if (!cfRef || !cfRef.isUser)
                        return

                    cfRef.addFilter("New filter " + filtersList.count,
                                    rSpin.value, gSpin.value, bSpin.value,
                                    wSpin.value, aSpin.value, uvSpin.value)
                    rootBox.currentFilterIndex = filtersList.count - 1
                    actionsMenu.close()
                }
            }

            ContextMenuEntry
            {
                height: UISettings.listItemHeight
                imgSource: "qrc:/remove.svg"
                iconHeight: UISettings.listItemHeight
                entryText: qsTr("Delete the selected filter")
                enabled: cfRef && cfRef.isUser ? true : false
                onClicked:
                {
                    if (!cfRef || !cfRef.isUser)
                        return
                    cfRef.removeFilterAt(rootBox.currentFilterIndex)
                    rootBox.currentFilterIndex = -1
                    actionsMenu.close()
                }
            }

            ContextMenuEntry
            {
                height: UISettings.listItemHeight
                imgSource: "qrc:/edit-paste.svg"
                iconHeight: UISettings.listItemHeight
                entryText: qsTr("Paste the latest picked color as new filter")
                enabled: cfRef && cfRef.isUser ? true : false
                onClicked:
                {
                    if (!cfRef || !cfRef.isUser)
                        return

                    cfRef.addFilter("New filter " + (rootBox.currentFilterIndex + 1),
                                    currentRGB.r * 255, currentRGB.g * 255, currentRGB.b * 255,
                                    currentWAUV.r * 255, currentWAUV.g * 255, currentWAUV.b * 255)
                    rootBox.currentFilterIndex = filtersList.count
                    actionsMenu.close()
                }
            }
        }
    }
}
