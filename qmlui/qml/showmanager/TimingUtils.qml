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

    property int activeField: fieldNone
    property var activeTarget: null
    property var selectedItems: []
    property var selectedItem: selectedItems.length === 1 ? selectedItems[0] : null
    property bool hasSelection: selectedItems.length > 0
    property bool hasSingleSelection: selectedItems.length === 1
    property bool hasMultipleSelection: selectedItems.length > 1
    property int tempoType: showManager.timeDivision === Show.Time ? QLCFunction.Time : QLCFunction.Beats

    function refreshSelection()
    {
        selectedItems = showManager.selectedItemRefs()

        if (selectedItems.length === 0)
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

    function toggleField(fieldId, target)
    {
        if (!hasSelection)
            return
        if (activeField === fieldId)
        {
            activeField = fieldNone
            activeTarget = null
        }
        else
        {
            activeField = fieldId
            activeTarget = target
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

    Component.onCompleted: refreshSelection()

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
                    property var activeTarget: panelContainer.activeTarget
                    property real targetX: 0
                    property real targetY: 0
                    property real targetWidth: 0
                    property real targetHeight: 0

                    function updateOverlay(target)
                    {
                        if (!target)
                            return
                        var pos = target.mapToItem(timingArea, 0, 0)
                        targetX = pos.x
                        targetY = pos.y
                        targetWidth = target.width
                        targetHeight = target.height
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

                    function updateSpinValues()
                    {
                        var value = activeFieldValue()
                        if (value < 0)
                            value = 0

                        hoursSpin.value = Math.floor(value / 3600000)
                        value -= hoursSpin.value * 3600000
                        minutesSpin.value = Math.floor(value / 60000)
                        value -= minutesSpin.value * 60000
                        secondsSpin.value = Math.floor(value / 1000)
                        value -= secondsSpin.value * 1000
                        millisSpin.value = value
                    }

                    function applySpinValue(value)
                    {
                        panelContainer.applyAbsoluteValue(activeField, value)
                    }

                    onActiveTargetChanged: updateOverlay(activeTarget)

                    Connections
                    {
                        target: panelContainer
                        ignoreUnknownSignals: true

                        function onActiveFieldChanged()
                        {
                            timingArea.updateOverlay(timingArea.activeTarget)
                            if (panelContainer.activeField !== fieldNone)
                                timingArea.updateSpinValues()
                        }
                    }

                    Connections
                    {
                        target: timingArea.activeTarget
                        ignoreUnknownSignals: true

                        function onXChanged() { timingArea.updateOverlay(timingArea.activeTarget) }
                        function onYChanged() { timingArea.updateOverlay(timingArea.activeTarget) }
                        function onWidthChanged() { timingArea.updateOverlay(timingArea.activeTarget) }
                        function onHeightChanged() { timingArea.updateOverlay(timingArea.activeTarget) }
                    }

                    onWidthChanged:
                    {
                        if (activeTarget)
                            updateOverlay(activeTarget)
                    }
                    onHeightChanged:
                    {
                        if (activeTarget)
                            updateOverlay(activeTarget)
                    }

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

                    Item
                    {
                        id: overlayContainer
                        visible: activeField !== fieldNone && activeTarget
                        z: 2
                        x: timingArea.targetX
                        y: timingArea.targetY
                        width: timingArea.targetWidth
                        height: timingArea.targetHeight

                        RowLayout
                        {
                            id: overlayContainerRow
                            anchors.fill: parent
                            spacing: 3

                            function totalValue()
                            {
                                return (hoursSpin.value * 3600000)
                                        + (minutesSpin.value * 60000)
                                        + (secondsSpin.value * 1000)
                                        + millisSpin.value
                            }

                            CustomSpinBox
                            {
                                id: hoursSpin
                                Layout.fillWidth: true
                                Layout.preferredHeight: timingArea.targetHeight
                                from: 0
                                to: 999
                                suffix: "h"
                                onValueModified: timingArea.applySpinValue(overlayContainerRow.totalValue())
                            }

                            CustomSpinBox
                            {
                                id: minutesSpin
                                Layout.fillWidth: true
                                Layout.preferredHeight: timingArea.targetHeight
                                from: 0
                                to: 59
                                suffix: "m"
                                onValueModified: timingArea.applySpinValue(overlayContainerRow.totalValue())
                            }

                            CustomSpinBox
                            {
                                id: secondsSpin
                                Layout.fillWidth: true
                                Layout.preferredHeight: timingArea.targetHeight
                                from: 0
                                to: 59
                                suffix: "s"
                                onValueModified: timingArea.applySpinValue(overlayContainerRow.totalValue())
                            }

                            CustomSpinBox
                            {
                                id: millisSpin
                                Layout.fillWidth: true
                                Layout.preferredHeight: timingArea.targetHeight
                                from: 0
                                to: 999
                                suffix: "ms"
                                onValueModified: timingArea.applySpinValue(overlayContainerRow.totalValue())
                            }
                        }
                    }
                } // timingArea
        } // SectionBox
    } // Column
}
