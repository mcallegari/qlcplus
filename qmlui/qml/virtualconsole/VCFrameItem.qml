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

import com.qlcplus.classes 1.0
import "."

VCWidgetItem
{
    id: frameRoot
    property VCFrame frameObj: null
    property bool dropActive: false
    property bool isSolo: false
    property bool isCollapsed: frameObj ? frameObj.isCollapsed : false

    color: dropActive ? "#9DFF52" : (frameObj ? frameObj.backgroundColor : "darkgray")
    clip: true

    onFrameObjChanged:
    {
        setCommonProperties(frameObj)
        if (isSolo)
            frameRoot.border.color = "red"
    }

    onIsCollapsedChanged:
    {
        frameRoot.width = isCollapsed ? 200 : frameObj.geometry.width
        frameRoot.height = isCollapsed ? 36 : frameObj.geometry.height
    }

    // Frame header
    Rectangle
    {
        x: 2
        y: 2
        width: parent.width
        height: 32
        color: "transparent"
        visible: frameObj ? frameObj.showHeader : false

        RowLayout
        {
            height: 32
            width: parent.width - 4
            spacing: 2

            // expand/collapse button
            IconButton
            {
                width: 32
                height: 32
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
                height: 32
                radius: 3
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
                width: 32
                height: 32
                checkable: true
                tooltip: qsTr("Enable/Disable this frame")
                imgSource: "qrc:/apply.svg"
                imgMargins: 1
                visible: frameObj ? frameObj.showEnable : true
            }

            // multi page controls
            Rectangle
            {
                visible: frameObj ? frameObj.multiPageMode : false
                width: 168
                height: 32
                color: "transparent"

                IconButton
                {
                    width: 32
                    height: 32
                    tooltip: qsTr("Previous page")
                    imgSource: "qrc:/back.svg"
                    imgMargins: 1
                    onClicked: frameObj.gotoPreviousPage()
                }
                Rectangle
                {
                    x: 34
                    width: 100
                    height: 32
                    radius: 3
                    color: "black"

                    Text
                    {
                        anchors.centerIn: parent
                        font.family: "Roboto Condensed"
                        font.pointSize: 12
                        font.bold: true
                        text: qsTr("Page") + " " + (frameObj ? frameObj.currentPage + 1 : "1")
                        color: "red"
                    }
                }
                IconButton
                {
                    x: 136
                    width: 32
                    height: 32
                    tooltip: qsTr("Next page")
                    imgSource: "qrc:/forward.svg"
                    imgMargins: 1
                    onClicked: frameObj.gotoNextPage()
                }
            }
        }
    }

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
                return;
            virtualConsole.setDropTarget(frameRoot, false)
            console.log("Item dropped in frame " + frameObj.id)
            var pos = drag.source.mapToItem(frameRoot, 0, 0);

            //console.log("Drop keys: " + drop.keys)
            if (drop.keys[0] === "vcwidget")
                frameObj.addWidget(dropArea, drag.source.widgetType, pos)
            else if (drop.keys[0] === "function")
                frameObj.addFunction(dropArea, drag.source.funcID, pos, false)

        }

        keys: [ "vcwidget", "function" ]
    }
}
