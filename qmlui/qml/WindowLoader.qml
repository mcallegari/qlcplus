/*
  Q Light Controller Plus
  WindowLoader.qml

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

import QtQuick 2.1

import "."

/** This is a wrapper item to achieve 3 things:
  * 1) load a detached context on a surface with the application background
  * 2) centralize the reattach process via context manager
  * 3) expose a dedicated mainView reference which is used by combo boxes and side panels
  *    to calculate absolute positions within a window
  */

Rectangle
{
    id: mainView
    anchors.fill: parent
    color: UISettings.bgMedium

    function closeWindow()
    {
        console.log("Closing window of context: " + contextName)
        if (contextManager)
            contextManager.enableContext(contextName, false, wLoader.item.contextItem)
    }

    Loader
    {
        id: wLoader
        anchors.fill: parent
        source: viewSource
        onLoaded: if (item.page) item.page = contextPage
    }
}

