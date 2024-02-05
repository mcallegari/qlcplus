/*
  Q Light Controller Plus
  ScriptEditor.qml

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
import QtQuick.Dialogs 1.1
import QtQuick.Controls 2.13

import org.qlcplus.classes 1.0
import "TimeUtils.js" as TimeUtils
import "."

Rectangle
{
    id: seContainer
    anchors.fill: parent
    color: "transparent"

    property int functionID: -1

    signal requestView(int ID, string qmlSrc)
    signal doubleClicked(int ID, int type)

    onFunctionIDChanged:
    {
        if (scriptEditor.scriptContent.length)
            scriptEdit.text = scriptEditor.scriptContent
        scriptEdit.forceActiveFocus()
    }

    Timer
    {
        id: updateTimer
        repeat: false
        running: false
        interval: 500
        onTriggered: scriptEditor.scriptContent = scriptEdit.text
    }

    SplitView
    {
        width: parent.width
        height: parent.height - topBar.height

        Loader
        {
            id: sideLoader
            width: UISettings.sidePanelWidth
            SplitView.preferredWidth: UISettings.sidePanelWidth
            visible: false

            height: seContainer.height
            source: ""

            onLoaded:
            {
                if (source)
                {
                    item.allowEditing = false
                    item.width = Qt.binding(function() { return sideLoader.width - 2 })
                }
            }

            Rectangle
            {
                width: 2
                height: parent.height
                x: parent.width - 2
                color: UISettings.bgLight
            }

            Connections
            {
                ignoreUnknownSignals: true
                target: sideLoader.item
                function onDoubleClicked()
                {
                    if (fixtureTreeButton.checked)
                        ID = fixtureManager.fixtureIDfromItemID(ID)
                    scriptEdit.insert(scriptEdit.cursorPosition, ID)
                }
            }
        }

        Column
        {
            SplitView.fillWidth: true

            EditorTopBar
            {
                id: topBar
                text: scriptEditor.functionName
                onTextChanged: scriptEditor.functionName = text

                onBackClicked:
                {
                    if (sideLoader.visible)
                    {
                        sideLoader.source = ""
                        sideLoader.visible = false
                        rightSidePanel.width -= sideLoader.width
                    }

                    var prevID = scriptEditor.previousID
                    functionManager.setEditorFunction(prevID, false, true)
                    requestView(prevID, functionManager.getEditorResource(prevID))
                }

                IconButton
                {
                    id: addButton
                    width: height
                    height: UISettings.iconSizeMedium
                    imgSource: "qrc:/add.svg"
                    tooltip: qsTr("Add a method call at cursor position")
                    onClicked: addMethodMenu.open()
                }

                IconButton
                {
                    id: functionTreeButton
                    width: height
                    height: UISettings.iconSizeMedium
                    imgSource: "qrc:/functions.svg"
                    checkable: true
                    tooltip: qsTr("Show/hide functions tree")
                    autoExclusive: true
                    onCheckedChanged:
                    {
                        //if (fixtureTreeButton.checked)
                        //    fixtureTreeButton.toggle()

                        if (checked)
                        {
                            if (!sideLoader.visible)
                                rightSidePanel.width += UISettings.sidePanelWidth
                            sideLoader.visible = true
                            sideLoader.source = "qrc:/FunctionManager.qml"
                        }
                        else
                        {
                            rightSidePanel.width -= sideLoader.width
                            sideLoader.source = ""
                            sideLoader.visible = false
                        }
                    }
                }

                IconButton
                {
                    id: fixtureTreeButton
                    width: height
                    height: UISettings.iconSizeMedium
                    imgSource: "qrc:/fixture.svg"
                    checkable: true
                    tooltip: qsTr("Show/hide fixture tree")
                    autoExclusive: true
                    onCheckedChanged:
                    {
                        //if (functionTreeButton.checked)
                        //    functionTreeButton.toggle()

                        if (checked)
                        {
                            if (!sideLoader.visible)
                                rightSidePanel.width += UISettings.sidePanelWidth
                            sideLoader.visible = true
                            sideLoader.source = "qrc:/FixtureGroupManager.qml"
                        }
                        else
                        {
                            rightSidePanel.width = rightSidePanel.width - sideLoader.width
                            sideLoader.source = ""
                            sideLoader.visible = false
                        }
                    }
                }

                IconButton
                {
                    id: removeFunc
                    width: height
                    height: UISettings.iconSizeMedium
                    imgSource: "qrc:/apply.svg"
                    tooltip: qsTr("Check the script syntax")
                    onClicked:
                    {
                        syntaxErrorsPopup.message = scriptEditor.syntaxErrors()
                        syntaxErrorsPopup.open()
                    }

                    CustomPopupDialog
                    {
                        id: syntaxErrorsPopup
                        width: UISettings.bigItemHeight * 5
                        standardButtons: Dialog.Close
                        title: qsTr("Syntax check")
                    }
                }
            }

            Flickable
            {
                id: editFlick
                width: parent.width
                height: parent.height - topBar.height
                boundsBehavior: Flickable.StopAtBounds

                contentWidth: width //scriptEdit.paintedWidth
                contentHeight: scriptEdit.paintedHeight
                clip: true

                function ensureVisible(r)
                {
                    if (contentX >= r.x)
                        contentX = r.x;
                    else if (contentX + width <= r.x + r.width)
                        contentX = r.x + r.width - width;
                    if (contentY >= r.y)
                        contentY = r.y;
                    else if (contentY + height <= r.y + r.height)
                        contentY = r.y + r.height - height;
                }

                DropArea
                {
                    anchors.fill: parent
                    onDropped:
                    {
                        if (drag.source.itemsList[0].itemType === App.FixtureDragItem)
                        {
                            //console.log("Fixture ID: " + drag.source.itemsList[0].cRef.id)
                            scriptEdit.insert(scriptEdit.cursorPosition, drag.source.itemsList[0].cRef.id)
                        }
                        else if (drag.source.hasOwnProperty("fromFunctionManager"))
                        {
                            //console.log("Function ID: " + drag.source.itemsList[0])
                            scriptEdit.insert(scriptEdit.cursorPosition, drag.source.itemsList[0])
                        }
                    }
                }

                TextEdit
                {
                    id: scriptEdit
                    width: parent.width
                    focus: true
                    font.family: "Roboto Mono"
                    font.pixelSize: UISettings.textSizeDefault * 0.8
                    color: UISettings.fgMain
                    wrapMode: TextEdit.Wrap
                    selectionColor: UISettings.highlightPressed
                    selectByMouse: true

                    onCursorRectangleChanged: editFlick.ensureVisible(cursorRectangle)
                    onTextChanged: updateTimer.restart()
                }

                ScrollBar.vertical: CustomScrollBar { }
                ScrollBar.horizontal: CustomScrollBar { }
            } // Flickable
        } // Column
    } // SplitView

    FileDialog
    {
        id: selectFileDialog
        visible: false
        onAccepted:
        {
            // strip "file://" and add single quotes
            var str = "'" + fileUrl.toString().slice(7) + "'"
            scriptEdit.insert(scriptEdit.cursorPosition, str)
            addMethodMenu.close()
        }
    }

    Popup
    {
        id: addMethodMenu
        x: (addButton.x + addButton.width) - width
        y: addButton.y + addButton.height
        padding: 0

        background:
            Rectangle
            {
                color: UISettings.bgStrong
                border.color: UISettings.bgStronger
            }

        function insertMethod(str)
        {
            scriptEdit.insert(scriptEdit.cursorPosition, str + "\n")
            scriptEdit.cursorPosition -= 3
            addMethodMenu.close()
        }

        Column
        {
            ContextMenuEntry
            {
                imgSource: "qrc:/play.svg"
                entryText: qsTr("Start function")
                onClicked: addMethodMenu.insertMethod("Engine.startFunction();")
            }
            ContextMenuEntry
            {
                imgSource: "qrc:/stop.svg"
                entryText: qsTr("Stop function")
                onClicked: addMethodMenu.insertMethod("Engine.stopFunction();")
            }
            ContextMenuEntry
            {
                imgSource: "qrc:/sliders.svg"
                entryText: qsTr("Set fixture channel")
                onClicked: addMethodMenu.insertMethod("Engine.setFixture();")
            }
            ContextMenuEntry
            {
                imgSource: "qrc:/clock.svg"
                entryText: qsTr("Wait time")
                onClicked: addMethodMenu.insertMethod("Engine.waitTime();")
            }
            ContextMenuEntry
            {
                imgSource: "qrc:/random.svg"
                entryText: qsTr("Random number")
                onClicked: addMethodMenu.insertMethod("Engine.random();")
            }
            ContextMenuEntry
            {
                imgSource: "qrc:/blackout.svg"
                entryText: qsTr("Blackout")
                onClicked: addMethodMenu.insertMethod("Engine.setBlackout();")
            }
            ContextMenuEntry
            {
                imgSource: "qrc:/script.svg"
                entryText: qsTr("System command")
                onClicked: addMethodMenu.insertMethod("Engine.systemCommand();")
            }
            ContextMenuEntry
            {
                imgSource: "qrc:/fileopen.svg"
                entryText: qsTr("File path")
                onClicked: selectFileDialog.open()
            }
        }
    }
}
