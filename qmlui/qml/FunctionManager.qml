/*
  Q Light Controller Plus
  FunctionManager.qml

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

Rectangle {
    id: fmContainer
    anchors.fill: parent
    color: "transparent"

    ColumnLayout {

      anchors.fill: parent
      spacing: 3

      Rectangle {
        id: topBar
        width: parent.width
        height: 44
        z: 5
        gradient: Gradient {
            id: ffMenuGradient
            GradientStop { position: 0 ; color: "#222" }
            GradientStop { position: 1 ; color: "#111" }
        }

        RowLayout {
            id: topBarRowLayout
            width: parent.width
            y: 1

            spacing: 4
            ExclusiveGroup { id: menuBarGroup3 }

            IconButton {
                id: sceneFunc
                z: 2
                width: height
                height: topBar.height - 2
                imgSource: "qrc:/scene.svg"
                checkable: true
                checked: true
                tooltip: qsTr("Scenes")
                exclusiveGroup: menuBarGroup3
                onCheckedChanged: {

                }
            }
            IconButton {
                id: chaserFunc
                z: 2
                width: height
                height: topBar.height - 2
                imgSource: "qrc:/chaser.svg"
                checkable: true
                tooltip: qsTr("Chasers")
                exclusiveGroup: menuBarGroup3
                onCheckedChanged: {

                }
            }
            IconButton {
                id: efxFunc
                z: 2
                width: height
                height: topBar.height - 2
                imgSource: "qrc:/efx.svg"
                checkable: true
                tooltip: qsTr("EFX")
                exclusiveGroup: menuBarGroup3
                onCheckedChanged: {

                }
            }
            IconButton {
                id: collectionFunc
                z: 2
                width: height
                height: topBar.height - 2
                imgSource: "qrc:/collection.svg"
                checkable: true
                tooltip: qsTr("Collections")
                exclusiveGroup: menuBarGroup3
                onCheckedChanged: {

                }
            }
            IconButton {
                id: rgbmFunc
                z: 2
                width: height
                height: topBar.height - 2
                imgSource: "qrc:/rgbmatrix.svg"
                checkable: true
                tooltip: qsTr("RGB Matrices")
                exclusiveGroup: menuBarGroup3
                onCheckedChanged: {

                }
            }
            IconButton {
                id: scriptFunc
                z: 2
                width: height
                height: topBar.height - 2
                imgSource: "qrc:/script.svg"
                checkable: true
                tooltip: qsTr("Scripts")
                exclusiveGroup: menuBarGroup3
                onCheckedChanged: {

                }
            }
            IconButton {
                id: audioFunc
                z: 2
                width: height
                height: topBar.height - 2
                imgSource: "qrc:/audio.svg"
                checkable: true
                tooltip: qsTr("Audio")
                exclusiveGroup: menuBarGroup3
                onCheckedChanged: {

                }
            }
            IconButton {
                id: videoFunc
                z: 2
                width: height
                height: topBar.height - 2
                imgSource: "qrc:/video.svg"
                checkable: true
                tooltip: qsTr("Videos")
                exclusiveGroup: menuBarGroup3
                onCheckedChanged: {

                }
            }
            Rectangle {
                Layout.fillWidth: true
            }
        }
      }

      ListView {
          id: functionsListView
          width: fmContainer.width
          height: fmContainer.height - topBar.height
          z: 4
          model: functionManager.functionsList
          delegate:
              Component {
                  Loader {
                      width: parent.width
                      source: hasChildren ? "FolderDelegate.qml" : "FunctionDelegate.qml"
                      onLoaded: {
                          item.textLabel = label
                          if (hasChildren)
                          {
                              item.folderChildren = childrenModel
                              item.childrenHeight = (childrenModel.rowCount() * 35)
                          }
                          else
                          {
                              item.functionID = funcID
                              item.functionType = funcType
                          }
                      }
                  }
              }
      }
    }
}
