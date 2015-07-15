/*
  Q Light Controller Plus
  SceneFixtureConsole.qml

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

Rectangle
{
    id: sfcContainer
    anchors.fill: parent
    color: "transparent"

    ListView
    {
        anchors.fill: parent
        orientation: ListView.Horizontal
        model: sceneEditor.fixtures
        boundsBehavior: Flickable.StopAtBounds

        delegate:
            FixtureConsole
            {
                fixtureObj: modelData
                height: parent.height
                color: index % 2 ? "#202020" : "#303030"
                showEnablers: true
                sceneConsole: true
            }
    }
}
