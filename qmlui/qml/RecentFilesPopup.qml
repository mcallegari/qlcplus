/*
  Q Light Controller Plus
  RecentFilesPopup.qml

  Copyright (c) Massimo Callegari

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
*/

import QtQuick
import QtQuick.Controls

import "."

Popup
{
    id: recentFilesPopup
    padding: 0
    height: recentFilesColumn.implicitHeight

    property var recentFiles: []

    signal fileSelected(string filePath)

    background:
        Rectangle
        {
            anchors.fill: parent
            border.width: 1
            border.color: UISettings.bgStronger
            color: UISettings.bgStrong
        }

    contentItem:
        Column
        {
            id: recentFilesColumn
            width: recentFilesPopup.width

            Repeater
            {
                model: recentFilesPopup.recentFiles

                delegate:
                    ContextMenuEntry
                    {
                        objectName: "recentFileEntry_" + index

                        property string filePath: modelData

                        entryText: filePath
                        labelMaximumWidth: Math.max(0, recentFilesPopup.width - 30)
                        labelElide: Text.ElideMiddle
                        onClicked: recentFilesPopup.fileSelected(filePath)
                    }
            }
        }
}
