/*
  Q Light Controller Plus
  FixtureSummary.qml

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
    id: fixtureSummaryView
    anchors.fill: parent
    color: UISettings.fgMain

    function hasSettings() { return false; }

    property var info: fixtureManager.fixtureInfo(fixtureManager.itemID)

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

            // filler
            Rectangle { color: "transparent"; Layout.fillWidth: true; height: UISettings.iconSizeDefault }

            IconButton
            {
                width: UISettings.bigItemHeight
                imgSource: "qrc:/printer.svg"
                tooltip: qsTr("Print the fixture summary")
                onClicked: qlcplus.printItem(flickView.contentItem)
            }
        }
    }

    Flickable
    {
        id: flickView
        y: selBar.height
        width: parent.width
        height: parent.height - y
        contentHeight: contentsColumn.height
        boundsBehavior: Flickable.StopAtBounds

        property color textColor: "black"
        property int textHeight: UISettings.iconSizeMedium

        Column
        {
            id: contentsColumn
            width: parent.width

            GridLayout
            {
                columns: 4
                columnSpacing: 5
                rowSpacing: 3
                width: parent.width

                // row 1
                RobotoText
                {
                    Layout.columnSpan: 4
                    Layout.fillWidth: true
                    height: UISettings.iconSizeDefault
                    color: UISettings.sectionHeader
                    label: info.classRef.name
                    fontSize: UISettings.textSizeDefault * 1.5
                }
                // row 2
                Image
                {
                    Layout.rowSpan: 7
                    width: UISettings.bigItemHeight * 1.5
                    height: width
                    source: info.classRef.iconResource(true)
                    sourceSize: Qt.size(width, height)
                }

                RobotoText
                {
                    height: flickView.textHeight
                    label: qsTr("Manufacturer");
                    fontBold: true;
                    labelColor: flickView.textColor
                }
                RobotoText
                {
                    height: flickView.textHeight
                    label: info.manuf
                    labelColor: flickView.textColor
                    Layout.fillWidth: true
                }

                DMXAddressWidget
                {
                    Layout.rowSpan: 7
                    height: UISettings.iconSizeDefault * 2
                    width: UISettings.bigItemHeight * 2
                    color: UISettings.bgLighter
                    currentValue: info.classRef.address + 1
                }
                // row 3
                RobotoText
                {
                    height: flickView.textHeight
                    label: qsTr("Model")
                    fontBold: true
                    labelColor: flickView.textColor
                }
                RobotoText
                {
                    height: flickView.textHeight
                    label: info.fmodel
                    labelColor: flickView.textColor
                }
                // row 4
                RobotoText
                {
                    height: flickView.textHeight
                    label: qsTr("Mode")
                    fontBold: true
                    labelColor: flickView.textColor
                }
                RobotoText
                {
                    height: flickView.textHeight
                    label: info.mode
                    labelColor: flickView.textColor
                }
                // row 5
                RobotoText
                {
                    height: flickView.textHeight
                    label: qsTr("Universe")
                    fontBold: true
                    labelColor: flickView.textColor
                }
                RobotoText
                {
                    height: flickView.textHeight
                    label: (info.classRef.universe + 1)
                    labelColor: flickView.textColor
                }
                // row 6
                RobotoText
                {
                    height: flickView.textHeight
                    label: qsTr("Address range")
                    fontBold: true
                    labelColor: flickView.textColor
                }
                RobotoText
                {
                    height: flickView.textHeight
                    label: (info.classRef.address + 1) + " - " + (info.classRef.address + info.classRef.channels + 1)
                    labelColor: flickView.textColor
                }
                // row 7
                RobotoText
                {
                    height: flickView.textHeight
                    label: qsTr("Channels")
                    fontBold: true
                    labelColor: flickView.textColor
                }
                RobotoText
                {
                    height: flickView.textHeight
                    label: info.classRef.channels
                    labelColor: flickView.textColor
                }
                // row 8
                RobotoText
                {
                    height: flickView.textHeight
                    label: qsTr("Author")
                    fontBold: true
                    labelColor: flickView.textColor
                }
                RobotoText
                {
                    height: flickView.textHeight
                    label: info.author
                    labelColor: flickView.textColor
                }
            } // GridLayout - basic info

            SectionBox
            {
                width: parent.width
                sectionLabel: qsTr("Channels")
                sectionContents:
                    GridLayout
                    {
                        width: parent.width
                        columns: 3
                        columnSpacing: 1
                        rowSpacing: 3

                        RobotoText
                        {
                            height: flickView.textHeight
                            label: qsTr("Number")
                            labelColor: flickView.textColor
                            color: UISettings.bgLighter
                        }
                        RobotoText
                        {
                            height: flickView.textHeight
                            label: qsTr("DMX address")
                            labelColor: flickView.textColor
                            color: UISettings.bgLighter
                        }
                        RobotoText
                        {
                            Layout.fillWidth: true
                            height: flickView.textHeight
                            label: qsTr("Name")
                            labelColor: flickView.textColor
                            color: UISettings.bgLighter
                        }

                        Repeater
                        {
                            model: info.classRef.channels
                            RobotoText
                            {
                                Layout.column: 0
                                Layout.row: index + 1
                                height: flickView.textHeight
                                label: (index + 1)
                                labelColor: flickView.textColor
                            }
                        }
                        Repeater
                        {
                            model: info.classRef.channels
                            RobotoText
                            {
                                Layout.column: 1
                                Layout.row: index + 1
                                height: flickView.textHeight
                                label: (info.classRef.address + index + 1)
                                labelColor: flickView.textColor
                            }
                        }
                        Repeater
                        {
                            model: info.channels
                            IconTextEntry
                            {
                                Layout.column: 2
                                Layout.row: index + 1
                                Layout.fillWidth: true
                                height: flickView.textHeight
                                tLabel: modelData.mLabel
                                tLabelColor: flickView.textColor
                                iSrc: modelData.mIcon
                            }
                        }
                    } // GridLayout - channels
            } // SectionBox - channels

            SectionBox
            {
                width: parent.width
                sectionLabel: qsTr("Physical")
                sectionContents:
                    GridLayout
                    {
                        width: parent.width
                        columns: 2
                        columnSpacing: 5
                        rowSpacing: 3

                        // row 1
                        RobotoText
                        {
                            height: flickView.textHeight
                            label: qsTr("Width")
                            fontBold: true
                            labelColor: flickView.textColor
                        }
                        RobotoText
                        {
                            Layout.fillWidth: true
                            height: flickView.textHeight
                            label: info.width + "mm (" + (info.width / 25.4).toFixed(2) + "\")"
                            labelColor: flickView.textColor
                        }

                        // row 2
                        RobotoText
                        {
                            height: flickView.textHeight
                            label: qsTr("Height")
                            fontBold: true
                            labelColor: flickView.textColor
                        }
                        RobotoText
                        {
                            height: flickView.textHeight
                            label: info.height + "mm (" + (info.height / 25.4).toFixed(2) + "\")"
                            labelColor: flickView.textColor
                        }

                        // row 3
                        RobotoText
                        {
                            height: flickView.textHeight
                            label: qsTr("Depth")
                            fontBold: true
                            labelColor: flickView.textColor
                        }
                        RobotoText
                        {
                            height: flickView.textHeight
                            label: info.depth + "mm (" + (info.depth / 25.4).toFixed(2) + "\")"
                            labelColor: flickView.textColor
                        }

                        // row 4
                        RobotoText
                        {
                            height: flickView.textHeight
                            label: qsTr("Weight")
                            fontBold: true
                            labelColor: flickView.textColor
                        }
                        RobotoText
                        {
                            height: flickView.textHeight
                            label: info.weight + "Kg (" + (info.weight / 0.4536).toFixed(2) + "lb)"
                            labelColor: flickView.textColor
                        }

                        // row 5
                        RobotoText
                        {
                            height: flickView.textHeight
                            label: qsTr("Power consumption")
                            fontBold: true
                            labelColor: flickView.textColor
                        }
                        RobotoText
                        {
                            height: flickView.textHeight
                            label: info.power + "W"
                            labelColor: flickView.textColor
                        }

                        // row 6
                        RobotoText
                        {
                            height: flickView.textHeight
                            label: qsTr("DMX connector")
                            fontBold: true
                            labelColor: flickView.textColor
                        }
                        RobotoText
                        {
                            height: flickView.textHeight
                            label: info.connector
                            labelColor: flickView.textColor
                        }
                    } // GridLayout - physical
            } // SectionBox - physical

            SectionBox
            {
                width: parent.width
                sectionLabel: qsTr("Focus")
                sectionContents:
                    GridLayout
                    {
                        width: parent.width
                        columns: 2
                        columnSpacing: 5
                        rowSpacing: 3

                        // row 1
                        RobotoText
                        {
                            height: flickView.textHeight
                            label: qsTr("Type")
                            fontBold: true
                            labelColor: flickView.textColor
                        }
                        RobotoText
                        {
                            Layout.fillWidth: true
                            height: flickView.textHeight
                            label: info.headType
                            labelColor: flickView.textColor
                        }

                        // row 2
                        RobotoText
                        {
                            height: flickView.textHeight
                            label: qsTr("Pan degrees")
                            fontBold: true
                            labelColor: flickView.textColor
                        }
                        RobotoText
                        {
                            height: flickView.textHeight
                            label: info.panDegrees + "°"
                            labelColor: flickView.textColor
                        }

                        // row 3
                        RobotoText
                        {
                            height: flickView.textHeight
                            label: qsTr("Tilt degrees")
                            fontBold: true
                            labelColor: flickView.textColor
                        }
                        RobotoText
                        {
                            height: flickView.textHeight
                            label: info.tiltDegrees + "°"
                            labelColor: flickView.textColor
                        }
                    } // GridLayout - focus
            } // SectionBox - focus

            SectionBox
            {
                width: parent.width
                sectionLabel: qsTr("Bulb")
                sectionContents:
                    GridLayout
                    {
                        width: parent.width
                        columns: 2
                        columnSpacing: 5
                        rowSpacing: 3

                        // row 1
                        RobotoText
                        {
                            height: flickView.textHeight
                            label: qsTr("Type")
                            fontBold: true
                            labelColor: flickView.textColor
                        }
                        RobotoText
                        {
                            Layout.fillWidth: true
                            height: flickView.textHeight
                            label: info.bulbType
                            labelColor: flickView.textColor
                        }

                        // row 2
                        RobotoText
                        {
                            height: flickView.textHeight
                            label: qsTr("Luminous flux")
                            fontBold: true
                            labelColor: flickView.textColor
                        }
                        RobotoText
                        {
                            height: flickView.textHeight
                            label: info.bulbLumens + "lm"
                            labelColor: flickView.textColor
                        }

                        // row 3
                        RobotoText
                        {
                            height: flickView.textHeight
                            label: qsTr("Color temperature")
                            fontBold: true
                            labelColor: flickView.textColor
                        }
                        RobotoText
                        {
                            height: flickView.textHeight
                            label: info.bulbTemp + "K"
                            labelColor: flickView.textColor
                        }
                    } // GridLayout - bulb
            } // SectionBox - bulb

            SectionBox
            {
                width: parent.width
                sectionLabel: qsTr("Lens")
                sectionContents:
                    GridLayout
                    {
                        width: parent.width
                        columns: 2
                        columnSpacing: 5
                        rowSpacing: 3

                        // row 1
                        RobotoText
                        {
                            height: flickView.textHeight
                            label: qsTr("Type")
                            fontBold: true
                            labelColor: flickView.textColor
                        }
                        RobotoText
                        {
                            Layout.fillWidth: true
                            height: flickView.textHeight
                            label: info.lensType
                            labelColor: flickView.textColor
                        }

                        // row 2
                        RobotoText
                        {
                            height: flickView.textHeight
                            label: qsTr("Beam range")
                            fontBold: true
                            labelColor: flickView.textColor
                        }
                        RobotoText
                        {
                            height: flickView.textHeight
                            label: info.beamMin + "° - " + info.beamMax + "°"
                            labelColor: flickView.textColor
                        }
                    } // GridLayout - lens
            } // SectionBox - lens
        } // Column
    } // Flickable
}
