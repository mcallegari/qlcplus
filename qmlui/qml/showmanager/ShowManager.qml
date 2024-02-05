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

import QtQuick 2.0
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.1

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

    onShowIDChanged: renderAndCenter()
    Component.onCompleted: renderAndCenter()

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

                onTextChanged: showManager.showName = text
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

                    onColorChanged: showManager.itemsColor = Qt.rgba(r, g, b, 1.0)
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
                id: stretchBtn
                width: parent.height - 6
                height: width
                imgSource: "qrc:/stretch.svg"
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
                imgSource: "qrc:/remove.svg"
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
                id: copyBtn
                width: parent.height - 6
                height: width
                imgSource: "qrc:/edit-copy.svg"
                tooltip: qsTr("Copy the selected items in the clipboard")
                counter: showManager.selectedItemsCount
                onClicked: showManager.copyToClipboard()
            }

            IconButton
            {
                id: pasteBtn
                width: parent.height - 6
                height: width
                imgSource: "qrc:/edit-paste.svg"
                tooltip: qsTr("Paste items in the clipboard at cursor position")
                counter: showManager.selectedItemsCount
                onClicked: showManager.pasteFromClipboard()
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
                imgSource: "qrc:/play.svg"
                tooltip: qsTr("Play or resume")
                checkable: true
                enabled: showManager.isEditing
                onToggled:
                {
                    if (checked)
                        showManager.playShow()
                    else
                        showManager.stopShow()
                }
            }
            IconButton
            {
                id: stopBtn
                width: parent.height - 6
                height: width
                imgSource: "qrc:/stop.svg"
                tooltip: qsTr("Stop or rewind")
                checkable: false
                enabled: showManager.isEditing
                onClicked:
                {
                    playbackBtn.checked = false
                    showManager.stopShow()
                }
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
                ListModel
                {
                    id: divModel
                    ListElement { mLabel: qsTr("Time"); mValue: Show.Time }
                    ListElement { mLabel: qsTr("BPM 4/4"); mValue: Show.BPM_4_4 }
                    ListElement { mLabel: qsTr("BPM 3/4"); mValue: Show.BPM_3_4 }
                    ListElement { mLabel: qsTr("BPM 2/4"); mValue: Show.BPM_2_4 }
                }

                model: divModel
                enabled: showManager.isEditing
                currValue: showManager.timeDivision
                onValueChanged: showManager.timeDivision = currentValue
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
                        showManager.timeScale += 0.1
                    centerView()
                }

                onZoomInClicked:
                {
                    if (showManager.timeScale > 1.0)
                        showManager.timeScale -= 1.0
                    else
                        showManager.timeScale -= 0.1
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
                visible: showManager.selectedTrackIndex > 0 ? true : false
                height: parent.height - 2
                width: height
                imgSource: "qrc:/up.svg"
                tooltip: qsTr("Move the selected track up")
                onClicked:
                {
                    showManager.moveTrack(showManager.selectedTrackIndex, -1)
                    showManager.selectedTrackIndex--
                    renderAndCenter()
                }
            }

            IconButton
            {
                visible: showManager.selectedTrackIndex < tracksBox.count - 1 ? true : false
                height: parent.height - 2
                width: height
                imgSource: "qrc:/down.svg"
                tooltip: qsTr("Move the selected track down")
                onClicked:
                {
                    showManager.moveTrack(showManager.selectedTrackIndex, 1)
                    showManager.selectedTrackIndex++
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
        width: showMgrContainer.width - trackWidth - verticalDivider.width - rightPanel.width

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

            onClicked:
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
                            trackIndex: index
                            isSelected: showManager.selectedTrackIndex === index ? true : false
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

            MouseArea
            {
                anchors.fill: parent
                onClicked:
                {
                    showManager.currentTime = TimeUtils.posToMs(mouse.x, timeScale, tickSize)
                    showManager.resetItemsSelection()
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

            HeaderAndCursor
            {
                id: gridItem
                visible: showManager.gridEnabled
                z: 2
                height: parent.height
                visibleWidth: itemsArea.width
                visibleX: xViewOffset
                headerHeight: parent.height
                duration: showManager.showDuration
                showTimeMarkers: false
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
        y: showMgrContainer.height - height
        z: 10
        width: timelineHeader.width
        orientation: Qt.Horizontal
    }
}
