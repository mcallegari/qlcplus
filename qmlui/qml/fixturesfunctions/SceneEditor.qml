/*
  Q Light Controller Plus
  SceneEditor.qml

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

import com.qlcplus.classes 1.0
import "."

Rectangle
{
    id: seContainer
    anchors.fill: parent
    color: "transparent"

    property int functionID
    property int selectedFixtureIndex: -1

    signal requestView(int ID, string qmlSrc)

    Component.onDestruction: functionManager.setEditorFunction(-1)

    function selectFixture(index)
    {
        if (selectedFixtureIndex != -1)
        sfxList.contentItem.children[selectedFixtureIndex].isSelected = false
        selectedFixtureIndex = index
    }

    Column
    {
        Rectangle
        {
            color: UISettings.bgMedium
            width: seContainer.width
            height: 40

            Rectangle
            {
                id: backBox
                width: 40
                height: 40
                color: "transparent"

                Image
                {
                    id: leftArrow
                    anchors.fill: parent
                    rotation: 180
                    source: "qrc:/arrow-right.svg"
                    sourceSize: Qt.size(width, height)
                }
                MouseArea
                {
                    anchors.fill: parent
                    hoverEnabled: true
                    onEntered: backBox.color = "#666"
                    onExited: backBox.color = "transparent"
                    onClicked:requestView(-1, "qrc:/FunctionManager.qml")
                }
            }
            TextInput
            {
                id: sNameEdit
                x: leftArrow.width + 5
                height: 40
                width: seContainer.width - backBox.width - removeFxButton.width - 10
                color: UISettings.fgMain
                clip: true
                text: sceneEditor.sceneName
                verticalAlignment: TextInput.AlignVCenter
                font.family: "Roboto Condensed"
                font.pixelSize: 20
                echoMode: TextInput.Normal
                selectByMouse: true
                Layout.fillWidth: true

                onTextChanged: sceneEditor.sceneName = text
            }
            IconButton
            {
                id: removeFxButton
                x: parent.width - 45
                width: height
                height: 40
                imgSource: "qrc:/remove.svg"
                tooltip: qsTr("Remove the selected fixture")
                onClicked: {   }
            }
        }

        ListView
        {
            id: sfxList
            width: seContainer.width
            height: seContainer.height - 40
            y: 40
            boundsBehavior: Flickable.StopAtBounds
            model: sceneEditor.fixtures
            delegate:
                FixtureDelegate
                {
                    cRef: modelData
                    width: seContainer.width

                    Component.onCompleted: contextManager.setFixtureSelection(cRef.id, true)
                    Component.onDestruction: contextManager.setFixtureSelection(cRef.id, false)
                    onClicked:
                    {
                        sceneEditor.setFixtureSelection(cRef.id)
                        seContainer.selectFixture(index)
                    }
                }
        }
    }
}
