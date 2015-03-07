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

import com.qlcplus.enums 1.0

Rectangle {
    id: fmContainer
    anchors.fill: parent
    color: "transparent"

    ColumnLayout {

      anchors.fill: parent
      spacing: 3

      Rectangle {
        id: topBar
        width: fmContainer.width
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

            IconButton {
                id: sceneFunc
                z: 2
                width: height
                height: topBar.height - 2
                imgSource: "qrc:/scene.svg"
                checkable: true
                tooltip: qsTr("Scenes")
                counter: functionManager.sceneCount
                onCheckedChanged: {
                    if (checked == true)
                        functionManager.setFunctionFilter(FunctionType.Scene, true);
                    else
                        functionManager.setFunctionFilter(FunctionType.Scene, false);
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
                counter: functionManager.chaserCount
                onCheckedChanged: {
                    if (checked == true)
                        functionManager.setFunctionFilter(FunctionType.Chaser, true);
                    else
                        functionManager.setFunctionFilter(FunctionType.Chaser, false);
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
                counter: functionManager.efxCount
                onCheckedChanged: {
                    if (checked == true)
                        functionManager.setFunctionFilter(FunctionType.EFX, true);
                    else
                        functionManager.setFunctionFilter(FunctionType.EFX, false);
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
                counter: functionManager.collectionCount
                onCheckedChanged: {
                    if (checked == true)
                        functionManager.setFunctionFilter(FunctionType.Collection, true);
                    else
                        functionManager.setFunctionFilter(FunctionType.Collection, false);
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
                counter: functionManager.rgbMatrixCount
                onCheckedChanged: {
                    if (checked == true)
                        functionManager.setFunctionFilter(FunctionType.RGBMatrix, true);
                    else
                        functionManager.setFunctionFilter(FunctionType.RGBMatrix, false);
                }
            }
            IconButton {
                id: showFunc
                z: 2
                width: height
                height: topBar.height - 2
                imgSource: "qrc:/showmanager.svg"
                checkable: true
                tooltip: qsTr("Shows")
                counter: functionManager.showCount
                onCheckedChanged: {
                    if (checked == true)
                        functionManager.setFunctionFilter(FunctionType.Show, true);
                    else
                        functionManager.setFunctionFilter(FunctionType.Show, false);
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
                counter: functionManager.scriptCount
                exclusiveGroup: menuBarGroup3
                onCheckedChanged: {
                    if (checked == true)
                        functionManager.setFunctionFilter(FunctionType.Script, true);
                    else
                        functionManager.setFunctionFilter(FunctionType.Script, false);
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
                counter: functionManager.audioCount
                exclusiveGroup: menuBarGroup3
                onCheckedChanged: {
                    if (checked == true)
                        functionManager.setFunctionFilter(FunctionType.Audio, true);
                    else
                        functionManager.setFunctionFilter(FunctionType.Audio, false);
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
                counter: functionManager.videoCount
                exclusiveGroup: menuBarGroup3
                onCheckedChanged: {
                    if (checked == true)
                        functionManager.setFunctionFilter(FunctionType.Video, true);
                    else
                        functionManager.setFunctionFilter(FunctionType.Video, false);
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
