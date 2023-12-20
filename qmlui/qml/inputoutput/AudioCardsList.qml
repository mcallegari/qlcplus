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
    property bool isInput: false

    function loadSources(input)
    {
        acListView.model = input ? ioManager.audioInputSources : ioManager.audioOutputSources
        isInput = input
    }

    ListView
    {
        id: acListView
        anchors.fill: parent
        boundsBehavior: Flickable.StopAtBounds
        //model: ioManager.audioInputSources()
        delegate:
            Item
            {
                id: root
                height: UISettings.listItemHeight * 1.7
                width: aclContainer.width

                MouseArea
                {
                    id: delegateRoot
                    width: aclContainer.width
                    height: parent.height

                    drag.target: acDelegate
                    drag.threshold: height / 2

                    onReleased:
                    {
                        if (acDelegate.Drag.target !== null)
                        {
                            acDelegate.Drag.drop()
                            if (aclContainer.isInput === false)
                                ioManager.setAudioOutput(modelData.privateName)
                            else
                                ioManager.setAudioInput(modelData.privateName)
                        }
                        else
                        {
                            // return the dragged item to its original position
                            parent = root
                        }
                        acDelegate.x = 3
                        acDelegate.y = 0
                    }

                    Rectangle
                    {
                        id: acDelegate
                        x: 3
                        width: aclContainer.width
                        height: UISettings.listItemHeight * 1.7
                        color: delegateRoot.pressed ? UISettings.highlightPressed : "transparent"

                        // this key must match the one in AudioIOItem, to avoid dragging
                        // an audio input on output and vice-versa
                        property string dragKey: isInput ? "audioInput" : "audioOutput"

                        Drag.active: delegateRoot.drag.active
                        Drag.source: acDelegate
                        Drag.keys: [ dragKey ]

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
                                label: modelData.mLabel
                                wrapText: true
                            }
                        }
                        Rectangle
                        {
                            width: acDelegate.width
                            height: 1
                            y: acDelegate.height - 1
                            color: UISettings.bgLight
                        }
                    }
                }// MouseArea
            } // Item
    } // ListView
}

