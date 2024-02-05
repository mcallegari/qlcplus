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
            RowLayout
            {
                visible: frameObj ? frameObj.multiPageMode : false
                height: parent.height
                spacing: 2

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
                CustomComboBox
                {
                    id: pageSelector
                    width: UISettings.bigItemHeight
                    height: parent.height
                    textRole: ""
                    model: frameObj ? frameObj.pageLabels : null
                    currentIndex: frameObj ? frameObj.currentPage : 0
                    onCurrentIndexChanged:
                    {
                        if (frameObj)
                            frameObj.currentPage = currentIndex
                        // binding got  broken, so restore it
                        currentIndex = Qt.binding(function() { return frameObj.currentPage })
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

        onEntered: frameRoot.dropActive = true
        onExited: frameRoot.dropActive = false
        onDropped:
        {
            if (frameObj == null || frameRoot.dropActive === false)
                return

            frameRoot.dropActive = false

            if (drag.source.wObj === frameObj)
            {
                console.log("ERROR: Source and target are the same!!!")
                return
            }

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
            else if (drop.keys[0] === "pasteWidgets")
            {
                frameObj.addWidgetsFromClipboard(dropArea, virtualConsole.clipboardItemsList(), pos);
            }
        }

        keys: [ "vcwidget", "function", "pasteWidgets" ]
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
