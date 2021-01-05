/*
  Q Light Controller Plus
  EditableTextBox.qml

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

import QtQuick 2.7
import "."

Rectangle
{
    id: etbRoot
    width: wrapText ? 100 : etbTextEdit.paintedWidth + 5
    height: (maximumHeight && maximumHeight < etbTextEdit.paintedHeight) ?
                maximumHeight : etbTextEdit.paintedHeight
    clip: true
    radius: 3
    color: UISettings.bgMedium

    property real maximumHeight

    property alias inputFocus: etbTextEdit.focus
    property alias inputText: etbTextEdit.text
    property alias readOnly: etbTextEdit.readOnly
    property int fontSize: UISettings.textSizeDefault
    property int textAlignment: TextInput.AlignLeft
    property bool wrapText: true

    signal clicked()
    signal textChanged(var text)

    property string originalText

    function enableEditing(enable)
    {
        if (enable === true)
        {
            originalText = etbTextEdit.text
            etbTextEdit.forceActiveFocus()
            etbTextEdit.cursorVisible = true
            etbTextEdit.cursorPosition = etbTextEdit.text.length
        }
        else
        {
            etbTextEdit.select(0, 0)
        }

        etbTextEdit.readOnly = !enable
        etbMouseArea.enabled = !enable
    }

    TextEdit
    {
        id: etbTextEdit
        width: wrapText ? parent.width : Text.paintedWidth
        height: wrapText ? parent.height : Text.paintedHeight
        readOnly: true
        color: UISettings.fgMain
        selectionColor: UISettings.highlightPressed
        //clip: true
        horizontalAlignment: textAlignment
        font.family: UISettings.robotoFontName
        font.pixelSize: fontSize
        selectByMouse: true
        mouseSelectionMode: TextEdit.SelectWords
        wrapMode: wrapText ? Text.Wrap : Text.NoWrap

        //onTextChanged: etbRoot.textChanged(text)

        onEditingFinished:
        {
            etbRoot.enableEditing(false)
            if (text !== "")
                etbRoot.textChanged(text)
            else
                text = etbRoot.originalText
        }
        Keys.onReturnPressed:
        {
            etbRoot.enableEditing(false)
            if (text !== "")
                etbRoot.textChanged(text)
            else
                text = etbRoot.originalText
        }
        Keys.onEscapePressed:
        {
            etbRoot.enableEditing(false)
            text = etbRoot.originalText
        }
    }

    MouseArea
    {
        id: etbMouseArea
        anchors.fill: parent
        z: 1

        onClicked: etbRoot.clicked()
        onDoubleClicked: etbRoot.enableEditing(true)
    }
}
