/*
  Q Light Controller Plus
  CustomTextInput.qml

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

TextInput
{
    id: controlRoot
    z: 0
    width: 100
    height: UISettings.listItemHeight
    readOnly: true
    text: ""
    horizontalAlignment: TextInput.AlignLeft
    verticalAlignment: TextInput.AlignVCenter
    color: UISettings.fgMain
    font.family: UISettings.robotoFontName
    font.pixelSize: UISettings.textSizeDefault
    echoMode: TextInput.Normal
    wrapMode: TextInput.NoWrap
    selectByMouse: true
    selectionColor: UISettings.highlightPressed
    selectedTextColor: UISettings.fgMain

    property string originalText
    property bool allowDoubleClick: false

    signal clicked()
    signal textConfirmed(string text)

    function setEditingStatus(enable)
    {
        z = enable ? 5 : 0
        readOnly = enable ? false : true
        cursorVisible = enable ? true : false

        if (enable)
        {
            originalText = text
            cursorPosition = text.length
            controlRoot.selectAll()
        }
        else
        {
            select(0, 0)

        }
    }

    Keys.onPressed:
    {
        switch(event.key)
        {
            case Qt.Key_F2:
                setEditingStatus(true)
            break;
            case Qt.Key_Escape:
                setEditingStatus(false)
                controlRoot.text = originalText
            break;
            default:
                event.accepted = false
                return
        }

        event.accepted = true
    }

    onEditingFinished:
    {
        if (readOnly)
            return
        setEditingStatus(false)
        controlRoot.textConfirmed(text)
    }

    MouseArea
    {
        anchors.fill: parent

        onClicked:
        {
            controlRoot.forceActiveFocus()
            controlRoot.clicked()
        }
        onDoubleClicked:
        {
            if (allowDoubleClick === false)
                return

            setEditingStatus(true)
        }
    }
}
