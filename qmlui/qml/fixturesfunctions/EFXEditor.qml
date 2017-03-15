/*
  Q Light Controller Plus
  EFXEditor.qml

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
import QtQuick.Layouts 1.2

import com.qlcplus.classes 1.0

import "TimeUtils.js" as TimeUtils
import "."

Rectangle
{
    id: efxeContainer
    color: "transparent"

    property int functionID: -1

    signal requestView(int ID, string qmlSrc)

    Rectangle
    {
        id: topBar
        color: UISettings.bgMedium
        width: efxeContainer.width
        height: UISettings.iconSizeMedium
        z: 2

        Rectangle
        {
            id: backBox
            width: UISettings.iconSizeMedium
            height: width
            color: "transparent"

            Image
            {
                id: leftArrow
                anchors.fill: parent
                rotation: 180
                source: "qrc:/arrow-right.svg"
                sourceSize: Qt.size(width, height)
            }
            MouseArea
            {
                anchors.fill: parent
                hoverEnabled: true
                onEntered: backBox.color = "#666"
                onExited: backBox.color = "transparent"
                onClicked:
                {
                    functionManager.setEditorFunction(-1, false)
                    requestView(-1, "qrc:/FunctionManager.qml")
                }
            }
        }
        TextInput
        {
            id: cNameEdit
            x: leftArrow.width + 5
            height: UISettings.iconSizeMedium
            width: topBar.width - x
            color: UISettings.fgMain
            clip: true
            text: efxEditor.functionName
            verticalAlignment: TextInput.AlignVCenter
            font.family: UISettings.robotoFontName
            font.pixelSize: UISettings.textSizeDefault
            selectByMouse: true
            Layout.fillWidth: true
            onTextChanged: efxEditor.functionName = text
        }
    }

    Flickable
    {
        id: editorFlickable
        x: 5
        y: topBar.height + 2
        width: parent.width - 10
        height: parent.height - y
        interactive: !previewBox.isRotating

        contentHeight: editorColumn.height
        boundsBehavior: Flickable.StopAtBounds

        Component.onCompleted: console.log("Flickable height: " + height + ", Grid height: " + editorColumn.height + ", parent height: " + parent.height)

        Column
        {
            id: editorColumn
            width: parent.width
            spacing: 2

            property int itemsHeight: UISettings.listItemHeight
            property int firstColumnWidth: 0
            property int colWidth: parent.width - (sbar.visible ? sbar.width : 0)

            // row 1
            EFXPreview
            {
                id: previewBox
                width: editorColumn.colWidth
                efxData: efxEditor.algorithmData
                maximumHeight: efxeContainer.height / 3
            }

            SectionBox
            {
                id: fixtureSection
                width: editorColumn.colWidth - 5
                isExpanded: false
                sectionLabel: qsTr("Fixtures")
                sectionContents:
                    GridLayout
                    {
                        width: parent.width
                        columns: 2
                        columnSpacing: 5
                        rowSpacing: 4
                    }
            }

            SectionBox
            {
                id: patternSection
                width: editorColumn.colWidth - 5
                isExpanded: true
                sectionLabel: qsTr("Pattern")
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
                            label: qsTr("Pattern")
                            height: editorColumn.itemsHeight
                        }
                        CustomComboBox
                        {
                            Layout.fillWidth: true
                            height: editorColumn.itemsHeight
                            model: efxEditor.algorithms
                            currentIndex: efxEditor.algorithmIndex
                            onCurrentTextChanged: efxEditor.algorithmIndex = currentIndex
                        }

                        // row 2
                        Row
                        {
                            Layout.columnSpan: 2
                            Layout.fillWidth: true
                            spacing: 4

                            CustomCheckBox
                            {
                                height: UISettings.listItemHeight
                                width: height
                            }

                            RobotoText
                            {
                                label: qsTr("Relative movement")
                                height: UISettings.listItemHeight
                            }
                        }

                        // row 3
                        RobotoText
                        {
                            label: qsTr("Width")
                            height: UISettings.listItemHeight
                        }
                        CustomSpinBox
                        {
                            Layout.fillWidth: true
                            from: 0
                            to: 127
                            value: efxEditor.algorithmWidth
                            onValueChanged: efxEditor.algorithmWidth = value
                        }

                        // row 4
                        RobotoText
                        {
                            label: qsTr("Height")
                            height: UISettings.listItemHeight
                        }
                        CustomSpinBox
                        {
                            Layout.fillWidth: true
                            from: 0
                            to: 127
                            value: efxEditor.algorithmHeight
                            onValueChanged: efxEditor.algorithmHeight = value
                        }

                        // row 5
                        RobotoText
                        {
                            label: qsTr("X offset")
                            height: UISettings.listItemHeight
                        }
                        CustomSpinBox
                        {
                            Layout.fillWidth: true
                            from: 0
                            to: 255
                            value: efxEditor.algorithmXOffset
                            onValueChanged: efxEditor.algorithmXOffset = value
                        }

                        // row 6
                        RobotoText
                        {
                            label: qsTr("Y offset")
                            height: UISettings.listItemHeight
                        }
                        CustomSpinBox
                        {
                            Layout.fillWidth: true
                            from: 0
                            to: 255
                            value: efxEditor.algorithmYOffset
                            onValueChanged: efxEditor.algorithmYOffset = value
                        }

                        // row 7
                        RobotoText
                        {
                            label: qsTr("Rotation")
                            height: UISettings.listItemHeight
                        }
                        CustomSpinBox
                        {
                            Layout.fillWidth: true
                            from: 0
                            to: 359
                            suffix: "°"
                            value: efxEditor.algorithmRotation
                            onValueChanged: efxEditor.algorithmRotation = value
                        }

                        // row 8
                        RobotoText
                        {
                            label: qsTr("Start offset")
                            height: UISettings.listItemHeight
                        }
                        CustomSpinBox
                        {
                            Layout.fillWidth: true
                            from: 0
                            to: 360
                            suffix: "°"
                            value: efxEditor.algorithmStartOffset
                            onValueChanged: efxEditor.algorithmStartOffset = value
                        }

                        // row 9
                        RobotoText
                        {
                            label: qsTr("X frequency")
                            height: UISettings.listItemHeight
                        }
                        CustomSpinBox
                        {
                            Layout.fillWidth: true
                            from: 0
                            to: 32
                            value: efxEditor.algorithmXFrequency
                            onValueChanged: efxEditor.algorithmXFrequency = value
                        }

                        // row 10
                        RobotoText
                        {
                            label: qsTr("Y frequency")
                            height: UISettings.listItemHeight
                        }
                        CustomSpinBox
                        {
                            Layout.fillWidth: true
                            from: 0
                            to: 32
                            value: efxEditor.algorithmYFrequency
                            onValueChanged: efxEditor.algorithmYFrequency = value
                        }

                        // row 11
                        RobotoText
                        {
                            label: qsTr("X phase")
                            height: UISettings.listItemHeight
                        }
                        CustomSpinBox
                        {
                            Layout.fillWidth: true
                            from: 0
                            to: 360
                            suffix: "°"
                            value: efxEditor.algorithmXPhase
                            onValueChanged: efxEditor.algorithmXPhase = value
                        }

                        // row 9
                        RobotoText
                        {
                            label: qsTr("Y phase")
                            height: UISettings.listItemHeight
                        }
                        CustomSpinBox
                        {
                            Layout.fillWidth: true
                            from: 0
                            to: 360
                            suffix: "°"
                            value: efxEditor.algorithmYPhase
                            onValueChanged: efxEditor.algorithmYPhase = value
                        }
                    }
            }

            SectionBox
            {
                id: speedSection
                width: editorColumn.colWidth - 5
                isExpanded: false
                sectionLabel: qsTr("Speed")
                sectionContents:
                    GridLayout
                    {
                        width: parent.width
                        columns: 2
                        columnSpacing: 5
                        rowSpacing: 4
                    }
            }

            SectionBox
            {
                id: directionSection
                width: editorColumn.colWidth - 5
                sectionLabel: qsTr("Order and direction")
                sectionContents:
                    GridLayout
                    {
                        width: parent.width
                        columns: 4
                        columnSpacing: 4
                        rowSpacing: 4

                        // Row 1
                        IconPopupButton
                        {
                            ListModel
                            {
                                id: runOrderModel
                                ListElement { mLabel: qsTr("Loop"); mIcon: "qrc:/loop.svg"; mValue: Function.Loop }
                                ListElement { mLabel: qsTr("Single Shot"); mIcon: "qrc:/arrow-end.svg"; mValue: Function.SingleShot }
                                ListElement { mLabel: qsTr("Ping Pong"); mIcon: "qrc:/pingpong.svg"; mValue: Function.PingPong }
                            }
                            model: runOrderModel

                            currentValue: efxEditor.runOrder
                            onValueChanged: efxEditor.runOrder = value
                        }
                        RobotoText
                        {
                            label: qsTr("Run Order")
                            Layout.fillWidth: true
                        }

                        IconPopupButton
                        {
                            ListModel
                            {
                                id: directionModel
                                ListElement { mLabel: qsTr("Forward"); mIcon: "qrc:/forward.svg"; mValue: Function.Forward }
                                ListElement { mLabel: qsTr("Backward"); mIcon: "qrc:/back.svg"; mValue: Function.Backward }
                            }
                            model: directionModel

                            currentValue: efxEditor.direction
                            onValueChanged: efxEditor.direction = value
                        }
                        RobotoText
                        {
                            label: qsTr("Direction")
                            Layout.fillWidth: true
                        }
                    }
            }

        } // Column
    } // Flickable
    ScrollBar { id: sbar; flickable: editorFlickable }
}
