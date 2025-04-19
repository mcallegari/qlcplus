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

import QtQuick
import QtQuick.Controls.Basic

import "."

TextField
{
    id: controlRoot
    implicitWidth: UISettings.bigItemHeight * 2
    height: UISettings.listItemHeight
    clip: true

    //anchors.margins: 4
    color: UISettings.fgMain
    selectedTextColor: UISettings.fgMain
    selectionColor: UISettings.highlightPressed
    horizontalAlignment: TextInput.AlignLeft
    verticalAlignment: TextInput.AlignVCenter
    font.family: UISettings.robotoFontName
    font.pixelSize: UISettings.textSizeDefault
    selectByMouse: true

    property int radius: 3
    property alias bgColor: controlBg.color

    function selectAndFocus()
    {
        controlRoot.selectAll()
        controlRoot.focus = true
        controlRoot.forceActiveFocus()
    }

    function appendText(text)
    {
        if (controlRoot.selectedText)
        {
            var sIdx = controlRoot.selectionStart
            controlRoot.remove(sIdx, controlRoot.selectionEnd)
            controlRoot.insert(sIdx, text)
        }
        else
            controlRoot.text += text
    }

    onFocusChanged: if (focus) selectAndFocus()

    background:
        Rectangle
        {
            id: controlBg
            radius: controlRoot.radius
            border.color: controlRoot.activeFocus ? UISettings.highlight : UISettings.bgStrong
            color: UISettings.bgControl
        }

    Rectangle
    {
        anchors.fill: parent
        z: 3
        color: "black"
        opacity: 0.5
        visible: !controlRoot.enabled
    }
}
