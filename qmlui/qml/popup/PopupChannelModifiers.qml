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

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

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
    property var localValues: []
    property bool updatingSpins: false

    function syncLocalValues()
    {
        if (modValues && modValues.length)
            localValues = modValues.slice(0)
        else
            localValues = [0, 0, 255, 255]

        modPreview.selectedPointIndex = -1
        updateSpinControls()
        modPreview.requestPaint()
    }

    function handlerCount()
    {
        return Math.floor(localValues.length / 2)
    }

    function updateSpinControls()
    {
        if (modPreview.selectedPointIndex < 0)
        {
            origValueSpin.enabled = false
            modValueSpin.enabled = false
            deleteHandlerButton.enabled = false
            return
        }

        var orig = localValues[modPreview.selectedPointIndex * 2]
        var mod = localValues[(modPreview.selectedPointIndex * 2) + 1]

        updatingSpins = true
        origValueSpin.value = orig
        modValueSpin.value = mod
        updatingSpins = false

        modValueSpin.enabled = true
        origValueSpin.enabled = (orig !== 0 && orig !== 255)
        deleteHandlerButton.enabled = (orig !== 0 && orig !== 255)
    }

    function clampPoint(index, xPos, yPos)
    {
        var minOrig = 0
        var maxOrig = 255

        if (index === 0)
        {
            minOrig = 0
            maxOrig = 0
        }
        else if (index === handlerCount() - 1)
        {
            minOrig = 255
            maxOrig = 255
        }
        else
        {
            minOrig = localValues[(index - 1) * 2]
            maxOrig = localValues[(index + 1) * 2]
        }

        var minX = minOrig * modPreview.dX
        var maxX = maxOrig * modPreview.dX
        var newX = Math.max(minX, Math.min(maxX, xPos))
        var newY = Math.max(0, Math.min(modPreview.height, yPos))

        return { "x": newX, "y": newY }
    }

    function setPointValues(index, orig, mod)
    {
        var values = localValues.slice(0)
        var minOrig = 0
        var maxOrig = 255

        if (index === 0)
        {
            minOrig = 0
            maxOrig = 0
        }
        else if (index === handlerCount() - 1)
        {
            minOrig = 255
            maxOrig = 255
        }
        else
        {
            minOrig = values[(index - 1) * 2]
            maxOrig = values[(index + 1) * 2]
        }

        var clampedOrig = Math.max(minOrig, Math.min(maxOrig, orig))
        var clampedMod = Math.max(0, Math.min(255, mod))

        values[index * 2] = clampedOrig
        values[(index * 2) + 1] = clampedMod
        localValues = values
        updateSpinControls()
        modPreview.requestPaint()
    }

    function updatePointFromMouse(index, mouseX, mouseY)
    {
        var pointItem = pointRepeater.itemAt(index)
        if (!pointItem)
            return

        var pos = modPreview.mapFromItem(pointItem, mouseX, mouseY)
        var clamped = clampPoint(index, pos.x, pos.y)
        var newOrig = Math.round(clamped.x / modPreview.dX)
        var newMod = Math.round((modPreview.height - clamped.y) / modPreview.dY)
        setPointValues(index, newOrig, newMod)
    }

    function selectPoint(index)
    {
        modPreview.selectedPointIndex = index
        updateSpinControls()
    }

    function clearSelection()
    {
        modPreview.selectedPointIndex = -1
        updateSpinControls()
    }

    function addHandler()
    {
        var values = localValues.slice(0)
        if (values.length < 4)
            values = [0, 0, 255, 255]

        var count = Math.floor(values.length / 2)
        var prevIndex = modPreview.selectedPointIndex >= 0 ? modPreview.selectedPointIndex : 0
        if (prevIndex >= count - 1)
            prevIndex = count - 2

        var nextIndex = prevIndex + 1
        var prevOrig = values[prevIndex * 2]
        var prevMod = values[(prevIndex * 2) + 1]
        var nextOrig = values[nextIndex * 2]
        var nextMod = values[(nextIndex * 2) + 1]

        var newOrig = prevOrig + Math.floor((nextOrig - prevOrig) / 2)
        var newMod = prevMod + Math.floor((nextMod - prevMod) / 2)

        values.splice(nextIndex * 2, 0, newOrig, newMod)
        localValues = values
        selectPoint(nextIndex)
    }

    function removeHandler()
    {
        var index = modPreview.selectedPointIndex
        if (index <= 0 || index >= handlerCount() - 1)
            return

        var values = localValues.slice(0)
        values.splice(index * 2, 2)
        localValues = values
        clearSelection()
    }

    function saveTemplate()
    {
        var trimmedName = templateNameEdit.text.trim()
        if (trimmedName.length === 0)
        {
            messagePopup.message = qsTr("The modifier name cannot be empty.")
            messagePopup.standardButtons = Dialog.Ok
            messagePopup.open()
            return
        }
        if (trimmedName.toLowerCase() === "none")
        {
            messagePopup.message = qsTr("Please choose a name other than \"None\".")
            messagePopup.standardButtons = Dialog.Ok
            messagePopup.open()
            return
        }

        if (fixtureManager.isSystemChannelModifier(trimmedName))
        {
            messagePopup.message = qsTr("You are trying to overwrite a system template! Please choose another name "
                                        + "and the template will be saved in your channel modifier's user folder.")
            messagePopup.standardButtons = Dialog.Ok
            messagePopup.open()
            return
        }

        if (!fixtureManager.saveChannelModifier(trimmedName, localValues))
        {
            messagePopup.message = qsTr("Unable to save the modifier template.")
            messagePopup.standardButtons = Dialog.Ok
            messagePopup.open()
            return
        }

        fixtureManager.selectChannelModifier(trimmedName)
        modName = trimmedName
    }

    onModValuesChanged: syncLocalValues()
    onOpened: syncLocalValues()
    onModNameChanged:
    {
        if (modName.length)
            templateNameEdit.text = modName
        else
            templateNameEdit.text = qsTr("New Template")
    }
    onLocalValuesChanged: modPreview.requestPaint()

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
                    id: addHandlerButton
                    width: UISettings.iconSizeMedium
                    height: width
                    faSource: FontAwesome.fa_plus
                    faColor: "limegreen"
                    tooltip: qsTr("Insert a modified value after the selected")
                    onClicked: popupRoot.addHandler()
                }

                IconButton
                {
                    id: deleteHandlerButton
                    width: UISettings.iconSizeMedium
                    height: width
                    faSource: FontAwesome.fa_minus
                    faColor: "crimson"
                    tooltip: qsTr("Delete the selected modifier value")
                    onClicked: popupRoot.removeHandler()
                }
                Rectangle
                {
                    Layout.fillWidth: true
                    height: UISettings.iconSizeMedium
                    color: "transparent"
                }
                IconButton
                {
                    id: renameTemplateButton
                    width: UISettings.iconSizeMedium
                    height: width
                    imgSource: "qrc:/rename.svg"
                    tooltip: qsTr("Rename the selected modifier template")
                    onClicked:
                    {
                        renamePopup.editText = templateNameEdit.text
                        renamePopup.open()
                    }
                }
                IconButton
                {
                    id: saveTemplateButton
                    width: UISettings.iconSizeMedium
                    height: width
                    imgSource: "qrc:/filesave.svg"
                    tooltip: qsTr("Save the selected modifier template")
                    onClicked: popupRoot.saveTemplate()
                }
            }

            RowLayout
            {
                Layout.columnSpan: 3
                spacing: 5

                RobotoText
                {
                    label: qsTr("Name")
                }
                CustomTextEdit
                {
                    id: templateNameEdit
                    Layout.fillWidth: true
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

                onAvailableChanged: if (available) requestPaint()
                onWidthChanged: if (available) requestPaint()
                onHeightChanged: if (available) requestPaint()

                onPaint:
                {
                    var ctx = getContext("2d")
                    ctx.globalAlpha = 1.0
                    ctx.fillStyle = UISettings.bgStronger
                    ctx.strokeStyle = "yellow"
                    ctx.lineWidth = 1

                    var x1, y1, x2, y2

                    ctx.fillRect(0, 0, width, height)

                    if (localValues.length)
                    {
                        context.beginPath()

                        for (var i = 0; i < localValues.length - 2; i+= 2)
                        {
                            x1 = localValues[i]
                            y1 = localValues[i + 1]
                            x2 = localValues[i + 2]
                            y2 = localValues[i + 3]
                            ctx.moveTo(x1 * dX, height - (y1 * dY))
                            ctx.lineTo(x2 * dX, height - (y2 * dY))
                        }

                        ctx.stroke()
                        ctx.closePath()
                    }
                }

                Repeater
                {
                    id: pointRepeater
                    model: localValues.length / 2

                    Rectangle
                    {
                        property int origDMX: localValues[index * 2]
                        property int modDMX: localValues[(index * 2) + 1]

                        width: 14
                        height: 14
                        x: (origDMX * modPreview.dX) - (width / 2)
                        y: modPreview.height - (modDMX * modPreview.dY) - (height / 2)
                        radius: width / 2
                        color: modPreview.selectedPointIndex === index ? UISettings.highlight : "yellow"

                        MouseArea
                        {
                            anchors.fill: parent
                            drag.threshold: 0
                            onClicked:
                            {
                                popupRoot.selectPoint(index)
                            }
                            onPressed: popupRoot.selectPoint(index)
                            onPositionChanged: (mouse) =>
                            {
                                if (pressed)
                                    popupRoot.updatePointFromMouse(index, mouse.x, mouse.y)
                            }
                        }
                    }
                }

                MouseArea
                {
                    anchors.fill: parent
                    z: -1
                    onClicked: popupRoot.clearSelection()
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
                        width: modList.width - UISettings.iconSizeMedium
                        color: modelData === popupRoot.modName ? UISettings.highlight : "transparent"

                        RobotoText
                        {
                            height: parent.height
                            label: modelData
                        }
                        // user channel modifier
                        Text
                        {
                            visible: modelData !== "None" && !fixtureManager.isSystemChannelModifier(modelData)
                            anchors.right: parent.right
                            anchors.rightMargin: 5
                            anchors.verticalCenter: parent.verticalCenter
                            color: UISettings.fgMain
                            font.family: UISettings.fontAwesomeFontName
                            font.pixelSize: parent.height - 5
                            text: FontAwesome.fa_user
                        }
                        MouseArea
                        {
                            anchors.fill: parent
                            onClicked:
                            {
                                popupRoot.clearSelection()
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
                onValueChanged:
                {
                    if (popupRoot.updatingSpins || modPreview.selectedPointIndex < 0)
                        return
                    popupRoot.setPointValues(modPreview.selectedPointIndex, value, modValueSpin.value)
                }
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
                onValueChanged:
                {
                    if (popupRoot.updatingSpins || modPreview.selectedPointIndex < 0)
                        return
                    popupRoot.setPointValues(modPreview.selectedPointIndex, origValueSpin.value, value)
                }
            }
        }

    CustomPopupDialog
    {
        id: messagePopup
        title: qsTr("Error")
        standardButtons: Dialog.Ok
    }

    PopupRenameItems
    {
        id: renamePopup
        title: qsTr("Rename modifier template")
        onAccepted: templateNameEdit.text = editText
    }
}
