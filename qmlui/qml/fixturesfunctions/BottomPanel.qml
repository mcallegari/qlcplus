/*
  Q Light Controller Plus
  BottomPanel.qml

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

import "."

Rectangle
{
    id: bottomSidePanel
    anchors.left: parent.left
    anchors.leftMargin: 0
    width: parent.width
    height: collapseHeight
    color: UISettings.bgStrong

    property bool isOpen: false
    property int collapseHeight: UISettings.iconSizeDefault * 1.25
    property int expandedHeight: mainView.height / 3
    property string editorSource: ""

    /** True when the loaded editor is a Scene fixture console */
    property bool isSceneConsole: editorLoader.item && editorLoader.item.hasOwnProperty("isSceneEditor")

    onVisibleChanged:
    {
        if (visible === false)
        {
            editorLoader.source = ""
            releaseExternalControl()
        }
        else
        {
            editorLoader.source = editorSource
        }
    }

    /** Hand the external controllers back to the Virtual Console */
    function releaseExternalControl()
    {
        if (typeof sceneEditor !== "undefined" && sceneEditor)
            sceneEditor.externalControlEnabled = false
    }

    function animatePanel(checked)
    {
        if (checked === isOpen)
            return

        if (isOpen == false)
        {
            animateOpen.start()
            isOpen = true
            editorLoader.source = editorSource
        }
        else
        {
            animateClose.start()
            isOpen = false
            editorLoader.source = ""
            // the panel is closed, so the Virtual Console
            // must receive input signals again
            releaseExternalControl()
        }
    }

    PropertyAnimation
    {
        id: animateOpen
        target: bottomSidePanel
        properties: "height"
        to: expandedHeight
        duration: 200
    }

    PropertyAnimation
    {
        id: animateClose
        target: bottomSidePanel
        properties: "height"
        to: collapseHeight
        duration: 200
    }

    Rectangle
    {
        id: gradientBorder
        height: collapseHeight
        color: "#141414"
        width: parent.width
        gradient: Gradient
        {
            GradientStop { position: 0; color: "#141414" }
            GradientStop { position: 0.21; color: UISettings.bgStrong }
            GradientStop { position: 0.79; color: UISettings.bgStrong }
            GradientStop { position: 1; color: "#141414" }
        }

        MouseArea
        {
            id: rpClickArea
            anchors.fill: parent
            z: 1
            hoverEnabled: true
            cursorShape: pressed ? Qt.ClosedHandCursor : Qt.OpenHandCursor
            drag.target: bottomSidePanel
            drag.axis: Drag.YAxis
            drag.minimumY: 0
            drag.maximumY: bottomSidePanel.parent.height - collapseHeight

            onPositionChanged:
            {
                if (drag.active === true)
                {
                    var newHeight = bottomSidePanel.parent.height - bottomSidePanel.y
                    if (newHeight < collapseHeight)
                        return
                    bottomSidePanel.height = newHeight
                }
            }
            //onClicked: animatePanel()
        }

        RowLayout
        {
            anchors.fill: parent
            z: 2

            // external controller: enable/disable the faders control
            IconButton
            {
                id: extControlButton
                visible: isOpen && isSceneConsole
                width: UISettings.iconSizeDefault
                height: UISettings.iconSizeDefault
                faSource: FontAwesome.fa_sliders
                faColor: UISettings.fgMain
                checkable: true
                checked: sceneEditor ? sceneEditor.externalControlEnabled : false
                tooltip: qsTr("Control the channels with an external controller. The Virtual Console will not receive input signals")
                onToggled: if (sceneEditor) sceneEditor.externalControlEnabled = checked
            }

            // external controller: normal / pan & tilt mode
            IconButton
            {
                visible: isOpen && isSceneConsole && extControlButton.checked
                width: UISettings.iconSizeDefault
                height: UISettings.iconSizeDefault
                imgSource: "qrc:/position.svg"
                checkable: true
                checked: sceneEditor ? sceneEditor.panTiltMode : false
                tooltip: qsTr("Toggle Pan & Tilt mode. The faders will control pan, pan fine, tilt and tilt fine of one fixture")
                onToggled: if (sceneEditor) sceneEditor.panTiltMode = checked
            }

            // external controller: previous page
            IconButton
            {
                visible: isOpen && isSceneConsole && extControlButton.checked
                width: UISettings.iconSizeDefault
                height: UISettings.iconSizeDefault
                faSource: FontAwesome.fa_angle_left
                faColor: UISettings.fgMain
                enabled: sceneEditor ? sceneEditor.externalPage > 0 : false
                tooltip: qsTr("Shift the faders mapping backward")
                onClicked: if (sceneEditor) sceneEditor.changeExternalPage(-1)
            }

            // external controller: next page
            IconButton
            {
                visible: isOpen && isSceneConsole && extControlButton.checked
                width: UISettings.iconSizeDefault
                height: UISettings.iconSizeDefault
                faSource: FontAwesome.fa_angle_right
                faColor: UISettings.fgMain
                enabled: sceneEditor ? sceneEditor.externalPage < sceneEditor.externalPageCount - 1 : false
                tooltip: qsTr("Shift the faders mapping forward")
                onClicked: if (sceneEditor) sceneEditor.changeExternalPage(1)
            }

            // filler
            Rectangle
            {
                height: parent.height
                Layout.fillWidth: true
                color: "transparent"
            }

            IconButton
            {
                visible: isOpen && isSceneConsole
                width: UISettings.iconSizeDefault
                height: UISettings.iconSizeDefault
                faSource: FontAwesome.fa_copy
                faColor: UISettings.fgMain
                tooltip: qsTr("Copy the selected channel values to all the fixtures of the same type")
                enabled: isOpen && sceneEditor.selectedChannelCount > 0 ? true : false
                onClicked: if (sceneEditor) sceneEditor.pasteToAllFixtureSameType()
            }

            IconButton
            {
                visible: isOpen && editorLoader.item && editorLoader.item.hasOwnProperty("isSceneEditor")
                width: UISettings.iconSizeDefault
                height: UISettings.iconSizeDefault
                imgSource: "qrc:/multiple.svg"
                tooltip: qsTr("Toggle multiple channel selection")
                checkable: true
                onToggled: editorLoader.item.multipleSelection = checked
            }

            IconButton
            {
                id: expandButton
                width: height * 1.2
                height: parent.height * 0.7
                checkable: true
                tooltip: qsTr("Expand/Collapse this panel")
                onToggled: animatePanel(checked)

                Text
                {
                    anchors.horizontalCenter: parent.horizontalCenter
                    color: UISettings.fgLight
                    font.family: UISettings.fontAwesomeFontName
                    font.pixelSize: parent.height - 8
                    text: expandButton.checked ? FontAwesome.fa_chevron_down : FontAwesome.fa_chevron_up
                }
            }
        }
    }

    Rectangle
    {
        id: editorArea
        y: gradientBorder.height
        width: parent.width
        height: parent.height - collapseHeight
        color: "transparent"

        Loader
        {
            id: editorLoader
            anchors.fill: parent
            //source: editorSource
        }
    }
}
