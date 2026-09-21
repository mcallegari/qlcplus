/*
  Q Light Controller Plus
  ShowManager.qml

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

Rectangle
{
    id: showMgrContainer
    anchors.fill: parent
    color: "transparent"

    property string contextName: "SHOWMGR"

    property int trackHeight: UISettings.mediumItemHeight
    property int trackWidth: UISettings.bigItemHeight * 1.6

    property real timeScale: showManager.timeScale
    property real tickSize: showManager.tickSize
    property int headerHeight: UISettings.iconSizeMedium
    property real xViewOffset: 0

    property int showID: showManager.currentShowID
    property int selectedTrackIndex: -1

    onShowIDChanged: renderAndCenter()
    Component.onCompleted:
    {
        renderAndCenter()
        syncSelectedTrackIndex()
    }

    // the Track selection also changes without a click on a Track (e.g. when
    // selecting items or deleting the selected Track), and it outlives this
    // view when switching to another context and back
    function syncSelectedTrackIndex()
    {
        var tracks = showManager.tracks
        selectedTrackIndex = -1
        for (var i = 0; tracks && i < tracks.length; i++)
        {
            if (tracks[i].id === showManager.selectedTrackId)
            {
                selectedTrackIndex = i
                break
            }
        }
    }

    Connections
    {
        target: showManager
        function onSelectedTrackIdChanged() { syncSelectedTrackIndex() }
    }

    function centerView()
    {
        var xPos = TimeUtils.timeToSize(showManager.currentTime, timeScale, tickSize) - (timelineHeader.width / 2)
        if (xPos >= 0)
            xViewOffset = xPos
    }

    function renderAndCenter()
    {
        //console.log("Show Manager tick size: " + tickSize + "pixel")
        showManager.renderView(itemsArea.contentItem)
        centerView()
    }

    Rectangle
    {
        id: topBar
        width: showMgrContainer.width - rightPanel.width
        height: UISettings.iconSizeDefault
        z: 5
        gradient: Gradient
        {
            GradientStop { position: 0; color: UISettings.toolbarStartSub }
            GradientStop { position: 1; color: UISettings.toolbarEnd }
        }

        RowLayout
        {
            id: topBarRowLayout
            anchors.fill: parent
            y: 1

            spacing: 4

            RobotoText { label: qsTr("Name") }

            CustomTextEdit
            {
                width: showMgrContainer.width / 5
                height: parent.height - 10
                text: showManager.showName
                enabled: showManager.isEditing

                onTextEdited: showManager.showName = text
            }

            IconButton
            {
                id: colPickButton
                z: 2
                width: parent.height - 6
                height: width
                imgSource: "qrc:/color.svg"
                checkable: true
                enabled: showManager.isEditing
                tooltip: qsTr("Show items color")
                onCheckedChanged: colTool.visible = !colTool.visible

                ColorTool
                {
                    id: colTool
                    parent: mainView
                    x: colPickButton.x
                    y: UISettings.bigItemHeight //colPickButton.y + colPickButton.height
                    z: 15
                    visible: false

                    onToolColorChanged:
                        function(r, g, b, w, a, uv)
                        {
                            showManager.itemsColor = Qt.rgba(r, g, b, 1.0)
                        }
                    onClose: colPickButton.toggle()
                }
            }

            IconButton
            {
                id: lockItem
                z: 2
                width: parent.height - 6
                height: width
                imgSource: "qrc:/lock.svg"
                counter: showManager.selectedItemsCount

                function checkLockStatus()
                {
                    if (showManager.selectedItemsLocked())
                    {
                        imgSource = "qrc:/unlock.svg"
                        tooltip = qsTr("Unlock the selected items")
                    }
                    else
                    {
                        imgSource = "qrc:/lock.svg"
                        tooltip = qsTr("Lock the selected items")
                    }
                }

                onCounterChanged:
                {
                    checkLockStatus()
                }

                onClicked:
                {
                    var lock = showManager.selectedItemsLocked()
                    if (lock === true)
                        showManager.setSelectedItemsLock(false)
                    else
                        showManager.setSelectedItemsLock(true)
                    checkLockStatus()
                }
            }

            IconButton
            {
                id: gridButton
                z: 2
                width: parent.height - 6
                height: width
                imgSource: "qrc:/grid.svg"
                tooltip: qsTr("Snap to grid")
                checkable: true
                checked: showManager.gridEnabled
                onToggled: showManager.gridEnabled = checked
            }

            IconButton
            {
                id: snapItemsButton
                z: 2
                width: parent.height - 6
                height: width
                faSource: FontAwesome.fa_magnet
                faColor: "lightsteelblue"
                tooltip: qsTr("Snap to nearby items (hold Ctrl while dragging to suspend)")
                checkable: true
                checked: showManager.snapToItems
                onToggled: showManager.snapToItems = checked
            }

            IconButton
            {
                id: stretchBtn
                width: parent.height - 6
                height: width
                faSource: FontAwesome.fa_arrows_left_right_to_line
                faColor: "lightyellow"
                tooltip: qsTr("Stretch the original function")
                checkable: true
                checked: showManager.stretchFunctions
                onToggled: showManager.stretchFunctions = checked
            }

            IconButton
            {
                id: removeItem
                z: 2
                width: parent.height - 6
                height: width
                faSource: FontAwesome.fa_minus
                faColor: "crimson"
                tooltip: qsTr("Remove the selected items")
                counter: showManager.selectedItemsCount
                onClicked:
                {
                    var selNames = showManager.selectedItemNames()
                    //console.log(selNames)
                    deleteItemsPopup.message = qsTr("Are you sure you want to remove the following items?\n" +
                                                    "(Note that the original functions will not be deleted)") + "\n" + selNames
                    deleteItemsPopup.open()
                }

                CustomPopupDialog
                {
                    id: deleteItemsPopup
                    title: qsTr("Delete show items")
                    onAccepted: showManager.deleteShowItems(showManager.selectedItemRefs())
                }
            }

            IconButton
            {
                id: cutBtn
                width: parent.height - 6
                height: width
                faSource: FontAwesome.fa_scissors
                faColor: UISettings.fgMain
                tooltip: qsTr("Cut the selected items, to move them on paste (Ctrl+X)")
                counter: showManager.selectedItemsCount
                onClicked: showManager.cutToClipboard()
            }

            IconButton
            {
                id: copyBtn
                width: parent.height - 6
                height: width
                faSource: FontAwesome.fa_copy
                faColor: UISettings.fgMain
                tooltip: qsTr("Copy the selected items in the clipboard (Ctrl+C)")
                counter: showManager.selectedItemsCount
                onClicked: showManager.copyToClipboard()
            }

            IconButton
            {
                id: pasteBtn
                width: parent.height - 6
                height: width
                faSource: FontAwesome.fa_paste
                faColor: UISettings.fgMain
                tooltip: qsTr("Paste items in the clipboard at cursor position (Ctrl+V)")
                counter: showManager.clipboardItemsCount
                onClicked: showManager.pasteFromClipboard()

                CustomPopupDialog
                {
                    id: clipboardErrorPopup
                    standardButtons: Dialog.Ok
                }

                Connections
                {
                    target: showManager
                    function onClipboardActionFailed(title, message)
                    {
                        clipboardErrorPopup.title = title
                        clipboardErrorPopup.message = message
                        clipboardErrorPopup.open()
                    }
                }
            }

            // filler
            Rectangle
            {
                Layout.fillWidth: true
            }

            RobotoText
            {
                id: timeBox
                radius: height / 5
                border.color: UISettings.fgMedium
                border.width: 1
                leftMargin: UISettings.textSizeDefault
                rightMargin: UISettings.textSizeDefault
                property int currentTime: showManager.currentTime

                label: "00:00:00.00"

                onCurrentTimeChanged:
                {
                    label = TimeUtils.msToStringWithPrecision(currentTime, showManager.isPlaying ? 1 : 2)
                }
            }

            IconButton
            {
                id: playbackBtn
                width: parent.height - 6
                height: width
                faSource: (showManager.isPlaying && !showManager.isPaused) ? FontAwesome.fa_pause : FontAwesome.fa_play
                faColor: UISettings.fgMain
                bgColor: showManager.isPaused ? "green" :
                         (showManager.isPlaying ? "darkorange" : UISettings.bgLight)
                tooltip: (showManager.isPlaying && !showManager.isPaused) ? qsTr("Pause (Space)") : qsTr("Play or resume (Space)")
                checkable: false
                enabled: showManager.isEditing
                onClicked: showManager.playShow()
            }
            IconButton
            {
                id: stopBtn
                width: parent.height - 6
                height: width
                faSource: FontAwesome.fa_stop
                faColor: UISettings.fgMain
                bgColor: showManager.isPlaying ? "red" : UISettings.bgLight
                tooltip: qsTr("Stop or rewind (Esc)")
                checkable: false
                enabled: showManager.isEditing
                onClicked: showManager.stopShow()
            }

            // filler
            Rectangle
            {
                Layout.fillWidth: true
            }

            RobotoText
            {
                label: qsTr("Markers")
            }

            CustomComboBox
            {
                id: timeDivisionCombo
                model: [
                    { mLabel: qsTr("Time"), mValue: Show.Time },
                    { mLabel: qsTr("BPM 4/4"), mValue: Show.BPM_4_4 },
                    { mLabel: qsTr("BPM 3/4"), mValue: Show.BPM_3_4 },
                    { mLabel: qsTr("BPM 2/4"), mValue: Show.BPM_2_4 }
                ]
                enabled: showManager.isEditing
                currValue: showManager.timeDivision
                onValueChanged:
                {
                    if (currValue !== Show.Time &&
                            showManager.timeDivision === Show.Time &&
                            showManager.hasBeatBasedItems())
                    {
                        beatAlignWarningPopup.pendingDivision = currValue
                        beatAlignWarningPopup.open()
                    }
                    else
                    {
                        showManager.timeDivision = currValue
                    }
                }

                CustomPopupDialog
                {
                    id: beatAlignWarningPopup
                    title: qsTr("Switch to BPM markers")
                    message: qsTr("Warning: all beat-based functions will be aligned to the nearest beat")
                    standardButtons: Dialog.Ok | Dialog.Cancel

                    property var pendingDivision: Show.Time

                    // the OK/Cancel buttons only emit clicked(role) (see
                    // CustomPopupDialog's footer), while accepted()/rejected()
                    // only fire when confirming with the Enter key, so both
                    // paths must be handled to cover mouse and keyboard
                    onClicked: (role) =>
                    {
                        if (role === Dialog.Ok)
                            showManager.timeDivision = pendingDivision
                        else
                            timeDivisionCombo.currValue = showManager.timeDivision
                        close()
                    }
                    onAccepted: showManager.timeDivision = pendingDivision
                    onRejected: timeDivisionCombo.currValue = showManager.timeDivision
                }
            }

            ZoomItem
            {
                implicitWidth: UISettings.mediumItemHeight * 1.3
                implicitHeight: parent.height - 2
                fontColor: "#222"

                onZoomOutClicked:
                {
                    if (showManager.timeScale >= 1.0)
                        showManager.timeScale += 1.0
                    else
                        showManager.timeScale = Math.round((showManager.timeScale + 0.1) * 10) / 10
                    centerView()
                }

                onZoomInClicked:
                {
                    if (showManager.timeScale > 1.0)
                        showManager.timeScale -= 1.0
                    else
                        showManager.timeScale = Math.round((showManager.timeScale - 0.1) * 10) / 10
                    centerView()
                }
            }
        }
    } // top bar

    RightPanel
    {
        id: rightPanel
        x: parent.width - width
        z: 5
        height: parent.height - (bottomPanel.visible ? bottomPanel.height : 0)
        inShowManager: true
    }

    BottomPanel
    {
        id: bottomPanel
        objectName: "bottomPanelItem"
        y: parent.height - height
        z: 8
        visible: false
    }

    // top left area to perform time scaling
    Rectangle
    {
        y: topBar.height
        z: 5
        width: trackWidth + verticalDivider.width
        height: showMgrContainer.headerHeight
        color: UISettings.bgStrong

        RowLayout
        {
            anchors.fill: parent

            IconButton
            {
                visible: selectedTrackIndex > 0 ? true : false
                height: parent.height - 2
                width: height
                faSource: FontAwesome.fa_angle_up
                faColor: UISettings.fgMain
                tooltip: qsTr("Move the selected track up")
                onClicked:
                {
                    showManager.moveTrack(selectedTrackIndex, -1)
                    selectedTrackIndex--
                    renderAndCenter()
                }
            }

            IconButton
            {
                visible: selectedTrackIndex < tracksBox.count - 1 ? true : false
                height: parent.height - 2
                width: height
                faSource: FontAwesome.fa_angle_down
                faColor: UISettings.fgMain
                tooltip: qsTr("Move the selected track down")
                onClicked:
                {
                    showManager.moveTrack(selectedTrackIndex, 1)
                    selectedTrackIndex++
                    renderAndCenter()
                }
            }

            // layout filler
            Rectangle
            {
                Layout.fillWidth: true
                color: "transparent"
            }

            Rectangle
            {
                width: verticalDivider.width
                height: parent.height
                color: UISettings.bgLight
            }
        }
    }

    // the timeline header can be flicked horizontally but not vertically
    // so this is kept outside the main vertical flickable
    Flickable
    {
        id: timelineHeader
        x: trackWidth + verticalDivider.width
        y: topBar.height
        z: 4
        height: showMgrContainer.headerHeight
        // the right panel and the tracks column can together be wider than the
        // Show Manager itself (a narrow window, a different screen density or a
        // smaller UI scaling factor), which would make this width negative and
        // hand HeaderAndCursor a zero or negative visibleWidth to divide by
        width: Math.max(0, showMgrContainer.width - trackWidth - verticalDivider.width - rightPanel.width)

        boundsBehavior: Flickable.StopAtBounds
        flickableDirection: Flickable.HorizontalFlick

        contentWidth: hdrItem.width //> width ? hdrItem.width : width
        contentX: xViewOffset

        onContentXChanged: xViewOffset = contentX

        HeaderAndCursor
        {
            id: hdrItem
            z: 2
            height: parent.height
            visibleWidth: timelineHeader.width
            visibleX: xViewOffset
            headerHeight: showMgrContainer.headerHeight
            cursorHeight: showMgrContainer.height - topBar.height - (bottomPanel.visible ? bottomPanel.height : 0)
            duration: showManager.showDuration

            onClicked: (mouseX, mouseY) =>
            {
                if (timeDivision === Show.Time)
                    showManager.currentTime = TimeUtils.posToMs(mouseX, timeScale, tickSize)
                else
                    showManager.currentTime = TimeUtils.posToBeatMs(mouseX, tickSize, ioManager.bpmNumber, showManager.beatsDivision)
                showManager.resetItemsSelection()
            }
        }
    }

    // the main flickable area containing the tracks list and the Show items
    // this can be flicked only vertically
    Flickable
    {
        id: showContents
        y: topBar.height + headerHeight
        z: 3 // below timelineHeader
        width: parent.width - rightPanel.width
        height: showMgrContainer.height - topBar.height - headerHeight - (bottomPanel.visible ? bottomPanel.height : 0)
        clip: true

        boundsBehavior: Flickable.StopAtBounds
        flickableDirection: Flickable.VerticalFlick
        ScrollBar.vertical: CustomScrollBar { }

        contentHeight: totalTracksHeight > height ? totalTracksHeight : height
        //contentWidth: timelineHeader.contentWidth

        property real totalTracksHeight: (tracksBox.count + 2) * trackHeight

        Rectangle
        {
            width: trackWidth
            height: parent.height
            color: UISettings.bgMedium
            z: 2

            Column
            {
                width: trackWidth

                Repeater
                {
                    id: tracksBox
                    width: parent.width
                    model: showManager.tracks

                    delegate:
                        TrackDelegate
                        {
                            width: tracksBox.width
                            height: trackHeight
                            trackRef: modelData
                            isSelected: showMgrContainer.selectedTrackIndex === index ? true : false

                            onTrackSelected: showMgrContainer.selectedTrackIndex = index
                            onTrackDeselected: showMgrContainer.selectedTrackIndex = -1
                        }
                }
            }
        }

        // tracks/timeline vertical divider
        Rectangle
        {
            id: verticalDivider
            x: tracksBox.width
            z: 2
            width: 3
            height: parent.height
            color: UISettings.bgLight
        }

        // the Show items area. This can be flicked horizontally,
        // together with the timelineHeader flickable
        Flickable
        {
            id: itemsArea
            objectName: "showItemsArea"
            x: verticalDivider.x + verticalDivider.width
            z: 1
            width: showMgrContainer.width - trackWidth
            height: parent.height
            clip: true

            boundsBehavior: Flickable.StopAtBounds
            contentHeight: showContents.contentHeight
            contentWidth: timelineHeader.contentWidth
            contentX: xViewOffset
            ScrollBar.horizontal: horScrollBar

            onContentXChanged: xViewOffset = contentX

            /* Clicking on the timeline background moves the cursor and clears
               the items selection, while dragging flicks the timeline.
               In box selection mode, or while holding Ctrl, dragging draws
               a box instead, selecting the items lying entirely within it.
               Holding Shift as well extends the current selection.
               The box selection mode lasts for a single box, or until a
               click that doesn't draw one */
            MouseArea
            {
                id: timelineMouseArea
                anchors.fill: parent
                cursorShape: showManager.boxSelectMode ? Qt.CrossCursor : Qt.ArrowCursor

                // box corners, in content coordinates
                property real boxStartX: 0
                property real boxStartY: 0
                property real boxEndX: 0
                property real boxEndY: 0
                // true from the press until the release of a box selection
                property bool boxPressed: false
                // true once the mouse has moved far enough to draw a box
                property bool boxActive: false
                // the mouse X position relative to the visible timeline
                property real viewMouseX: 0
                // the clicked() signal that follows a box selection release
                property bool swallowClick: false

                readonly property real edgeWidth: UISettings.bigItemHeight * 0.6

                function updateBoxEnd(mouseX, mouseY)
                {
                    viewMouseX = mouseX - itemsArea.contentX
                    boxEndX = Math.max(0, Math.min(mouseX, itemsArea.contentWidth))
                    boxEndY = Math.max(0, Math.min(mouseY, itemsArea.contentHeight))
                }

                /* Scrolling speed in pixels per second. It grows as the mouse
                   gets closer to, or goes past, the visible timeline edges,
                   up to half of the visible timeline per second */
                function edgeScrollSpeed()
                {
                    var visibleWidth = timelineHeader.width
                    if (!boxActive || visibleWidth <= edgeWidth * 2)
                        return 0

                    var maxSpeed = visibleWidth / 2
                    if (viewMouseX < edgeWidth)
                        return -maxSpeed * Math.min(1.0, (edgeWidth - viewMouseX) / edgeWidth)
                    if (viewMouseX > visibleWidth - edgeWidth)
                        return maxSpeed * Math.min(1.0, (viewMouseX - visibleWidth + edgeWidth) / edgeWidth)
                    return 0
                }

                function endBoxSelection()
                {
                    boxPressed = false
                    boxActive = false
                    preventStealing = false
                }

                onPressed: (mouse) =>
                {
                    swallowClick = false
                    if (!showManager.boxSelectMode && !(mouse.modifiers & Qt.ControlModifier))
                        return

                    // don't let the Flickables steal the drag
                    preventStealing = true
                    boxPressed = true
                    boxActive = false
                    boxStartX = mouse.x
                    boxStartY = mouse.y
                    updateBoxEnd(mouse.x, mouse.y)
                }

                onPositionChanged: (mouse) =>
                {
                    if (!boxPressed)
                        return

                    if (!boxActive &&
                        Math.abs(mouse.x - boxStartX) < Qt.styleHints.startDragDistance &&
                        Math.abs(mouse.y - boxStartY) < Qt.styleHints.startDragDistance)
                        return

                    boxActive = true
                    updateBoxEnd(mouse.x, mouse.y)
                }

                onReleased: (mouse) =>
                {
                    if (!boxPressed)
                        return

                    if (boxActive)
                    {
                        updateBoxEnd(mouse.x, mouse.y)
                        showManager.selectItemsInRect(Qt.rect(Math.min(boxStartX, boxEndX), Math.min(boxStartY, boxEndY),
                                                              Math.abs(boxEndX - boxStartX), Math.abs(boxEndY - boxStartY)),
                                                      (mouse.modifiers & Qt.ShiftModifier) ? true : false)
                        showManager.boxSelectMode = false
                        swallowClick = true
                    }
                    else if (showManager.boxSelectMode)
                    {
                        // a click without a box just leaves the mode
                        showManager.boxSelectMode = false
                        swallowClick = true
                    }
                    endBoxSelection()
                }

                onCanceled: endBoxSelection()

                onClicked: (mouse) =>
                {
                    if (swallowClick)
                    {
                        swallowClick = false
                        return
                    }
                    showManager.currentTime = TimeUtils.posToMs(mouse.x, timeScale, tickSize)
                    showManager.resetItemsSelection()
                    // the timeline now owns the keyboard shortcuts (e.g. Ctrl+A)
                    showManager.itemClicked(App.ShowDragItem)
                }

                Timer
                {
                    interval: 16
                    repeat: true
                    running: timelineMouseArea.boxActive

                    // timers can fire late, so scroll by the time actually elapsed
                    property double lastTick: 0

                    onRunningChanged: lastTick = Date.now()

                    onTriggered:
                    {
                        var now = Date.now()
                        var step = timelineMouseArea.edgeScrollSpeed() * (now - lastTick) / 1000
                        lastTick = now
                        if (step === 0)
                            return

                        var maxX = Math.max(0, itemsArea.contentWidth - itemsArea.width)
                        var newX = Math.max(0, Math.min(itemsArea.contentX + step, maxX))
                        if (newX === itemsArea.contentX)
                            return

                        // the mouse stays still over the view while the content
                        // scrolls underneath it, so the box end follows the content
                        var viewX = timelineMouseArea.viewMouseX
                        xViewOffset = newX
                        timelineMouseArea.updateBoxEnd(newX + viewX, timelineMouseArea.boxEndY)
                    }
                }
            }

            /* Selection box. Its Canvas only covers the visible part of the
               box, which can grow much wider than the view while scrolling */
            Canvas
            {
                id: selectionBox
                z: 11
                visible: timelineMouseArea.boxActive

                property real boxLeft: Math.min(timelineMouseArea.boxStartX, timelineMouseArea.boxEndX)
                property real boxRight: Math.max(timelineMouseArea.boxStartX, timelineMouseArea.boxEndX)
                property real viewLeft: itemsArea.contentX - 2
                property real viewRight: itemsArea.contentX + timelineHeader.width + 2

                x: Math.max(boxLeft, viewLeft)
                y: Math.min(timelineMouseArea.boxStartY, timelineMouseArea.boxEndY)
                width: Math.max(1, Math.min(boxRight, viewRight) - x)
                height: Math.max(1, Math.abs(timelineMouseArea.boxEndY - timelineMouseArea.boxStartY))

                onXChanged: requestPaint()
                onWidthChanged: requestPaint()
                onHeightChanged: requestPaint()
                onVisibleChanged: if (visible) requestPaint()

                onPaint:
                {
                    var ctx = getContext("2d")
                    ctx.clearRect(0, 0, width, height)
                    ctx.fillStyle = Qt.rgba(UISettings.selection.r, UISettings.selection.g, UISettings.selection.b, 0.15)
                    ctx.fillRect(0, 0, width, height)

                    ctx.strokeStyle = UISettings.selection
                    ctx.lineWidth = 1
                    ctx.setLineDash([4, 4])
                    // keep the dashes still while the Canvas moves along the box
                    ctx.lineDashOffset = x - boxLeft

                    var r = width - 0.5
                    var b = height - 0.5
                    ctx.beginPath()
                    ctx.moveTo(0, 0.5)
                    ctx.lineTo(r, 0.5)
                    ctx.moveTo(0, b)
                    ctx.lineTo(r, b)
                    if (boxLeft >= viewLeft)
                    {
                        ctx.moveTo(0.5, 0)
                        ctx.lineTo(0.5, height)
                    }
                    if (boxRight <= viewRight)
                    {
                        ctx.moveTo(r, 0)
                        ctx.lineTo(r, height)
                    }
                    ctx.stroke()
                }
            }

            // track divider horizontal lines
            Repeater
            {
                model: tracksBox.count
                delegate:
                    Rectangle
                    {
                        height: 1
                        width: timelineHeader.contentWidth
                        y: (index * trackHeight) + trackHeight - 1
                        color: UISettings.bgLight
                    }
            }

            /** Vertical grid dividers, drawn with the same chunk strategy
              * and the same marker calculations of the timeline header:
              * the Canvas is 3 times the visible area and gets shifted
              * and repainted while flicking horizontally */
            Canvas
            {
                id: gridCanvas
                visible: showManager.gridEnabled
                z: 0
                x: -itemsArea.width
                y: 0
                width: itemsArea.width * 3
                height: itemsArea.contentHeight
                antialiasing: true
                contextType: "2d"

                property real tickSize: showManager.tickSize
                property int beatsDivision: showManager.beatsDivision

                function updatePosition()
                {
                    if (itemsArea.width <= 0)
                        return

                    var chunk = parseInt(xViewOffset / itemsArea.width) * itemsArea.width
                    gridCanvas.x = chunk - itemsArea.width
                    gridCanvas.requestPaint()
                }

                onTickSizeChanged: requestPaint()
                onBeatsDivisionChanged: requestPaint()
                onHeightChanged: requestPaint()
                onVisibleChanged: if (visible) updatePosition()

                Connections
                {
                    target: showMgrContainer

                    function onXViewOffsetChanged()
                    {
                        if (itemsArea.width <= 0)
                            return

                        if (xViewOffset < gridCanvas.x + itemsArea.width ||
                            xViewOffset > gridCanvas.x + (itemsArea.width * 2))
                            gridCanvas.updatePosition()
                    }
                }

                onPaint:
                {
                    var subDividers = showManager.beatsDivision

                    context.globalAlpha = 1.0
                    context.lineWidth = 1
                    context.strokeStyle = UISettings.bgLight
                    context.clearRect(0, 0, width, height)

                    if (tickSize <= 0)
                        return

                    var divNum = width / tickSize
                    var absPos = parseInt((x + width) / tickSize) * tickSize
                    var xPos = absPos - x

                    context.beginPath()

                    // paint dividers from the end to the beginning
                    for (var i = 0; i < divNum; i++)
                    {
                        // don't even bother to paint if we're outside the timeline
                        if (absPos >= 0)
                        {
                            if (subDividers > 1)
                            {
                                var subX = xPos - (tickSize / subDividers)
                                for (var sd = 0; sd < subDividers - 1; sd++)
                                {
                                    context.moveTo(subX, 0)
                                    context.lineTo(subX, height)
                                    subX -= (tickSize / subDividers)
                                }
                            }

                            context.moveTo(xPos, 0)
                            context.lineTo(xPos, height)
                        }
                        absPos -= tickSize
                        xPos -= tickSize
                    }
                    context.closePath()
                    context.stroke()
                }
            }

            /* Snap-to-item guide line */
            Rectangle
            {
                id: snapGuide
                x: showManager.snapGuideX
                y: 0
                z: 10
                width: 1
                height: parent.height
                color: "#00FF00"
                visible: showManager.snapGuideX >= 0
            }

            DropArea
            {
                id: newFuncDrop
                x: xViewOffset
                width: showMgrContainer.width - trackWidth
                height: tracksBox.count * trackHeight
                z: 2

                keys: [ "function" ]
                onDropped:
                {
                    console.log("Function items dropped here. x: " + drag.x + " y: " + drag.y)

                    /* Check if the dragging was started from a Function Manager */
                    if (drag.source.hasOwnProperty("fromFunctionManager"))
                    {
                        var trackIdx = (itemsArea.contentY + drag.y) / trackHeight
                        var fTime
                        if (showManager.timeDivision === Show.Time)
                            fTime = TimeUtils.posToMs(itemsArea.contentX + drag.x, timeScale, tickSize)
                        else
                            fTime = TimeUtils.posToBeat(itemsArea.contentX + drag.x, tickSize, showManager.beatsDivision)
                        console.log("Drop on time1: " + fTime)
                        showManager.addItems(itemsArea.contentItem, trackIdx, fTime, drag.source.itemsList)
                    }
/*
                    if (drag.source.funcID !== showID)
                    {
                        showManager.addItem(itemsArea.contentItem, trackIdx, fTime, drag.source.funcID)
                    }
                    else
                    {
                        var args = []
                        actionManager.requestActionPopup(ActionManager.None,
                                                         qsTr("Cannot drag a Show into itself!"),
                                                         ActionManager.OK, args)
                    }
*/
                }
            }

            Rectangle
            {
                id: newTrackBox
                x: xViewOffset
                y: tracksBox.count * trackHeight
                height: trackHeight
                width: itemsArea.width
                color: "transparent"
                radius: 10

                RobotoText
                {
                    id: ntText
                    visible: false
                    anchors.centerIn: parent
                    label: qsTr("Create a new track")
                }

                DropArea
                {
                    id: funcNewTrackDrop
                    anchors.fill: parent

                    keys: [ "function" ]

                    states: [
                        State
                        {
                            when: funcNewTrackDrop.containsDrag
                            PropertyChanges
                            {
                                target: newTrackBox
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
                        console.log("Function item dropped here. x: " + drag.x + " y: " + drag.y)

                        /* Check if the dragging was started from a Function Manager */
                        if (drag.source.hasOwnProperty("fromFunctionManager"))
                        {
                            var fTime

                            if (showManager.timeDivision === Show.Time)
                                fTime = TimeUtils.posToMs(xViewOffset + drag.x, timeScale, tickSize)
                            else
                                fTime = TimeUtils.posToBeat(xViewOffset + drag.x, tickSize, showManager.beatsDivision)

                            console.log("Drop on time2: " + fTime)
                            showManager.addItems(itemsArea.contentItem, -1, fTime, drag.source.itemsList)
                        }
                    }
                }
            }
        } // Flickable (horizontal)
    } // Flickable (vertical)

    Rectangle
    {
        anchors.centerIn: parent
        width: parent.width / 3
        height: UISettings.bigItemHeight
        visible: !showManager.isEditing
        radius: height / 4
        color: UISettings.bgLight

        RobotoText
        {
            anchors.centerIn: parent
            label: qsTr("Create/Edit a Show function or\ndrag a function on the timeline")
        }
    }

    CustomScrollBar
    {
        id: horScrollBar
        x: timelineHeader.x
        // keep it at the bottom of the Show items area, above the bottom panel
        y: showContents.y + showContents.height - height
        z: 10
        width: timelineHeader.width
        orientation: Qt.Horizontal
    }
}
