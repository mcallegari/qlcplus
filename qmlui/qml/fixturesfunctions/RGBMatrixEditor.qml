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

import com.qlcplus.classes 1.0

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
            if (speedType == Function.FadeIn)
                rgbMatrixEditor.fadeInSpeed = val
            else if (speedType == Function.Hold)
                rgbMatrixEditor.holdSpeed = val
            else if (speedType == Function.FadeOut)
                rgbMatrixEditor.fadeOutSpeed = val
        }
    }

    Rectangle
    {
        id: topBar
        color: UISettings.bgMedium
        width: rgbmeContainer.width
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
            text: rgbMatrixEditor.functionName
            verticalAlignment: TextInput.AlignVCenter
            font.family: UISettings.robotoFontName
            font.pixelSize: UISettings.textSizeDefault
            selectByMouse: true
            Layout.fillWidth: true
            onTextChanged: rgbMatrixEditor.functionName = text
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
                    currentValue: rgbMatrixEditor.fixtureGroup
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
                    model: rgbMatrixEditor.algorithms
                    currentIndex: rgbMatrixEditor.algorithmIndex
                    onCurrentTextChanged:
                    {
                        rgbMatrixEditor.algorithmIndex = currentIndex
                        paramSection.sectionContents = null
                        if (currentText == "Text")
                            paramSection.sectionContents = textAlgoComponent
                        else if (currentText == "Image")
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
                    //currentIndex: rgbMatrixEditor.currentAlgo
                    //onCurrentIndexChanged: rgbMatrixEditor.currentAlgo = currentIndex
                }
            }

            // row 5
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
                    id: startColButton
                    width: UISettings.iconSizeDefault * 2
                    height: editorColumn.itemsHeight
                    radius: 5
                    border.color: scMouseArea.containsMouse ? "white" : UISettings.bgLight
                    border.width: 2
                    color: startColTool.selectedColor
                    visible: rgbMatrixEditor.algoColors > 0 ? true : false

                    MouseArea
                    {
                        id: scMouseArea
                        anchors.fill: parent
                        hoverEnabled: true
                        onClicked: startColTool.visible = !startColTool.visible
                    }

                    ColorTool
                    {
                        id: startColTool
                        parent: mainView
                        x: rightSidePanel.x - width
                        y: rightSidePanel.y
                        visible: false
                        closeOnSelect: true
                        selectedColor: rgbMatrixEditor.startColor

                        onColorChanged:
                        {
                            startColButton.color = Qt.rgba(r, g, b, 1.0)
                            rgbMatrixEditor.startColor = startColButton.color
                        }
                    }
                }
                Rectangle
                {
                    id: endColButton
                    width: UISettings.iconSizeDefault * 2
                    height: editorColumn.itemsHeight
                    radius: 5
                    border.color: ecMouseArea.containsMouse ? "white" : UISettings.bgLight
                    border.width: 2
                    color: rgbMatrixEditor.hasEndColor ? rgbMatrixEditor.endColor : "transparent"
                    visible: rgbMatrixEditor.algoColors > 1 ? true : false

                    MouseArea
                    {
                        id: ecMouseArea
                        anchors.fill: parent
                        hoverEnabled: true
                        onClicked: endColTool.visible = !startColTool.visible
                    }

                    ColorTool
                    {
                        id: endColTool
                        parent: mainView
                        x: rightSidePanel.x - width
                        y: rightSidePanel.y
                        visible: false
                        closeOnSelect: true
                        selectedColor: rgbMatrixEditor.endColor

                        onColorChanged: rgbMatrixEditor.endColor = Qt.rgba(r, g, b, 1.0)
                    }
                }
                IconButton
                {
                    width: UISettings.listItemHeight
                    height: width
                    imgSource: "qrc:/cancel.svg"
                    visible: rgbMatrixEditor.algoColors > 1 ? true : false
                    onClicked: rgbMatrixEditor.hasEndColor = false
                }
                // filler
                //Rectangle { Layout.fillWidth: true; height: parent.height; color: "transparent" }
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
                                        timeEditTool.allowFractions = Function.ByTwoFractions
                                        timeEditTool.show(-1, this.mapToItem(mainView, 0, 0).y,
                                                          fiLabel.label, parent.label, Function.FadeIn)
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
                                        timeEditTool.allowFractions = Function.ByTwoFractions
                                        timeEditTool.show(-1, this.mapToItem(mainView, 0, 0).y,
                                                          hLabel.label, parent.label, Function.Hold)
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
                                        timeEditTool.allowFractions = Function.ByTwoFractions
                                        timeEditTool.show(-1, this.mapToItem(mainView, 0, 0).y,
                                                          foLabel.label, parent.label, Function.FadeOut)
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
                                ListElement { mLabel: qsTr("Time"); mValue: Function.Time }
                                ListElement { mLabel: qsTr("Beats"); mValue: Function.Beats }
                            }
                            Layout.fillWidth: true
                            height: UISettings.listItemHeight
                            model: tempoModel

                            currentValue: rgbMatrixEditor.tempoType
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
                                ListElement { mLabel: qsTr("Loop"); mIcon: "qrc:/loop.svg"; mValue: Function.Loop }
                                ListElement { mLabel: qsTr("Single Shot"); mIcon: "qrc:/arrow-end.svg"; mValue: Function.SingleShot }
                                ListElement { mLabel: qsTr("Ping Pong"); mIcon: "qrc:/pingpong.svg"; mValue: Function.PingPong }
                            }
                            model: runOrderModel

                            currentValue: rgbMatrixEditor.runOrder
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
                                ListElement { mLabel: qsTr("Forward"); mIcon: "qrc:/forward.svg"; mValue: Function.Forward }
                                ListElement { mLabel: qsTr("Backward"); mIcon: "qrc:/back.svg"; mValue: Function.Backward }
                            }
                            model: directionModel

                            currentValue: rgbMatrixEditor.direction
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
    } // Flickable
    CustomScrollBar { id: sbar; flickable: editorFlickable }

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
                        onValueChanged:
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
                        onValueChanged:
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
                        onValueChanged:
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
                        onValueChanged:
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
            height: UISettings.listItemHeight
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

            onValueChanged: rgbMatrixEditor.setScriptIntProperty(propName, value)
        }
    }

}
