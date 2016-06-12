/*
  Q Light Controller Plus
  AudioCardsList.qml

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
    id: aclContainer
    anchors.fill: parent
    color: "transparent"

    property int universeIndex: 0

    function loadSources(input)
    {
        if (input === true)
            acListView.model = ioManager.audioInputSources()
        else
            acListView.model = ioManager.audioOutputSources()
    }

    ListView
    {
        id: acListView
        anchors.fill: parent
        boundsBehavior: Flickable.StopAtBounds
        //model: ioManager.audioInputSources()
        delegate:
            Rectangle
            {
                id: acDelegate
                width: aclContainer.width
                height: UISettings.listItemHeight * 1.7
                color: "transparent"
                Row
                {
                    Image
                    {
                        id: pIcon
                        height: acDelegate.height - 4
                        width: height
                        x: 2
                        y: 2
                        source: "qrc:/audiocard.svg"
                        sourceSize: Qt.size(width, height)
                        fillMode: Image.Stretch
                    }

                    RobotoText
                    {
                        height: acDelegate.height
                        width: acDelegate.width - pIcon.width
                        label: modelData.name
                        wrapText: true
                    }
                }
                Rectangle
                {
                    width: acDelegate.width
                    height: 1
                    y: acDelegate.height - 1
                    color: "#555"
                }
            }
    }
}

