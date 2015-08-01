/*
  Q Light Controller Plus
  RightPanel.qml

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

Rectangle
{
    id: rightSidePanel
    width: collapseWidth
    height: 500
    color: "#232323"

    property bool isOpen: false
    property int collapseWidth: 50
    property int expandedWidth: 450
    property string editorSource: ""

    function createFunctionAndEditor(fType, fEditor)
    {
        var newFuncID = functionManager.createFunction(fType)
        functionManager.setEditorFunction(newFuncID)
        editorLoader.functionID = newFuncID
        editorSource = fEditor
        if (isOpen == false)
            animatePanel()
        else
            editorLoader.source = editorSource;
        addFunctionMenu.visible = false
        addFunction.checked = false
        funcEditor.checked = true
    }

    function animatePanel()
    {
        if (isOpen == false)
        {
            editorLoader.source = editorSource;
            animateOpen.start();
            isOpen = true;
        }
        else
        {
            animateClose.start();
            editorLoader.source = ""
            isOpen = false;
        }
    }

    Rectangle
    {
        id: editorArea
        x: collapseWidth
        width: rightSidePanel.width - collapseWidth;
        height: parent.height
        color: "transparent"

        Loader
        {
            id: editorLoader
            anchors.fill: parent

            property int functionID

            onLoaded:
            {
                item.functionID = functionID
            }
        }
    }

    Rectangle
    {
        x: 3
        width: collapseWidth
        height: parent.height
        color: "#00000000"
        z: 2

        Column
        {
            anchors.fill: parent
            spacing: 3

            IconButton
            {
                id: funcEditor
                z: 2
                width: collapseWidth - 4
                height: collapseWidth - 4
                imgSource: "qrc:/functions.svg"
                tooltip: qsTr("Function Manager")
                checkable: true
                onToggled:
                {
                    editorSource = "qrc:/FunctionManager.qml"
                    animatePanel();
                }
            }
            IconButton
            {
                id: addFunction
                z: 2
                width: collapseWidth - 4
                height: collapseWidth - 4
                imgSource: "qrc:/add.svg"
                tooltip: qsTr("Add a new function")
                checkable: true
                onToggled: addFunctionMenu.visible = !addFunctionMenu.visible

                AddFunctionMenu
                {
                    id: addFunctionMenu
                    visible: false
                    x: -width

                    onEntryClicked: createFunctionAndEditor(fType, fEditor)
                }
            }
            IconButton
            {
                id: sceneDump
                objectName: "dumpButton"
                z: 2
                width: collapseWidth - 4
                height: collapseWidth - 4
                imgSource: "qrc:/dmxdump.svg"
                tooltip: qsTr("Dump to a Scene")
                visible: false
                onClicked:
                {
                    contextManager.dumpDmxChannels()
                    editorSource = "qrc:///FunctionManager.qml"
                    if (rightSidePanel.isOpen == false)
                    {
                        editorLoader.source = editorSource;
                        animateOpen.start();
                        rightSidePanel.isOpen = true;
                    }
                    funcEditor.checked = true
                }
            }
            IconButton
            {
                id: previewFunc
                objectName: "previewButton"
                z: 2
                width: collapseWidth - 4
                height: collapseWidth - 4
                imgSource: "qrc:/play.svg"
                tooltip: qsTr("Function Preview")
                checkable: true
                visible: false
                onToggled: functionManager.setPreview(checked)
            }
        }
    }

    PropertyAnimation
    {
        id: animateOpen
        target: rightSidePanel
        properties: "width"
        to: expandedWidth
        duration: 200
    }

    PropertyAnimation
    {
        id: animateClose
        target: rightSidePanel
        properties: "width"
        to: collapseWidth
        duration: 200
    }

    Rectangle
    {
        id: gradientBorder
        y: 0
        x: height
        height: collapseWidth
        color: "#141414"
        width: parent.height
        transformOrigin: Item.TopLeft
        rotation: 90
        gradient: Gradient
        {
            GradientStop { position: 0; color: "#141414" }
            GradientStop { position: 0.213; color: "#232323" }
            GradientStop { position: 0.79; color: "#232323" }
            GradientStop { position: 1; color: "#141414" }
        }

        MouseArea
        {
            id: rpClickArea
            anchors.fill: parent
            z: 1
            x: parent.width - width
            hoverEnabled: true
            cursorShape: Qt.OpenHandCursor
            drag.target: rightSidePanel
            drag.axis: Drag.XAxis
            drag.minimumX: collapseWidth

            onPositionChanged:
            {
                if (drag.active == true)
                    rightSidePanel.width = rightSidePanel.parent.width - rightSidePanel.x
            }
            //onClicked: animatePanel("")
        }
    }
}

