/*
  Q Light Controller Plus
  CustomTextEdit.qml

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
import "."

Rectangle
{
    id: customTextEditRect
    implicitWidth: 200
    implicitHeight: UISettings.listItemHeight
    clip: true
    radius: 3
    color: UISettings.bgMedium

    property alias inputFocus: ctEdit.focus
    property alias inputText: ctEdit.text
    property alias inputMethodHints: ctEdit.inputMethodHints
    property alias readOnly: ctEdit.readOnly
    property alias echoMode: ctEdit.echoMode
    property alias maximumLength: ctEdit.maximumLength
    property int fontSize: UISettings.textSizeDefault
    property int textAlignment: TextInput.AlignLeft

    property Item nextTabItem: null
    property Item previousTabItem: null

    signal textChanged(var text)
    signal enterPressed()
    signal escapePressed()

    function selectAndFocus()
    {
        ctEdit.selectAll()
        ctEdit.focus = true
        ctEdit.forceActiveFocus()
    }

    function appendText(text)
    {
        if (ctEdit.selectedText)
        {
            var sIdx = ctEdit.selectionStart
            ctEdit.remove(sIdx, ctEdit.selectionEnd)
            ctEdit.insert(sIdx, text)
        }
        else
            ctEdit.text += text
    }

    border.color: "#222"

    onFocusChanged: if (focus) selectAndFocus()

    TextInput
    {
        id: ctEdit
        anchors.fill: parent
        //anchors.verticalCenter: parent.verticalCenter
        anchors.margins: 4
        color: UISettings.fgMain
        selectionColor: UISettings.highlightPressed
        clip: true
        horizontalAlignment: textAlignment
        verticalAlignment: TextInput.AlignVCenter
        font.family: UISettings.robotoFontName
        font.pixelSize: fontSize
        selectByMouse: true

        onTextChanged: customTextEditRect.textChanged(text)
        onAccepted: customTextEditRect.enterPressed()
        Keys.onEscapePressed: customTextEditRect.escapePressed()
        KeyNavigation.tab: nextTabItem
        KeyNavigation.backtab: previousTabItem
    }
}
