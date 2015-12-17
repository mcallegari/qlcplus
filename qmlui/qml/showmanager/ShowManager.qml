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

import "TimeUtils.js" as TimeUtils
import "."

Rectangle
{
    id: showMgrContainer
    anchors.fill: parent
    color: "transparent"

    property string contextName: "SHOWMGR"

    property int trackHeight: 80
    property int trackWidth: 200

    property real timeScale: showManager.timeScale
    property int headerHeight: 40

    property int showID: showManager.currentShowID

    Component.onCompleted: showManager.renderView(itemsArea.contentItem)

    onShowIDChanged:
    {
        showManager.renderView(itemsArea.contentItem)
    }

    Rectangle
    {
        id: topBar
        width: showMgrContainer.width
        height: 44
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

            RobotoText
            {
                label: qsTr("Name")
            }

            CustomTextEdit
            {
                width: 200
                height: parent.height - 6
                fontSize: 18
                inputText: showManager.showName

                onTextChanged: showManager.showName = text
            }

            RobotoText
            {
                id: timeBox
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
                onClicked:
                {
                    playbackBtn.checked = false
                    showManager.stopShow()
                }
            }

            Rectangle
            {
                Layout.fillWidth: true
            }
        }
    }

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

        Rectangle
        {
            anchors.right: parent.right
            width: verticalDivider.width
            height: parent.height
            color: UISettings.bgLight
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
        height: showMgrContainer.headerHeight //showMgrContainer.height - topBar.height - (bottomPanel.visible ? bottomPanel.height : 0)
        width: showMgrContainer.width - trackWidth

        boundsBehavior: Flickable.StopAtBounds
        flickableDirection: Flickable.HorizontalFlick

        contentWidth: hdrItem.width //> width ? hdrItem.width : width
        contentX: itemsArea.contentX

        HeaderAndCursor
        {
            id: hdrItem
            z: 2
            height: parent.height
            headerHeight: showMgrContainer.headerHeight
            cursorHeight: showMgrContainer.height - topBar.height - (bottomPanel.visible ? bottomPanel.height : 0)
            timeScale: showMgrContainer.timeScale
            duration: showManager.showDuration
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

        boundsBehavior: Flickable.StopAtBounds
        flickableDirection: Flickable.VerticalFlick

        contentHeight: tracksHeight > height ? tracksHeight : height
        //contentWidth: timelineHeader.contentWidth

        property int tracksHeight: (tracksBox.count + 1) * trackHeight

        Column
        {
            width: trackWidth
            z: 2

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

            boundsBehavior: Flickable.StopAtBounds
            contentHeight: showContents.contentHeight
            contentWidth: timelineHeader.contentWidth
            contentX: timelineHeader.contentX

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
            DropArea
            {
                id: newFuncDrop
                width: showMgrContainer.width - trackWidth
                height: tracksBox.count * trackHeight
                z: 2

                keys: [ "function" ]
                onDropped:
                {
                    console.log("Function item dropped here. x: " + drag.x + " y: " + drag.y)
                    var trackIdx = (itemsArea.contentY + drag.y) / trackHeight
                    var fTime = TimeUtils.posToMs(itemsArea.contentX + drag.x, timeScale)
                    console.log("Drop on time: " + fTime)
                    showManager.addItem(itemsArea.contentItem, trackIdx, fTime, drag.source.funcID)
                }
            }

            Rectangle
            {
                id: newTrackBox
                x: itemsArea.contentX
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
                        var fTime = TimeUtils.posToMs(itemsArea.contentX + drag.x, timeScale)
                        console.log("Drop on time: " + fTime)
                        showManager.addItem(itemsArea.contentItem, -1, fTime, drag.source.funcID)
                    }
                }
            }
        }
    }

    ScrollBar
    {
        id: horScrollbar
        z: 4
        orientation: Qt.Horizontal
        anchors.bottom: parent.bottom
        x: trackWidth
        width: parent.width - trackWidth - rightPanel.width
        flickable: timelineHeader
    }
    ScrollBar { z: 5; flickable: showContents; doubleBars: true }

}
