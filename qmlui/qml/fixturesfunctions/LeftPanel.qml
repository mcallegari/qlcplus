/*
  Q Light Controller Plus
  LeftPanel.qml

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

Rectangle
{
    id: leftSidePanel
    anchors.left: parent.left;
    anchors.leftMargin: 0
    width: collapseWidth
    height: parent.height
    color: "#232323"

    property bool isOpen: false
    property int collapseWidth: 50
    property int expandedWidth: 450
    property string editorSource: ""
    property int iconSize: collapseWidth - 4

    function animatePanel(checked)
    {
        if (checked === isOpen)
            return

        if (isOpen == false)
        {
            animateOpen.start();
            isOpen = true;
        }
        else
        {
            animateClose.start();
            isOpen = false;
            editorSource = ""
        }
    }

    Rectangle
    {
        id: editorArea
        width: leftSidePanel.width - collapseWidth;
        height: parent.height
        color: "transparent"

        Loader
        {
            id: editorLoader
            //objectName: "editorLoader"
            anchors.fill: parent
            source: editorSource
        }
    }

    Rectangle
    {
        id: sideBar
        x: parent.width - collapseWidth
        width: collapseWidth
        height: parent.height
        color: "#00000000"
        z: 2

        ExclusiveGroup { id: fxManagerGroup }
        ExclusiveGroup { id: capabilitiesGroup }

        Column
        {
            anchors.fill: parent
            anchors.leftMargin: 1
            spacing: 3

            IconButton
            {
                id: fxEditor
                z: 2
                width: iconSize
                height: iconSize
                imgSource: "qrc:/fixture.svg"
                checkable: true
                tooltip: qsTr("Add Fixtures")
                exclusiveGroup: fxManagerGroup
                onToggled:
                {
                    if (checked == true)
                        editorSource = "qrc:/FixtureBrowser.qml"
                    animatePanel(checked);
                }
            }

            IconButton
            {
                id: grpEditor
                z: 2
                width: iconSize
                height: iconSize
                imgSource: "qrc:/group.svg"
                checkable: true
                tooltip: qsTr("Fixture Groups")
                exclusiveGroup: fxManagerGroup
                onToggled:
                {
                    if (checked == true)
                        editorSource = "qrc:/GroupEditor.qml"
                    animatePanel(checked);
                }
            }

            IconButton
            {
                objectName: "capIntensity"
                z: 2
                width: iconSize
                height: iconSize
                imgSource: "qrc:/intensity.svg"
                checkable: true
                tooltip: qsTr("Intensity")
                counter: 0
                exclusiveGroup: capabilitiesGroup
                onCheckedChanged: { intTool.visible = !intTool.visible }
                IntensityTool
                {
                    id: intTool
                    x: iconSize + 4
                    visible: false
                }
            }

            IconButton
            {
                objectName: "capColor"
                z: 2
                width: iconSize
                height: iconSize
                imgSource: "qrc:/color.svg"
                checkable: true
                tooltip: qsTr("Color")
                counter: 0
                exclusiveGroup: capabilitiesGroup
                onCheckedChanged: { colTool.visible = !colTool.visible }
                ColorTool
                {
                    id: colTool
                    x: iconSize + 4
                    visible: false
                }
            }

            IconButton
            {
                objectName: "capPosition"
                z: 2
                width: iconSize
                height: iconSize
                imgSource: "qrc:/position.svg"
                checkable: true
                tooltip: qsTr("Position")
                counter: 0
                exclusiveGroup: capabilitiesGroup
                onCheckedChanged: { posTool.visible = !posTool.visible }
                PositionTool
                {
                    id: posTool
                    x: iconSize + 4
                    visible: false
                }
            }

            IconButton
            {
                objectName: "capColorWheel"
                z: 2
                width: iconSize
                height: iconSize
                imgSource: "qrc:/colorwheel.svg"
                checkable: true
                tooltip: qsTr("Color Wheel")
                counter: 0
                exclusiveGroup: capabilitiesGroup

                onCheckedChanged: { cWheelTool.visible = !cWheelTool.visible }
                PresetsTool
                {
                    id: cWheelTool
                    x: iconSize + 4
                    visible: false
                }
            }

            IconButton
            {
                objectName: "capGobos"
                z: 2
                width: iconSize
                height: iconSize
                imgSource: "qrc:/gobo.svg"
                checkable: true
                tooltip: qsTr("Gobos")
                counter: 0
                exclusiveGroup: capabilitiesGroup

                onCheckedChanged: { gobosTool.visible = !gobosTool.visible }
                PresetsTool
                {
                    id: gobosTool
                    x: iconSize + 4
                    visible: false
                    goboPresets: true
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
            GradientStop { position: 0.213; color: "#232323" }
            GradientStop { position: 0.79; color: "#232323" }
            GradientStop { position: 1; color: "#141414" }
        }

        MouseArea
        {
            id: lpClickArea
            anchors.fill: parent
            z: 1
            x: parent.width - width
            hoverEnabled: true
            cursorShape: Qt.OpenHandCursor
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
