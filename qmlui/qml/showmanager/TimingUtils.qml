/*
  Q Light Controller Plus
  TimingUtils.qml

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
import "TimeUtils.js" as TimeUtils
import "."

Rectangle
{
    id: panelContainer
    anchors.fill: parent
    color: "transparent"
    clip: true

    readonly property int fieldNone: 0
    readonly property int fieldStart: 1
    readonly property int fieldEnd: 2
    readonly property int fieldDuration: 3
    readonly property int fieldLength: 4

    property int activeField: fieldNone
    property var activeTarget: null
    property var overlayHost: null
    property var selectedItems: []
    property var selectedItem: selectedItems.length === 1 ? selectedItems[0] : null
    property int cutInsertLength: 1000
    property int lastTimingSpinValue: 0
    property bool hasSelection: selectedItems.length > 0
    property bool hasSingleSelection: selectedItems.length === 1
    property bool hasMultipleSelection: selectedItems.length > 1
    property int tempoType: showManager.timeDivision === Show.Time ? QLCFunction.Time : QLCFunction.Beats

    function refreshSelection()
    {
        selectedItems = showManager.selectedItemRefs()

        if (selectedItems.length === 0 && activeField !== fieldLength)
            activeField = fieldNone
    }

    function cursorValue()
    {
        if (showManager.timeDivision === Show.Time)
            return showManager.currentTime

        var bpm = ioManager ? ioManager.bpmNumber : 0
        if (bpm <= 0)
            return 0
        var beatDuration = 60000 / bpm
        return Math.round((showManager.currentTime / beatDuration) * 1000)
    }

    function minDurationValue()
    {
        return showManager.timeDivision === Show.Time ? 1 : 125
    }

    function timeLabelForValue(value)
    {
        if (!hasSelection)
            return qsTr("--")
        if (hasMultipleSelection)
            return qsTr("Multiple")
        return TimeUtils.timeToQlcString(value, tempoType)
    }

    function cutInsertLengthLabel()
    {
        return TimeUtils.timeToQlcString(cutInsertLength, tempoType)
    }

    function isTimingField(fieldId)
    {
        return fieldId === fieldStart || fieldId === fieldEnd || fieldId === fieldDuration
    }

    function isOverlayField(fieldId)
    {
        return isTimingField(fieldId) || fieldId === fieldLength
    }

    function activeFieldValue()
    {
        if (!selectedItem)
            return 0
        if (activeField === fieldStart)
            return selectedItem.startTime
        if (activeField === fieldEnd)
            return selectedItem.startTime + selectedItem.duration
        if (activeField === fieldDuration)
            return selectedItem.duration
        return 0
    }

    function syncOverlaySpinValues()
    {
        if (!isOverlayField(activeField))
            return

        var value = activeField === fieldLength ? cutInsertLength : activeFieldValue()
        if (value < 0)
            value = 0
        if (hasMultipleSelection && isTimingField(activeField))
            value = 0

        lastTimingSpinValue = value

        overlayHoursSpin.value = Math.floor(value / 3600000)
        value -= overlayHoursSpin.value * 3600000
        overlayMinutesSpin.value = Math.floor(value / 60000)
        value -= overlayMinutesSpin.value * 60000
        overlaySecondsSpin.value = Math.floor(value / 1000)
        value -= overlaySecondsSpin.value * 1000
        overlayMillisSpin.value = value
    }

    function overlayTotalValue()
    {
        return (overlayHoursSpin.value * 3600000)
                + (overlayMinutesSpin.value * 60000)
                + (overlaySecondsSpin.value * 1000)
                + overlayMillisSpin.value
    }

    function applyOverlaySpinValue()
    {
        var value = overlayTotalValue()

        if (activeField === fieldLength)
        {
            setCutInsertLength(value)
            return
        }

        if (!isTimingField(activeField))
            return

        if (hasMultipleSelection)
        {
            var delta = value - lastTimingSpinValue
            if (!delta)
                return
            lastTimingSpinValue = value
            applyRelativeValue(activeField, delta)
        }
        else
        {
            applyAbsoluteValue(activeField, value)
        }
    }

    function updateOverlayGeometry()
    {
        if (!overlayHost || !activeTarget)
            return

        var pos = activeTarget.mapToItem(overlayHost, 0, 0)
        timeEditOverlay.x = pos.x
        timeEditOverlay.y = pos.y
        timeEditOverlay.width = activeTarget.width
        timeEditOverlay.height = activeTarget.height
    }

    function attachOverlayToField()
    {
        var host = null
        if (isOverlayField(activeField) && activeTarget && activeTarget.parent)
            host = activeTarget.parent.parent ? activeTarget.parent.parent : activeTarget.parent
        overlayHost = host

        if (!host)
        {
            if (timeEditOverlay.parent !== panelContainer)
                timeEditOverlay.parent = panelContainer
            return
        }

        if (timeEditOverlay.parent !== host)
            timeEditOverlay.parent = host
        updateOverlayGeometry()
        syncOverlaySpinValues()
    }

    function toggleField(fieldId, target)
    {
        if (!hasSelection && isTimingField(fieldId))
            return
        if (activeField === fieldId)
        {
            activeTarget = null
            activeField = fieldNone
        }
        else
        {
            activeTarget = target
            activeField = fieldId
        }
    }

    function alignStartToCursor()
    {
        var target = cursorValue()
        for (var i = 0; i < selectedItems.length; i++)
        {
            var sf = selectedItems[i]
            if (!sf || sf.locked)
                continue
            showManager.setShowItemStartTime(sf, target)
        }
    }

    function alignEndToCursor()
    {
        var target = cursorValue()
        for (var i = 0; i < selectedItems.length; i++)
        {
            var sf = selectedItems[i]
            if (!sf || sf.locked)
                continue
            var newDuration = target - sf.startTime
            var minDuration = minDurationValue()
            if (newDuration < minDuration)
                newDuration = minDuration
            showManager.setShowItemDuration(sf, newDuration)
        }
    }

    function applyAbsoluteValue(fieldId, value)
    {
        if (!hasSelection)
            return

        if (hasMultipleSelection)
        {
            applyRelativeValue(fieldId, value)
            return
        }

        for (var i = 0; i < selectedItems.length; i++)
        {
            var sf = selectedItems[i]
            if (!sf || sf.locked)
                continue

            if (fieldId === fieldStart)
            {
                var newStart = value
                if (newStart < 0)
                    newStart = 0
                showManager.setShowItemStartTime(sf, newStart)
            }
            else if (fieldId === fieldEnd)
            {
                var newDuration = value - sf.startTime
                var minDuration = minDurationValue()
                if (newDuration < minDuration)
                    newDuration = minDuration
                showManager.setShowItemDuration(sf, newDuration)
            }
            else if (fieldId === fieldDuration)
            {
                var newDur = value
                var minDur = minDurationValue()
                if (newDur < minDur)
                    newDur = minDur
                showManager.setShowItemDuration(sf, newDur)
            }
        }
    }

    function applyRelativeValue(fieldId, delta)
    {
        if (!hasSelection)
            return

        for (var i = 0; i < selectedItems.length; i++)
        {
            var sf = selectedItems[i]
            if (!sf || sf.locked)
                continue

            if (fieldId === fieldStart)
            {
                var newStart = sf.startTime + delta
                if (newStart < 0)
                    newStart = 0
                showManager.setShowItemStartTime(sf, newStart)
            }
            else if (fieldId === fieldEnd)
            {
                var newDuration = sf.duration + delta
                var minDuration = minDurationValue()
                if (newDuration < minDuration)
                    newDuration = minDuration
                showManager.setShowItemDuration(sf, newDuration)
            }
            else if (fieldId === fieldDuration)
            {
                var newDur = sf.duration + delta
                var minDur = minDurationValue()
                if (newDur < minDur)
                    newDur = minDur
                showManager.setShowItemDuration(sf, newDur)
            }
        }
    }

    function insertTime()
    {
        var length = cutInsertLength
        var minValue = minDurationValue()
        if (length < minValue)
            length = minValue
        showManager.insertTimeAtCursor(length, cursorValue())
    }

    function cutTime()
    {
        var length = cutInsertLength
        var minValue = minDurationValue()
        if (length < minValue)
            length = minValue
        showManager.cutTimeAtCursor(length, cursorValue())
    }

    function setCutInsertLength(value)
    {
        var minValue = minDurationValue()
        if (value < minValue)
            value = minValue

        cutInsertLength = value
    }

    Component.onCompleted:
    {
        refreshSelection()
        setCutInsertLength(cutInsertLength)
    }

    Connections
    {
        target: showManager

        function onSelectedItemsCountChanged(count)
        {
            refreshSelection()
        }

        function onItemClicked(itemType)
        {
            refreshSelection()
        }

        function onTimeDivisionChanged(division)
        {
            panelContainer.setCutInsertLength(panelContainer.cutInsertLength)
        }
    }

    Connections
    {
        target: panelContainer
        ignoreUnknownSignals: true

        function onActiveFieldChanged()
        {
            panelContainer.attachOverlayToField()
        }

        function onActiveTargetChanged()
        {
            panelContainer.attachOverlayToField()
        }
    }

    Connections
    {
        target: panelContainer.activeTarget
        ignoreUnknownSignals: true

        function onXChanged() { panelContainer.updateOverlayGeometry() }
        function onYChanged() { panelContainer.updateOverlayGeometry() }
        function onWidthChanged() { panelContainer.updateOverlayGeometry() }
        function onHeightChanged() { panelContainer.updateOverlayGeometry() }
    }

    Column
    {
        width: parent.width
        anchors.margins: Math.round(UISettings.iconSizeDefault * 0.3)
        spacing: Math.round(UISettings.iconSizeDefault * 0.2)

        SectionBox
        {
            width: parent.width
            sectionLabel: qsTr("Alignment")
            sectionContents:
                Column
                {
                    width: panelContainer.width

                    GenericButton
                    {
                        width: parent.width
                        enabled: hasSelection
                        label: qsTr("Align start to cursor")
                        onClicked: alignStartToCursor()
                    }

                    GenericButton
                    {
                        width: parent.width
                        enabled: hasSelection
                        label: qsTr("Align end to cursor")
                        onClicked: alignEndToCursor()
                    }
                }
        }

/*
        RobotoText
        {
            Layout.fillWidth: true
            fontSize: UISettings.textSizeDefault * 0.8
            label: hasSelection ? qsTr("%1 item(s) selected").arg(selectedItems.length) : qsTr("No items selected")
            labelColor: hasSelection ? UISettings.fgMain : UISettings.bgLight
        }

        Rectangle
        {
            Layout.fillWidth: true
            height: 1
            color: UISettings.bgLight
        }
*/
        SectionBox
        {
            width: parent.width
            sectionLabel: qsTr("Timings")
            sectionContents:
                Item
                {
                    id: timingArea
                    width: panelContainer.width
                    height: timingsGrid.implicitHeight
                    onWidthChanged: if (panelContainer.overlayHost === timingArea) panelContainer.updateOverlayGeometry()
                    onHeightChanged: if (panelContainer.overlayHost === timingArea) panelContainer.updateOverlayGeometry()

                    GridLayout
                    {
                        id: timingsGrid
                        width: panelContainer.width
                        columns: 2
                        columnSpacing: 5
                        rowSpacing: 5

                        RobotoText
                        {
                            label: qsTr("Start time")
                        }

                        GenericButton
                        {
                            id: startTimeButton
                            Layout.fillWidth: true
                            opacity: activeField === fieldStart ? 0 : 1
                            enabled: hasSelection && activeField !== fieldStart
                            bgColor: UISettings.bgMedium
                            hoverColor: UISettings.fgLight
                            label: timeLabelForValue(selectedItem ? selectedItem.startTime : 0)
                            onClicked: toggleField(fieldStart, startTimeButton)
                        }

                        RobotoText
                        {
                            label: qsTr("End time")
                        }

                        GenericButton
                        {
                            id: endTimeButton
                            Layout.fillWidth: true
                            opacity: activeField === fieldEnd ? 0 : 1
                            enabled: hasSelection && activeField !== fieldEnd
                            bgColor: UISettings.bgMedium
                            hoverColor: UISettings.fgLight
                            label: timeLabelForValue(selectedItem ? (selectedItem.startTime + selectedItem.duration) : 0)
                            onClicked: toggleField(fieldEnd, endTimeButton)
                        }

                        RobotoText
                        {
                            label: qsTr("Duration")
                        }

                        GenericButton
                        {
                            id: durationButton
                            Layout.fillWidth: true
                            opacity: activeField === fieldDuration ? 0 : 1
                            enabled: hasSelection && activeField !== fieldDuration
                            bgColor: UISettings.bgMedium
                            hoverColor: UISettings.fgLight
                            label: timeLabelForValue(selectedItem ? selectedItem.duration : 0)
                            onClicked: toggleField(fieldDuration, durationButton)
                        }
                    } // GridLayout

                } // timingArea
        } // SectionBox

        SectionBox
        {
            width: parent.width
            sectionLabel: qsTr("Cut/Insert")
            sectionContents:
                Item
                {
                    id: cutInsertArea
                    width: panelContainer.width
                    height: cutInsertGrid.implicitHeight
                            + 5 + insertButton.height
                            + 5 + cutButton.height
                    onWidthChanged: if (panelContainer.overlayHost === cutInsertArea) panelContainer.updateOverlayGeometry()
                    onHeightChanged: if (panelContainer.overlayHost === cutInsertArea) panelContainer.updateOverlayGeometry()

                    GridLayout
                    {
                        id: cutInsertGrid
                        width: panelContainer.width
                        columns: 2
                        columnSpacing: 5
                        rowSpacing: 5

                        RobotoText
                        {
                            label: qsTr("Length")
                        }

                        GenericButton
                        {
                            id: lengthButton
                            Layout.fillWidth: true
                            opacity: activeField === fieldLength ? 0 : 1
                            enabled: activeField !== fieldLength
                            bgColor: UISettings.bgMedium
                            hoverColor: UISettings.fgLight
                            label: cutInsertLengthLabel()
                            onClicked: toggleField(fieldLength, lengthButton)
                        }
                    }

                    GenericButton
                    {
                        id: insertButton
                        anchors.top: cutInsertGrid.bottom
                        anchors.topMargin: 5
                        width: parent.width
                        height: UISettings.iconSizeDefault
                        enabled: showManager.isEditing
                        label: qsTr("Insert time")
                        onClicked: insertTime()
                    }

                    GenericButton
                    {
                        id: cutButton
                        anchors.top: insertButton.bottom
                        anchors.topMargin: 5
                        width: parent.width
                        height: UISettings.iconSizeDefault
                        enabled: showManager.isEditing
                        label: qsTr("Cut time")
                        onClicked: cutTime()
                    }

                }
        } // SectionBox
    } // Column

    Item
    {
        id: timeEditOverlay
        visible: panelContainer.isOverlayField(activeField) && activeTarget && panelContainer.overlayHost
        z: 2

        RowLayout
        {
            anchors.fill: parent
            spacing: 3

            CustomSpinBox
            {
                id: overlayHoursSpin
                Layout.fillWidth: true
                Layout.preferredHeight: timeEditOverlay.height
                from: (panelContainer.isTimingField(activeField) && hasMultipleSelection) ? -999 : 0
                to: 999
                suffix: "h"
                onValueModified: panelContainer.applyOverlaySpinValue()
            }

            CustomSpinBox
            {
                id: overlayMinutesSpin
                Layout.fillWidth: true
                Layout.preferredHeight: timeEditOverlay.height
                from: (panelContainer.isTimingField(activeField) && hasMultipleSelection) ? -59 : 0
                to: 59
                suffix: "m"
                onValueModified: panelContainer.applyOverlaySpinValue()
            }

            CustomSpinBox
            {
                id: overlaySecondsSpin
                Layout.fillWidth: true
                Layout.preferredHeight: timeEditOverlay.height
                from: (panelContainer.isTimingField(activeField) && hasMultipleSelection) ? -59 : 0
                to: 59
                suffix: "s"
                onValueModified: panelContainer.applyOverlaySpinValue()
            }

            CustomSpinBox
            {
                id: overlayMillisSpin
                Layout.fillWidth: true
                Layout.preferredHeight: timeEditOverlay.height
                from: (panelContainer.isTimingField(activeField) && hasMultipleSelection) ? -999 : 0
                to: 999
                suffix: "ms"
                onValueModified: panelContainer.applyOverlaySpinValue()
            }
        }
    }
}
