/*
  Q Light Controller Plus
  BeatGeneratorsPanel.qml

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

import QtQuick 2.3
import QtQuick.Controls 1.2
import QtQuick.Layouts 1.1
import "."

Rectangle
{
    id: beatChooserBox
    width: UISettings.bigItemHeight * 3
    height: generatorsList.height + keyPadBox.height + 20
    color: UISettings.bgMedium
    border.color: "#666"
    border.width: 2

    onVisibleChanged:
    {
        if (visible == true)
            generatorsList.model = ioManager.beatGeneratorsList()
    }

    ExclusiveGroup { id: selGeneratorGroup }

    ListView
    {
        id: generatorsList

        x: 10
        y: 10
        width: parent.width - 20
        height: UISettings.bigItemHeight * 2
        boundsBehavior: Flickable.StopAtBounds

        delegate:
            Rectangle
            {
                color: "transparent"
                height: UISettings.iconSizeDefault + 5

                Component.onCompleted:
                {
                    if (modelData.type === "MIDI")
                    {
                        iconBox.color = "white"
                        iconBox.visible = true
                        genIcon.source = "qrc:/midiplugin.svg"
                    }
                    else if (modelData.type === "AUDIO")
                    {
                        iconBox.color = "transparent"
                        iconBox.visible = true
                        genIcon.source = "qrc:/audiocard.svg"
                    }
                    else
                        iconBox.visible = false
                }

                Row
                {
                    spacing: 5

                    CustomCheckBox
                    {
                        exclusiveGroup: selGeneratorGroup
                        checked: ioManager.beatType === modelData.type
                        onCheckedChanged: if (checked) ioManager.beatType = modelData.type
                    }
                    Rectangle
                    {
                        id: iconBox
                        width: UISettings.iconSizeDefault
                        height: width
                        color: "transparent"

                        Image
                        {
                            id: genIcon
                            anchors.fill: parent
                            sourceSize: Qt.size(width, height)
                        }
                    }
                    RobotoText
                    {
                        label: modelData.name
                    }
                }
            }
    } // end of ListView

    KeyPad
    {
        id: keyPadBox
        anchors.top: generatorsList.bottom
        x: 10
        width: parent.width - 20
        showDMXcontrol: false
        visible: ioManager.beatType === "INTERNAL"
        commandString: ioManager.bpmNumber

        onExecuteCommand:
        {
            var intCmd = parseInt(cmd)
            if (intCmd === 0 || intCmd > 300)
                return

            ioManager.bpmNumber = cmd
        }
        onEscapePressed: beatChooserBox.visible = false
    }
}
