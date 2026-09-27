/*
  Q Light Controller Plus
  VCXYPadItem.qml

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

VCWidgetItem
{
    id: xyPadRoot
    property VCXYPad xyPadObj: null
    property point currPosition: xyPadObj ? xyPadObj.currentPosition : Qt.point(0, 0)
    property point horizRange: xyPadObj ? xyPadObj.horizontalRange : Qt.point(0, 255)
    property point vertRange: xyPadObj ? xyPadObj.verticalRange : Qt.point(0, 255)
    property var fixturePositions: xyPadObj ? xyPadObj.fixturePositions : []

    property bool floorControl: xyPadObj ? xyPadObj.floorControl : false
    property vector3d floorPosition: xyPadObj ? xyPadObj.floorPosition : Qt.vector3d(0, 0, 0)
    /* The environment size has no change notification of its own, so it is
       re-read whenever this widget shows up or floor mode is toggled */
    property vector3d floorSize: Qt.vector3d(10, 10, 10)

    function refreshFloorSize()
    {
        if (xyPadObj)
            floorSize = xyPadObj.floorSize
    }

    onVisibleChanged: refreshFloorSize()
    onFloorControlChanged: refreshFloorSize()
    property real floorHeightMax: xyPadObj ? xyPadObj.floorHeightMax : 20
    property real floorHeightStep: xyPadObj ? xyPadObj.floorHeightStep : 0.5

    function presetIcon(type)
    {
        switch (type)
        {
            case "Scene": return "qrc:/scene.svg"
            case "EFX": return "qrc:/efx.svg"
            case "FixtureGroup": return "qrc:/group.svg"
            default: return "qrc:/position.svg"
        }
    }

    clip: true

    onXyPadObjChanged:
    {
        setCommonProperties(xyPadObj)
        refreshFloorSize()
    }

    GridLayout
    {
        id: itemsLayout
        anchors.fill: parent
        rowSpacing: 0
        columnSpacing: 0
        columns: 3

        /* Range sliders. In floor mode they limit the reachable portion of
           the stage rather than the Pan/Tilt travel, but they are driven by
           the same 0-255 window, so they stay visible in both modes. */

        // row 1
        Rectangle
        {
            Layout.row: 0
            Layout.column: 0
            height: UISettings.listItemHeight
            width: height
            color: "transparent"
        }

        CustomRangeSlider
        {
            Layout.row: 0
            Layout.column: 1
            Layout.fillWidth: true
            topPadding: 0
            bottomPadding: 0
            from: 0
            to: 255
            bgColor: "turquoise"
            first.value: horizRange.x
            second.value: horizRange.y
            first.onMoved: if (xyPadObj) xyPadObj.horizontalRange = Qt.point(first.value, second.value)
            second.onMoved: if (xyPadObj) xyPadObj.horizontalRange = Qt.point(first.value, second.value)
        }

        Rectangle
        {
            Layout.row: 0
            Layout.column: 2
            height: UISettings.listItemHeight
            width: height
            color: "transparent"
        }

        // row 2
        CustomRangeSlider
        {
            Layout.row: 1
            Layout.column: 0
            Layout.fillHeight: true
            rightPadding: 0
            orientation: Qt.Vertical
            // in floor mode the Z axis grows downwards (towards the audience),
            // matching the pad area, so the slider is not flipped
            rotation: xyPadRoot.floorControl ? 0 : 180
            from: 0
            to: 255
            bgColor: "turquoise"
            first.value: vertRange.x
            second.value: vertRange.y
            first.onMoved: if (xyPadObj) xyPadObj.verticalRange = Qt.point(first.value, second.value)
            second.onMoved: if (xyPadObj) xyPadObj.verticalRange = Qt.point(first.value, second.value)
        }

        // center area
        Rectangle
        {
            id: previewArea
            Layout.row: 1
            Layout.column: 1
            Layout.fillHeight: true
            Layout.fillWidth: true
            clip: true
            color: UISettings.bgStrong

            /* Range window. The same 0-255 window drives both modes, so the
               geometry is identical: in floor mode it marks the reachable
               portion of the stage. It is filled in Pan/Tilt mode, but only
               outlined in floor mode so the stage grid stays readable. */
            Rectangle
            {
                id: rangeWindow
                property bool limited: horizRange.x !== 0 || horizRange.y !== 255 ||
                                       vertRange.x !== 0 || vertRange.y !== 255

                visible: limited && !xyPadRoot.floorControl
                x: (horizRange.x * previewArea.width) / 255.0
                y: (vertRange.x * previewArea.height) / 255.0
                width: ((horizRange.y * previewArea.width) / 255.0) - x
                height: ((vertRange.y * previewArea.height) / 255.0) - y
                color: "darkcyan"
                border.width: 1
                border.color: "cyan"
                opacity: 0.5
            }

            // Cursor indicator
            Repeater
            {
                model: xyPadRoot.floorControl ? [] : fixturePositions

                Rectangle
                {
                    width: UISettings.iconSizeMedium * 0.4
                    height: width
                    radius: width / 2
                    x: ((Math.min(modelData.x, 255.0) * previewArea.width) / 255.0) - (width / 2)
                    y: ((Math.min(modelData.y, 255.0) * previewArea.height) / 255.0) - (height / 2)
                    color: "#FFD95A"
                    border.width: 1
                    border.color: "#5E4A00"
                    opacity: 0.9
                }
            }

            /* Floor control: the area represents the stage floor seen from
               above. X grows to the right, Z grows towards the audience
               (bottom of the area). */
            Canvas
            {
                id: floorGrid
                anchors.fill: parent
                visible: xyPadRoot.floorControl
                antialiasing: true

                property real stageWidth: xyPadRoot.floorSize.x
                property real stageDepth: xyPadRoot.floorSize.z

                onStageWidthChanged: requestPaint()
                onStageDepthChanged: requestPaint()
                onVisibleChanged: requestPaint()

                onPaint:
                {
                    var ctx = getContext("2d")
                    ctx.reset()

                    if (stageWidth <= 0 || stageDepth <= 0)
                        return

                    // one line per metre, brighter every 5 metres
                    ctx.lineWidth = 1

                    for (var mx = 0; mx <= stageWidth; mx++)
                    {
                        var px = (mx * width) / stageWidth
                        ctx.strokeStyle = (mx % 5 === 0) ? "#5A6B7A" : "#39434D"
                        ctx.beginPath()
                        ctx.moveTo(px, 0)
                        ctx.lineTo(px, height)
                        ctx.stroke()
                    }

                    for (var mz = 0; mz <= stageDepth; mz++)
                    {
                        var pz = (mz * height) / stageDepth
                        ctx.strokeStyle = (mz % 5 === 0) ? "#5A6B7A" : "#39434D"
                        ctx.beginPath()
                        ctx.moveTo(0, pz)
                        ctx.lineTo(width, pz)
                        ctx.stroke()
                    }
                }
            }

            /* Floor mode range window: the area outside it is dimmed and the
               reachable region outlined, so the stage grid stays visible */
            Item
            {
                anchors.fill: parent
                visible: xyPadRoot.floorControl && rangeWindow.limited

                Rectangle
                {
                    id: floorRangeRect
                    x: rangeWindow.x
                    y: rangeWindow.y
                    width: rangeWindow.width
                    height: rangeWindow.height
                    color: "transparent"
                    border.width: 1
                    border.color: "cyan"
                }

                // dim the four bands around the reachable area
                Rectangle
                {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: parent.top
                    height: floorRangeRect.y
                    color: "#99000000"
                }
                Rectangle
                {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom
                    y: floorRangeRect.y + floorRangeRect.height
                    height: parent.height - y
                    color: "#99000000"
                }
                Rectangle
                {
                    anchors.left: parent.left
                    y: floorRangeRect.y
                    width: floorRangeRect.x
                    height: floorRangeRect.height
                    color: "#99000000"
                }
                Rectangle
                {
                    x: floorRangeRect.x + floorRangeRect.width
                    y: floorRangeRect.y
                    width: parent.width - x
                    height: floorRangeRect.height
                    color: "#99000000"
                }
            }

            // stage front edge marker (audience side)
            Rectangle
            {
                visible: xyPadRoot.floorControl
                anchors.bottom: parent.bottom
                width: parent.width
                height: 2
                color: "#B0783C"
            }

            // floor target marker, centered on the targeted floor coordinate
            Item
            {
                id: floorCursor
                visible: xyPadRoot.floorControl
                width: UISettings.iconSizeMedium * 1.4
                height: width
                x: (xyPadRoot.floorSize.x > 0
                    ? (xyPadRoot.floorPosition.x * previewArea.width) / xyPadRoot.floorSize.x : 0)
                   - (width / 2)
                y: (xyPadRoot.floorSize.z > 0
                    ? (xyPadRoot.floorPosition.z * previewArea.height) / xyPadRoot.floorSize.z : 0)
                   - (height / 2)

                // crosshair
                Rectangle
                {
                    anchors.centerIn: parent
                    width: parent.width
                    height: 1
                    color: UISettings.highlight
                    opacity: 0.7
                }

                Rectangle
                {
                    anchors.centerIn: parent
                    width: 1
                    height: parent.height
                    color: UISettings.highlight
                    opacity: 0.7
                }

                // the marker grows with the target height, to hint that the
                // aiming point is lifted above the floor
                Rectangle
                {
                    anchors.centerIn: parent
                    width: UISettings.iconSizeMedium *
                           (0.5 + (0.5 * xyPadRoot.floorPosition.y / xyPadRoot.floorHeightMax))
                    height: width
                    radius: width / 2
                    color: "transparent"
                    border.width: 2
                    border.color: UISettings.highlight
                    opacity: xyPadRoot.floorPosition.y > 0 ? 0.9 : 0
                }

                Rectangle
                {
                    anchors.centerIn: parent
                    width: UISettings.iconSizeMedium * 0.5
                    height: width
                    radius: width / 2
                    color: UISettings.highlight
                    border.width: 1
                    border.color: UISettings.highlightPressed
                }
            }

            // floor target coordinates
            RobotoText
            {
                visible: xyPadRoot.floorControl
                anchors.left: parent.left
                anchors.top: parent.top
                anchors.margins: 3
                fontSize: Math.round(UISettings.textSizeDefault * 0.75)
                labelColor: "#C8D2DC"
                label: qsTr("X: %1m  Z: %2m  H: %3m")
                       .arg(xyPadRoot.floorPosition.x.toFixed(1))
                       .arg(xyPadRoot.floorPosition.z.toFixed(1))
                       .arg(xyPadRoot.floorPosition.y.toFixed(1))
            }

            Rectangle
            {
                id: cursor
                visible: !xyPadRoot.floorControl
                // previewArea.width : 255 = x : currPosition.x
                x: ((currPosition.x * previewArea.width) / 255.0) - (width / 2)
                y: ((currPosition.y * previewArea.height) / 255.0) - (height / 2)
                width: UISettings.iconSizeMedium * 0.5
                height: width
                radius: width / 2
                color: UISettings.highlight
                border.width: 1
                border.color: UISettings.highlightPressed
            }

            MouseArea
            {
                anchors.fill: parent
                hoverEnabled: true

                function clamp(num, min, max)
                {
                  return Math.min(Math.max(num, min), max);
                }

                function getXYPosition(mouse)
                {
                    var x = clamp(mouse.x, rangeWindow.x, rangeWindow.x + rangeWindow.width)
                    var y = clamp(mouse.y, rangeWindow.y, rangeWindow.y + rangeWindow.height)
                    return Qt.point((x * 255.0) / previewArea.width,
                                    (y * 255.0) / previewArea.height)
                }

                // map a point of the pad area to a stage floor coordinate,
                // keeping the current target height. The range window limits
                // the reachable portion of the stage, just like it limits the
                // Pan/Tilt travel in the other mode.
                function getFloorPosition(mouse)
                {
                    var fx = clamp(mouse.x, rangeWindow.x, rangeWindow.x + rangeWindow.width)
                    var fz = clamp(mouse.y, rangeWindow.y, rangeWindow.y + rangeWindow.height)
                    return Qt.vector3d((fx * xyPadRoot.floorSize.x) / previewArea.width,
                                       xyPadRoot.floorPosition.y,
                                       (fz * xyPadRoot.floorSize.z) / previewArea.height)
                }

                function updatePosition(mouse)
                {
                    if (!xyPadObj)
                        return

                    if (xyPadRoot.floorControl)
                        xyPadObj.floorPosition = getFloorPosition(mouse)
                    else
                        xyPadObj.currentPosition = getXYPosition(mouse)
                }

                onPressed: (mouse) =>
                {
                    virtualConsole.enableFlicking(false)
                    updatePosition(mouse)
                }

                onPositionChanged: (mouse) =>
                {
                    if (pressed)
                        updatePosition(mouse)
                }
                onReleased: virtualConsole.enableFlicking(true)
            }
        }

        /* Right column: Tilt in normal mode, target height (Y axis) in
           floor mode. The stage depth (Z) is driven by the pad area and by
           the horizontal slider below, so it needs no dedicated fader. */
        CustomSlider
        {
            id: ySlider
            Layout.row: 1
            Layout.column: 2
            visible: !xyPadRoot.floorControl
            Layout.fillHeight: true
            orientation: Qt.Vertical
            from: 0
            to: 255
            value: to - currPosition.y
            onMoved: if (xyPadObj) xyPadObj.currentPosition = Qt.point(xSlider.value, to - ySlider.value)
        }

        // floor target height (Y axis), 0 .. 20 m with 0.5 m steps
        ColumnLayout
        {
            Layout.row: 1
            Layout.column: 2
            visible: xyPadRoot.floorControl
            Layout.fillHeight: true
            spacing: 2

            RobotoText
            {
                Layout.alignment: Qt.AlignHCenter
                height: UISettings.listItemHeight * 0.8
                fontSize: Math.round(UISettings.textSizeDefault * 0.7)
                label: xyPadRoot.floorPosition.y.toFixed(1) + "m"
            }

            CustomSlider
            {
                id: heightSlider
                Layout.fillHeight: true
                Layout.alignment: Qt.AlignHCenter
                orientation: Qt.Vertical
                from: 0
                to: xyPadRoot.floorHeightMax
                stepSize: xyPadRoot.floorHeightStep
                snapMode: Slider.SnapAlways
                value: xyPadRoot.floorPosition.y
                onMoved:
                {
                    if (xyPadObj)
                        xyPadObj.floorPosition = Qt.vector3d(xyPadRoot.floorPosition.x,
                                                             value,
                                                             xyPadRoot.floorPosition.z)
                }
            }
        }

        // row 3
        Rectangle
        {
            Layout.row: 2
            Layout.column: 0
            visible: !xyPadRoot.floorControl
            height: UISettings.listItemHeight
            width: height
            color: "transparent"
        }

        // Pan slider. In floor mode the X axis is driven by the pad area and
        // limited by the range slider on top, so no fader is needed here
        CustomSlider
        {
            id: xSlider
            Layout.row: 2
            Layout.column: 1
            visible: !xyPadRoot.floorControl
            Layout.fillWidth: true
            from: 0
            to: 255
            value: currPosition.x
            onMoved: if (xyPadObj) xyPadObj.currentPosition = Qt.point(xSlider.value, ySlider.to - ySlider.value)
        }

        Rectangle
        {
            Layout.row: 2
            Layout.column: 2
            visible: !xyPadRoot.floorControl
            height: UISettings.listItemHeight
            width: height
            color: "transparent"
        }

        Flow
        {
            Layout.row: 3
            Layout.column: 0
            Layout.columnSpan: itemsLayout.columns
            Layout.fillWidth: true
            spacing: 4
            visible: xyPadObj && xyPadObj.presetsList && xyPadObj.presetsList.length > 0

            Repeater
            {
                model: xyPadObj ? xyPadObj.presetsList : []

                GenericButton
                {
                    height: UISettings.listItemHeight
                    width: Math.max(Math.ceil(height * 1.2), contentWidth + 5)
                    iconSource: xyPadRoot.presetIcon(modelData.typeString)
                    label: modelData.name
                    fontSize: Math.round(UISettings.textSizeDefault * 0.8)
                    bgColor: modelData.active ? UISettings.highlight : UISettings.bgControl
                    onClicked:
                    {
                        if (xyPadObj)
                            xyPadObj.applyPreset(modelData.id)
                    }
                }
            }
        }
    }
}
