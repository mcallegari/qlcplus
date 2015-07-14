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

Rectangle
{
    id: seContainer
    anchors.fill: parent
    color: "transparent"

    property int functionID: -1

    Component.onDestruction: functionManager.setEditorFunction(-1)

    Column
    {
        Rectangle
        {
            color: "#333"
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
                    onClicked:
                    {
                        editorLoader.source = "qrc:/FunctionManager.qml"
                    }
                    onEntered: backBox.color = "#666"
                    onExited: backBox.color = "transparent"
                }
            }
            TextInput
            {
                id: sNameEdit
                x: leftArrow.width + 5
                y: 3
                height: 40
                width: seContainer.width // - addFunc.width - removeFunc.width
                color: "#ffffff"
                text: sceneEditor.sceneName
                font.pixelSize: 20
                echoMode: TextInput.Normal
                Layout.fillWidth: true

                onTextChanged: sceneEditor.sceneName = text
            }
        }

        ListView
        {
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
                }
        }
    }
}
