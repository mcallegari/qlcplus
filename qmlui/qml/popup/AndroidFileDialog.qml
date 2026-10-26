/*
  Q Light Controller Plus
  AndroidFileDialog.qml

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

import Qt.labs.platform as Labs

/* Android only, created with Qt.createComponent so other platforms never
   import Qt.labs.platform.
   The QtQuick.Dialogs FileDialog (up to Qt 6.12) drops the content:// URI
   returned by the Android picker, because it checks that the file exists
   locally. The Qt.labs.platform dialog keeps it.
   No name filters: Android maps them to MIME types and .qxw/.qxf have
   none, so those files could not be selected. */
Labs.FileDialog
{
    id: labsDialog

    property bool saveMode: false

    signal fileAccepted(url selected)

    fileMode: saveMode ? Labs.FileDialog.SaveFile : Labs.FileDialog.OpenFile
    onAccepted: fileAccepted(file)
}
