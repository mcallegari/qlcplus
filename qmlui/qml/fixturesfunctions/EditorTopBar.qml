/*
  Q Light Controller Plus
  EditorTopBar.qml

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

import org.qlcplus.classes 1.0
import "."

RowLayout
{
    width: parent.width
    height: UISettings.iconSizeMedium
    z: 2

    property alias text: fNameEdit.text

    signal backClicked()

    IconButton
    {
        width: UISettings.iconSizeMedium
        height: width
        rotation: 180
        bgColor: UISettings.bgMedium
        imgSource: "qrc:/arrow-right.svg"
        tooltip: qsTr("Go back to the previous view")
        onClicked: backClicked()
    }

    Rectangle
    {
        Layout.fillWidth: true
        height: UISettings.iconSizeMedium
        color: UISettings.bgMedium

        TextInput
        {
            id: fNameEdit
            anchors.fill: parent
            color: UISettings.fgMain
            clip: true
            verticalAlignment: TextInput.AlignVCenter
            font.family: UISettings.robotoFontName
            font.pixelSize: UISettings.textSizeDefault
            selectByMouse: true
        }
    }
}
