/*
  Q Light Controller Plus
  VCFrameItem.qml

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

import org.qlcplus.classes 1.0
import "."

VCWidgetItem
{
    id: frameRoot
    property VCFrame frameObj: null
    property bool dropActive: false
    property bool isSolo: false
    property bool isCollapsed: frameObj ? frameObj.isCollapsed : false

    color: dropActive ? UISettings.activeDropArea : (frameObj ? frameObj.backgroundColor : "darkgray")
    clip: true

    onFrameObjChanged:
    {
        setCommonProperties(frameObj)
        if (isSolo)
            frameRoot.border.color = "red"
    }

    onIsCollapsedChanged:
    {
        frameRoot.width = isCollapsed ? UISettings.bigItemHeight * 2 : frameObj.geometry.width
        frameRoot.height = isCollapsed ? UISettings.listItemHeight : frameObj.geometry.height
    }

    // Frame header
    Rectangle
    {
        id: frameHeader
        width: parent.width
        height: UISettings.listItemHeight
        color: "transparent"
        visible: frameObj ? frameObj.showHeader : false

        RowLayout
        {
            x: 1
            y: 1
            height: parent.height - 2
            width: parent.width - 2
            spacing: 1

            // expand/collapse button
            IconButton
            {
                width: height
                height: parent.height
                radius: 0
                border.width: 0
                tooltip: qsTr("Expand/Collapse this frame")
                faSource: checked ? FontAwesome.fa_expand : FontAwesome.fa_compress
                faColor: UISettings.fgMain
                checkable: true
                checked: isCollapsed
                //checkedColor: bgColor
                onToggled: frameObj.isCollapsed = checked
            }

            // header bar and caption
            Rectangle
            {
                height: parent.height
                //radius: 3
                gradient: Gradient
                {
                    GradientStop { position: 0; color: isSolo ? "#BC0A0A" : "#666666" }
                    GradientStop { position: 1; color: isSolo ? "#370303" : "#000000" }
                }
                Layout.fillWidth: true

                Text
                {
                    x: 2
                    width: parent.width - 4
                    height: parent.height
                    font: frameObj ? frameObj.font : ""
                    text: frameObj ? frameObj.caption : ""
                    verticalAlignment: Text.AlignVCenter
                    color: frameObj ? frameObj.foregroundColor : "white"
                }
            }

            // enable button
            IconButton
            {
                width: height
                height: parent.height
                radius: 0
                border.width: 0
                checkable: true
                tooltip: qsTr("Enable/Disable this frame")
                imgSource: "qrc:/apply.svg"
                imgMargins: 1
                checked: frameObj ? !frameObj.isDisabled : true
                visible: frameObj ? frameObj.showEnable : true
                onToggled: if (frameObj) frameObj.isDisabled = !checked
            }

            // multi page controls
            Rectangle
            {
                visible: frameObj ? frameObj.multiPageMode : false
                width: 168
                height: parent.height
                color: "transparent"

                IconButton
                {
                    width: height
                    height: parent.height
                    radius: 0
                    border.width: 0
                    tooltip: qsTr("Previous page")
                    imgSource: "qrc:/back.svg"
                    imgMargins: 1
                    onClicked: frameObj.gotoPreviousPage()
                }
                Rectangle
                {
                    x: parent.height + 2
                    width: 100
                    height: parent.height
                    radius: 3
                    color: "black"

                    Text
                    {
                        anchors.centerIn: parent
                        font.family: UISettings.robotoFontName
                        font.pixelSize: UISettings.textSizeDefault
                        font.bold: true
                        text: qsTr("Page") + " " + (frameObj ? frameObj.currentPage + 1 : "1")
                        color: "red"
                    }
                }
                IconButton
                {
                    x: parent.width - width - 2
                    width: height
                    height: parent.height
                    radius: 0
                    border.width: 0
                    tooltip: qsTr("Next page")
                    imgSource: "qrc:/forward.svg"
                    imgMargins: 1
                    onClicked: frameObj.gotoNextPage()
                }
            }
        }
    }

    /* This DropArea has a dual usage:
     * 1- it is the parent of the frame chidren
     * 2- it is an actual drop area to drag/drop new or existing widgets
     */
    DropArea
    {
        id: dropArea
        anchors.fill: parent
        objectName: frameObj ? "frameDropArea" + frameObj.id : ""
        z: 5 // children must be above the VCWidget resizeLayer

        onEntered: virtualConsole.setDropTarget(frameRoot, true)
        onExited: virtualConsole.setDropTarget(frameRoot, false)
        onDropped:
        {
            if (frameObj === null || dropActive === false)
                return

            virtualConsole.setDropTarget(frameRoot, false)

            var pos = drag.source.mapToItem(frameRoot, 0, 0)
            console.log("Item dropped in frame " + frameObj.id + " at pos " + pos)

            //console.log("Drop keys: " + drop.keys)
            if (drop.keys[0] === "vcwidget")
            {
                if (drag.source.widgetType)
                {
                    if (drag.source.widgetType === "buttonmatrix" || drag.source.widgetType === "slidermatrix")
                        virtualConsole.requestAddMatrixPopup(frameObj, dropArea, drag.source.widgetType, pos)
                    else
                        frameObj.addWidget(dropArea, drag.source.widgetType, pos)
                }
                else
                {
                    // reparent the QML item first
                    drag.source.parent = dropArea
                    virtualConsole.moveWidget(drag.source.wObj, frameObj, pos)

                }
            }
            else if (drop.keys[0] === "function")
            {
                frameObj.addFunctions(dropArea, drag.source.itemsList, pos, drag.source.modifiers)
            }
        }

        keys: [ "vcwidget", "function" ]
    }

    // disable layer
    Rectangle
    {
        y: frameHeader.height
        width: parent.width
        height: parent.height - y
        z: 5
        visible: frameObj ? frameObj.isDisabled : false
        opacity: 0.4
        color: "black"
    }
}
