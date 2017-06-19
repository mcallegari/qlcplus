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

import org.qlcplus.classes 1.0
import "."

Rectangle
{
    id: fmContainer
    anchors.fill: parent
    color: "transparent"
    clip: true

    signal requestView(int ID, string qmlSrc)

    function loadFunctionEditor(funcID, funcType)
    {
        //console.log("Request to open Function editor. ID: " + funcID + " type: " + funcType)
        functionManager.setEditorFunction(funcID, false)
        functionManager.viewPosition = functionsListView.contentY
        var editorRes = functionManager.getEditorResource(funcType)

        if (funcType === Function.ShowType)
        {
            showManager.currentShowID = funcID
            mainView.switchToContext("SHOWMGR", editorRes)
        }
        else
            fmContainer.requestView(funcID, editorRes)
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
        height: UISettings.iconSizeMedium
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
            height: parent.height
            y: 1

            spacing: 4

            IconButton
            {
                z: 2
                width: height
                height: topBar.height - 2
                imgSource: "qrc:/scene.svg"
                checkable: true
                checked: functionManager.functionsFilter & Function.SceneType
                tooltip: qsTr("Scenes")
                counter: functionManager.sceneCount
                onCheckedChanged: setFunctionFilter(Function.SceneType, checked)
            }
            IconButton
            {
                z: 2
                width: height
                height: topBar.height - 2
                imgSource: "qrc:/chaser.svg"
                checkable: true
                checked: functionManager.functionsFilter & Function.ChaserType
                tooltip: qsTr("Chasers")
                counter: functionManager.chaserCount
                onCheckedChanged: setFunctionFilter(Function.ChaserType, checked)
            }
            IconButton
            {
                z: 2
                width: height
                height: topBar.height - 2
                imgSource: "qrc:/sequence.svg"
                checkable: true
                checked: functionManager.functionsFilter & Function.SequenceType
                tooltip: qsTr("Sequences")
                counter: functionManager.sequenceCount
                onCheckedChanged: setFunctionFilter(Function.SequenceType, checked)
            }
            IconButton
            {
                z: 2
                width: height
                height: topBar.height - 2
                imgSource: "qrc:/efx.svg"
                checkable: true
                checked: functionManager.functionsFilter & Function.EFXType
                tooltip: qsTr("EFX")
                counter: functionManager.efxCount
                onCheckedChanged: setFunctionFilter(Function.EFXType, checked)
            }
            IconButton
            {
                z: 2
                width: height
                height: topBar.height - 2
                imgSource: "qrc:/collection.svg"
                checkable: true
                checked: functionManager.functionsFilter & Function.CollectionType
                tooltip: qsTr("Collections")
                counter: functionManager.collectionCount
                onCheckedChanged: setFunctionFilter(Function.CollectionType, checked)
            }
            IconButton
            {
                z: 2
                width: height
                height: topBar.height - 2
                imgSource: "qrc:/rgbmatrix.svg"
                checkable: true
                checked: functionManager.functionsFilter & Function.RGBMatrixType
                tooltip: qsTr("RGB Matrices")
                counter: functionManager.rgbMatrixCount
                onCheckedChanged: setFunctionFilter(Function.RGBMatrixType, checked)
            }
            IconButton
            {
                z: 2
                width: height
                height: topBar.height - 2
                imgSource: "qrc:/showmanager.svg"
                checkable: true
                checked: functionManager.functionsFilter & Function.ShowType
                tooltip: qsTr("Shows")
                counter: functionManager.showCount
                onCheckedChanged: setFunctionFilter(Function.ShowType, checked)
            }
            IconButton
            {
                z: 2
                width: height
                height: topBar.height - 2
                imgSource: "qrc:/script.svg"
                checkable: true
                checked: functionManager.functionsFilter & Function.ScriptType
                tooltip: qsTr("Scripts")
                counter: functionManager.scriptCount
                onCheckedChanged: setFunctionFilter(Function.ScriptType, checked)
            }
            IconButton
            {
                z: 2
                width: height
                height: topBar.height - 2
                imgSource: "qrc:/audio.svg"
                checkable: true
                checked: functionManager.functionsFilter & Function.AudioType
                tooltip: qsTr("Audio")
                counter: functionManager.audioCount
                onCheckedChanged: setFunctionFilter(Function.AudioType, checked)
            }
            IconButton
            {
                z: 2
                width: height
                height: topBar.height - 2
                imgSource: "qrc:/video.svg"
                checkable: true
                checked: functionManager.functionsFilter & Function.VideoType
                tooltip: qsTr("Videos")
                counter: functionManager.videoCount
                onCheckedChanged: setFunctionFilter(Function.VideoType, checked)
            }

            Rectangle { Layout.fillWidth: true }

            IconButton
            {
                id: searchFunc
                z: 2
                width: height
                height: topBar.height - 2
                bgColor: UISettings.bgMain
                faColor: checked ? "white" : "gray"
                faSource: FontAwesome.fa_search
                checkable: true
                tooltip: qsTr("Set a Function search filter")
                onToggled:
                {
                    functionManager.searchFilter = ""
                    if (checked)
                        sTextInput.forceActiveFocus()
                }
            }
        }
      }

      Rectangle
      {
          id: searchBox
          visible: searchFunc.checked
          width: fmContainer.width
          height: UISettings.iconSizeMedium
          z: 5
          color: UISettings.bgMain
          radius: 5
          border.width: 2
          border.color: "#111"

          TextInput
          {
              id: sTextInput
              y: 3
              height: parent.height - 6
              width: searchBox.width
              color: UISettings.fgMain
              text: functionManager.searchFilter
              font.family: "Roboto Condensed"
              font.pixelSize: parent.height - 6
              selectionColor: UISettings.highlightPressed
              selectByMouse: true

              onTextChanged: functionManager.searchFilter = text
          }
      }

      ListView
      {
          id: functionsListView
          width: fmContainer.width
          height: fmContainer.height - topBar.height
          //anchors.fill: parent
          z: 4
          boundsBehavior: Flickable.StopAtBounds
          Layout.fillHeight: true

          Component.onCompleted: contentY = functionManager.viewPosition

          property bool dragActive: false

          model: functionManager.functionsList
          delegate:
              Component
              {
                  Loader
                  {
                      width: functionsListView.width
                      source: hasChildren ? "qrc:/TreeNodeDelegate.qml" : "qrc:/FunctionDelegate.qml"

                      onLoaded:
                      {
                          item.textLabel = label
                          item.isSelected = Qt.binding(function() { return isSelected })
                          item.dragItem = fDragItem

                          if (hasChildren)
                          {
                              console.log("Item path: " + path + ",label: " + label)
                              item.itemType = App.FolderDragItem
                              item.nodePath = path
                              item.isExpanded = isExpanded
                              item.nodeChildren = childrenModel
                          }
                          else
                          {
                              item.cRef = classRef
                              item.itemType = App.FunctionDragItem
                              //item.functionType = funcType
                          }
                      }
                      Connections
                      {
                          target: item
                          onMouseEvent:
                          {
                              //console.log("Got a mouse event in Function Manager: " + type)
                              switch (type)
                              {
                                case App.Pressed:
                                    var posnInWindow = qItem.mapToItem(mainView, qItem.x, qItem.y)
                                    fDragItem.parent = mainView
                                    fDragItem.x = posnInWindow.x - (fDragItem.width / 4)
                                    fDragItem.y = posnInWindow.y - (fDragItem.height / 4)
                                break;
                                case App.Clicked:
                                    if (qItem == item)
                                    {
                                        model.isSelected = (mouseMods & Qt.ControlModifier) ? 2 : 1
                                        if (model.hasChildren)
                                            model.isExpanded = item.isExpanded
                                    }
                                    functionManager.selectFunctionID(iID, mouseMods & Qt.ControlModifier)
                                break;
                                case App.DoubleClicked:
                                    loadFunctionEditor(iID, iType)
                                break;
                                case App.DragStarted:
                                    if (qItem == item && !model.isSelected)
                                    {
                                        model.isSelected = 1
                                        // invalidate the modifiers to force a single selection
                                        mouseMods = -1
                                    }

                                    if (mouseMods == -1)
                                        functionManager.selectFunctionID(iID, false)

                                    fDragItem.itemsList = functionManager.selectedFunctionsID()
                                    fDragItem.itemLabel = qItem.textLabel
                                    if (qItem.hasOwnProperty("itemIcon"))
                                        fDragItem.itemIcon = qItem.itemIcon
                                    else
                                        fDragItem.itemIcon = ""
                                    functionsListView.dragActive = true
                                break;
                                case App.DragFinished:
                                    fDragItem.Drag.drop()
                                    fDragItem.parent = functionsListView
                                    fDragItem.x = 0
                                    fDragItem.y = 0
                                    functionsListView.dragActive = false
                                break;
                              }
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
              CustomScrollBar { id: fMgrScrollBar; flickable: functionsListView }

              GenericMultiDragItem
              {
                  id: fDragItem

                  property bool fromFunctionManager: true

                  visible: functionsListView.dragActive

                  Drag.active: functionsListView.dragActive
                  Drag.source: fDragItem
                  Drag.keys: [ "function" ]

                  onItemsListChanged:
                  {
                      console.log("Items in list: " + itemsList.length)
                      if (itemsList.length)
                      {
                          var funcRef = functionManager.getFunction(itemsList[0])
                          itemLabel = funcRef.name
                          itemIcon = functionManager.functionIcon(funcRef.type)
                          //multipleItems = itemsList.length > 1 ? true : false
                      }
                  }
              }
        } // ListView

    } // ColumnLayout
}
