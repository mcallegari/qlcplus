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
import QtQuick.Controls 1.0
import QtQuick.Window 2.0

import "."

ApplicationWindow
{
    id: window
    width: 800
    height: 600
    color: UISettings.bgMain

    property string viewSource
    property string loadedContext: ""

    onClosing:
    {
        // force the Loader to destroy the QML item
        viewSource = ""
        contextManager.reattachContext(loadedContext)
    }

    Loader
    {
        anchors.fill: parent
        source: viewSource
        onLoaded:
        {
            window.loadedContext = item.contextName
            //console.log("Detached context: " + window.loadedContext)
            contextManager.detachContext(loadedContext)
        }
    }
}

