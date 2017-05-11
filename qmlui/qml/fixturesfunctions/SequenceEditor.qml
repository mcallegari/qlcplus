/*
  Q Light Controller Plus
  SequenceEditor.qml

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
import QtQuick.Controls 1.2

import com.qlcplus.classes 1.0
import "."

Rectangle
{
    id: seqContainer
    anchors.fill: parent
    color: "transparent"

    property int functionID: -1

    signal requestView(int ID, string qmlSrc)

    Column
    {
        width: parent.width

        EditorTopBar
        {
            text: chaserEditor.functionName
            onTextChanged: chaserEditor.functionName = text

            onBackClicked:
            {
                functionManager.setEditorFunction(-1, false)
                requestView(-1, "qrc:/FunctionManager.qml")
            }

            IconButton
            {
                visible: stepsView.checked
                width: height
                height: UISettings.iconSizeMedium - 2
                imgSource: "qrc:/add.svg"
                tooltip: qsTr("Add a new step")
                onClicked: chaserEditor.addStep(chaserEditor.playbackIndex)
            }

            IconButton
            {
                width: height
                height: UISettings.iconSizeMedium - 2
                imgSource: "qrc:/remove.svg"
                tooltip: stepsView.checked ? qsTr("Remove the selected steps") : qsTr("Remove the selected fixtures")
                onClicked: {   }
            }
        }

        Rectangle
        {
            id: selectToolBar
            width: parent.width
            height: UISettings.listItemHeight
            z: 10
            gradient:
                Gradient
                {
                    id: cBarGradient
                    GradientStop { position: 0; color: UISettings.toolbarStartSub }
                    GradientStop { position: 1; color: UISettings.toolbarEnd }
                }

            ExclusiveGroup { id: seqExGroup }

            MenuBarEntry
            {
                id: stepsView
                width: parent.width / 2
                entryText: qsTr("Steps")
                checkable: true
                checked: true
                checkedColor: UISettings.toolbarSelectionSub
                bgGradient: cBarGradient
                exclusiveGroup: seqExGroup
                mFontSize: UISettings.textSizeDefault
            }

            MenuBarEntry
            {
                id: fixturesView
                width: parent.width / 2
                anchors.left: stepsView.right
                entryText: qsTr("Fixtures")
                checkable: true
                checkedColor: UISettings.toolbarSelectionSub
                bgGradient: cBarGradient
                exclusiveGroup: seqExGroup
                mFontSize: UISettings.textSizeDefault
            }
        }

        Loader
        {
            id: chaserEditorLoader
            width: parent.width
            height: seqContainer.height - UISettings.iconSizeDefault - selectToolBar.height
            visible: stepsView.checked
            source: "qrc:/ChaserEditor.qml"

            onLoaded:
            {
                item.functionID = functionID
            }
        }

        Loader
        {
            id: sceneEditorLoader
            width: parent.width
            height: seqContainer.height - UISettings.iconSizeDefault - selectToolBar.height
            visible: fixturesView.checked
            source: "qrc:/SceneEditor.qml"

            onLoaded:
            {
                item.showToolBar = false
            }
        }
    }
}
