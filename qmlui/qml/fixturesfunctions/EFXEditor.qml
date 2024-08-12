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
import QtQuick.Controls 2.13

import org.qlcplus.classes 1.0

import "TimeUtils.js" as TimeUtils
import "."

Rectangle
{
    id: efxeContainer
    color: "transparent"

    property int functionID: -1

    signal requestView(int ID, string qmlSrc)

    ModelSelector
    {
        id: eeSelector
        onItemsCountChanged: console.log("EFX Editor selected items changed!")
    }

    TimeEditTool
    {
        id: timeEditTool

        parent: mainView
        z: 99
        x: rightSidePanel.x - width
        visible: false
        tempoType: efxEditor.tempoType

        onValueChanged:
        {
            if (speedType == QLCFunction.FadeIn)
                efxEditor.fadeInSpeed = val
            else if (speedType == QLCFunction.Hold)
                efxEditor.holdSpeed = val
            else if (speedType == QLCFunction.FadeOut)
                efxEditor.fadeOutSpeed = val
        }
    }

    SplitView
    {
        anchors.fill: parent

        Loader
        {
            id: fxTreeLoader
            width: UISettings.sidePanelWidth
            SplitView.preferredWidth: UISettings.sidePanelWidth
            visible: false
            height: efxeContainer.height
            source: ""

            property var modelProvider: null

            onLoaded:
            {
                if (modelProvider && item.hasOwnProperty('modelProvider'))
                    item.modelProvider = modelProvider
            }

            Rectangle
            {
                width: 2
                height: parent.height
                x: parent.width - 2
                color: UISettings.bgLight
            }
        }

        Rectangle
        {
            SplitView.fillWidth: true
            color: "transparent"

            EditorTopBar
            {
                id: topBar
                text: efxEditor.functionName
                onTextChanged: efxEditor.functionName = text

                onBackClicked:
                {
                    if (fxTreeLoader.visible)
                    {
                        fxTreeLoader.source = ""
                        fxTreeLoader.visible = false
                        rightSidePanel.width -= fxTreeLoader.width
                    }

                    var prevID = efxEditor.previousID
                    functionManager.setEditorFunction(prevID, false, true)
                    requestView(prevID, functionManager.getEditorResource(prevID))
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

                //Component.onCompleted: console.log("Flickable height: " + height + ", Grid height: " + editorColumn.height + ", parent height: " + parent.height)

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
                        width: editorColumn.colWidth - 5
                        minimumHeight: efxeContainer.height / 6
                        maximumHeight: efxeContainer.height / 3
                        maxPanDegrees: efxEditor.maxPanDegrees
                        maxTiltDegrees: efxEditor.maxTiltDegrees

                        efxData: efxEditor.algorithmData
                        fixturesData: efxEditor.fixturesData
                        animationInterval: efxEditor.duration / (efxData.length / 2)
                        isRelative: efxEditor.isRelative
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
                                columns: 5
                                columnSpacing: 0
                                rowSpacing: 0

                                // row 1 - toolbar
                                Rectangle
                                {
                                    Layout.fillWidth: true
                                    Layout.columnSpan: 5
                                    color: UISettings.bgMedium
                                    height: UISettings.listItemHeight

                                    IconButton
                                    {
                                        id: addFixture
                                        anchors.top: parent.top
                                        anchors.right: removeFixture.left

                                        width: height
                                        height: parent.height
                                        imgSource: "qrc:/add.svg"
                                        checkable: true
                                        tooltip: qsTr("Add a fixture/head")
                                        onCheckedChanged:
                                        {
                                            if (checked)
                                            {
                                                if (!fxTreeLoader.visible)
                                                    rightSidePanel.width += UISettings.sidePanelWidth
                                                fxTreeLoader.visible = true
                                                fxTreeLoader.modelProvider = efxEditor
                                                fxTreeLoader.source = "qrc:/FixtureGroupManager.qml"
                                            }
                                            else
                                            {
                                                rightSidePanel.width -= fxTreeLoader.width
                                                fxTreeLoader.source = ""
                                                fxTreeLoader.visible = false
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
                                        imgSource: "qrc:/remove.svg"
                                        tooltip: qsTr("Remove the selected fixture head(s)")
                                        onClicked: efxEditor.removeHeads(eeSelector.itemsList())
                                    }
                                }

                                // row 2 - header
                                RobotoText
                                {
                                    id: numCol
                                    width: UISettings.iconSizeMedium
                                    height: UISettings.listItemHeight
                                    label: "#"
                                    textHAlign: Text.AlignHCenter
                                    //fontSize: chListHeader.fSize

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
                                    id: modeCol
                                    height: UISettings.listItemHeight
                                    width: UISettings.bigItemHeight
                                    label: qsTr("Mode")

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
                                    id: reverseCol
                                    height: UISettings.listItemHeight
                                    label: qsTr("Reverse")

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
                                    id: offsetCol
                                    height: UISettings.listItemHeight
                                    label: qsTr("Start offset")
                                }

                                // row 3
                                ListView
                                {
                                    id: fixtureListView
                                    Layout.fillWidth: true
                                    Layout.columnSpan: 5
                                    model: efxEditor.fixtureList
                                    implicitHeight: count * UISettings.listItemHeight
                                    delegate:
                                        Rectangle
                                        {
                                            width: fixtureListView.width
                                            height: UISettings.listItemHeight
                                            color: "transparent"

                                            property int headMode: model.mode

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
                                                    eeSelector.selectItem(index, fixtureListView.model, mouse.modifiers)
                                                }
                                            }

                                            Row
                                            {
                                                //width: fixtureListView.width
                                                height: UISettings.listItemHeight

                                                /* Head number */
                                                RobotoText
                                                {
                                                    width: numCol.width
                                                    height: UISettings.listItemHeight
                                                    label: index

                                                    Rectangle
                                                    {
                                                        height: UISettings.listItemHeight
                                                        width: 1
                                                        anchors.right: parent.right
                                                        color: UISettings.fgMedium
                                                    }
                                                }
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
                                                /* Head mode */
                                                CustomComboBox
                                                {
                                                    height: editorColumn.itemsHeight
                                                    width: modeCol.width

                                                    ListModel
                                                    {
                                                        id: modeModel
                                                        ListElement { mLabel: qsTr("Position"); }
                                                        ListElement { mLabel: qsTr("Dimmer"); }
                                                        ListElement { mLabel: qsTr("RGB"); }
                                                    }
                                                    model: modeModel
                                                    currentIndex: headMode
                                                    onCurrentIndexChanged: efxEditor.setFixtureMode(fxID, head, currentIndex)

                                                    Rectangle
                                                    {
                                                        height: UISettings.listItemHeight
                                                        width: 1
                                                        anchors.right: parent.right
                                                        color: UISettings.fgMedium
                                                    }
                                                }
                                                /* Head reverse flag */
                                                Rectangle
                                                {
                                                    color: "transparent"
                                                    height: UISettings.listItemHeight
                                                    width: reverseCol.width

                                                    CustomCheckBox
                                                    {
                                                        anchors.centerIn: parent
                                                        implicitWidth: parent.height
                                                        implicitHeight: implicitWidth
                                                        checked: model.reverse
                                                        onCheckedChanged: efxEditor.setFixtureReversed(fxID, head, checked)
                                                    }
                                                    Rectangle
                                                    {
                                                        height: UISettings.listItemHeight
                                                        width: 1
                                                        anchors.right: parent.right
                                                        color: UISettings.fgMedium
                                                    }
                                                }
                                                /* Head offset */
                                                CustomSpinBox
                                                {
                                                    width: offsetCol.width
                                                    height: UISettings.listItemHeight
                                                    from: 0
                                                    to: 359
                                                    suffix: "°"
                                                    value: model.offset
                                                    onValueModified: efxEditor.setFixtureOffset(fxID, head, value)
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
                                    id: patternCombo
                                    Layout.fillWidth: true
                                    height: editorColumn.itemsHeight
                                    textRole: ""
                                    model: efxEditor.algorithms
                                    currentIndex: efxEditor.algorithmIndex
                                    onCurrentIndexChanged: efxEditor.algorithmIndex = currentIndex
                                }

                                // row 2
                                Row
                                {
                                    Layout.columnSpan: 2
                                    Layout.fillWidth: true
                                    spacing: 4

                                    CustomCheckBox
                                    {
                                        id: relativeCheck
                                        implicitWidth: UISettings.listItemHeight
                                        implicitHeight: implicitWidth
                                        autoExclusive: false
                                        checked: efxEditor.isRelative
                                        onCheckedChanged: efxEditor.isRelative = checked
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
                                    onValueModified: efxEditor.algorithmWidth = value
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
                                    onValueModified: efxEditor.algorithmHeight = value
                                }

                                // row 5
                                RobotoText
                                {
                                    visible: !relativeCheck.checked
                                    label: qsTr("X offset")
                                    height: UISettings.listItemHeight
                                }
                                CustomSpinBox
                                {
                                    visible: !relativeCheck.checked
                                    Layout.fillWidth: true
                                    from: 0
                                    to: 255
                                    value: efxEditor.algorithmXOffset
                                    onValueModified: efxEditor.algorithmXOffset = value
                                }

                                // row 6
                                RobotoText
                                {
                                    visible: !relativeCheck.checked
                                    label: qsTr("Y offset")
                                    height: UISettings.listItemHeight
                                }
                                CustomSpinBox
                                {
                                    visible: !relativeCheck.checked
                                    Layout.fillWidth: true
                                    from: 0
                                    to: 255
                                    value: efxEditor.algorithmYOffset
                                    onValueModified: efxEditor.algorithmYOffset = value
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
                                    onValueModified: efxEditor.algorithmRotation = value
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
                                    onValueModified: efxEditor.algorithmStartOffset = value
                                }

                                // row 9
                                RobotoText
                                {
                                    visible: patternCombo.displayText === "Lissajous"
                                    label: qsTr("X frequency")
                                    height: UISettings.listItemHeight
                                }
                                CustomSpinBox
                                {
                                    visible: patternCombo.displayText === "Lissajous"
                                    Layout.fillWidth: true
                                    from: 0
                                    to: 32
                                    value: efxEditor.algorithmXFrequency
                                    onValueModified: efxEditor.algorithmXFrequency = value
                                }

                                // row 10
                                RobotoText
                                {
                                    visible: patternCombo.displayText === "Lissajous"
                                    label: qsTr("Y frequency")
                                    height: UISettings.listItemHeight
                                }
                                CustomSpinBox
                                {
                                    visible: patternCombo.displayText === "Lissajous"
                                    Layout.fillWidth: true
                                    from: 0
                                    to: 32
                                    value: efxEditor.algorithmYFrequency
                                    onValueModified: efxEditor.algorithmYFrequency = value
                                }

                                // row 11
                                RobotoText
                                {
                                    visible: patternCombo.displayText === "Lissajous"
                                    label: qsTr("X phase")
                                    height: UISettings.listItemHeight
                                }
                                CustomSpinBox
                                {
                                    visible: patternCombo.displayText === "Lissajous"
                                    Layout.fillWidth: true
                                    from: 0
                                    to: 360
                                    suffix: "°"
                                    value: efxEditor.algorithmXPhase
                                    onValueModified: efxEditor.algorithmXPhase = value
                                }

                                // row 9
                                RobotoText
                                {
                                    visible: patternCombo.displayText === "Lissajous"
                                    label: qsTr("Y phase")
                                    height: UISettings.listItemHeight
                                }
                                CustomSpinBox
                                {
                                    visible: patternCombo.displayText === "Lissajous"
                                    Layout.fillWidth: true
                                    from: 0
                                    to: 360
                                    suffix: "°"
                                    value: efxEditor.algorithmYPhase
                                    onValueModified: efxEditor.algorithmYPhase = value
                                }
                            } // GridLayout
                    } // SectionBox

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

                                // Row 1
                                RobotoText
                                {
                                    id: fiLabel
                                    label: qsTr("Fade in")
                                    height: UISettings.listItemHeight
                                }

                                Rectangle
                                {
                                    Layout.fillWidth: true
                                    height: UISettings.listItemHeight
                                    color: UISettings.bgMedium

                                    RobotoText
                                    {
                                        anchors.fill: parent
                                        label: TimeUtils.timeToQlcString(efxEditor.fadeInSpeed, efxEditor.tempoType)

                                        MouseArea
                                        {
                                            anchors.fill: parent
                                            onDoubleClicked:
                                            {
                                                timeEditTool.allowFractions = QLCFunction.ByTwoFractions
                                                timeEditTool.show(-1, this.mapToItem(mainView, 0, 0).y,
                                                                  fiLabel.label, parent.label, QLCFunction.FadeIn)
                                            }
                                        }
                                    }
                                }

                                // Row 2
                                RobotoText
                                {
                                    id: hLabel
                                    height: UISettings.listItemHeight
                                    label: qsTr("Loop")
                                }

                                Rectangle
                                {
                                    Layout.fillWidth: true
                                    height: UISettings.listItemHeight
                                    color: UISettings.bgMedium

                                    RobotoText
                                    {
                                        anchors.fill: parent
                                        label: TimeUtils.timeToQlcString(efxEditor.holdSpeed, efxEditor.tempoType)

                                        MouseArea
                                        {
                                            anchors.fill: parent
                                            onDoubleClicked:
                                            {
                                                timeEditTool.allowFractions = QLCFunction.ByTwoFractions
                                                timeEditTool.show(-1, this.mapToItem(mainView, 0, 0).y,
                                                                  hLabel.label, parent.label, QLCFunction.Hold)
                                            }
                                        }
                                    }
                                }

                                // Row 3
                                RobotoText
                                {
                                    id: foLabel
                                    height: UISettings.listItemHeight
                                    label: qsTr("Fade out")
                                }

                                Rectangle
                                {
                                    Layout.fillWidth: true
                                    height: UISettings.listItemHeight
                                    color: UISettings.bgMedium

                                    RobotoText
                                    {
                                        anchors.fill: parent
                                        label: TimeUtils.timeToQlcString(efxEditor.fadeOutSpeed, efxEditor.tempoType)

                                        MouseArea
                                        {
                                            anchors.fill: parent
                                            onDoubleClicked:
                                            {
                                                timeEditTool.allowFractions = QLCFunction.ByTwoFractions
                                                timeEditTool.show(-1, this.mapToItem(mainView, 0, 0).y,
                                                                  foLabel.label, parent.label, QLCFunction.FadeOut)
                                            }
                                        }
                                    }
                                }
                            } // GridLayout
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
                                        ListElement { mLabel: qsTr("Loop"); mIcon: "qrc:/loop.svg"; mValue: QLCFunction.Loop }
                                        ListElement { mLabel: qsTr("Single Shot"); mIcon: "qrc:/arrow-end.svg"; mValue: QLCFunction.SingleShot }
                                        ListElement { mLabel: qsTr("Ping Pong"); mIcon: "qrc:/pingpong.svg"; mValue: QLCFunction.PingPong }
                                    }
                                    model: runOrderModel

                                    currValue: efxEditor.runOrder
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
                                        ListElement { mLabel: qsTr("Forward"); mIcon: "qrc:/forward.svg"; mValue: QLCFunction.Forward }
                                        ListElement { mLabel: qsTr("Backward"); mIcon: "qrc:/back.svg"; mValue: QLCFunction.Backward }
                                    }
                                    model: directionModel

                                    currValue: efxEditor.direction
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
                ScrollBar.vertical: CustomScrollBar { id: sbar }
            } // Flickable
        } // Column
    } // SplitView
}
