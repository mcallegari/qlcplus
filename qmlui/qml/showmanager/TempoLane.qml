/*
  Q Light Controller Plus
  TempoLane.qml

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

import "TimeUtils.js" as TimeUtils
import "."

/** The strip below the Show Manager ruler holding the Show tempo sections.
  * Its coordinates are the timeline coordinates, like the Show items. */
Rectangle
{
    id: laneRoot
    color: UISettings.bgStronger

    property real timeScale: showManager.timeScale
    property real tickSize: showManager.tickSize
    property real visibleX: 0
    property real visibleWidth: 0
    property var sections: showManager.tempoSections

    // the section selected by tapping it, which the lane buttons act on
    property int selectedIndex: -1
    property bool cursorInSelection: selectedIndex >= 0 && selectedIndex < sections.length &&
                                     showManager.currentTime > sections[selectedIndex].startTime &&
                                     showManager.currentTime < sections[selectedIndex].startTime +
                                                               sections[selectedIndex].duration

    onSectionsChanged:
    {
        if (selectedIndex >= sections.length)
            selectedIndex = -1
    }

    readonly property real snapThreshold: 15
    readonly property real edgeWidth: 6

    function timeToX(time)
    {
        return TimeUtils.timeToSize(time, timeScale, tickSize)
    }

    function xToTime(xPos)
    {
        return Math.max(0, TimeUtils.posToMs(xPos, timeScale, tickSize))
    }

    /* The X positions a section edge can snap to: the Show items edges and
       the other sections edges, when snapping to items is enabled */
    function snapEdgesFor(index)
    {
        if (!showManager.snapToItems)
            return []

        var own = sections[index]
        var ownStart = timeToX(own.startTime)
        var ownEnd = timeToX(own.startTime + own.duration)
        // 4294967295 is Function::invalidId(): no Show item is left out
        var edges = showManager.getSnapEdges(4294967295, visibleX, visibleX + visibleWidth)
        var result = []
        for (var i = 0; i < edges.length; i++)
        {
            if (Math.abs(edges[i] - ownStart) < 0.5 || Math.abs(edges[i] - ownEnd) < 0.5)
                continue
            result.push(edges[i])
        }
        return result
    }

    function snapX(xPos, edges, modifiers)
    {
        if (modifiers & Qt.ControlModifier)
            return { x: xPos, snapped: false }

        var best = -1
        var bestDist = snapThreshold + 1
        for (var i = 0; i < edges.length; i++)
        {
            var dist = Math.abs(edges[i] - xPos)
            if (dist < bestDist)
            {
                bestDist = dist
                best = edges[i]
            }
        }
        if (best >= 0 && bestDist <= snapThreshold)
            return { x: best, snapped: true }
        return { x: xPos, snapped: false }
    }

    // the time range a section can take without overlapping its neighbours
    function minStartFor(index)
    {
        return index > 0 ? sections[index - 1].startTime + sections[index - 1].duration : 0
    }

    function maxEndFor(index)
    {
        return index < sections.length - 1 ? sections[index + 1].startTime : -1
    }

    function addSectionAt(time)
    {
        var index = showManager.addTempoSection(time)
        if (index < 0)
        {
            messagePopup.message = qsTr("A tempo section can't start inside another one.\n" +
                                        "Move the cursor outside the existing sections.")
            messagePopup.open()
            return
        }
        // the tempo of a new section always needs setting
        selectedIndex = index
        openEditor(index)
    }

    function addSectionsFromSelection()
    {
        var info = showManager.tempoSelectionInfo()
        if (info.audio === 0)
        {
            messagePopup.message = qsTr("Select one or more audio items to add tempo sections for.")
            messagePopup.open()
            return
        }
        if (info.overlapping > 0)
        {
            overlapPopup.audioCount = info.audio
            overlapPopup.overlapCount = info.overlapping
            overlapPopup.open()
            return
        }
        finishAddFromSelection(false)
    }

    function finishAddFromSelection(startPrecedence)
    {
        var indices = showManager.addTempoSectionsFromSelection(startPrecedence)
        if (indices.length === 1)
        {
            selectedIndex = indices[0]
            openEditor(indices[0])
        }
        else if (indices.length === 0)
        {
            messagePopup.message = startPrecedence ?
                        qsTr("No tempo section was added.\n" +
                             "A tempo section already starts where each selected audio item starts.") :
                        qsTr("No tempo section was added.\n" +
                             "Every selected audio item overlaps a tempo section.")
            messagePopup.open()
        }
    }

    function openEditor(index)
    {
        var section = sections[index]
        sectionEditor.sectionIndex = index
        sectionEditor.nameText = section.name
        bpmSpin.setValue(Math.round(section.bpm * 100))
        beatsPerBarSpin.value = section.beatsPerBar
        sectionEditor.open()
    }

    /** Beat grid ticks and bar numbers, drawn with the same chunk strategy
      * as the timeline header: the Canvas is 3 times the visible area and
      * gets shifted and repainted while flicking horizontally */
    Canvas
    {
        id: tickCanvas
        x: 0
        width: Math.max(1, visibleWidth * 3)
        height: laneRoot.height
        z: 2
        antialiasing: true
        contextType: "2d"

        function updatePosition()
        {
            if (visibleWidth <= 0)
                return

            var chunk = parseInt(visibleX / visibleWidth) * visibleWidth
            x = Math.max(0, chunk - visibleWidth)
            requestPaint()
        }

        Connections
        {
            target: laneRoot

            function onVisibleXChanged()
            {
                if (laneRoot.visibleX < tickCanvas.x || laneRoot.visibleX + laneRoot.visibleWidth > tickCanvas.x + tickCanvas.width)
                    tickCanvas.updatePosition()
            }
            function onVisibleWidthChanged() { tickCanvas.updatePosition() }
            function onTickSizeChanged() { tickCanvas.requestPaint() }
            function onTimeScaleChanged() { tickCanvas.requestPaint() }
            function onSectionsChanged() { tickCanvas.requestPaint() }
        }

        onPaint:
        {
            if (context === null || context === undefined)
                return

            context.reset()
            context.clearRect(0, 0, width, height)

            var lines = showManager.tempoGridLines(x, x + width)
            var fontSize = height * 0.4

            context.lineWidth = 1
            context.strokeStyle = UISettings.fgMedium
            context.fillStyle = UISettings.fgMain
            context.font = fontSize + "px \"" + UISettings.robotoFontName + "\""

            for (var w = 0; w <= 2; w++)
            {
                var top = w === 2 ? height * 0.45 : (w === 1 ? height * 0.7 : height * 0.85)
                context.globalAlpha = w === 0 ? 0.5 : 1.0
                context.beginPath()
                for (var i = 0; i < lines.length; i += 3)
                {
                    if (lines[i + 1] !== w)
                        continue
                    var lx = Math.round(lines[i] - x) + 0.5
                    context.moveTo(lx, top)
                    context.lineTo(lx, height)
                }
                context.stroke()
            }

            // bar numbers, when there is room for them
            context.globalAlpha = 1.0
            var lastTextEnd = -1
            for (var j = 0; j < lines.length; j += 3)
            {
                if (lines[j + 1] !== 2)
                    continue
                var tx = lines[j] - x + 3
                if (tx < lastTextEnd)
                    continue
                var label = "" + lines[j + 2]
                context.fillText(label, tx, height - 3)
                lastTextEnd = tx + context.measureText(label).width + 6
            }
        }
    }

    /* Background: a tap moves the cursor and clears the section selection */
    MouseArea
    {
        anchors.fill: parent
        z: 0

        onClicked: (mouse) =>
        {
            laneRoot.selectedIndex = -1
            showManager.currentTime = laneRoot.xToTime(mouse.x)
        }
    }

    Repeater
    {
        model: laneRoot.sections

        delegate:
            Rectangle
            {
                id: sectionItem
                z: 1
                height: laneRoot.height
                color: Qt.rgba(0.35, 0.55, 0.85, sectionMa.containsMouse || dragging ? 0.55 : 0.4)
                border.width: isSelected ? 2 : 1
                border.color: isSelected ? UISettings.selection : "#6fa0e0"

                property bool isSelected: laneRoot.selectedIndex === index
                clip: true

                property var section: modelData
                property bool dragging: false
                // position and width while dragging, in pixels
                property real dragX: 0
                property real dragWidth: 0

                x: dragging ? dragX : laneRoot.timeToX(section.startTime)
                width: dragging ? dragWidth : Math.max(1, laneRoot.timeToX(section.duration))

                // keep the label in view when the section starts off screen
                RobotoText
                {
                    x: Math.max(4, laneRoot.visibleX - sectionItem.x + 4)
                    height: parent.height * 0.55
                    textVAlign: Text.AlignVCenter
                    fontSize: UISettings.textSizeDefault * 0.8
                    labelColor: UISettings.fgMain
                    label: section.name + " · " + (Math.round(section.bpm * 100) / 100) + " BPM · " + section.beatsPerBar + "/4"
                }

                /* body: drag to move, double click to edit, right click menu */
                MouseArea
                {
                    id: sectionMa
                    anchors.fill: parent
                    anchors.leftMargin: laneRoot.edgeWidth
                    anchors.rightMargin: laneRoot.edgeWidth
                    hoverEnabled: true
                    preventStealing: true
                    acceptedButtons: Qt.LeftButton | Qt.RightButton
                    cursorShape: pressed ? Qt.ClosedHandCursor : Qt.OpenHandCursor

                    property real pressX: 0
                    property var edges: []

                    onPressed: (mouse) =>
                    {
                        laneRoot.selectedIndex = index
                        if (mouse.button !== Qt.LeftButton)
                            return
                        pressX = mapToItem(laneRoot, mouse.x, 0).x
                        edges = laneRoot.snapEdgesFor(index)
                        sectionItem.dragX = sectionItem.x
                        sectionItem.dragWidth = sectionItem.width
                        sectionItem.dragging = true
                    }

                    onPositionChanged: (mouse) =>
                    {
                        if (!sectionItem.dragging)
                            return

                        var origX = laneRoot.timeToX(section.startTime)
                        var newX = origX + mapToItem(laneRoot, mouse.x, 0).x - pressX

                        // snap either edge, whichever is closer to a target
                        var s1 = laneRoot.snapX(newX, edges, mouse.modifiers)
                        var s2 = laneRoot.snapX(newX + sectionItem.dragWidth, edges, mouse.modifiers)
                        showManager.snapGuideX = -1
                        if (s1.snapped && (!s2.snapped || Math.abs(s1.x - newX) <= Math.abs(s2.x - newX - sectionItem.dragWidth)))
                        {
                            newX = s1.x
                            showManager.snapGuideX = s1.x
                        }
                        else if (s2.snapped)
                        {
                            newX = s2.x - sectionItem.dragWidth
                            showManager.snapGuideX = s2.x
                        }

                        // stop at the neighbours, which overrides any snapping
                        var snappedX = newX
                        var minX = laneRoot.timeToX(laneRoot.minStartFor(index))
                        var maxEnd = laneRoot.maxEndFor(index)
                        if (maxEnd >= 0)
                            newX = Math.min(newX, laneRoot.timeToX(maxEnd) - sectionItem.dragWidth)
                        newX = Math.max(minX, newX)
                        if (newX !== snappedX)
                            showManager.snapGuideX = -1
                        sectionItem.dragX = newX
                    }

                    onReleased: (mouse) =>
                    {
                        if (!sectionItem.dragging)
                            return

                        showManager.snapGuideX = -1
                        var newStart = laneRoot.xToTime(sectionItem.dragX)
                        sectionItem.dragging = false
                        if (newStart !== section.startTime)
                            showManager.updateTempoSection(index, newStart, section.duration,
                                                           section.bpm, section.beatsPerBar, section.name)
                    }

                    onCanceled:
                    {
                        showManager.snapGuideX = -1
                        sectionItem.dragging = false
                    }

                    // right click is a shortcut to the editor, like a double click
                    onClicked: (mouse) =>
                    {
                        if (mouse.button === Qt.RightButton)
                            laneRoot.openEditor(index)
                    }

                    onDoubleClicked: (mouse) =>
                    {
                        if (mouse.button === Qt.LeftButton)
                            laneRoot.openEditor(index)
                    }
                }

                /* left and right edges: drag to resize */
                Repeater
                {
                    model: 2

                    MouseArea
                    {
                        property bool isLeft: index === 0
                        property int sectionIndex: sectionItem.section.index
                        property real pressX: 0
                        property var edges: []

                        x: isLeft ? 0 : sectionItem.width - laneRoot.edgeWidth
                        width: laneRoot.edgeWidth
                        height: sectionItem.height
                        hoverEnabled: true
                        preventStealing: true
                        cursorShape: Qt.SizeHorCursor

                        Rectangle
                        {
                            anchors.fill: parent
                            color: parent.containsMouse || parent.pressed ? "#7FFFFF00" : "transparent"
                        }

                        onPressed: (mouse) =>
                        {
                            laneRoot.selectedIndex = sectionIndex
                            pressX = mapToItem(laneRoot, mouse.x, 0).x
                            edges = laneRoot.snapEdgesFor(sectionIndex)
                            sectionItem.dragX = sectionItem.x
                            sectionItem.dragWidth = sectionItem.width
                            sectionItem.dragging = true
                        }

                        onPositionChanged: (mouse) =>
                        {
                            if (!sectionItem.dragging)
                                return

                            var section = sectionItem.section
                            var startX = laneRoot.timeToX(section.startTime)
                            var endX = laneRoot.timeToX(section.startTime + section.duration)
                            var delta = mapToItem(laneRoot, mouse.x, 0).x - pressX
                            var s

                            showManager.snapGuideX = -1

                            if (isLeft)
                            {
                                var newStart = startX + delta
                                s = laneRoot.snapX(newStart, edges, mouse.modifiers)
                                if (s.snapped)
                                    showManager.snapGuideX = s.x
                                newStart = Math.max(laneRoot.timeToX(laneRoot.minStartFor(sectionIndex)),
                                                    Math.min(s.x, endX - laneRoot.edgeWidth * 2))
                                sectionItem.dragX = newStart
                                sectionItem.dragWidth = endX - newStart
                            }
                            else
                            {
                                var newEnd = endX + delta
                                s = laneRoot.snapX(newEnd, edges, mouse.modifiers)
                                if (s.snapped)
                                    showManager.snapGuideX = s.x
                                newEnd = Math.max(startX + laneRoot.edgeWidth * 2, s.x)
                                var maxEnd = laneRoot.maxEndFor(sectionIndex)
                                if (maxEnd >= 0)
                                    newEnd = Math.min(newEnd, laneRoot.timeToX(maxEnd))
                                sectionItem.dragX = startX
                                sectionItem.dragWidth = newEnd - startX
                            }
                        }

                        onReleased:
                        {
                            if (!sectionItem.dragging)
                                return

                            showManager.snapGuideX = -1
                            var section = sectionItem.section
                            var newStart = isLeft ? laneRoot.xToTime(sectionItem.dragX) : section.startTime
                            var newEnd = laneRoot.xToTime(sectionItem.dragX + sectionItem.dragWidth)
                            sectionItem.dragging = false
                            if (newEnd > newStart)
                                showManager.updateTempoSection(sectionIndex, newStart, newEnd - newStart,
                                                               section.bpm, section.beatsPerBar, section.name)
                        }

                        onCanceled:
                        {
                            showManager.snapGuideX = -1
                            sectionItem.dragging = false
                        }
                    }
                }
            }
    }

    CustomPopupDialog
    {
        id: messagePopup
        parent: mainView
        title: qsTr("Tempo sections")
        standardButtons: Dialog.Ok
    }

    CustomPopupDialog
    {
        id: overlapPopup
        parent: mainView
        width: mainView.width / 3
        title: qsTr("Tempo sections")
        standardButtons: Dialog.Ok | Dialog.Cancel

        property int audioCount: 0
        property int overlapCount: 0

        // opened after this popup has closed
        onAccepted: Qt.callLater(laneRoot.finishAddFromSelection, precedenceCheck.checked)

        contentItem:
            ColumnLayout
            {
                spacing: 10

                Text
                {
                    Layout.fillWidth: true
                    wrapMode: Text.Wrap
                    color: UISettings.fgMain
                    font.family: UISettings.robotoFontName
                    font.pixelSize: UISettings.textSizeDefault
                    text: overlapPopup.audioCount === 1 ?
                               qsTr("The selected audio item overlaps a tempo section.") :
                               qsTr("%1 of the %2 selected audio items overlap tempo sections.")
                                   .arg(overlapPopup.overlapCount).arg(overlapPopup.audioCount)
                }

                RowLayout
                {
                    Layout.fillWidth: true

                    CustomCheckBox
                    {
                        id: precedenceCheck
                        implicitWidth: UISettings.iconSizeMedium
                        implicitHeight: implicitWidth
                        checked: true
                    }
                    Text
                    {
                        Layout.fillWidth: true
                        wrapMode: Text.Wrap
                        color: UISettings.fgMain
                        font.family: UISettings.robotoFontName
                        font.pixelSize: UISettings.textSizeDefault
                        text: qsTr("Take over from the start of each new section: an existing section is " +
                                    "cut where a new one starts and, if it ran past the end of the new one, " +
                                    "resumes after it on its own beat grid. A new section ends where an " +
                                    "existing one starts.")
                    }
                }

                Text
                {
                    Layout.fillWidth: true
                    wrapMode: Text.Wrap
                    color: UISettings.fgMain
                    font.family: UISettings.robotoFontName
                    font.pixelSize: UISettings.textSizeDefault
                    visible: !precedenceCheck.checked
                    text: qsTr("Only the audio items that don't overlap a tempo section get one.")
                }
            }
    }

    CustomPopupDialog
    {
        id: sectionEditor
        parent: mainView
        title: qsTr("Tempo section")
        width: mainView.width / 3

        property int sectionIndex: -1
        property alias nameText: nameEdit.text

        onAccepted:
        {
            var section = laneRoot.sections[sectionIndex]
            if (section === undefined)
                return
            showManager.updateTempoSection(sectionIndex, section.startTime, section.duration,
                                           bpmSpin.realValue, beatsPerBarSpin.value, nameEdit.text)
        }

        contentItem:
            GridLayout
            {
                columns: 2
                rowSpacing: 5
                columnSpacing: 5

                RobotoText { label: qsTr("Name") }

                CustomTextEdit
                {
                    id: nameEdit
                    Layout.fillWidth: true
                    onAccepted: sectionEditor.accept()
                }

                RobotoText { label: qsTr("BPM") }

                CustomDoubleSpinBox
                {
                    id: bpmSpin
                    Layout.fillWidth: true
                    realFrom: 1
                    realTo: 999
                    realStep: 0.5
                    decimals: 2
                    suffix: ""
                }

                RobotoText { label: qsTr("Beats per bar") }

                CustomSpinBox
                {
                    id: beatsPerBarSpin
                    Layout.fillWidth: true
                    from: 1
                    to: 16
                }
            }
    }
}
