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
import QtQuick.Controls 2.1

import org.qlcplus.classes 1.0
import "TimeUtils.js" as TimeUtils
import "."

Rectangle
{
    anchors.fill: parent
    color: "transparent"

    property int functionID: -1

    signal requestView(int ID, string qmlSrc)

    onFunctionIDChanged:
    {
        if (scriptEditor.scriptContent.length)
            scriptEdit.text = scriptEditor.scriptContent
        scriptEdit.forceActiveFocus()
    }

    EditorTopBar
    {
        id: topBar
        text: scriptEditor.functionName
        onTextChanged: scriptEditor.functionName = text

        onBackClicked:
        {
            var prevID = scriptEditor.previousID
            functionManager.setEditorFunction(prevID, false, true)
            requestView(prevID, functionManager.getEditorResource(prevID))
        }
    }

    Timer
    {
        id: updateTimer
        repeat: false
        running: false
        interval: 500
        onTriggered: scriptEditor.scriptContent = scriptEdit.text
    }

    Flickable
    {
        id: editFlick
        y: topBar.height
        width: parent.width
        height: parent.height - topBar.height
        boundsBehavior: Flickable.StopAtBounds

        contentWidth: scriptEdit.paintedWidth
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

        TextEdit
        {
            id: scriptEdit
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
    }

}
