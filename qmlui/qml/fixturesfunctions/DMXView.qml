/*
  Q Light Controller Plus
  DMXView.qml

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
    id: dmxViewRoot
    anchors.fill: parent
    color: UISettings.bgMain

    property alias contextItem: flowLayout

    function hasSettings()
    {
        return false;
    }

    Flickable
    {
        id: fixtureDMXView
        anchors.fill: parent
        anchors.leftMargin: 20
        anchors.topMargin: 20
        anchors.bottomMargin: 20

        contentHeight: flowLayout.height
        contentWidth: flowLayout.width
        interactive: false

        boundsBehavior: Flickable.StopAtBounds

        property string contextName: "DMX"

        Flow
        {
            id: flowLayout
            objectName: "DMXFlowView"
            spacing: 5
            width: dmxViewRoot.width

            Component.onCompleted: contextManager.enableContext("DMX", true, flowLayout)
            Component.onDestruction: contextManager.enableContext("DMX", false, flowLayout)
        }
    }
    CustomScrollBar { flickable: fixtureDMXView }
}
