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

import QtQuick
import QtQuick.Layouts

import org.qlcplus.classes 1.0
import "."

Rectangle
{
    id: uniSummaryView
    anchors.fill: parent
    color: UISettings.fgMain

    function hasSettings() { return false; }

    // the single definition of the table columns: headers and data cells are
    // both built from this, so they can never drift apart.
    // 'role' tells the delegate what to display, 'text' is the header label
    property var columns:
    [
        { role: "id", text: "ID", width: UISettings.bigItemHeight * 0.5, visible: true },
        { role: "icon", text: "", width: UISettings.iconSizeDefault, visible: true },
        { role: "name", text: qsTr("Name"), width: UISettings.bigItemHeight * 2, visible: true },
        { role: "manuf", text: qsTr("Manufacturer"), width: UISettings.bigItemHeight * 1.5, visible: manufCheck.checked },
        { role: "fmodel", text: qsTr("Model"), width: UISettings.bigItemHeight * 1.5, visible: modelCheck.checked },
        { role: "address", text: qsTr("Address"), width: UISettings.bigItemHeight, visible: true },
        { role: "channels", text: qsTr("Channels"), width: UISettings.bigItemHeight * 0.7, visible: true },
        { role: "weight", text: qsTr("Weight"), width: UISettings.bigItemHeight * 0.7, visible: weightCheck.checked },
        { role: "power", text: qsTr("Consumption"), width: UISettings.bigItemHeight * 0.7, visible: powerCheck.checked },
        { role: "dip", text: qsTr("DIP switch"), width: dipWidth + 10, visible: dipCheck.checked }
    ]

    // DMXAddressWidget derives the size of a single switch from its width
    // (10 switches plus a fixed 10px gap each), while the bit box is half of
    // its height and the two numbering labels take 20% each. Sizing both from
    // one figure keeps them in proportion: too wide and the switch squares
    // overflow their box, too short and the numbers become unreadable.
    property real dipSwitchSize: UISettings.iconSizeDefault * 0.4
    property real dipWidth: (dipSwitchSize + 10) * 10
    property real dipHeight: dipSwitchSize * 3.5

    // a DIP switch needs a bit more vertical room than a line of text
    property real rowHeight: dipCheck.checked ? dipHeight + 6
                                              : UISettings.iconSizeDefault

    property real tableWidth:
    {
        let total = 0
        for (let i = 0; i < columns.length; i++)
            if (columns[i].visible)
                total += columns[i].width
        return total
    }

    // index of the rightmost visible column, which doesn't get a separator
    property int lastVisibleColumn:
    {
        let last = 0
        for (let i = 0; i < columns.length; i++)
            if (columns[i].visible)
                last = i
        return last
    }

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
            anchors.verticalCenter: parent.verticalCenter
            spacing: 5

            CustomCheckBox
            {
                id: manufCheck
                implicitWidth: UISettings.iconSizeMedium
                implicitHeight: implicitWidth
                autoExclusive: false
            }
            RobotoText { label: qsTr("Manufacturer") }

            CustomCheckBox
            {
                id: modelCheck
                implicitWidth: UISettings.iconSizeMedium
                implicitHeight: implicitWidth
                autoExclusive: false
            }
            RobotoText { label: qsTr("Model") }

            CustomCheckBox
            {
                id: weightCheck
                implicitWidth: UISettings.iconSizeMedium
                implicitHeight: implicitWidth
                autoExclusive: false
            }
            RobotoText { label: qsTr("Weight") }

            CustomCheckBox
            {
                id: powerCheck
                implicitWidth: UISettings.iconSizeMedium
                implicitHeight: implicitWidth
                autoExclusive: false
            }
            RobotoText { label: qsTr("Consumption") }

            CustomCheckBox
            {
                id: dipCheck
                implicitWidth: UISettings.iconSizeMedium
                implicitHeight: implicitWidth
                autoExclusive: false
            }
            RobotoText { label: qsTr("DIP switch") }

            // filler
            Rectangle { color: "transparent"; Layout.fillWidth: true; height: UISettings.iconSizeMedium }

            IconButton
            {
                width: UISettings.bigItemHeight
                height: UISettings.iconSizeDefault * 0.9
                faSource: FontAwesome.fa_print
                faColor: UISettings.fgMain
                tooltip: qsTr("Print the universe summary")
                onClicked: qlcplus.printItem(flickView.contentItem)
            }
        }
    }

    Flickable
    {
        id: flickView
        x: 5
        y: selBar.height
        width: Math.min(uniSummaryView.width - x, tableContent.width)
        height: parent.height - y
        contentWidth: tableContent.width
        contentHeight: tableContent.height
        boundsBehavior: Flickable.StopAtBounds

        property color textColor: "black"
        property var summaryModel: fixtureManager.universeInfo(fixtureManager.itemID)

        // totals are computed from the model rather than accumulated by the
        // delegates, which would count twice every time a column is toggled
        property int totalChannels:
        {
            let total = 0
            for (let i = 0; i < summaryModel.length; i++)
                if (summaryModel[i].classRef)
                    total += summaryModel[i].classRef.channels
            return total
        }
        property real totalWeight:
        {
            let total = 0.0
            for (let i = 0; i < summaryModel.length; i++)
                total += summaryModel[i].weight
            return total
        }
        property int totalPower:
        {
            let total = 0
            for (let i = 0; i < summaryModel.length; i++)
                total += summaryModel[i].power
            return total
        }

        Column
        {
            id: tableContent
            width: uniSummaryView.tableWidth
            spacing: 0

            // header row: headers are a single line of text,
            // so they don't follow the data row height
            Row
            {
                id: headerRow
                width: parent.width
                height: UISettings.iconSizeDefault
                spacing: 0

                Repeater
                {
                    model: uniSummaryView.columns

                    delegate:
                        Rectangle
                        {
                            visible: modelData.visible
                            width: visible ? modelData.width : 0
                            height: headerRow.height
                            color: "transparent"
                            clip: true

                            RobotoText
                            {
                                anchors.fill: parent
                                leftMargin: 5
                                rightMargin: 5
                                label: modelData.text
                                labelColor: flickView.textColor
                                fontBold: true
                                wrapText: true
                            }

                            // column separator, skipped on the last visible column
                            Rectangle
                            {
                                visible: index < uniSummaryView.lastVisibleColumn
                                anchors.right: parent.right
                                width: 1
                                height: parent.height
                                color: "black"
                            }
                        }
                }
            }

            Rectangle { width: parent.width; height: 1; color: "black" }

            Repeater
            {
                id: fixtureRepeater
                model: flickView.summaryModel

                delegate:
                    Column
                    {
                        id: fxDelegate
                        width: tableContent.width

                        property Fixture cRef: modelData.classRef
                        property string fxManuf: modelData.manuf
                        property string fxModel: modelData.fmodel
                        property real fxWeight: modelData.weight
                        property int fxPower: modelData.power

                        // returns the text to display for a plain text column
                        function cellText(role)
                        {
                            if (!cRef)
                                return ""

                            switch (role)
                            {
                                case "id": return cRef.id
                                case "name": return cRef.name
                                case "manuf": return fxManuf
                                case "fmodel": return fxModel
                                case "address": return "" + (cRef.address + 1) + "-" +
                                                       (cRef.address + cRef.channels)
                                case "channels": return cRef.channels
                                case "weight": return fxWeight + "Kg"
                                case "power": return fxPower + "W"
                            }
                            return ""
                        }

                        // every cell is given the full row height, so the column
                        // separators join up even when the DIP switch is shown
                        Row
                        {
                            width: parent.width
                            spacing: 0

                            Repeater
                            {
                                model: uniSummaryView.columns

                                delegate:
                                    Rectangle
                                    {
                                        visible: modelData.visible
                                        width: visible ? modelData.width : 0
                                        height: uniSummaryView.rowHeight
                                        color: "transparent"
                                        clip: true

                                        // text columns
                                        RobotoText
                                        {
                                            anchors.fill: parent
                                            visible: modelData.role !== "icon" && modelData.role !== "dip"
                                            leftMargin: 5
                                            rightMargin: 5
                                            label: fxDelegate.cellText(modelData.role)
                                            labelColor: flickView.textColor
                                            wrapText: true
                                        }

                                        // fixture icon
                                        Image
                                        {
                                            anchors.centerIn: parent
                                            visible: modelData.role === "icon"
                                            source: visible && fxDelegate.cRef ? fxDelegate.cRef.iconResource(true) : ""
                                            width: UISettings.iconSizeDefault
                                            height: width
                                            sourceSize: Qt.size(width, height)
                                        }

                                        // DIP switch
                                        DMXAddressWidget
                                        {
                                            anchors.centerIn: parent
                                            visible: modelData.role === "dip"
                                            width: uniSummaryView.dipWidth
                                            height: uniSummaryView.dipHeight
                                            color: UISettings.bgLighter
                                            editable: false
                                            currentValue: fxDelegate.cRef ? fxDelegate.cRef.address + 1 : 1
                                        }

                                        // column separator, skipped on the last visible column
                                        Rectangle
                                        {
                                            visible: index < uniSummaryView.lastVisibleColumn
                                            anchors.right: parent.right
                                            width: 1
                                            height: parent.height
                                            color: "black"
                                        }
                                    }
                            }
                        }

                        Rectangle { width: parent.width; height: 1; color: "black" }
                    } // delegate
            } // Repeater

            Grid
            {
                id: summaryGrid
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
                    label: " " + flickView.totalWeight.toFixed(2) + "Kg"
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
        } // Column
    } // Flickable
}
