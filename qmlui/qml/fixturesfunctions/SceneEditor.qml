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
    property bool showToolBar: true

    signal requestView(int ID, string qmlSrc)

    ModelSelector
    {
        id: seSelector
        onItemsCountChanged: console.log("Scene Editor selected items changed !")
    }

    Column
    {
        EditorTopBar
        {
            visible: showToolBar
            text: sceneEditor.functionName
            onTextChanged: sceneEditor.functionName = text

            onBackClicked:
            {
                functionManager.setEditorFunction(-1, false)
                requestView(-1, "qrc:/FunctionManager.qml")
            }

            IconButton
            {
                id: removeFxButton
                x: parent.width - UISettings.iconSizeMedium - 5
                width: height
                height: UISettings.iconSizeMedium
                imgSource: "qrc:/remove.svg"
                tooltip: qsTr("Remove the selected fixtures")
                onClicked: { /* TODO */  }
            }
        }

        ListView
        {
            id: sfxList
            width: seContainer.width
            height: seContainer.height - UISettings.iconSizeMedium
            y: UISettings.iconSizeMedium
            boundsBehavior: Flickable.StopAtBounds
            model: sceneEditor.fixtureList
            delegate:
                FixtureDelegate
                {
                    cRef: model.fxRef
                    width: seContainer.width
                    isSelected: model.isSelected
                    Component.onCompleted: contextManager.setFixtureSelection(cRef.id, true)
                    Component.onDestruction: contextManager.setFixtureSelection(cRef.id, false)
                    onMouseEvent:
                    {
                        if (type === App.Clicked)
                        {
                            seSelector.selectItem(index, sfxList.model, mouseMods & Qt.ControlModifier)

                            if (!(mouseMods & Qt.ControlModifier))
                                contextManager.resetFixtureSelection()

                            contextManager.setFixtureSelection(cRef.id, true)
                            sceneEditor.setFixtureSelection(cRef.id)
                        }
                    }
                }
            CustomScrollBar { flickable: sfxList }
        }
    }
}
