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
    property color selectedColor

    signal colorChanged(real r, real g, real b, int w, int a, int uv)

    //onSelectedColorChanged: colorChanged(selectedColor.r, selectedColor.g, selectedColor.b, whiteValue, amberValue, uvValue)

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
                Layout.fillWidth: true
                model: fixtureManager.colorFiltersList
                currentIndex: fixtureManager.colorFilterIndex
                onCurrentIndexChanged: fixtureManager.colorFilterIndex = currentIndex
            }

            IconButton
            {
                height: UISettings.listItemHeight
                width: height
                imgSource: "qrc:/add.svg"
                tooltip: qsTr("Add new color filters")
            }

            IconButton
            {
                height: UISettings.listItemHeight
                width: height
                imgSource: "qrc:/remove.svg"
                tooltip: qsTr("Delete the current color filters")
                enabled: cfRef && cfRef.isUser ? true : false
            }

            IconButton
            {
                height: UISettings.listItemHeight
                width: height
                imgSource: "qrc:/rename.svg"
                tooltip: qsTr("Rename the current color filters")
                enabled: cfRef && cfRef.isUser ? true : false
            }

            IconButton
            {
                height: UISettings.listItemHeight
                width: height
                imgSource: "qrc:/filesave.svg"
                tooltip: qsTr("Save the current color filters")
                enabled: cfRef && cfRef.isUser ? true : false
            }
        }

        ListView
        {
            id: filtersList
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

                    Rectangle
                    {
                        y: 1
                        width: UISettings.bigItemHeight
                        height: parent.height - 2
                        color: modelData.rgb
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
                            selectedColor = modelData.rgb
                            rootBox.colorChanged(selectedColor.r, selectedColor.g, selectedColor.b, 0, 0, 0)
                            filtersList.currentIndex = index
                        }
                    }
                }

            CustomScrollBar { flickable: filtersList }
        }

        Grid
        {
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
                    selectedColor = Qt.rgba(rSpin.value / 256, gSpin.value / 256, bSpin.value / 256, 1.0)
                    htmlText.inputText = Helpers.getHTMLColor(rSpin.value, gSpin.value, bSpin.value)
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
                    selectedColor = Qt.rgba(rSpin.value / 256, gSpin.value / 256, bSpin.value / 256, 1.0)
                    htmlText.inputText = Helpers.getHTMLColor(rSpin.value, gSpin.value, bSpin.value)
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
                    selectedColor = Qt.rgba(rSpin.value / 256, gSpin.value / 256, bSpin.value / 256, 1.0)
                    htmlText.inputText = Helpers.getHTMLColor(rSpin.value, gSpin.value, bSpin.value)
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
                }
            }

            // row 4
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
                label: qsTr("CMY")
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

    }
}
