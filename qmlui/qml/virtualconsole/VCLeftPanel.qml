/*
  Q Light Controller Plus
  VCLeftPanel.qml

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
import QtQuick.Controls 1.0
import "."

Rectangle
{
    id: leftSidePanel
    anchors.left: parent.left;
    anchors.leftMargin: 0
    width: collapseWidth
    height: parent.height
    color: UISettings.bgStrong

    property bool isOpen: false
    property int collapseWidth: 50
    property int expandedWidth: 400
    property string editorSource: ""
    property int iconSize: collapseWidth - 4

    function animatePanel(checked)
    {
        if (checked === isOpen)
            return

        if (isOpen == false)
        {
            editorLoader.source = editorSource
            animateOpen.start()
            isOpen = true
        }
        else
        {
            animateClose.start()
            isOpen = false
            editorLoader.source = ""
        }
    }

    Rectangle
    {
        id: editorArea
        z: 5
        width: leftSidePanel.width - collapseWidth;
        height: parent.height
        color: "transparent"

        Loader
        {
            id: editorLoader
            anchors.fill: parent
            source: editorSource
            onLoaded:
            {

            }
        }
    }

    Rectangle
    {
        id: sideBar
        x: parent.width - collapseWidth
        width: collapseWidth
        height: parent.height
        color: "transparent"
        z: 2

        Column
        {
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 3

            ExclusiveGroup { id: vcButtonsGroup }

            IconButton
            {
                id: addWidgetButton
                z: 2
                width: iconSize
                height: iconSize
                imgSource: "qrc:/add.svg"
                checkable: true
                exclusiveGroup: vcButtonsGroup
                tooltip: qsTr("Add a new widget to the console")
                onToggled:
                {
                    if (checked == true)
                        editorSource = "qrc:/WidgetsList.qml"
                    animatePanel(checked);
                }
            }

            IconButton
            {
                id: resizeModeButton
                z: 2
                width: iconSize
                height: iconSize
                imgSource: "qrc:/resize.svg"
                checkable: true
                checked: virtualConsole.resizeMode
                tooltip: qsTr("Enable/Disable the widgets resize mode")
                onToggled:
                {
                    virtualConsole.resizeMode = checked
                }
            }
            IconButton
            {
                id: funcEditor
                z: 2
                width: iconSize
                height: iconSize
                imgSource: "qrc:/functions.svg"
                tooltip: qsTr("Function Manager")
                checkable: true
                onToggled:
                {
                    if (checked == true)
                        editorSource = "qrc:/FunctionManager.qml"
                    animatePanel(checked);
                }
            }
        }
    }

    PropertyAnimation
    {
        id: animateOpen
        target: leftSidePanel
        properties: "width"
        to: expandedWidth
        duration: 200
    }

    PropertyAnimation
    {
        id: animateClose
        target: leftSidePanel
        properties: "width"
        to: collapseWidth
        duration: 200
    }

    Rectangle
    {
        id: gradientBorder
        y: width
        x: parent.width - height
        height: collapseWidth
        color: "#141414"
        width: parent.height
        transformOrigin: Item.TopLeft
        rotation: 270
        gradient: Gradient
        {
            GradientStop { position: 0; color: "#141414" }
            GradientStop { position: 0.21; color: UISettings.bgStrong }
            GradientStop { position: 0.79; color: UISettings.bgStrong }
            GradientStop { position: 1; color: "#141414" }
        }

        MouseArea
        {
            id: lpClickArea
            anchors.fill: parent
            z: 1
            x: parent.width - width
            hoverEnabled: true
            cursorShape: pressed ? Qt.ClosedHandCursor : Qt.OpenHandCursor
            drag.target: leftSidePanel
            drag.axis: Drag.XAxis
            drag.minimumX: collapseWidth

            onPositionChanged:
            {
                if (drag.active == true)
                {
                    var obj = mapToItem(null, mouseX, mouseY);
                    leftSidePanel.width = obj.x + (collapseWidth / 2);
                    //console.log("mouseX:", mouseX, "mapToItem().x:", obj.x);
                }
            }

            //onClicked: animatePanel("")
        }
    }
}

