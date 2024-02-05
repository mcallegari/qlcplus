/*
  Q Light Controller Plus
  PopupChannelModifiers.qml

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

CustomPopupDialog
{
    id: popupRoot
    width: mainView.width / 2
    title: qsTr("Channel Modifiers Editor")
    standardButtons: Dialog.Ok | Dialog.Cancel

    property int itemID
    property int chIndex
    property string modName: "None"
    property variant modValues: fixtureManager.channelModifierValues

    onModValuesChanged: modPreview.requestPaint()

    contentItem:
        GridLayout
        {
            width: popupRoot.width
            columns: 3
            columnSpacing: UISettings.iconSizeMedium
            rowSpacing: 5

            // row 1
            RowLayout
            {
                Layout.columnSpan: 3
                height: UISettings.iconSizeMedium

                IconButton
                {
                    width: UISettings.iconSizeMedium
                    height: width
                    imgSource: "qrc:/add.svg"
                    tooltip: qsTr("Insert a modified value after the selected")
                    onClicked: {}
                }

                IconButton
                {
                    width: UISettings.iconSizeMedium
                    height: width
                    imgSource: "qrc:/remove.svg"
                    tooltip: qsTr("Delete the selected modifier value")
                    onClicked: {}
                }
                Rectangle
                {
                    Layout.fillWidth: true
                    height: UISettings.iconSizeMedium
                    color: "transparent"
                }
                IconButton
                {
                    width: UISettings.iconSizeMedium
                    height: width
                    imgSource: "qrc:/rename.svg"
                    tooltip: qsTr("Rename the selected modifier template")
                    onClicked: {}
                }
                IconButton
                {
                    width: UISettings.iconSizeMedium
                    height: width
                    imgSource: "qrc:/filesave.svg"
                    tooltip: qsTr("Save the selected modifier template")
                    onClicked: {}
                }
            }

            // row 2
            Canvas
            {
                id: modPreview
                Layout.columnSpan: 2
                Layout.fillWidth: true
                height: UISettings.bigItemHeight * 2
                antialiasing: true
                contextType: "2d"

                property real dX: width / 256.0
                property real dY: height / 256.0
                property int selectedPointIndex: -1

                onPaint:
                {
                    context.globalAlpha = 1.0
                    context.fillStyle = UISettings.bgStronger
                    context.strokeStyle = "yellow"
                    context.lineWidth = 1

                    var x1, y1, x2, y2

                    context.fillRect(0, 0, width, height)

                    if (modValues.length)
                    {
                        context.beginPath()

                        for (var i = 0; i < modValues.length - 2; i+= 2)
                        {
                            x1 = modValues[i]
                            y1 = modValues[i + 1]
                            x2 = modValues[i + 2]
                            y2 = modValues[i + 3]
                            context.moveTo(x1 * dX, height - (y1 * dY))
                            context.lineTo(x2 * dX, height - (y2 * dY))
                        }

                        context.stroke()
                        context.closePath()
                    }
                }

                Repeater
                {
                    model: modValues.length / 2

                    Rectangle
                    {
                        property int origDMX: modValues[index * 2]
                        property int modDMX: modValues[(index * 2) + 1]

                        width: 14
                        height: 14
                        x: (origDMX * modPreview.dX) - (width / 2)
                        y: modPreview.height - (modDMX * modPreview.dY) - (height / 2)
                        radius: width / 2
                        color: modPreview.selectedPointIndex === index ? UISettings.highlight : "yellow"

                        MouseArea
                        {
                            anchors.fill: parent
                            onClicked:
                            {
                                modPreview.selectedPointIndex = index
                                origValueSpin.value = origDMX
                                modValueSpin.value = modDMX
                            }
                        }
                    }
                }
            }

            ListView
            {
                id: modList
                z: 1
                Layout.rowSpan: 4
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true
                boundsBehavior: Flickable.StopAtBounds
                headerPositioning: ListView.OverlayHeader

                model: fixtureManager.channelModifiersList

                header:
                    RobotoText
                    {
                        z: 2
                        width: modList.width
                        height: UISettings.listItemHeight
                        label: qsTr("Templates")
                        color: UISettings.sectionHeader
                    }

                delegate:
                    Rectangle
                    {
                        height: UISettings.listItemHeight
                        width: modList.width
                        color: modelData === popupRoot.modName ? UISettings.highlight : "transparent"

                        RobotoText
                        {
                            height: parent.height
                            label: modelData
                        }
                        MouseArea
                        {
                            anchors.fill: parent
                            onClicked:
                            {
                                modPreview.selectedPointIndex = -1
                                popupRoot.modName = modelData
                                fixtureManager.selectChannelModifier(modelData)
                            }
                        }

                        Rectangle
                        {
                            y: parent.height - 1
                            height: 1
                            width: parent.width
                            color: UISettings.fgMedium
                        }
                    }

                ScrollBar.vertical: CustomScrollBar {}
            }

            Rectangle
            {
                Layout.columnSpan: 2
                Layout.fillWidth: true
                height: 10
                color: "transparent"
            }

            // row 3
            RobotoText
            {
                height: UISettings.listItemHeight
                label: qsTr("Original DMX value")
            }
            CustomSpinBox
            {
                id: origValueSpin
                from: 0
                to: 255
            }

            // row 4
            RobotoText
            {
                height: UISettings.listItemHeight
                label: qsTr("Modified DMX value")
            }
            CustomSpinBox
            {
                id: modValueSpin
                from: 0
                to: 255
            }
        }
}
