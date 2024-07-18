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

import QtQuick 2.6
import QtQuick.Controls 2.1
import QtQuick.Layouts 1.1

import "GenericHelpers.js" as Helpers
import "."

Rectangle
{
    id: beatChooserBox
    width: UISettings.bigItemHeight * 3
    height: toolbar.height + contentsColumn.height + 10
    color: UISettings.bgMedium
    border.color: UISettings.bgLight
    border.width: 2

    onVisibleChanged:
    {
        if (visible == true)
            generatorsList.model = ioManager.beatGeneratorsList()
    }

    ButtonGroup { id: selGeneratorGroup }

    Rectangle
    {
        id: toolbar
        x: 2
        y: 2
        z: 1
        width: parent.width - 4
        height: UISettings.listItemHeight

        gradient:
            Gradient
            {
                id: cBarGradient
                GradientStop { position: 0; color: UISettings.toolbarStartSub }
                GradientStop { position: 1; color: UISettings.toolbarEnd }
            }

        // allow the tool to be dragged around
        // by holding it on the title bar
        MouseArea
        {
            anchors.fill: parent
            drag.target: beatChooserBox
        }
        GenericButton
        {
            width: height
            height: parent.height
            anchors.right: parent.right
            border.color: UISettings.bgMedium
            useFontawesome: true
            label: FontAwesome.fa_times
            onClicked: beatChooserBox.visible = false
        }
    }

    Column
    {
        id: contentsColumn
        spacing: 3
        x: 5
        y: toolbar.height + 7
        width: parent.width - 10

        ListView
        {
            id: generatorsList

            width: parent.width
            height: UISettings.bigItemHeight * 2
            clip: true
            boundsBehavior: Flickable.StopAtBounds

            delegate:
                Rectangle
                {
                    color: "transparent"
                    height: UISettings.iconSizeDefault + 5

                    Component.onCompleted:
                    {
                        if (modelData.type === "PLUGIN")
                        {
                            iconBox.color = "white"
                            iconBox.visible = true
                            genIcon.source = Helpers.pluginIconFromName(modelData.privateName)
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
                            ButtonGroup.group: selGeneratorGroup
                            checked: ioManager.beatType === modelData.type
                            onClicked: if (checked) ioManager.beatType = modelData.type
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
            width: parent.width
            height: UISettings.iconSizeDefault * 6
            showDMXcontrol: false
            showTapButton: true
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

            onTapTimeChanged: ioManager.bpmNumber = Math.min(parseInt(60000 / time), 300)
        }
    }
}
