/*
  Q Light Controller Plus
  ExternalControls.qml

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

import org.qlcplus.classes 1.0
import "."

Column
{
    width: parent.width

    /** This is a reference to a generic object exposing
      * an input sources list */
    property var objRef: null

    Rectangle
    {
        color: UISettings.bgMedium
        width: parent.width
        height: UISettings.iconSizeMedium

        // add/remove "toolbar"
        RowLayout
        {
            anchors.fill: parent
            spacing: 5

            Rectangle
            {
                Layout.fillWidth: true
                height: parent.height
                color: "transparent"
            }

            IconButton
            {
                id: addInputSource

                width: height * 2
                height: parent.height
                imgSource: "qrc:/inputoutput.svg"
                tooltip: qsTr("Add an external controller input")

                Image
                {
                    anchors.right: parent.right
                    width: parent.height / 2
                    height: width
                    source: "qrc:/add.svg"
                    sourceSize: Qt.size(width, height)
                }

                onClicked: virtualConsole.createAndDetectInputSource(objRef)
            }

            IconButton
            {
                id: addKeyboardSource

                width: height * 2
                height: parent.height
                imgSource: "qrc:/keybinding.svg"
                tooltip: qsTr("Add a keyboard combination")

                Image
                {
                    anchors.right: parent.right
                    width: parent.height / 2
                    height: width
                    source: "qrc:/add.svg"
                    sourceSize: Qt.size(width, height)
                }

                onClicked: virtualConsole.createAndDetectInputKey(objRef)
            }
        }
    }

    ListView
    {
        id: sourcesListView
        width: parent.width
        height: contentHeight
        boundsBehavior: Flickable.StopAtBounds

        model: objRef ? objRef.inputSourcesList : null

        delegate:
            Loader
            {
                width: sourcesListView.width
                source: modelData.type === VCWidget.Controller ? "qrc:/ExternalControlDelegate.qml" : "qrc:/KeyboardSequenceDelegate.qml"
                onLoaded:
                {
                    item.dObjRef = objRef
                    item.controlID = modelData.id
                    if (modelData.invalid)
                        item.invalid = modelData.invalid

                    if (modelData.type === VCWidget.Controller)
                    {
                        item.universe = modelData.universe
                        item.channel = modelData.channel
                        item.uniName = modelData.uniString
                        item.chName = modelData.chString
                        item.customFeedback = modelData.customFeedback
                        item.lowerFb = modelData.lower
                        item.upperFb = modelData.upper
                    }
                    else if (modelData.type === VCWidget.Keyboard)
                    {
                        item.sequence = modelData.keySequence
                    }
                }
            }
    }
}
