/*
  Q Light Controller Plus
  FixtureRemap.qml

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

Rectangle
{
    id: remapRoot
    anchors.fill: parent
    color: UISettings.bgMedium

    // Required by FixtureDrag.js so endDrag() can route drops here
    property string contextName: "REMAP"

    CustomPopupDialog
    {
        id: cloneErrorPopup
        standardButtons: Dialog.Ok
        title: qsTr("Error")
        message: qsTr("Cannot clone fixture: the target address range is already in use.\nRemove or move the conflicting fixture first.")
        onAccepted: close()
    }

    readonly property real rowH: UISettings.listItemHeight
    readonly property real canvasW: UISettings.bigItemHeight

    // Expansion state maps: fxId → true = expanded.  Absent = collapsed.
    property var srcExpanded: ({})
    property var tgtExpanded: ({})
    property var srcCanvasCache: ({ rows: {}, colors: {} })
    property var tgtCanvasCache: ({ rows: {}, colors: {} })
    property var tgtConnectedFixtures: ({})
    property var tgtConnectedChannels: ({})
    property int srcVisibleRows: 0
    property int tgtVisibleRows: 0

    // -------------------------------------------------------------------------
    // Helpers
    // -------------------------------------------------------------------------

    function hasSettings()
    {
        return false
    }

    function toggleExpanded(map, fxId)
    {
        var m = Object.assign({}, map)
        m[fxId] = !m[fxId]
        return m
    }

    function channelKey(fxId, chIdx)
    {
        return fxId + ":" + chIdx
    }

    function colorFromIcon(icon)
    {
        if (icon.indexOf("intensity") >= 0) return "#e8e04a"
        if (icon.indexOf("pan")       >= 0 ||
            icon.indexOf("tilt")      >= 0) return "#4ac8e8"
        if (icon.indexOf("color")     >= 0) return "#e84ac8"
        if (icon.indexOf("gobo")      >= 0) return "#c8e84a"
        if (icon.indexOf("shutter")   >= 0) return "#e8a84a"
        if (icon.indexOf("speed")     >= 0) return "#a84ae8"
        return "#aaaaaa"
    }

    function visibleRowCount(model, expandedMap)
    {
        var count = 0
        for (var i = 0; i < model.length; i++)
        {
            var e = model[i]
            if (e.isUniverse === true || e.isHeader === true || expandedMap[e.fxId] === true)
                count++
        }
        return count
    }

    /** Precompute channel row centres and colours once per list state change.
     *  This avoids rescanning the full source and target models for every
     *  connection on each canvas repaint. */
    function buildCanvasCache(listView, model, expandedMap)
    {
        var rows = {}
        var colors = {}
        var y = 0
        var headerCentres = {}
        var baseY = listView.y - listView.contentY

        for (var i = 0; i < model.length; i++)
        {
            var e = model[i]

            if (e.isUniverse === true)
            {
                y += rowH
                continue
            }

            if (e.isHeader === true)
            {
                headerCentres[e.fxId] = baseY + y + rowH * 0.5
                y += rowH
                continue
            }

            var key = channelKey(e.fxId, e.chIdx)
            var expanded = expandedMap[e.fxId] === true
            rows[key] = expanded
                    ? (baseY + y + rowH * 0.5)
                    : (headerCentres[e.fxId] !== undefined ? headerCentres[e.fxId] : baseY + y)
            colors[key] = colorFromIcon(e.chIcon || "")

            if (expanded)
                y += rowH
        }

        return { rows: rows, colors: colors }
    }

    function rebuildCanvasCaches()
    {
        srcCanvasCache = buildCanvasCache(srcList, fixtureRemapManager.sourceFixtures, srcExpanded)
        tgtCanvasCache = buildCanvasCache(tgtList, fixtureRemapManager.targetFixtures, tgtExpanded)

        bezierCanvas.requestPaint()
    }

    function rebuildConnectedFixtures()
    {
        var fixtures = {}
        var channels = {}
        var conns = fixtureRemapManager.connections
        for (var i = 0; i < conns.length; i++)
        {
            fixtures[conns[i].tgtFxId] = true
            channels[channelKey(conns[i].tgtFxId, conns[i].tgtCh)] = true
        }
        tgtConnectedFixtures = fixtures
        tgtConnectedChannels = channels
    }

    function rebuildScrollMetrics()
    {
        srcVisibleRows = visibleRowCount(fixtureRemapManager.sourceFixtures, srcExpanded)
        tgtVisibleRows = visibleRowCount(fixtureRemapManager.targetFixtures, tgtExpanded)
    }

    function rebuildLayoutState()
    {
        rebuildScrollMetrics()
        rebuildCanvasCaches()
    }

    function refreshConnectionsState()
    {
        rebuildConnectedFixtures()
        bezierCanvas.requestPaint()
    }

    function computedContentHeight(visibleRows)
    {
        return visibleRows * rowH
    }

    function scrollbarSize(listView, visibleRows)
    {
        var contentH = computedContentHeight(visibleRows)
        if (contentH <= 0 || contentH <= listView.height)
            return 1.0
        return Math.max(0.0, Math.min(1.0, listView.height / contentH))
    }

    function scrollbarPosition(listView, visibleRows)
    {
        var contentH = computedContentHeight(visibleRows)
        var maxY = Math.max(0, contentH - listView.height)
        if (maxY <= 0)
            return 0.0

        var size = scrollbarSize(listView, visibleRows)
        var maxPos = Math.max(0.0, 1.0 - size)
        if (maxPos <= 0)
            return 0.0

        return Math.max(0.0, Math.min(maxPos, (listView.contentY / maxY) * maxPos))
    }

    function applyScrollbarPosition(listView, visibleRows, position)
    {
        var contentH = computedContentHeight(visibleRows)
        var maxY = Math.max(0, contentH - listView.height)
        if (maxY <= 0)
        {
            listView.contentY = 0
            return
        }

        var size = scrollbarSize(listView, visibleRows)
        var maxPos = Math.max(0.0, 1.0 - size)
        if (maxPos <= 0)
        {
            listView.contentY = 0
            return
        }

        listView.contentY = Math.max(0, Math.min(maxY, (position / maxPos) * maxY))
    }

    // -------------------------------------------------------------------------
    // Top toolbar
    // -------------------------------------------------------------------------
    Rectangle
    {
        id: toolbar
        width: parent.width
        height: UISettings.iconSizeMedium
        z: 2
        gradient: Gradient
        {
            GradientStop { position: 0; color: UISettings.toolbarStartSub }
            GradientStop { position: 1; color: UISettings.toolbarEnd }
        }

        RowLayout
        {
            anchors.fill: parent
            spacing: 4

            RobotoText
            {
                Layout.leftMargin: 6
                label: qsTr("Fixture Remap")
                implicitHeight: toolbar.height
                fontSize: UISettings.textSizeDefault
                fontBold: true
                labelColor: UISettings.fgMain
            }

            Item { Layout.fillWidth: true }

            // Clone the selected source fixture into the target list and auto-connect
            IconButton
            {
                enabled: fixtureRemapManager.hasPendingSource
                height: toolbar.height - 4
                width: height * 3
                faSource: FontAwesome.fa_clone
                faColor: enabled ? "cyan" : UISettings.fgMedium
                tooltip: qsTr("Clone the selected source fixture and auto-connect its channels")
                onClicked:
                {
                    if (!fixtureRemapManager.cloneAndAutoRemap(fixtureRemapManager.pendingSourceFxId))
                        cloneErrorPopup.open()
                }
            }

            IconButton
            {
                Layout.rightMargin: 8
                height: toolbar.height - 4
                width: height * 3
                faSource: FontAwesome.fa_check
                faColor: "limegreen"
                tooltip: qsTr("Apply remap and save project")
                onClicked: fixtureRemapManager.applyRemap()
            }

            IconButton
            {
                Layout.rightMargin: 4
                height: toolbar.height - 4
                width: height * 2.5
                faSource: FontAwesome.fa_xmark
                faColor: UISettings.fgMain
                tooltip: qsTr("Cancel — discard and go back")
                onClicked:
                {
                    fixtureRemapManager.reset()
                    fixtureAndFunctions.currentViewQML = previousView
                }
            }
        }
    }

    // -------------------------------------------------------------------------
    // Main three-column layout
    // -------------------------------------------------------------------------
    RowLayout
    {
        anchors.top: toolbar.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        spacing: 0

        // ---- Left: source fixtures ----
        Rectangle
        {
            id: srcPanel
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: UISettings.bgStrong

            Rectangle
            {
                id: srcHeader
                width: parent.width
                height: UISettings.listItemHeight
                color: UISettings.sectionHeader

                RobotoText
                {
                    anchors.centerIn: parent
                    label: qsTr("Source — current project fixtures")
                    fontSize: UISettings.textSizeDefault
                    fontBold: true
                }
            }

            ListView
            {
                id: srcList
                anchors.top: srcHeader.bottom
                anchors.bottom: parent.bottom
                anchors.left: parent.left
                anchors.leftMargin: srcScrollBar.visible ? UISettings.scrollBarWidth : 0
                anchors.right: parent.right
                clip: true
                boundsBehavior: Flickable.StopAtBounds

                model: fixtureRemapManager.sourceFixtures
                contentHeight: computedContentHeight(srcVisibleRows)
                cacheBuffer: 0
                pixelAligned: true

                onContentYChanged: rebuildCanvasCaches()

                delegate: RemapRowDelegate
                {
                    width: srcList.width
                    height: (modelData.isUniverse || modelData.isHeader || srcExpanded[modelData.fxId] === true)
                            ? rowH : 0
                    rowData: modelData
                    isExpanded: srcExpanded[modelData.fxId] === true
                    isSelectedFixture: modelData.isHeader &&
                                       fixtureRemapManager.hasPendingSource &&
                                       fixtureRemapManager.pendingSourceFxId === modelData.fxId
                    isPending: !modelData.isHeader && !modelData.isUniverse &&
                               fixtureRemapManager.pendingSourceFxId === modelData.fxId &&
                               fixtureRemapManager.pendingSourceCh  === modelData.chIdx

                    // Single click on fixture header: select as pending remap source
                    onHeaderClicked: (fxId) =>
                    {
                        if (fixtureRemapManager.hasPendingSource &&
                            fixtureRemapManager.pendingSourceFxId === fxId)
                            fixtureRemapManager.setPendingSource(-1, -1)  // deselect
                        else
                            fixtureRemapManager.setPendingSource(fxId, -1)
                    }

                    // Chevron click: expand / collapse channels
                    onHeaderExpandToggled: (fxId) =>
                    {
                        srcExpanded = toggleExpanded(srcExpanded, fxId)
                        rebuildLayoutState()
                    }

                    onChannelClicked: (fxId, chIdx) =>
                    {
                        if (fixtureRemapManager.pendingSourceFxId === fxId &&
                            fixtureRemapManager.pendingSourceCh   === chIdx)
                            fixtureRemapManager.setPendingSource(-1, -1)  // deselect
                        else
                            fixtureRemapManager.setPendingSource(fxId, chIdx)
                    }
                }
            }

            CustomScrollBar
            {
                id: srcScrollBar
                parent: srcPanel
                x: 0
                y: srcList.y
                height: srcList.height
                size: scrollbarSize(srcList, srcVisibleRows)

                Binding on position
                {
                    when: !srcScrollBar.pressed
                    value: scrollbarPosition(srcList, srcVisibleRows)
                }

                onPositionChanged:
                {
                    if (pressed)
                        applyScrollbarPosition(srcList, srcVisibleRows, position)
                }
            }
        }

        // ---- Centre: bezier canvas ----
        Canvas
        {
            id: bezierCanvas
            Layout.preferredWidth: canvasW
            Layout.fillHeight: true
            //renderStrategy: Canvas.Threaded

            Connections
            {
                target: fixtureRemapManager
                function onConnectionsChanged()     { refreshConnectionsState() }
                function onSourceFixturesChanged()  { rebuildConnectedFixtures(); rebuildLayoutState() }
                function onTargetFixturesChanged()  { rebuildConnectedFixtures(); rebuildLayoutState() }
                function onRemapApplied()           { fixtureAndFunctions.currentViewQML = previousView }
            }

            onPaint:
            {
                var ctx = getContext("2d")
                ctx.clearRect(0, 0, width, height)

                var conns = fixtureRemapManager.connections
                var srcRows = srcCanvasCache.rows || {}
                var tgtRows = tgtCanvasCache.rows || {}
                var srcColors = srcCanvasCache.colors || {}

                for (var i = 0; i < conns.length; i++)
                {
                    var c = conns[i]
                    var sy = srcRows[channelKey(c.srcFxId, c.srcCh)]
                    var ty = tgtRows[channelKey(c.tgtFxId, c.tgtCh)]

                    if (sy === undefined || ty === undefined)
                        continue
                    if ((sy < 0 || sy > height) && (ty < 0 || ty > height))
                        continue

                    ctx.beginPath()
                    ctx.strokeStyle = srcColors[channelKey(c.srcFxId, c.srcCh)] || "#aaaaaa"
                    ctx.lineWidth = 1.5
                    ctx.moveTo(0, sy)
                    ctx.bezierCurveTo(canvasW * 0.4, sy,
                                      canvasW * 0.6, ty,
                                      canvasW, ty)
                    ctx.stroke()
                }
            }
        }

        // ---- Right: target fixtures ----
        Rectangle
        {
            id: tgtPanel
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: UISettings.bgStrong

            Rectangle
            {
                id: tgtHeader
                width: parent.width
                height: UISettings.listItemHeight
                color: UISettings.sectionHeader

                RobotoText
                {
                    anchors.centerIn: parent
                    label: qsTr("Target — drop new fixtures here")
                    fontSize: UISettings.textSizeDefault
                    fontBold: true
                }
            }

            ListView
            {
                id: tgtList
                anchors.top: tgtHeader.bottom
                anchors.bottom: parent.bottom
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.rightMargin: tgtScrollBar.visible ? UISettings.scrollBarWidth : 0
                clip: true
                boundsBehavior: Flickable.StopAtBounds

                model: fixtureRemapManager.targetFixtures
                contentHeight: computedContentHeight(tgtVisibleRows)
                cacheBuffer: 0
                pixelAligned: true

                onContentYChanged: rebuildCanvasCaches()

                // Drop hint when list is empty
                Rectangle
                {
                    visible: fixtureRemapManager.targetFixtures.length === 0
                    anchors.fill: parent
                    color: "transparent"
                    border.color: tgtDropArea.containsDrag
                                  ? UISettings.activeDropArea : UISettings.borderColorDark
                    border.width: 2
                    radius: 4

                    Column
                    {
                        anchors.centerIn: parent
                        spacing: 8

                        Text
                        {
                            anchors.horizontalCenter: parent.horizontalCenter
                            color: UISettings.fgMedium
                            font.family: UISettings.fontAwesomeFontName
                            font.pixelSize: 48
                            text: FontAwesome.fa_circle_arrow_down
                        }

                        RobotoText
                        {
                            anchors.horizontalCenter: parent.horizontalCenter
                            label: qsTr("Drag fixtures from the browser\ninto this area")
                            //wrapText: true
                            textHAlign: Text.AlignHCenter
                            labelColor: UISettings.fgMedium
                            fontSize: UISettings.textSizeDefault
                        }
                    }
                }

                delegate: RemapRowDelegate
                {
                    width: tgtList.width
                    height: (modelData.isUniverse || modelData.isHeader || tgtExpanded[modelData.fxId] === true)
                            ? rowH : 0
                    rowData: modelData
                    isTarget: true
                    isExpanded: tgtExpanded[modelData.fxId] === true
                    isPending: false
                    hasConnections: modelData.isHeader &&
                                    tgtConnectedFixtures[modelData.fxId] === true
                    isChannelConnected: !modelData.isHeader && !modelData.isUniverse &&
                                        tgtConnectedChannels[channelKey(modelData.fxId, modelData.chIdx)] === true

                    // Single click on target fixture header:
                    // if a source fixture is pending → fixture-to-fixture auto-connect
                    onHeaderClicked: (fxId) =>
                    {
                        if (fixtureRemapManager.hasPendingSource)
                            fixtureRemapManager.connectFixtureToFixture(fxId)
                    }

                    onHeaderExpandToggled: (fxId) =>
                    {
                        tgtExpanded = toggleExpanded(tgtExpanded, fxId)
                        rebuildLayoutState()
                    }

                    onChannelClicked: (fxId, chIdx) =>
                    {
                        fixtureRemapManager.connectToTarget(fxId, chIdx)
                    }

                    onDisconnectClicked: (fxId) =>
                    {
                        fixtureRemapManager.removeConnectionsForFixture(fxId)
                    }

                    onDisconnectChannelClicked: (fxId, chIdx) =>
                    {
                        // Find and remove the connection that targets this channel
                        var conns = fixtureRemapManager.connections
                        for (var i = 0; i < conns.length; i++)
                        {
                            if (conns[i].tgtFxId === fxId && conns[i].tgtCh === chIdx)
                            {
                                fixtureRemapManager.removeConnection(i)
                                break
                            }
                        }
                    }

                    onRemoveClicked: (fxId) =>
                    {
                        fixtureRemapManager.removeTargetFixture(fxId)
                    }
                }
            }

            DropArea
            {
                id: tgtDropArea
                anchors.fill: parent
                keys: [ "fixture" ]

                onDropped: (drag) =>
                {
                    if (drag.source && drag.source.hasOwnProperty("itemsList"))
                    {
                        for (var i = 0; i < drag.source.itemsList.length; i++)
                        {
                            var item = drag.source.itemsList[i]
                            if (item.itemType === App.FixtureDragItem)
                            {
                                var fxi = item.cRef
                                if (fxi)
                                    fixtureRemapManager.addTargetFixture(
                                        fxi.fixtureDef ? fxi.fixtureDef.manufacturer : "",
                                        fxi.fixtureDef ? fxi.fixtureDef.model : "",
                                        fxi.fixtureMode ? fxi.fixtureMode.name : "",
                                        fxi.name, fxi.universe, fxi.address, 1, 0)
                            }
                        }
                    }
                }
            }

            CustomScrollBar
            {
                id: tgtScrollBar
                parent: tgtPanel
                x: tgtPanel.width - width
                y: tgtList.y
                height: tgtList.height
                size: scrollbarSize(tgtList, tgtVisibleRows)

                Binding on position
                {
                    when: !tgtScrollBar.pressed
                    value: scrollbarPosition(tgtList, tgtVisibleRows)
                }

                onPositionChanged:
                {
                    if (pressed)
                        applyScrollbarPosition(tgtList, tgtVisibleRows, position)
                }
            }
        }
    }

    Component.onCompleted:
    {
        rebuildLayoutState()
        rebuildConnectedFixtures()
    }
}
