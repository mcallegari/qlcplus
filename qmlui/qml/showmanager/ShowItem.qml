/*
  Q Light Controller Plus
  ShowItem.qml

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
import QtQuick.Controls

import org.qlcplus.classes 1.0
import "TimeUtils.js" as TimeUtils
import "."

Item
{
    id: itemRoot
    height: UISettings.mediumItemHeight
    y: trackIndex >= 0 ? parseInt(height) * trackIndex : 0
    // raised above the other items while it's being dragged
    z: (dragActive || groupFollow) ? 3 : 2
    property ShowFunction sfRef: null
    property QLCFunction funcRef: null
    property int startTime: sfRef ? sfRef.startTime : -1
    property int duration: sfRef ? sfRef.duration : -1
    property int trackIndex: -1
    property int timeDivision: showManager.timeDivision
    property real timeScale: showManager.timeScale
    property real tickSize: showManager.tickSize
    property int beatsDivision: showManager.beatsDivision
    property bool isSelected: false
    // cut items are dimmed until they are moved by a paste
    property bool isCutPending: sfRef ? showManager.cutItemIds.indexOf(sfRef.id) >= 0 : false
    opacity: isCutPending ? 0.5 : 1.0
    property bool isDragging: false
    property color globalColor: showManager.itemsColor
    property string infoText: ""
    property string toolTipText: ""

    // mouse position within the item, used to place the tooltip
    property real tooltipX: 0
    property real tooltipY: 0

    // Snap-to-item properties
    property var snapEdges: []
    property real snapThreshold: 15
    property real pressMouseX: 0
    property real pressMouseY: 0
    property bool dragActive: false
    /* MouseArea emits clicked() right after released(), by which point
       dragActive has already been cleared, so the end of a drag would
       otherwise run the selection handler below - and with Ctrl held down to
       suspend snapping, or Shift to lock the drag axis, that would toggle the
       item out of the selection */
    property bool dragWasActive: false
    property bool itemSnapped: false

    /* Offset of the item body from its original position while dragging.
       The body snaps vertically to the Tracks, so dragOffsetY is always a
       whole number of Tracks, stored in dragTrackDelta */
    property real dragOffsetX: 0
    property real dragOffsetY: 0
    property int dragTrackDelta: 0

    /* Holding Shift while dragging locks the movement to the axis the item
       was moving along when Shift was seen: horizontally to change only the
       start time, or vertically to change only the Track */
    readonly property int axisNone: 0
    readonly property int axisHorizontal: 1
    readonly property int axisVertical: 2
    property int dragAxisLock: axisNone

    /* Dragging a selected item drags the whole selection. groupItems holds
       the other selected (and unlocked) items, which follow the dragged one
       through showManager.groupDragOffset */
    property bool groupDrag: false
    property var groupItems: []
    property bool groupFollow: isSelected && !dragActive && showManager.groupDragActive
                               && !(sfRef && sfRef.locked)

    // the range of Tracks the dragged item(s) can be moved by
    property int minTrackDelta: 0
    property int maxTrackDelta: 0
    // how far left the dragged item(s) can be moved before reaching 0
    property real minOffsetX: 0

    /* Width of the resize handles: at most 10 pixels, but never more than a
       quarter of the item each, so that at least half of a thin item is left
       to click on to select it or drag it. It is updated in updateGeometry(),
       so that it doesn't change while resizing */
    property real handleWidth: 10
    property bool handlesVisible: (sfRef ? (sfRef.locked ? false : true) : false) && handleWidth >= 2

    /* Snapping is suspended for the duration of a single gesture while the
       Ctrl modifier is held down, which is what allows an item to be placed
       freely right next to another one's edge or to a grid division without
       having to turn snapping off in the toolbar. Both this and the toolbar
       flag are read on every mouse move and on release, so pressing or
       releasing Ctrl halfway through a drag takes effect immediately */
    function snapSuspended(modifiers)
    {
        return (modifiers & Qt.ControlModifier) !== 0
    }

    function snappingActive(modifiers)
    {
        return showManager.snapToItems && !snapSuspended(modifiers)
    }

    function getVisibleSnapEdges()
    {
        // nothing to snap to when snapping is off, which spares us from
        // walking all the Show tracks on every press
        if (!showManager.snapToItems)
            return []

        // itemRoot.parent is the Flickable's contentItem,
        // itemRoot.parent.parent is the Flickable (itemsArea)
        var flickable = itemRoot.parent ? itemRoot.parent.parent : null
        if (flickable && flickable.contentX !== undefined)
            return showManager.getSnapEdges(sfRef.id, flickable.contentX, flickable.contentX + flickable.width)
        return showManager.getSnapEdges(sfRef.id)
    }

    onStartTimeChanged: updateGeometry()
    onDurationChanged: updateGeometry()
    onTimeScaleChanged: updateGeometry()
    onTimeDivisionChanged: updateGeometry()
    onBeatsDivisionChanged: updateGeometry()

    onGlobalColorChanged:
    {
        if (isSelected && sfRef)
            sfRef.color = globalColor
    }

    onFuncRefChanged:
    {
        updateGeometry()
        updateTooltipText()
    }

    Connections
    {
        target: funcRef
        function onTempoTypeChanged() { updateGeometry() }
    }

    Connections
    {
        // the current BPM affects the on-screen position of items whose
        // own tempo type differs from the Show's timeline division (see
        // updateGeometry())
        target: ioManager
        function onBpmNumberChanged() { updateGeometry() }
    }

    /* Returns true if the item position is stored in "beats as ms": a Beats
       tempo Function, unless the Show keeps all its items in ms, as it does
       once it has had tempo sections. The property is read live rather than
       bound, since adding the first section converts the items before
       notifying it */
    function isBeatsItem()
    {
        return funcRef !== null && funcRef.tempoType === QLCFunction.Beats
               && showManager.itemsInMs === false
    }

    /* Returns true if the item runs on the Show tempo map */
    function isTempoItem()
    {
        return funcRef !== null && funcRef.tempoType === QLCFunction.Beats
               && showManager.itemsInMs === true
    }

    /* Convert an item duration to the Function own unit: on tempo sections
       a Beats tempo item lasts ms, while its Function lasts beats */
    function itemToFunctionDuration(value)
    {
        if (isTempoItem())
            return Math.round((value / showManager.tempoBeatDuration(startTime)) * 1000)
        return value
    }

    /* Snap an X position to the grid: the tempo section grid inside a
       section, otherwise multiples of $fallbackStep (0 = no snapping) */
    function gridSnap(xPos, fallbackStep)
    {
        if (showManager.tempoGridActive)
            return showManager.snapToTempoGrid(xPos, fallbackStep)
        return fallbackStep > 0 ? Math.round(xPos / fallbackStep) * fallbackStep : xPos
    }

    Connections
    {
        target: showManager
        function onTempoSectionsChanged()
        {
            itemRoot.updateGeometry()
            prCanvas.requestPaint()
        }
    }

    function updateGeometry()
    {
        if (isDragging || funcRef == null)
            return

        /* A Function keeps its own tempo type regardless of the Show's
           timeline division (a Show can mix time-based and beat-based
           items), so startTime/duration are stored in real milliseconds
           for a Time tempo Function and in "beats as ms" (1000 units per
           beat) for a Beats tempo one. The item's on-screen position must
           always be computed according to ITS OWN unit, converted to
           match whichever ruler (Time or BPM) the Show is currently
           displaying - never assume the Show's division tells us the
           item's own unit. */
        var itemIsBeats = isBeatsItem()

        if (timeDivision === Show.Time)
        {
            if (itemIsBeats)
            {
                x = TimeUtils.beatsToTimeSize(startTime, ioManager.bpmNumber, timeScale, tickSize)
                width = TimeUtils.beatsToTimeSize(duration, ioManager.bpmNumber, timeScale, tickSize)
            }
            else
            {
                x = TimeUtils.timeToSize(startTime, timeScale, tickSize)
                width = TimeUtils.timeToSize(duration, timeScale, tickSize)
            }
        }
        else
        {
            if (itemIsBeats)
            {
                x = TimeUtils.beatsToSize(startTime, tickSize, beatsDivision)
                width = TimeUtils.beatsToSize(duration, tickSize, beatsDivision)
            }
            else
            {
                x = TimeUtils.timeToBeatSize(startTime, ioManager.bpmNumber, beatsDivision, tickSize)
                width = TimeUtils.timeToBeatSize(duration, ioManager.bpmNumber, beatsDivision, tickSize)
            }
        }

        handleWidth = Math.min(10, width / 4)
        // dragging the right handler breaks its position binding
        horRightHandler.x = Qt.binding(function() { return itemRoot.width - itemRoot.handleWidth })
    }

    /* Convert an X position on the timeline to a start time expressed in
       THIS item's own Function unit (see updateGeometry()) */
    function positionToTime(xPos)
    {
        var itemIsBeats = isBeatsItem()

        if (timeDivision === Show.Time)
            return itemIsBeats
                    ? TimeUtils.posToBeatsMsOnTimeline(xPos, timeScale, tickSize, ioManager.bpmNumber)
                    : TimeUtils.posToMs(xPos, timeScale, tickSize)
        else
            return itemIsBeats
                    ? TimeUtils.posToBeat(xPos, tickSize, beatsDivision)
                    : TimeUtils.posToBeatMs(xPos, tickSize, ioManager.bpmNumber, beatsDivision)
    }

    /* Convert a value expressed in THIS item's own Function unit (real ms
       for a Time tempo Function, "beats as ms" for a Beats tempo one) to
       a pixel size/position on whichever ruler (Time or BPM) the Show is
       currently displaying. Same conversion rules as updateGeometry(),
       factored out for the step/fade preview painting below. */
    function timeValueToPixels(value)
    {
        if (funcRef == null)
            return 0

        var itemIsBeats = isBeatsItem()

        // on the Show tempo map, the Function beats last as long as they do
        // at the tempo of the item start
        if (isTempoItem())
        {
            var ms = (value / 1000) * showManager.tempoBeatDuration(startTime)
            return timeDivision === Show.Time
                    ? TimeUtils.timeToSize(ms, timeScale, tickSize)
                    : TimeUtils.timeToBeatSize(ms, ioManager.bpmNumber, beatsDivision, tickSize)
        }

        if (timeDivision === Show.Time)
        {
            return itemIsBeats
                    ? TimeUtils.beatsToTimeSize(value, ioManager.bpmNumber, timeScale, tickSize)
                    : TimeUtils.timeToSize(value, timeScale, tickSize)
        }
        else
        {
            return itemIsBeats
                    ? TimeUtils.beatsToSize(value, tickSize, beatsDivision)
                    : TimeUtils.timeToBeatSize(value, ioManager.bpmNumber, beatsDivision, tickSize)
        }
    }

    function updateTooltipText()
    {
        var tooltip = funcRef ? funcRef.name + "\n" : ""
        var pos = 0
        var dur = 0

        if (timeDivision === Show.Time)
        {
            pos = TimeUtils.msToString(TimeUtils.posToMs(itemRoot.x + showItemBody.x, timeScale, tickSize))
            dur = TimeUtils.msToString(TimeUtils.posToMs(itemRoot.width, timeScale, tickSize))
        }
        else
        {
            pos = TimeUtils.beatsToString((itemRoot.x + showItemBody.x) / (tickSize / beatsDivision), beatsDivision)
            dur = TimeUtils.beatsToString(itemRoot.width / (tickSize / beatsDivision), beatsDivision)
        }

        tooltip += qsTr("Position: ") + pos
        tooltip += "\n" + qsTr("Duration: ") + dur
        toolTipText = tooltip
    }

    Canvas
    {
        id: prCanvas
        z: 3
        height: itemRoot.height
        contextType: "2d"

        /** A Canvas allocates a backing image, and on the scene graph side a
          * texture, for its whole size - so filling the item would mean one
          * allocation per Show item as wide as that item is on the timeline,
          * whether or not any of it is on screen. Zooming in multiplies every
          * item's width by the same factor, so that grows without bound: a
          * long Show can end up holding a gigabyte of backing images, and any
          * item wider than the maximum texture size the driver supports (a
          * common limit is 16384 pixels, which a four minute item reaches at a
          * time scale of 1) has to be rescaled on the CPU on every repaint
          * before it can be uploaded.
          *
          * The timeline header and the grid already avoid this by keeping
          * their Canvas no bigger than the visible area and moving it as the
          * view scrolls; do the same here, clipping the Canvas to the part of
          * the item that is actually on screen and painting with the item's
          * own coordinates shifted by the Canvas position, so the painting
          * code below stays unchanged. */
        property Item timelineView:
        {
            /* itemRoot.parent is the Flickable's contentItem,
               itemRoot.parent.parent is the Flickable itself */
            var f = itemRoot.parent ? itemRoot.parent.parent : null
            return (f && f.contentX !== undefined) ? f : null
        }

        property real viewLeft: timelineView ? timelineView.contentX - itemRoot.x : 0
        property real viewRight: timelineView ? viewLeft + timelineView.width : itemRoot.width

        x: timelineView ? Math.max(0, Math.min(viewLeft, itemRoot.width)) : 0
        width: timelineView ? Math.max(0, Math.min(viewRight, itemRoot.width) - x)
                            : itemRoot.width

        onXChanged: requestPaint()

        /* The painted positions come from the item's own size and from the
           timeline scale, but the Canvas is now sized by the visible area
           instead of by the item, so zooming no longer resizes it and no
           repaint is triggered by itself: ask for one explicitly */
        Connections
        {
            target: itemRoot

            function onWidthChanged() { prCanvas.requestPaint() }
            function onTimeScaleChanged() { prCanvas.requestPaint() }
            function onTickSizeChanged() { prCanvas.requestPaint() }
            function onTimeDivisionChanged() { prCanvas.requestPaint() }
            function onBeatsDivisionChanged() { prCanvas.requestPaint() }
        }

        /* Repaint the preview lines when the referenced Function
           is modified (e.g. a Chaser step time or an EFX duration) */
        Connections
        {
            target: showManager

            function onFunctionChanged(fid)
            {
                if (funcRef && fid === funcRef.id)
                    prCanvas.requestPaint()
            }
        }

        onPaint:
        {
            /* an item scrolled out of the visible area has no width, and then
               Qt never creates a drawing context for the Canvas */
            if (context === null || context === undefined)
                return

            context.reset()
            context.clearRect(0, 0, width, height)

            if (sfRef === null || funcRef === null)
                return

            var previewData = showManager.previewData(funcRef)

            if (previewData === null || previewData === undefined)
                return

            /* paint in the item's own coordinates: the Canvas covers only the
               visible slice of the item (see above), so shift it into place */
            context.save()
            context.translate(-prCanvas.x, 0)

            var visLeft = prCanvas.x
            var visRight = prCanvas.x + prCanvas.width

            context.strokeStyle = "#ddd"
            context.fillStyle = "transparent"
            context.lineWidth = 1

            context.beginPath()

            //console.log("About to paint " + previewData.length + " values")

            var lastTime = 0
            var xPos = 0
            var stepsCount = 0

            for (var i = 0; i < previewData.length; i += 2)
            {
                if (i + 1 >= previewData.length)
                    break

                switch (previewData[i])
                {
                    case ShowManager.RepeatingDuration:
                        var funcDuration = isTempoItem()
                                ? (funcRef.totalDuration / 1000) * showManager.tempoBeatDuration(startTime)
                                : funcRef.totalDuration
                        var loopCount = funcDuration ? Math.floor(sfRef.duration / funcDuration) : 0
                        for (var l = 0; l < loopCount; l++)
                        {
                            lastTime += previewData[i + 1]
                            xPos = timeValueToPixels(lastTime)
                            /* the number of repeats is a ratio of times, so it
                               is not bounded by the item's width on screen:
                               stop as soon as the lines leave the painted area */
                            if (xPos > visRight)
                                break
                            if (xPos < visLeft)
                                continue
                            context.moveTo(xPos, 0)
                            context.lineTo(xPos, itemRoot.height)
                        }
                        context.stroke()
                        lastTime = 0
                        xPos = 0
                    break
                    case ShowManager.FadeIn:
                        var fiEnd = timeValueToPixels(lastTime + previewData[i + 1])
                        context.moveTo(xPos, itemRoot.height)
                        context.lineTo(fiEnd, 0)
                    break
                    case ShowManager.StepDivider:
                        lastTime = previewData[i + 1]
                        xPos = timeValueToPixels(lastTime)
                        context.moveTo(xPos, 0)
                        context.lineTo(xPos, itemRoot.height)
                        stepsCount++
                    break
                    case ShowManager.FadeOut:
                        var foEnd = timeValueToPixels(lastTime + previewData[i + 1])
                        context.moveTo(stepsCount ? xPos : itemRoot.width - foEnd, 0)
                        context.lineTo(stepsCount ? foEnd : itemRoot.width, itemRoot.height)
                    break
                }

            }
            context.stroke()
            context.restore()
        }
    }

    /* Body mouse area (covers the whole item) */
    MouseArea
    {
        id: sfMouseArea
        anchors.fill: parent
        // in box selection mode, presses fall through to the timeline
        // so that a selection box can be started on top of an item
        enabled: !showManager.boxSelectMode
        hoverEnabled: true
        preventStealing: true

        Rectangle
        {
            id: showItemBody
            x: dragActive ? dragOffsetX : (groupFollow ? showManager.groupDragOffset.x : 0)
            y: dragActive ? dragOffsetY : (groupFollow ? showManager.groupDragOffset.y : 0)
            width: itemRoot.width
            height: itemRoot.height
            color: sfRef ? sfRef.color : UISettings.bgLight
            border.width: isSelected ? 2 : 1
            border.color: isSelected ? UISettings.selection : "white"
            clip: true

            Drag.active: itemRoot.dragActive
            Drag.keys: [ "function" ]

            /* Waveform for audio items. It is a child of the item body, and
               declared before the labels, so that it is painted over the body
               background but behind the Function name and info texts */
            Image
            {
                id: waveformImage
                x: 0
                y: 0
                // Natural width spans the full audio duration so the waveform is
                // not stretched; the body's clip:true crops it to the
                // show item's visible width.
                width: (funcRef && funcRef.totalDuration && sfRef && sfRef.duration)
                       ? itemRoot.width * (funcRef.totalDuration / sfRef.duration)
                       : itemRoot.width
                height: itemRoot.height
                cache: false
                fillMode: Image.Stretch
                visible: funcRef && funcRef.type === QLCFunction.AudioType

                source: (funcRef && funcRef.type === QLCFunction.AudioType) ? "image://waveform/" + funcRef.id : ""

                function reload()
                {
                    const old = source;
                    source = "";
                    source = old;
                }

                Connections
                {
                    target: waveformProvider

                    function onWaveformUpdated(fid)
                    {
                        if (funcRef && fid === funcRef.id)
                            waveformImage.reload()
                    }
                }
            }

            RobotoText
            {
                x: 3
                y: 3
                width: parent.width - 6
                height: parent.height - 6
                label: funcRef ? funcRef.name : ""
                fontSize: UISettings.textSizeDefault * 0.7
                textVAlign: Text.AlignTop
                wrapText: true
            }

            RobotoText
            {
                id: infoTextBox
                x: 3
                y: itemRoot.height - height - 3
                width: itemRoot.width - 6
                height: itemRoot.height / 4
                fontSize: UISettings.textSizeDefault * 0.6
                textHAlign: Text.AlignLeft
                wrapText: true
                label: infoText
            }
        }

        onPressed: (mouse) =>
        {
            if (sfRef && sfRef.locked)
                return;
            showManager.enableFlicking(false)
            pressMouseX = mouse.x
            pressMouseY = mouse.y
            isDragging = true
            dragActive = false
            dragWasActive = false
            itemSnapped = false
            dragOffsetX = 0
            dragOffsetY = 0
            dragTrackDelta = 0
            dragAxisLock = axisNone
            snapEdges = getVisibleSnapEdges()

            groupItems = []
            if (isSelected && showManager.selectedItemsCount > 1)
            {
                var views = showManager.selectedItemViews()
                var others = []
                for (var i = 0; i < views.length; i++)
                {
                    var v = views[i]
                    if (v && v !== itemRoot && v.sfRef && !v.sfRef.locked)
                        others.push(v)
                }
                groupItems = others
            }
            groupDrag = groupItems.length > 0

            // a single item can be dropped past the last Track to create a
            // new one, while a selection can only be moved across existing ones
            var minTrack = trackIndex
            var maxTrack = trackIndex
            var minX = itemRoot.x
            for (var j = 0; j < groupItems.length; j++)
            {
                minTrack = Math.min(minTrack, groupItems[j].trackIndex)
                maxTrack = Math.max(maxTrack, groupItems[j].trackIndex)
                minX = Math.min(minX, groupItems[j].x)
            }
            var tracksCount = showManager.tracksCount()
            minTrackDelta = -minTrack
            maxTrackDelta = (groupDrag ? tracksCount - 1 : tracksCount) - maxTrack
            minOffsetX = -minX
        }
        onPositionChanged: (mouse) =>
        {
            // keep track of the hovering position to place the tooltip
            itemRoot.tooltipX = mouse.x
            itemRoot.tooltipY = mouse.y

            if (!isDragging)
                return

            var dx = mouse.x - pressMouseX
            var dy = mouse.y - pressMouseY

            if (!dragActive)
            {
                if (Math.abs(dx) < 30 && Math.abs(dy) < 30)
                    return
                dragActive = true
                infoTextBox.height = itemRoot.height / 4
                infoTextBox.textHAlign = Text.AlignLeft

                if (groupDrag)
                {
                    // the other selected items move along, so their
                    // edges are no targets to snap to
                    var edges = []
                    for (var e = 0; e < snapEdges.length; e++)
                    {
                        var moving = false
                        for (var g = 0; g < groupItems.length; g++)
                        {
                            var gi = groupItems[g]
                            if (Math.abs(snapEdges[e] - gi.x) < 0.5 ||
                                Math.abs(snapEdges[e] - (gi.x + gi.width)) < 0.5)
                            {
                                moving = true
                                break
                            }
                        }
                        if (!moving)
                            edges.push(snapEdges[e])
                    }
                    snapEdges = edges
                    showManager.groupDragOffset = Qt.point(0, 0)
                    showManager.groupDragActive = true
                }
            }

            if (mouse.modifiers & Qt.ShiftModifier)
            {
                if (dragAxisLock === axisNone)
                    dragAxisLock = Math.abs(dx) >= Math.abs(dy) ? axisHorizontal : axisVertical
            }
            else
            {
                dragAxisLock = axisNone
            }

            if (dragAxisLock === axisHorizontal)
                dy = 0
            else if (dragAxisLock === axisVertical)
                dx = 0

            // snap vertically to the nearest Track
            dragTrackDelta = Math.max(minTrackDelta, Math.min(maxTrackDelta, Math.round(dy / itemRoot.height)))
            dy = dragTrackDelta * itemRoot.height

            showManager.snapGuideX = -1
            itemSnapped = false

            if (dragAxisLock !== axisVertical && snappingActive(mouse.modifiers))
            {
                // snap-to-item: check start edge if clicked on first half,
                // end edge if clicked on second half
                var checkStart = (pressMouseX < itemRoot.width / 2)
                var edgePos = checkStart ? (itemRoot.x + dx) : (itemRoot.x + dx + itemRoot.width)
                var bestDelta = snapThreshold + 1
                var bestSnapX = -1

                for (var i = 0; i < snapEdges.length; i++)
                {
                    var d = snapEdges[i] - edgePos
                    if (Math.abs(d) < Math.abs(bestDelta))
                    {
                        bestDelta = d
                        bestSnapX = snapEdges[i]
                    }
                }

                if (Math.abs(bestDelta) <= snapThreshold)
                {
                    dx += bestDelta
                    showManager.snapGuideX = bestSnapX
                    itemSnapped = true
                }
            }

            // never move an item before the beginning of the Show
            dx = Math.max(dx, minOffsetX)

            dragOffsetX = dx
            dragOffsetY = dy
            if (groupDrag)
                showManager.groupDragOffset = Qt.point(dx, dy)

            var txt
            if (timeDivision === Show.Time)
                txt = TimeUtils.msToString(TimeUtils.posToMs(itemRoot.x + showItemBody.x, timeScale, tickSize))
            else
                txt = TimeUtils.beatsToString((itemRoot.x + showItemBody.x) / (tickSize / beatsDivision), beatsDivision)

            infoText = qsTr("Position: ") + txt
        }
        onReleased: (mouse) =>
        {
            if (sfRef && sfRef.locked)
                return;

            showManager.snapGuideX = -1

            if (dragActive)
            {
                infoText = ""

                var moveX = dragOffsetX
                var dropX = itemRoot.x + moveX

                // grid snapping: snap to the nearest beat on a BPM ruler, or
                // to the tempo section grid on a Time ruler
                // (skipped if already snapped to another item's edge, if the
                // item hasn't moved horizontally, or while Ctrl suspends snapping)
                if (showManager.gridEnabled && !itemSnapped && moveX !== 0
                        && !snapSuspended(mouse.modifiers)
                        && (timeDivision !== Show.Time || showManager.tempoGridActive))
                {
                    dropX = gridSnap(dropX, timeDivision !== Show.Time ? tickSize / beatsDivision : 0)
                    moveX = dropX - itemRoot.x
                }

                // a Function keeps its own tempo type regardless of the Show's
                // ruler (see updateGeometry() above), so the dropped position
                // must be converted using ITS OWN unit, like the resize handlers do.
                // An item moved only across Tracks keeps its exact start time.
                // Round to the nearest unit: truncating a snapped position that is
                // a hair below the edge would make the item overlap its neighbour
                var newTime = moveX === 0 ? startTime : Math.round(positionToTime(dropX))
                var newTrackIdx = trackIndex + dragTrackDelta
                if (newTime < 0)
                    newTime = 0

                if (groupDrag)
                {
                    var items = [ itemRoot ]
                    var tracks = [ newTrackIdx ]
                    var times = [ newTime ]

                    for (var i = 0; i < groupItems.length; i++)
                    {
                        var gi = groupItems[i]
                        items.push(gi)
                        tracks.push(gi.trackIndex + dragTrackDelta)
                        times.push(moveX === 0 ? gi.startTime : Math.max(0, Math.round(gi.positionToTime(gi.x + moveX))))
                    }

                    showManager.moveShowItems(items, tracks, times)
                    showManager.groupDragActive = false
                    showManager.groupDragOffset = Qt.point(0, 0)
                    prCanvas.requestPaint()
                }
                else if (newTrackIdx >= 0)
                {
                    var res = showManager.checkAndMoveItem(sfRef, trackIndex, newTrackIdx, newTime)

                    if (res === true)
                        trackIndex = newTrackIdx

                    prCanvas.requestPaint()
                }

                dragOffsetX = 0
                dragOffsetY = 0
            }

            showManager.enableFlicking(true)
            updateTooltipText()
            isDragging = false
            dragWasActive = dragActive
            dragActive = false
            groupDrag = false
            groupItems = []
            itemSnapped = false
            updateGeometry()
        }

        onCanceled:
        {
            // put everything back, including the items following a group drag
            if (groupDrag)
            {
                showManager.groupDragActive = false
                showManager.groupDragOffset = Qt.point(0, 0)
            }
            showManager.snapGuideX = -1
            showManager.enableFlicking(true)
            infoText = ""
            dragOffsetX = 0
            dragOffsetY = 0
            isDragging = false
            dragActive = false
            groupDrag = false
            groupItems = []
            itemSnapped = false
            updateGeometry()
        }

        onClicked: (mouse) =>
        {
            if (dragWasActive)
            {
                dragWasActive = false
                return
            }
            var multi = ((mouse.modifiers & Qt.ControlModifier) || (mouse.modifiers & Qt.ShiftModifier))
                    || (showManager && showManager.multipleSelection)
            if (multi)
                itemRoot.isSelected = !itemRoot.isSelected
            else
                itemRoot.isSelected = true
            showManager.setItemSelection(trackIndex, sfRef, itemRoot, itemRoot.isSelected, mouse.modifiers)
        }

        onDoubleClicked: functionManager.setEditorFunction(sfRef.functionID, true, false)
    }

    /* Function type icon and locker image. These are kept at root level with a
       z above prCanvas, so they are drawn on top of step dividers and fade lines.
       They follow showItemBody so they move along with the item while dragging */
    Image
    {
        id: funcIcon
        x: showItemBody.x + 3
        y: showItemBody.y + itemRoot.height - height - 3
        z: 4
        visible: infoText ? false : true
        width: itemRoot.height / 3
        height: width
        source: funcRef ? functionManager.functionIcon(funcRef.type) : ""
        sourceSize: Qt.size(width, height)
    }

    Image
    {
        x: showItemBody.x + (funcIcon.visible ? funcIcon.width + 6 : 3)
        y: showItemBody.y + itemRoot.height - height - 3
        z: 4
        width: itemRoot.height / 3
        height: width
        source: "qrc:/lock.svg"
        sourceSize: Qt.size(width, height)
        visible: sfRef ? (sfRef.locked ? true : false) : false
    }

    /* Item information tooltip, displayed at the mouse position */
    ToolTip
    {
        id: itemToolTip
        x: itemRoot.tooltipX + (UISettings.iconSizeMedium / 2)
        y: itemRoot.tooltipY
        visible: sfMouseArea.containsMouse && !isDragging && text !== ""
        delay: 1000
        text: toolTipText
    }

    /* horizontal left handler */
    Rectangle
    {
        id: horLeftHandler
        z: 2
        width: handleWidth
        height: itemRoot.height
        color: horLeftHdlMa.containsMouse ? "#7FFFFF00" : "transparent"
        visible: handlesVisible

        MouseArea
        {
            id: horLeftHdlMa
            anchors.fill: parent
            enabled: !showManager.boxSelectMode
            preventStealing: true
            hoverEnabled: true
            cursorShape: containsMouse ? Qt.SizeHorCursor : Qt.ArrowCursor

            property real pressX: 0
            property real origItemX: 0
            property real origItemW: 0

            onPressed: (mouse) =>
            {
                isDragging = true
                itemSnapped = false
                snapEdges = getVisibleSnapEdges()
                pressX = mapToItem(itemRoot.parent, mouse.x, mouse.y).x
                origItemX = itemRoot.x
                origItemW = itemRoot.width
            }

            onPositionChanged: (mouse) =>
            {
                if (!pressed)
                    return

                var globalX = mapToItem(itemRoot.parent, mouse.x, mouse.y).x
                var dx = globalX - pressX
                var newX = origItemX + dx

                // snap-to-item: check left edge
                showManager.snapGuideX = -1
                itemSnapped = false

                if (snappingActive(mouse.modifiers))
                {
                    var bestDist = snapThreshold + 1
                    var bestSnapX = -1
                    for (var i = 0; i < snapEdges.length; i++)
                    {
                        var dist = Math.abs(snapEdges[i] - newX)
                        if (dist < bestDist)
                        {
                            bestDist = dist
                            bestSnapX = snapEdges[i]
                        }
                    }
                    if (bestSnapX >= 0 && bestDist <= snapThreshold)
                    {
                        newX = bestSnapX
                        showManager.snapGuideX = bestSnapX
                        itemSnapped = true
                    }
                }

                // clamp: don't allow shrinking past minimum width
                var maxX = origItemX + origItemW - horLeftHandler.width
                if (newX > maxX)
                    newX = maxX

                itemRoot.width = origItemW + (origItemX - newX)
                itemRoot.x = newX
                infoTextBox.height = itemRoot.height / 2
                infoTextBox.textHAlign = Text.AlignLeft
                updateTooltipText()
            }
            onReleased: (mouse) =>
            {
                showManager.snapGuideX = -1

                if (sfRef)
                {
                    if (itemRoot.x < 0)
                    {
                        itemRoot.width += itemRoot.x
                        itemRoot.x = 0
                    }

                    // check grid snapping (skip if item-snapped)
                    if (!itemSnapped && itemRoot.x && showManager.gridEnabled
                            && !snapSuspended(mouse.modifiers))
                    {
                        var currX = itemRoot.x
                        itemRoot.x = gridSnap(itemRoot.x, tickSize)
                        itemRoot.width += (currX - itemRoot.x)
                    }

                    var newDuration, newStartTime
                    var itemIsBeats = isBeatsItem()

                    if (timeDivision === Show.Time)
                    {
                        if (itemIsBeats)
                        {
                            newStartTime = TimeUtils.posToBeatsMsOnTimeline(itemRoot.x, timeScale, tickSize, ioManager.bpmNumber)
                            newDuration = TimeUtils.posToBeatsMsOnTimeline(itemRoot.width, timeScale, tickSize, ioManager.bpmNumber)
                        }
                        else
                        {
                            newStartTime = TimeUtils.posToMs(itemRoot.x, timeScale, tickSize)
                            newDuration = TimeUtils.posToMs(itemRoot.width, timeScale, tickSize)
                        }
                    }
                    else
                    {
                        if (itemIsBeats)
                        {
                            newStartTime = TimeUtils.posToBeat(itemRoot.x, tickSize, beatsDivision)
                            newDuration = TimeUtils.posToBeat(itemRoot.width, tickSize, beatsDivision)
                        }
                        else
                        {
                            newStartTime = TimeUtils.posToBeatMs(itemRoot.x, tickSize, ioManager.bpmNumber, beatsDivision)
                            newDuration = TimeUtils.posToBeatMs(itemRoot.width, tickSize, ioManager.bpmNumber, beatsDivision)
                        }
                    }

                    newStartTime = Math.round(newStartTime)
                    newDuration = Math.round(newDuration)

                    // the left edge moves the start and changes the duration while
                    // the end stays put, so both must be checked together
                    if (showManager.setShowItemStartTimeAndDuration(sfRef, newStartTime, newDuration) === false)
                        updateGeometry()

                    if (funcRef && showManager.stretchFunctions === true)
                        funcRef.totalDuration = itemToFunctionDuration(sfRef.duration)

                    prCanvas.requestPaint()
                }
                infoText = ""
                isDragging = false
                itemSnapped = false
                updateGeometry()
            }
        }
    }

    /* horizontal right handler */
    Rectangle
    {
        id: horRightHandler
        x: itemRoot.width - handleWidth
        z: 2
        width: handleWidth
        height: itemRoot.height
        color: horRightHdlMa.containsMouse ? "#7FFFFF00" : "transparent"
        visible: handlesVisible

        MouseArea
        {
            id: horRightHdlMa
            anchors.fill: parent
            enabled: !showManager.boxSelectMode
            preventStealing: true
            hoverEnabled: true
            cursorShape: containsMouse ? Qt.SizeHorCursor : Qt.ArrowCursor

            drag.target: horRightHandler
            drag.axis: Drag.XAxis
            drag.minimumX: horLeftHandler.x + width

            onPressed:
            {
                isDragging = true
                itemSnapped = false
                snapEdges = getVisibleSnapEdges()
            }

            onPositionChanged: (mouse) =>
            {
                if (drag.active === true)
                {
                    var obj = mapToItem(itemRoot, mouseX, mouseY)
                    var newWidth = obj.x + (horRightHdlMa.width - mouse.x)

                    // snap-to-item: check right edge
                    showManager.snapGuideX = -1
                    itemSnapped = false

                    if (snappingActive(mouse.modifiers))
                    {
                        var rightEdge = itemRoot.x + newWidth
                        var bestDist = snapThreshold + 1
                        var bestSnapX = -1
                        for (var i = 0; i < snapEdges.length; i++)
                        {
                            var dist = Math.abs(snapEdges[i] - rightEdge)
                            if (dist < bestDist)
                            {
                                bestDist = dist
                                bestSnapX = snapEdges[i]
                            }
                        }
                        if (bestSnapX >= 0 && bestDist <= snapThreshold)
                        {
                            newWidth = bestSnapX - itemRoot.x
                            showManager.snapGuideX = bestSnapX
                            itemSnapped = true
                        }
                    }

                    itemRoot.width = newWidth
                    infoTextBox.height = itemRoot.height / 4
                    infoTextBox.textHAlign = Text.AlignRight
                    updateTooltipText()
                }
            }
            onReleased: (mouse) =>
            {
                if (drag.active === false)
                    return

                showManager.snapGuideX = -1

                if (sfRef)
                {
                    // check grid snapping (skip if item-snapped)
                    if (!itemSnapped && showManager.gridEnabled
                            && !snapSuspended(mouse.modifiers))
                    {
                        var snappedEndPos = gridSnap(itemRoot.x + itemRoot.width, tickSize)
                        itemRoot.width = snappedEndPos - itemRoot.x
                    }

                    var newDuration
                    var itemIsBeats = isBeatsItem()

                    if (timeDivision === Show.Time)
                    {
                        newDuration = itemIsBeats
                                ? TimeUtils.posToBeatsMsOnTimeline(itemRoot.width, timeScale, tickSize, ioManager.bpmNumber)
                                : TimeUtils.posToMs(itemRoot.width, timeScale, tickSize)
                    }
                    else
                    {
                        newDuration = itemIsBeats
                                ? (Math.round(itemRoot.width / (tickSize / beatsDivision)) * 1000)
                                : TimeUtils.posToBeatMs(itemRoot.width, tickSize, ioManager.bpmNumber, beatsDivision)
                    }

                    newDuration = Math.round(newDuration)

                    if (showManager.setShowItemDuration(sfRef, newDuration) === false)
                        updateGeometry()

                    if (funcRef && showManager.stretchFunctions === true)
                        funcRef.totalDuration = itemToFunctionDuration(sfRef.duration)

                    prCanvas.requestPaint()
                }
                infoText = ""
                isDragging = false
                itemSnapped = false
                updateGeometry()
            }
        }
    }
}
