/*
  Q Light Controller Plus
  RGBMatrixEditor.qml

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
import QtQuick.Dialogs 1.1
import QtQuick.Controls 2.1

import org.qlcplus.classes 1.0

import "TimeUtils.js" as TimeUtils
import "."

Rectangle
{
    id: rgbmeContainer
    //anchors.fill: parent
    color: "transparent"

    property int functionID: -1

    signal requestView(int ID, string qmlSrc)

    TimeEditTool
    {
        id: timeEditTool

        parent: mainView
        z: 99
        x: rightSidePanel.x - width
        visible: false
        tempoType: rgbMatrixEditor.tempoType

        onValueChanged:
        {
            if (speedType == QLCFunction.FadeIn)
                rgbMatrixEditor.fadeInSpeed = val
            else if (speedType == QLCFunction.Hold)
                rgbMatrixEditor.holdSpeed = val
            else if (speedType == QLCFunction.FadeOut)
                rgbMatrixEditor.fadeOutSpeed = val
        }
    }

    ColorTool
    {
        id: colorTool
        x: -width - (UISettings.iconSizeDefault * 1.25)
        y: UISettings.bigItemHeight
        visible: false
        closeOnSelect: true

        property int colorIndex: -1
        property Item previewBtn

        function showTool(index, button)
        {
            colorIndex = index
            previewBtn = button
            currentRGB = rgbMatrixEditor.colorAtIndex(colorIndex)
            visible = true
        }

        function hide()
        {
            visible = false
            colorIndex = -1
            previewBtn = null
        }

        onColorChanged:
        {
            previewBtn.color = Qt.rgba(r, g, b, 1.0)
            rgbMatrixEditor.setColorAtIndex(colorIndex, previewBtn.color)
        }
        onClose: visible = false
    }

    EditorTopBar
    {
        id: topBar
        text: rgbMatrixEditor.functionName
        onTextChanged: rgbMatrixEditor.functionName = text

        onBackClicked:
        {
            var prevID = rgbMatrixEditor.previousID
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

            //onHeightChanged: editorFlickable.contentHeight = height //console.log("Grid layout height changed: " + height)

            function checkLabelWidth(w)
            {
                firstColumnWidth = Math.max(w, firstColumnWidth)
            }

            // row 1
            RowLayout
            {
                width: editorColumn.colWidth

                RobotoText
                {
                    label: qsTr("Fixture Group")
                    height: editorColumn.itemsHeight
                    onWidthChanged:
                    {
                        editorColumn.checkLabelWidth(width)
                        width = Qt.binding(function() { return editorColumn.firstColumnWidth })
                    }
                }
                CustomComboBox
                {
                    Layout.fillWidth: true
                    height: editorColumn.itemsHeight
                    model: fixtureGroupEditor.groupsListModel
                    currValue: rgbMatrixEditor.fixtureGroup
                    onValueChanged: rgbMatrixEditor.fixtureGroup = value
                }
            }

            // row 2
            RGBMatrixPreview
            {
                width: editorColumn.width
                matrixSize: rgbMatrixEditor.previewSize
                matrixData: rgbMatrixEditor.previewData
                maximumHeight: rgbmeContainer.height / 3
            }

            // row 3
            RowLayout
            {
                width: editorColumn.colWidth

                RobotoText
                {
                    id: patternLabel
                    label: qsTr("Pattern")
                    height: editorColumn.itemsHeight
                    onWidthChanged:
                    {
                        editorColumn.checkLabelWidth(width)
                        width = Qt.binding(function() { return editorColumn.firstColumnWidth })
                    }
                }
                CustomComboBox
                {
                    id: algoCombo
                    Layout.fillWidth: true
                    height: editorColumn.itemsHeight
                    textRole: ""
                    model: rgbMatrixEditor.algorithms
                    currentIndex: rgbMatrixEditor.algorithmIndex
                    onDisplayTextChanged:
                    {
                        rgbMatrixEditor.algorithmIndex = currentIndex
                        paramSection.sectionContents = null
                        if (displayText == "Text")
                            paramSection.sectionContents = textAlgoComponent
                        else if (displayText == "Image")
                            paramSection.sectionContents = imageAlgoComponent
                        else
                            paramSection.sectionContents = scriptAlgoComponent
                    }
                }
            }

            // row 4
            RowLayout
            {
                width: editorColumn.colWidth

                RobotoText
                {
                    label: qsTr("Blend mode")
                    height: editorColumn.itemsHeight
                    onWidthChanged:
                    {
                        editorColumn.checkLabelWidth(width)
                        width = Qt.binding(function() { return editorColumn.firstColumnWidth })
                    }
                }
                CustomComboBox
                {
                    Layout.fillWidth: true
                    height: editorColumn.itemsHeight

                    ListModel
                    {
                        id: blendModel
                        ListElement { mLabel: qsTr("Default (HTP)"); }
                        ListElement { mLabel: qsTr("Mask"); }
                        ListElement { mLabel: qsTr("Additive"); }
                        ListElement { mLabel: qsTr("Subtractive"); }
                    }
                    model: blendModel

                    currentIndex: rgbMatrixEditor.blendMode
                    onCurrentIndexChanged: rgbMatrixEditor.blendMode = currentIndex
                }
            }

            // row 5
            RowLayout
            {
                width: editorColumn.colWidth

                RobotoText
                {
                    label: qsTr("Color mode")
                    height: editorColumn.itemsHeight
                    onWidthChanged:
                    {
                        editorColumn.checkLabelWidth(width)
                        width = Qt.binding(function() { return editorColumn.firstColumnWidth })
                    }
                }
                CustomComboBox
                {
                    Layout.fillWidth: true
                    height: editorColumn.itemsHeight

                    ListModel
                    {
                        id: controlModel
                        ListElement { mLabel: qsTr("Default (RGB)"); }
                        ListElement { mLabel: qsTr("White"); }
                        ListElement { mLabel: qsTr("Amber"); }
                        ListElement { mLabel: qsTr("UV"); }
                        ListElement { mLabel: qsTr("Dimmer"); }
                        ListElement { mLabel: qsTr("Shutter"); }
                    }
                    model: controlModel

                    currentIndex: rgbMatrixEditor.controlMode
                    onCurrentIndexChanged: rgbMatrixEditor.controlMode = currentIndex
                }
            }

            // row 6
            Row
            {
                width: editorColumn.colWidth
                height: editorColumn.itemsHeight
                spacing: 4

                RobotoText
                {
                    label: qsTr("Colors")
                    visible: rgbMatrixEditor.algoColors > 0 ? true : false
                    height: editorColumn.itemsHeight
                    onWidthChanged:
                    {
                        editorColumn.checkLabelWidth(width)
                        width = Qt.binding(function() { return editorColumn.firstColumnWidth })
                    }
                }

                Rectangle
                {
                    id: color1Button
                    width: UISettings.iconSizeDefault * 2
                    height: editorColumn.itemsHeight
                    radius: 5
                    border.color: color1MouseArea.containsMouse ? "white" : UISettings.bgLight
                    border.width: 2
                    color: rgbMatrixEditor.colorAtIndex(0)
                    visible: rgbMatrixEditor.algoColors > 0 ? true : false

                    MouseArea
                    {
                        id: color1MouseArea
                        anchors.fill: parent
                        hoverEnabled: true
                        onClicked:
                        {
                            if (colorTool.visible)
                                colorTool.hide()
                            else
                                colorTool.showTool(0, color1Button)
                        }
                    }
                }
                Rectangle
                {
                    width: UISettings.listItemHeight
                    height: width
                    color: "transparent"
                    visible: rgbMatrixEditor.algoColors > 2 ? true : false
                }

                Rectangle
                {
                    id: color2Button
                    width: UISettings.iconSizeDefault * 2
                    height: editorColumn.itemsHeight
                    radius: 5
                    border.color: color2MouseArea.containsMouse ? "white" : UISettings.bgLight
                    border.width: 2
                    color: rgbMatrixEditor.colorAtIndex(1)
                    visible: rgbMatrixEditor.algoColors > 1 ? true : false

                    MouseArea
                    {
                        id: color2MouseArea
                        anchors.fill: parent
                        hoverEnabled: true
                        onClicked:
                        {
                            if (colorTool.visible)
                                colorTool.hide()
                            else
                                colorTool.showTool(1, color2Button)
                        }
                    }
                }
                IconButton
                {
                    width: UISettings.listItemHeight
                    height: width
                    imgSource: "qrc:/cancel.svg"
                    tooltip: qsTr("Reset color 2")
                    visible: rgbMatrixEditor.algoColors > 1 ? true : false
                    onClicked:
                    {
                        color2Button.color = "transparent"
                        rgbMatrixEditor.resetColorAtIndex(1)
                    }
                }
            }

            // row 7
            Row
            {
                width: editorColumn.colWidth
                height: editorColumn.itemsHeight
                spacing: 4
                visible: rgbMatrixEditor.algoColors > 4 ? true : false

                Rectangle
                {
                    id: colorRow1
                    height: editorColumn.itemsHeight
                    width: editorColumn.firstColumnWidth
                    color: "transparent"
                    visible: rgbMatrixEditor.algoColors > 4 ? true : false

                    onWidthChanged:
                    {
                        editorColumn.checkLabelWidth(width)
                        width = Qt.binding(function() { return editorColumn.firstColumnWidth })
                    }
                }

                Rectangle
                {
                    id: color3Button
                    width: UISettings.iconSizeDefault * 2
                    height: editorColumn.itemsHeight
                    radius: 5
                    border.color: color3MouseArea.containsMouse ? "white" : UISettings.bgLight
                    border.width: 2
                    color: rgbMatrixEditor.hasColorAtIndex(2) ? rgbMatrixEditor.colorAtIndex(2) : "transparent"
                    visible: rgbMatrixEditor.algoColors > 2 ? true : false

                    MouseArea
                    {
                        id: color3MouseArea
                        anchors.fill: parent
                        hoverEnabled: true
                        onClicked:
                        {
                            if (colorTool.visible)
                                colorTool.hide()
                            else
                                colorTool.showTool(2, color3Button)
                        }
                    }
                }
                IconButton
                {
                    width: UISettings.listItemHeight
                    height: width
                    imgSource: "qrc:/cancel.svg"
                    tooltip: qsTr("Reset color 3")
                    visible: rgbMatrixEditor.algoColors > 2 ? true : false
                    onClicked:
                    {
                        color3Button.color = "transparent"
                        rgbMatrixEditor.resetColorAtIndex(2)
                    }
                }

                Rectangle
                {
                    id: color4Button
                    width: UISettings.iconSizeDefault * 2
                    height: editorColumn.itemsHeight
                    radius: 5
                    border.color: color4MouseArea.containsMouse ? "white" : UISettings.bgLight
                    border.width: 2
                    color: rgbMatrixEditor.hasColorAtIndex(3) ? rgbMatrixEditor.colorAtIndex(3) : "transparent"
                    visible: rgbMatrixEditor.algoColors > 3 ? true : false

                    MouseArea
                    {
                        id: color4MouseArea
                        anchors.fill: parent
                        hoverEnabled: true
                        onClicked:
                        {
                            if (colorTool.visible)
                                colorTool.hide()
                            else
                                colorTool.showTool(3, color4Button)
                        }
                    }
                }
                IconButton
                {
                    width: UISettings.listItemHeight
                    height: width
                    imgSource: "qrc:/cancel.svg"
                    tooltip: qsTr("Reset color 4")
                    visible: rgbMatrixEditor.algoColors > 3 ? true : false
                    onClicked:
                    {
                        color4Button.color = "transparent"
                        rgbMatrixEditor.resetColorAtIndex(3)
                    }
                }
            }

            // row 8
            Row
            {
                width: editorColumn.colWidth
                height: editorColumn.itemsHeight
                spacing: 4
                visible: rgbMatrixEditor.algoColors > 4 ? true : false

                Rectangle
                {
                    id: colorRow2
                    height: editorColumn.itemsHeight
                    width: editorColumn.firstColumnWidth
                    color: "transparent"
                    visible: rgbMatrixEditor.algoColors > 4 ? true : false
                    onWidthChanged:
                    {
                        editorColumn.checkLabelWidth(width)
                        width = Qt.binding(function() { return editorColumn.firstColumnWidth })
                    }
                }

                Rectangle
                {
                    id: color5Button
                    width: UISettings.iconSizeDefault * 2
                    height: editorColumn.itemsHeight
                    radius: 5
                    border.color: color5MouseArea.containsMouse ? "white" : UISettings.bgLight
                    border.width: 2
                    color: rgbMatrixEditor.hasColorAtIndex(4) ? rgbMatrixEditor.colorAtIndex(4) : "transparent"
                    visible: rgbMatrixEditor.algoColors > 4 ? true : false

                    MouseArea
                    {
                        id: color5MouseArea
                        anchors.fill: parent
                        hoverEnabled: true
                        onClicked:
                        {
                            if (colorTool.visible)
                                colorTool.hide()
                            else
                                colorTool.showTool(4, color5Button)
                        }
                    }
                }
                IconButton
                {
                    width: UISettings.listItemHeight
                    height: width
                    imgSource: "qrc:/cancel.svg"
                    tooltip: qsTr("Reset color 5")
                    visible: rgbMatrixEditor.algoColors > 4 ? true : false
                    onClicked:
                    {
                        color5Button.color = "transparent"
                        rgbMatrixEditor.resetColorAtIndex(4)
                    }
                }
            }

            SectionBox
            {
                id: paramSection
                width: editorColumn.colWidth - 5
                visible: sectionContents ? true : false

                sectionLabel: qsTr("Parameters")
                sectionContents: null
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

                        // Row 1
                        RobotoText
                        {
                            id: fiLabel
                            label: qsTr("Steps fade in")
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
                                label: TimeUtils.timeToQlcString(rgbMatrixEditor.fadeInSpeed, rgbMatrixEditor.tempoType)

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
                            label: qsTr("Steps hold")
                        }

                        Rectangle
                        {
                            Layout.fillWidth: true
                            height: UISettings.listItemHeight
                            color: UISettings.bgMedium

                            RobotoText
                            {
                                anchors.fill: parent
                                label: TimeUtils.timeToQlcString(rgbMatrixEditor.holdSpeed, rgbMatrixEditor.tempoType)

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
                            label: qsTr("Steps fade out")
                        }

                        Rectangle
                        {
                            Layout.fillWidth: true
                            height: UISettings.listItemHeight
                            color: UISettings.bgMedium

                            RobotoText
                            {
                                anchors.fill: parent
                                label: TimeUtils.timeToQlcString(rgbMatrixEditor.fadeOutSpeed, rgbMatrixEditor.tempoType)

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

                        // Row 4
                        RobotoText
                        {
                            id: ttLabel
                            height: UISettings.listItemHeight
                            label: qsTr("Tempo type")
                        }
                        CustomComboBox
                        {
                            ListModel
                            {
                                id: tempoModel
                                ListElement { mLabel: qsTr("Time"); mValue: QLCFunction.Time }
                                ListElement { mLabel: qsTr("Beats"); mValue: QLCFunction.Beats }
                            }
                            Layout.fillWidth: true
                            height: UISettings.listItemHeight
                            model: tempoModel

                            currValue: rgbMatrixEditor.tempoType
                            onValueChanged: rgbMatrixEditor.tempoType = value
                        }
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
                                ListElement { mLabel: qsTr("Loop"); mIcon: "qrc:/loop.svg"; mValue: QLCFunction.Loop }
                                ListElement { mLabel: qsTr("Single Shot"); mIcon: "qrc:/arrow-end.svg"; mValue: QLCFunction.SingleShot }
                                ListElement { mLabel: qsTr("Ping Pong"); mIcon: "qrc:/pingpong.svg"; mValue: QLCFunction.PingPong }
                            }
                            model: runOrderModel

                            currValue: rgbMatrixEditor.runOrder
                            onValueChanged: rgbMatrixEditor.runOrder = value
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

                            currValue: rgbMatrixEditor.direction
                            onValueChanged: rgbMatrixEditor.direction = value
                        }
                        RobotoText
                        {
                            label: qsTr("Direction")
                            Layout.fillWidth: true
                        }

                    } // GridLayout
            }
        } // Column
        ScrollBar.vertical: CustomScrollBar { id: sbar }
    } // Flickable

    /* *************************************************************
     * Here starts all the Algorithm-specific Component definitions,
     * loaded at runtime depending on the selected algorithm
     * *********************************************************** */

    /* *************************************************************
     * **************** Text Algorithm parameters **************** */
    Component
    {
        id: textAlgoComponent
        GridLayout
        {
            columns: 2
            columnSpacing: 5

            // Row 1
            RobotoText
            {
                height: UISettings.listItemHeight
                label: qsTr("Text")
            }

            Rectangle
            {
                Layout.fillWidth: true
                height: editorColumn.itemsHeight
                color: "transparent"

                Rectangle
                {
                    height: parent.height
                    width: parent.width - fontButton.width - 5
                    radius: 3
                    color: UISettings.bgMedium
                    border.color: "#222"

                    TextInput
                    {
                        id: algoTextEdit
                        anchors.fill: parent
                        anchors.margins: 4
                        anchors.verticalCenter: parent.verticalCenter
                        text: rgbMatrixEditor.algoText
                        font.family: rgbMatrixEditor.algoTextFont.font.family
                        font.bold: rgbMatrixEditor.algoTextFont.font.bold
                        font.italic: rgbMatrixEditor.algoTextFont.font.italic
                        font.pixelSize: UISettings.textSizeDefault * 0.8
                        color: "white"

                        onTextChanged: rgbMatrixEditor.algoText = text
                    }
                }
                IconButton
                {
                    id: fontButton
                    width: UISettings.iconSizeMedium
                    height: width
                    anchors.right: parent.right
                    imgSource: "qrc:/font.svg"

                    onClicked: fontDialog.visible = true

                    FontDialog
                    {
                        id: fontDialog
                        title: qsTr("Please choose a font")
                        font: rgbMatrixEditor.algoTextFont
                        visible: false

                        onAccepted:
                        {
                            console.log("Selected font: " + fontDialog.font)
                            rgbMatrixEditor.algoTextFont = font
                        }
                    }
                }
            }

            // Row 2
            RobotoText
            {
                height: UISettings.listItemHeight
                label: qsTr("Animation")
            }
            CustomComboBox
            {
                Layout.fillWidth: true
                height: editorColumn.itemsHeight

                ListModel
                {
                    id: textAnimModel
                    ListElement { mLabel: qsTr("Letters"); }
                    ListElement { mLabel: qsTr("Horizontal"); }
                    ListElement { mLabel: qsTr("Vertical"); }
                }
                model: textAnimModel
                currentIndex: rgbMatrixEditor.animationStyle
                onCurrentIndexChanged: rgbMatrixEditor.animationStyle = currentIndex
            }

            // Row 3
            RobotoText
            {
                height: UISettings.listItemHeight
                label: qsTr("Offset")
            }
            Rectangle
            {
                Layout.fillWidth: true
                height: editorColumn.itemsHeight
                color: "transparent"

                Row
                {
                    id: toffRow
                    spacing: 20
                    anchors.fill: parent

                    property size algoOffset: rgbMatrixEditor.algoOffset

                    RobotoText { height: UISettings.listItemHeight; label: qsTr("X") }
                    CustomSpinBox
                    {
                        height: parent.height
                        from: -255
                        to: 255
                        value: toffRow.algoOffset.width
                        onValueModified:
                        {
                            var newOffset = toffRow.algoOffset
                            newOffset.width = value
                            rgbMatrixEditor.algoOffset = newOffset
                        }
                    }

                    RobotoText { height: UISettings.listItemHeight; label: qsTr("Y") }
                    CustomSpinBox
                    {
                        height: parent.height
                        from: -255
                        to: 255
                        value: toffRow.algoOffset.height
                        onValueModified:
                        {
                            var newOffset = toffRow.algoOffset
                            newOffset.height = value
                            rgbMatrixEditor.algoOffset = newOffset
                        }
                    }
                }
            }
        }
    }

    /* *************************************************************
     * **************** Image Algorithm parameters *************** */
    Component
    {
        id: imageAlgoComponent

        GridLayout
        {
            id: imageAlgoGrid
            columns: 2
            columnSpacing: 5

            // Row 1
            RobotoText
            {
                height: UISettings.listItemHeight
                label: qsTr("Image")
            }
            Rectangle
            {
                Layout.fillWidth: true
                height: editorColumn.itemsHeight
                color: "transparent"

                Rectangle
                {
                    height: parent.height
                    width: parent.width - imgButton.width - 5
                    radius: 3
                    color: UISettings.bgMedium
                    border.color: "#222"
                    clip: true

                    TextInput
                    {
                        id: algoTextEdit
                        anchors.fill: parent
                        anchors.margins: 4
                        anchors.verticalCenter: parent.verticalCenter
                        text: rgbMatrixEditor.algoImagePath
                        font.pixelSize: UISettings.textSizeDefault
                        color: "white"

                        onTextChanged: rgbMatrixEditor.algoImagePath = text
                    }
                }
                IconButton
                {
                    id: imgButton
                    width: UISettings.iconSizeMedium
                    height: width
                    anchors.right: parent.right
                    imgSource: "qrc:/background.svg"

                    onClicked: fileDialog.visible = true

                    FileDialog
                    {
                        id: fileDialog
                        visible: false
                        title: qsTr("Select an image")
                        nameFilters: [ "Image files (*.png *.bmp *.jpg *.jpeg *.gif)", "All files (*)" ]

                        onAccepted: rgbMatrixEditor.algoImagePath = fileDialog.fileUrl
                    }
                }
            }

            // Row 2
            RobotoText
            {
                height: UISettings.listItemHeight
                label: qsTr("Animation")
            }
            CustomComboBox
            {
                Layout.fillWidth: true
                height: editorColumn.itemsHeight

                ListModel
                {
                    id: imageAnimModel
                    ListElement { mLabel: qsTr("Static"); }
                    ListElement { mLabel: qsTr("Horizontal"); }
                    ListElement { mLabel: qsTr("Vertical"); }
                    ListElement { mLabel: qsTr("Animation"); }
                }
                model: imageAnimModel
                currentIndex: rgbMatrixEditor.animationStyle
                onCurrentIndexChanged: rgbMatrixEditor.animationStyle = currentIndex
            }

            // Row 3
            RobotoText
            {
                height: UISettings.listItemHeight
                label: qsTr("Offset")
            }
            Rectangle
            {
                Layout.fillWidth: true
                height: editorColumn.itemsHeight
                color: "transparent"

                Row
                {
                    id: ioffRow
                    spacing: 20
                    anchors.fill: parent

                    property size algoOffset: rgbMatrixEditor.algoOffset

                    RobotoText { height: UISettings.listItemHeight; label: qsTr("X") }
                    CustomSpinBox
                    {
                        height: parent.height
                        from: -255
                        to: 255
                        value: ioffRow.algoOffset.width
                        onValueModified:
                        {
                            var newOffset = ioffRow.algoOffset
                            newOffset.width = value
                            rgbMatrixEditor.algoOffset = newOffset
                        }
                    }

                    RobotoText { height: UISettings.listItemHeight; label: qsTr("Y") }
                    CustomSpinBox
                    {
                        height: parent.height
                        from: -255
                        to: 255
                        value: ioffRow.algoOffset.height
                        onValueModified:
                        {
                            var newOffset = ioffRow.algoOffset
                            newOffset.height = value
                            rgbMatrixEditor.algoOffset = newOffset
                        }
                    }
                }
            }
        }
    }

    /* ************************************************************ */
    /* ***************  Script Algorithm parameters *************** */
    Component
    {
        id: scriptAlgoComponent

        GridLayout
        {
            id: scriptAlgoGrid
            columns: 2
            columnSpacing: 5

            function addLabel(text)
            {
                labelComponent.createObject(scriptAlgoGrid,
                               {"propName": text });
                if (labelComponent.status !== Component.Ready)
                    console.log("Label component is not ready !!")
            }

            function addComboBox(propName, model, currentIndex)
            {
                comboComponent.createObject(scriptAlgoGrid,
                               {"propName": propName, "model": model, "currentIndex": currentIndex });
                if (comboComponent.status !== Component.Ready)
                    console.log("Combo component is not ready !!")
            }

            function addSpinBox(propName, min, max, currentValue)
            {
                spinComponent.createObject(scriptAlgoGrid,
                              {"propName": propName, "from": min, "to": max, "value": currentValue });
                if (spinComponent.status !== Component.Ready)
                    console.log("Spin component is not ready !!")
            }

            function addDoubleSpinBox(propName, currentValue)
            {
                doubleSpinComponent.createObject(scriptAlgoGrid,
                              {"propName": propName, "realValue": currentValue });
                if (spinComponent.status !== Component.Ready)
                    console.log("Double spin component is not ready !!")
            }

            function addTextEdit(propName, currentText)
            {
                textEditComponent.createObject(scriptAlgoGrid,
                               {"propName": propName, "text": currentText });
                if (comboComponent.status !== Component.Ready)
                    console.log("TextEdit component is not ready !!")
            }

            Component.onCompleted:
            {
                rgbMatrixEditor.createScriptObjects(scriptAlgoGrid)
            }
        }
    }

    // Script algorithm text label
    Component
    {
        id: labelComponent

        RobotoText
        {
            implicitHeight: UISettings.listItemHeight
            implicitWidth: width
            property string propName

            label: propName
        }
    }

    // Script algorithm combo box property
    Component
    {
        id: comboComponent

        CustomComboBox
        {
            Layout.fillWidth: true
            property string propName

            onCurrentTextChanged: rgbMatrixEditor.setScriptStringProperty(propName, currentText)
        }
    }

    // Script algorithm spin box property
    Component
    {
        id: spinComponent

        CustomSpinBox
        {
            Layout.fillWidth: true
            property string propName

            onValueModified: rgbMatrixEditor.setScriptIntProperty(propName, value)
        }
    }

    // Script algorithm float box property
    Component
    {
        id: doubleSpinComponent

        CustomDoubleSpinBox
        {
            Layout.fillWidth: true
            property string propName

            decimals: 3
            suffix: ""
            onRealValueChanged: rgbMatrixEditor.setScriptFloatProperty(propName, realValue)
        }
    }

    // Script algorithm combo box property
    Component
    {
        id: textEditComponent

        CustomTextEdit
        {
            Layout.fillWidth: true
            property string propName

            onTextChanged: rgbMatrixEditor.setScriptStringProperty(propName, text)
        }
    }
}
