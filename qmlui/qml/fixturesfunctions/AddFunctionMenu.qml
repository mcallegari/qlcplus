/*
  Q Light Controller Plus
  AddFunctionMenu.qml

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
import QtQuick.Controls 2.1

import org.qlcplus.classes 1.0
import "."

Popup
{
    id: menuRoot
    padding: 0

    signal requestFolder()
    signal entryClicked(int fType)

    background:
        Rectangle
        {
            color: UISettings.bgMedium
        }

    Column
    {
        id: addFuncMenuEntries
        ContextMenuEntry
        {
            imgSource: "qrc:/folder.svg"
            entryText: qsTr("New folder")
            onClicked:
            {
                functionManager.createFolder()
                menuRoot.close()
            }
        }

        ContextMenuEntry
        {
            imgSource: "qrc:/scene.svg"
            entryText: qsTr("New Scene")
            onClicked: entryClicked(QLCFunction.SceneType)
        }
        ContextMenuEntry
        {
            imgSource: "qrc:/chaser.svg"
            entryText: qsTr("New Chaser")
            onClicked: entryClicked(QLCFunction.ChaserType)
        }
        ContextMenuEntry
        {
            imgSource: "qrc:/sequence.svg"
            entryText: qsTr("New Sequence")
            onClicked: entryClicked(QLCFunction.SequenceType)
        }
        ContextMenuEntry
        {
            imgSource: "qrc:/efx.svg"
            entryText: qsTr("New EFX")
            onClicked: entryClicked(QLCFunction.EFXType)
        }
        ContextMenuEntry
        {
            imgSource: "qrc:/collection.svg"
            entryText: qsTr("New Collection")
            onClicked: entryClicked(QLCFunction.CollectionType)
        }
        ContextMenuEntry
        {
            imgSource: "qrc:/rgbmatrix.svg"
            entryText: qsTr("New RGB Matrix")
            onClicked: entryClicked(QLCFunction.RGBMatrixType)
        }
        ContextMenuEntry
        {
            imgSource: "qrc:/showmanager.svg"
            entryText: qsTr("New Show")
            onClicked: entryClicked(QLCFunction.ShowType)
        }
        ContextMenuEntry
        {
            imgSource: "qrc:/script.svg"
            entryText: qsTr("New Script")
            onClicked: entryClicked(QLCFunction.ScriptType)
        }
        ContextMenuEntry
        {
            imgSource: "qrc:/audio.svg"
            entryText: qsTr("New Audio")
            onClicked: entryClicked(QLCFunction.AudioType)
        }
        ContextMenuEntry
        {
            imgSource: "qrc:/video.svg"
            entryText: qsTr("New Video")
            onClicked: entryClicked(QLCFunction.VideoType)
        }
    }
}
