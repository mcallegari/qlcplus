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

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import org.qlcplus.classes 1.0
import "."

Rectangle
{
    id: seqContainer
    anchors.fill: parent
    color: "transparent"

    property int functionID: -1

    signal requestView(int ID, string qmlSrc, bool back)

    SplitView
    {
        anchors.fill: parent

        handle: Rectangle
        {
            implicitWidth: screenPixelDensity * UISettings.scalingFactor * 0.9
            color: SplitHandle.hovered || SplitHandle.pressed ? UISettings.highlight : UISettings.bgLighter
        }

        Loader
        {
            id: sideLoader
            width: UISettings.sidePanelWidth
            SplitView.preferredWidth: UISettings.sidePanelWidth
            visible: false
            height: seqContainer.height
            source: ""

            onLoaded:
            {
                if (source)
                    item.allowEditing = false
            }

            Rectangle
            {
                width: 2
                height: parent.height
                x: parent.width - 2
                color: UISettings.bgLight
            }
        }

        Column
        {
            SplitView.fillWidth: true

            EditorTopBar
            {
                id: topBar
                text: chaserEditor.functionName
                onTextChanged: chaserEditor.functionName = text

                onBackClicked:
                {
                    if (addFixture.checked)
                        addFixture.checked = false

                    var prevID = chaserEditor.previousID
                    requestView(prevID, functionManager.getEditorResource(prevID), true)
                }

                IconButton
                {
                    visible: stepsView.checked
                    width: height
                    height: UISettings.iconSizeMedium - 2
                    faSource: FontAwesome.fa_plus
                    faColor: "limegreen"
                    tooltip: qsTr("Add a new step")
                    onClicked: chaserEditor.addStep(chaserEditor.playbackIndex)
                }

                IconButton
                {
                    id: addFixture
                    visible: fixturesView.checked
                    width: height
                    height: UISettings.iconSizeMedium - 2
                    imgSource: "qrc:/fixture.svg"
                    checkable: true
                    tooltip: qsTr("Add a fixture/group")

                    Image
                    {
                        x: parent.width - width - 2
                        y: 2
                        width: parent.height / 3
                        height: width
                        source: "qrc:/add.svg"
                        sourceSize: Qt.size(width, height)
                    }

                    onCheckedChanged:
                    {
                        if (checked)
                        {
                            if (!sideLoader.visible)
                                rightSidePanel.width += UISettings.sidePanelWidth
                            sideLoader.visible = true
                            sideLoader.source = "qrc:/FixtureGroupManager.qml"
                        }
                        else if (sideLoader.visible)
                        {
                            rightSidePanel.width -= sideLoader.width
                            sideLoader.source = ""
                            sideLoader.visible = false
                        }
                    }
                }

                IconButton
                {
                    width: height
                    height: UISettings.iconSizeMedium - 2
                    faSource: FontAwesome.fa_minus
                    faColor: "crimson"
                    tooltip: stepsView.checked ? qsTr("Remove the selected steps") : qsTr("Remove the selected fixtures")
                    onClicked:
                    {
                        if (stepsView.checked)
                        {
                            chaserEditorLoader.item.deleteSelectedItems()
                        }
                        else
                        {
                            sceneEditorLoader.item.deleteSelectedItems()
                        }
                    }
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

                ButtonGroup { id: seqExGroup }

                MenuBarEntry
                {
                    id: stepsView
                    width: parent.width / 2
                    entryText: qsTr("Steps")
                    checked: true
                    checkedColor: UISettings.toolbarSelectionSub
                    bgGradient: cBarGradient
                    ButtonGroup.group: seqExGroup
                    mFontSize: UISettings.textSizeDefault

                    onCheckedChanged:
                    {
                        if (checked)
                            addFixture.checked = false
                    }
                }

                MenuBarEntry
                {
                    id: fixturesView
                    width: parent.width / 2
                    anchors.left: stepsView.right
                    entryText: qsTr("Fixtures")
                    checkedColor: UISettings.toolbarSelectionSub
                    bgGradient: cBarGradient
                    ButtonGroup.group: seqExGroup
                    mFontSize: UISettings.textSizeDefault
                }
            }

            Loader
            {
                id: chaserEditorLoader
                width: parent.width
                height: seqContainer.height - topBar.height - selectToolBar.height
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
                height: seqContainer.height - topBar.height - selectToolBar.height
                visible: fixturesView.checked
                source: "qrc:/SceneEditor.qml"

                onLoaded:
                {
                    item.boundToSequence = true
                }
            }
        }
    }
}
