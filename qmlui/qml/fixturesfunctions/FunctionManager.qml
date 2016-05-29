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

import com.qlcplus.classes 1.0
import "."

Rectangle
{
    id: fmContainer
    anchors.fill: parent
    color: "transparent"
    clip: true

    property int functionID: -1

    signal requestView(int ID, string qmlSrc)

    //Component.onDestruction: functionManager.clearTree()

    function loadFunctionEditor(funcID, funcType)
    {
        //console.log("Request to open Function editor. ID: " + funcID + " type: " + funcType)
        functionManager.setEditorFunction(funcID)
        functionManager.viewPosition = functionsListView.contentY

        switch(funcType)
        {
            case Function.Scene:
                fmContainer.requestView(funcID, "qrc:/SceneEditor.qml")
            break;
            case Function.Collection:
                fmContainer.requestView(funcID, "qrc:/CollectionEditor.qml")
            break;
            case Function.Chaser:
                fmContainer.requestView(funcID, "qrc:/ChaserEditor.qml")
            break;
            case Function.RGBMatrix:
                fmContainer.requestView(funcID, "qrc:/RGBMatrixEditor.qml")
            break;
            case Function.Show:
                showManager.currentShowID = funcID
                mainView.switchToContext("SHOWMGR", "qrc:/ShowManager.qml")
            break;
        }
    }

    function setFunctionFilter(fType, checked)
    {
        if (checked === true)
            functionManager.setFunctionFilter(fType, true);
        else
            functionManager.setFunctionFilter(fType, false);
    }

    ColumnLayout
    {
      anchors.fill: parent
      spacing: 3

      Rectangle
      {
        id: topBar
        width: fmContainer.width
        height: 44
        z: 5
        gradient: Gradient
        {
            GradientStop { position: 0; color: UISettings.toolbarStartSub }
            GradientStop { position: 1; color: UISettings.toolbarEnd }
        }

        RowLayout
        {
            id: topBarRowLayout
            width: parent.width
            y: 1

            spacing: 4

            IconButton
            {
                id: sceneFunc
                z: 2
                width: height
                height: topBar.height - 2
                imgSource: "qrc:/scene.svg"
                checkable: true
                checked: functionManager.functionsFilter & Function.Scene
                tooltip: qsTr("Scenes")
                counter: functionManager.sceneCount
                onCheckedChanged: setFunctionFilter(Function.Scene, checked)
            }
            IconButton
            {
                id: chaserFunc
                z: 2
                width: height
                height: topBar.height - 2
                imgSource: "qrc:/chaser.svg"
                checkable: true
                checked: functionManager.functionsFilter & Function.Chaser
                tooltip: qsTr("Chasers")
                counter: functionManager.chaserCount
                onCheckedChanged: setFunctionFilter(Function.Chaser, checked)
            }
            IconButton
            {
                id: efxFunc
                z: 2
                width: height
                height: topBar.height - 2
                imgSource: "qrc:/efx.svg"
                checkable: true
                checked: functionManager.functionsFilter & Function.EFX
                tooltip: qsTr("EFX")
                counter: functionManager.efxCount
                onCheckedChanged: setFunctionFilter(Function.EFX, checked)
            }
            IconButton
            {
                id: collectionFunc
                z: 2
                width: height
                height: topBar.height - 2
                imgSource: "qrc:/collection.svg"
                checkable: true
                checked: functionManager.functionsFilter & Function.Collection
                tooltip: qsTr("Collections")
                counter: functionManager.collectionCount
                onCheckedChanged: setFunctionFilter(Function.Collection, checked)
            }
            IconButton
            {
                id: rgbmFunc
                z: 2
                width: height
                height: topBar.height - 2
                imgSource: "qrc:/rgbmatrix.svg"
                checkable: true
                checked: functionManager.functionsFilter & Function.RGBMatrix
                tooltip: qsTr("RGB Matrices")
                counter: functionManager.rgbMatrixCount
                onCheckedChanged: setFunctionFilter(Function.RGBMatrix, checked)
            }
            IconButton
            {
                id: showFunc
                z: 2
                width: height
                height: topBar.height - 2
                imgSource: "qrc:/showmanager.svg"
                checkable: true
                checked: functionManager.functionsFilter & Function.Show
                tooltip: qsTr("Shows")
                counter: functionManager.showCount
                onCheckedChanged: setFunctionFilter(Function.Show, checked)
            }
            IconButton
            {
                id: scriptFunc
                z: 2
                width: height
                height: topBar.height - 2
                imgSource: "qrc:/script.svg"
                checkable: true
                checked: functionManager.functionsFilter & Function.Script
                tooltip: qsTr("Scripts")
                counter: functionManager.scriptCount
                onCheckedChanged: setFunctionFilter(Function.Script, checked)
            }
            IconButton
            {
                id: audioFunc
                z: 2
                width: height
                height: topBar.height - 2
                imgSource: "qrc:/audio.svg"
                checkable: true
                checked: functionManager.functionsFilter & Function.Audio
                tooltip: qsTr("Audio")
                counter: functionManager.audioCount
                onCheckedChanged: setFunctionFilter(Function.Audio, checked)
            }
            IconButton
            {
                id: videoFunc
                z: 2
                width: height
                height: topBar.height - 2
                imgSource: "qrc:/video.svg"
                checkable: true
                checked: functionManager.functionsFilter & Function.Video
                tooltip: qsTr("Videos")
                counter: functionManager.videoCount
                onCheckedChanged: setFunctionFilter(Function.Video, checked)
            }
            Rectangle { Layout.fillWidth: true }
        }
      }

      ListView
      {
          id: functionsListView
          width: fmContainer.width
          height: fmContainer.height - topBar.height
          z: 4
          boundsBehavior: Flickable.StopAtBounds
          Layout.fillHeight: true

          Component.onCompleted: contentY = functionManager.viewPosition

          model: functionManager.functionsList
          delegate:
              Component
              {
                  Loader
                  {
                      width: parent.width
                      source: hasChildren ? "qrc:/TreeNodeDelegate.qml" : "qrc:/FunctionDelegate.qml"

                      onLoaded:
                      {
                          item.textLabel = label
                          item.isSelected = Qt.binding(function() { return isSelected })

                          if (hasChildren)
                          {
                              console.log("Item path: " + path + ",label: " + label)
                              item.nodePath = path
                              item.isExpanded = isExpanded
                              item.folderChildren = childrenModel
                          }
                          else
                          {
                              item.cRef = classRef
                              //item.functionType = funcType
                          }
                      }
                      Connections
                      {
                          target: item
                          onDoubleClicked: loadFunctionEditor(ID, Type)
                      }
                      Connections
                      {
                          target: item
                          onClicked:
                          {
                              if (qItem == item)
                              {
                                  model.isSelected = (mouseMods & Qt.ControlModifier) ? 2 : 1
                                  if (model.hasChildren)
                                      model.isExpanded = item.isExpanded
                              }
                              functionManager.selectFunctionID(ID, mouseMods & Qt.ControlModifier)
                          }
                      }
                      Connections
                      {
                          ignoreUnknownSignals: true
                          target: item
                          onPathChanged: functionManager.setFolderPath(oldPath, newPath)
                      }
                  } // Loader
              } // Component
              ScrollBar { flickable: functionsListView }
        } // ListView
    } // ColumnLayout
}
