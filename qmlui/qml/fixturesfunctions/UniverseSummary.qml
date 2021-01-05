/*
  Q Light Controller Plus
  UniverseSummary.qml

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
import QtQuick.Layouts 1.0

import org.qlcplus.classes 1.0
import "."

Rectangle
{
    id: uniSummaryView
    anchors.fill: parent
    color: UISettings.fgMain

    function hasSettings() { return false; }

    Rectangle
    {
        id: selBar
        z: 1
        width: parent.width
        height: UISettings.iconSizeDefault
        color: UISettings.bgStrong

        RowLayout
        {
            width: parent.width
            spacing: 5

            CustomCheckBox
            {
                id: manufCheck
                autoExclusive: false
            }
            RobotoText { label: qsTr("Manufacturer") }

            CustomCheckBox
            {
                id: modelCheck
                autoExclusive: false
            }
            RobotoText { label: qsTr("Model") }

            CustomCheckBox
            {
                id: weightCheck
                autoExclusive: false
            }
            RobotoText { label: qsTr("Weight") }

            CustomCheckBox
            {
                id: powerCheck
                autoExclusive: false
            }
            RobotoText { label: qsTr("Consumption") }

            CustomCheckBox
            {
                id: dipCheck
                autoExclusive: false
            }
            RobotoText { label: qsTr("DIP switch") }

            // filler
            Rectangle { color: "transparent"; Layout.fillWidth: true; height: UISettings.iconSizeDefault }

            IconButton
            {
                width: UISettings.bigItemHeight
                imgSource: "qrc:/printer.svg"
                tooltip: qsTr("Print the universe summary")
                onClicked: qlcplus.printItem(flickView.contentItem)
            }
        }
    }

    Flickable
    {
        id: flickView
        y: selBar.height
        width: gridBox.width
        height: parent.height - y
        contentHeight: gridBox.height + summaryGrid.height
        boundsBehavior: Flickable.StopAtBounds

        property color textColor: "black"
        property int totalChannels: 0
        property real totalWeight: 0.0
        property int totalPower: 0

        GridLayout
        {
            id: gridBox
            columns: 5 + (manufCheck.checked ? 1 : 0) + (modelCheck.checked ? 1 : 0) +
                         (weightCheck.checked ? 1 : 0) + (powerCheck.checked ? 1 : 0) + (dipCheck.checked ? 1 : 0)
            columnSpacing: 5
            rowSpacing: 0


            RobotoText { label: "ID"; labelColor: flickView.textColor }
            Rectangle  { width: UISettings.iconSizeDefault }
            RobotoText { label: qsTr("Name"); labelColor: flickView.textColor }
            RobotoText { visible: manufCheck.checked; label: qsTr("Manufacturer"); labelColor: flickView.textColor }
            RobotoText { visible: modelCheck.checked; label: qsTr("Model"); labelColor: flickView.textColor }
            RobotoText { label: qsTr("Address"); labelColor: flickView.textColor }
            RobotoText { label: qsTr("Channels"); labelColor: flickView.textColor }
            RobotoText { visible: weightCheck.checked; label: qsTr("Weight"); labelColor: flickView.textColor }
            RobotoText { visible: powerCheck.checked; label: qsTr("Consumption"); labelColor: flickView.textColor }
            RobotoText { visible: dipCheck.checked; label: qsTr("DIP switch"); labelColor: flickView.textColor }

            Rectangle { Layout.columnSpan: gridBox.columns; Layout.fillWidth: true; height: 1; color: "black" }

            Repeater
            {
                model: fixtureManager.universeInfo(fixtureManager.itemID)

                delegate:
                    Item
                    {
                        property Fixture cRef: modelData.classRef

                        Rectangle
                        {
                            parent: gridBox
                            Layout.columnSpan: gridBox.columns
                            Layout.fillWidth: true
                            height: 1
                            color: "black"
                        }

                        // DIP switch
                        DMXAddressWidget
                        {
                            parent: gridBox
                            visible: dipCheck.checked
                            height: UISettings.iconSizeDefault * 1.5
                            width: UISettings.bigItemHeight * 2
                            color: UISettings.bgLighter
                            Layout.fillWidth: true
                            currentValue: cRef ? cRef.address + 1 : 1
                        }

                        // power comsumption
                        RobotoText
                        {
                            parent: gridBox
                            visible: powerCheck.checked
                            label: modelData.power + "W"
                            labelColor: flickView.textColor
                            rightMargin: 5
                            Layout.fillWidth: true

                            Rectangle { anchors.right: parent.right; height: parent.height; width: 1; color: "black" }
                            Component.onCompleted: flickView.totalPower += modelData.power
                        }

                        // weight
                        RobotoText
                        {
                            parent: gridBox
                            visible: weightCheck.checked
                            label: modelData.weight + "Kg"
                            labelColor: flickView.textColor
                            rightMargin: 5
                            Layout.fillWidth: true

                            Rectangle { anchors.right: parent.right; height: parent.height; width: 1; color: "black" }
                            Component.onCompleted: flickView.totalWeight += modelData.weight
                        }

                        // channels
                        RobotoText
                        {
                            parent: gridBox
                            label: cRef ? cRef.channels : 0
                            labelColor: flickView.textColor
                            rightMargin: 5
                            Layout.fillWidth: true

                            Rectangle { anchors.right: parent.right; height: parent.height; width: 1; color: "black" }

                            Component.onCompleted: flickView.totalChannels += cRef.channels
                        }

                        // address range
                        RobotoText
                        {
                            parent: gridBox
                            label: cRef ? "" + (cRef.address + 1) + "-" + (cRef.address + cRef.channels + 1) : ""
                            labelColor: flickView.textColor
                            rightMargin: 5
                            Layout.fillWidth: true

                            Rectangle { anchors.right: parent.right; height: parent.height; width: 1; color: "black" }
                        }

                        // model
                        RobotoText
                        {
                            parent: gridBox
                            visible: modelCheck.checked
                            label: modelData.fmodel
                            labelColor: flickView.textColor
                            rightMargin: 5
                            Layout.fillWidth: true

                            Rectangle { anchors.right: parent.right; height: parent.height; width: 1; color: "black" }
                        }

                        // manufacturer
                        RobotoText
                        {
                            parent: gridBox
                            visible: manufCheck.checked
                            label: modelData.manuf
                            labelColor: flickView.textColor
                            rightMargin: 5
                            Layout.fillWidth: true

                            Rectangle { anchors.right: parent.right; height: parent.height; width: 1; color: "black" }
                        }

                        // fixture name
                        RobotoText
                        {
                            parent: gridBox
                            label: cRef ? cRef.name : ""
                            labelColor: flickView.textColor
                            rightMargin: 5
                            Layout.fillWidth: true

                            Rectangle { anchors.right: parent.right; height: parent.height; width: 1; color: "black" }
                        }

                        // icon
                        Image
                        {
                            parent: gridBox
                            source: cRef ? cRef.iconResource(true) : ""
                            width: UISettings.iconSizeDefault
                            height: width
                            sourceSize: Qt.size(width, height)
                        }

                        // ID
                        RobotoText
                        {
                            parent: gridBox
                            label: cRef ? cRef.id : ""
                            labelColor: flickView.textColor
                            Layout.fillWidth: true

                            Rectangle { anchors.right: parent.right; height: parent.height; width: 1; color: "black" }
                        }
                    } // delegate
            } // Repeater
        } // GridLayout

        Grid
        {
            id: summaryGrid
            anchors.top: gridBox.bottom
            columns: 2
            columnSpacing: 5
            rowSpacing: 0

            RobotoText
            {
                height: UISettings.listItemHeight
                label: qsTr("Summary")
                labelColor: flickView.textColor
                fontBold: true
            }
            Rectangle { width: UISettings.listItemHeight; height: width; color: "transparent" }

            RobotoText
            {
                height: UISettings.listItemHeight
                label: qsTr("DMX channels used:")
                labelColor: flickView.textColor
                fontBold: true
            }
            RobotoText
            {
                height: UISettings.listItemHeight
                label: " " + flickView.totalChannels
                labelColor: flickView.textColor
            }

            RobotoText
            {
                height: UISettings.listItemHeight
                label: qsTr("Total weight:")
                labelColor: flickView.textColor
                fontBold: true
            }
            RobotoText
            {
                height: UISettings.listItemHeight
                label: " " + flickView.totalWeight + "Kg"
                labelColor: flickView.textColor
            }

            RobotoText
            {
                height: UISettings.listItemHeight
                label: qsTr("Estimated power consumption:")
                labelColor: flickView.textColor
                fontBold: true
            }
            RobotoText
            {
                height: UISettings.listItemHeight
                label: " " + flickView.totalPower + "W";
                labelColor: flickView.textColor
            }
        }

    } // Flickable

}
