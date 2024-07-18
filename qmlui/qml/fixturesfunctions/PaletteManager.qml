/*
  Q Light Controller Plus
  PaletteManager.qml

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
import QtQuick.Controls 2.2

import org.qlcplus.classes 1.0
import "."

Rectangle
{
    id: pmContainer
    anchors.fill: parent
    color: "transparent"

    property bool allowEditing: true

    function setTypeFilter(pType, checked)
    {
        if (checked === true)
            paletteManager.typeFilter |= pType
        else
            paletteManager.typeFilter &= ~pType
    }

    ModelSelector
    {
        id: pmSelector
    }

    ColumnLayout
    {
      anchors.fill: parent
      spacing: 3

      Rectangle
      {
        id: topBar
        width: pmContainer.width
        height: UISettings.iconSizeMedium
        z: 5
        gradient: Gradient
        {
            GradientStop { position: 0; color: UISettings.toolbarStartSub }
            GradientStop { position: 1; color: UISettings.toolbarEnd }
        }

        RowLayout
        {
            width: parent.width
            height: parent.height
            y: 1

            spacing: 4

            IconButton
            {
                id: searchFunc
                z: 2
                width: height
                height: topBar.height - 2
                bgColor: UISettings.bgMedium
                faColor: checked ? "white" : "gray"
                faSource: FontAwesome.fa_search
                checkable: true
                tooltip: qsTr("Search a palette")
                onToggled:
                {
                    paletteManager.searchFilter = ""
                    if (checked)
                        sTextInput.forceActiveFocus()
                }
            }
            IconButton
            {
                z: 2
                width: height
                height: topBar.height - 2
                imgSource: "qrc:/intensity.svg"
                checkable: true
                checked: paletteManager.typeFilter & QLCPalette.Dimmer
                tooltip: qsTr("Intensity")
                counter: paletteManager.dimmerCount
                onCheckedChanged: setTypeFilter(QLCPalette.Dimmer, checked)
            }
            IconButton
            {
                z: 2
                width: height
                height: topBar.height - 2
                imgSource: "qrc:/color.svg"
                checkable: true
                checked: paletteManager.typeFilter & QLCPalette.Color
                tooltip: qsTr("Color")
                counter: paletteManager.colorCount
                onCheckedChanged: setTypeFilter(QLCPalette.Color, checked)
            }
            IconButton
            {
                z: 2
                width: height
                height: topBar.height - 2
                imgSource: "qrc:/position.svg"
                checkable: true
                checked: paletteManager.typeFilter & (QLCPalette.Pan || QLCPalette.Tilt || QLCPalette.PanTilt)
                tooltip: qsTr("Position")
                counter: paletteManager.positionCount
                onCheckedChanged: setTypeFilter(QLCPalette.Pan | QLCPalette.Tilt | QLCPalette.PanTilt, checked)
            }

            Rectangle { Layout.fillWidth: true }

            IconButton
            {
                z: 2
                width: height
                height: topBar.height - 2
                visible: allowEditing
                imgSource: "qrc:/remove.svg"
                tooltip: qsTr("Delete the selected palette(s)")
                enabled: pDragItem.itemsList.length
                onClicked:
                {
                    var selNames = []
                    for (var i = 0; i < pDragItem.itemsList.length; i++)
                        selNames.push(pDragItem.itemsList[i].cRef.name)

                    //console.log(selNames)
                    deleteItemsPopup.message = qsTr("Are you sure you want to delete the following items?") + "\n" + selNames
                    deleteItemsPopup.open()
                }

                CustomPopupDialog
                {
                    id: deleteItemsPopup
                    title: qsTr("Delete items")
                    onAccepted:
                    {
                        var idList = []
                        for (var i = 0; i < pDragItem.itemsList.length; i++)
                            idList.push(pDragItem.itemsList[i].cRef.id)

                        paletteManager.deletePalettes(idList)
                        pDragItem.itemsList = []
                    }
                }
            }
        } // RowLayout
      } // Rectangle - topBar

      Rectangle
      {
          id: searchBox
          visible: searchFunc.checked
          width: pmContainer.width
          height: UISettings.iconSizeMedium
          z: 5
          color: UISettings.bgMedium
          radius: 5
          border.width: 2
          border.color: UISettings.borderColorDark

          TextInput
          {
              id: sTextInput
              y: 3
              height: parent.height - 6
              width: parent.width
              color: UISettings.fgMain
              text: paletteManager.searchFilter
              font.family: "Roboto Condensed"
              font.pixelSize: parent.height - 6
              selectionColor: UISettings.highlightPressed
              selectByMouse: true

              onTextChanged: paletteManager.searchFilter = text
          }
      } // Rectangle - searchBox

      ListView
      {
          id: pListView
          width: pmContainer.width
          Layout.fillHeight: true
          z: 4
          boundsBehavior: Flickable.StopAtBounds

          property bool dragActive: false

          Component.onDestruction: pmSelector.resetSelection(pListView.model)

          model: paletteManager.paletteList
          delegate:
              Item
              {
                  width: pListView.width
                  height: UISettings.listItemHeight

                  MouseArea
                  {
                      id: delegateRoot
                      width: pListView.width
                      height: parent.height

                      property bool dragActive: drag.active

                      drag.target: pDragItem
                      drag.threshold: height / 2

                      onPressed:
                      {
                          if (model.isSelected)
                              return

                          var posnInWindow = pDelegate.mapToItem(mainView, pDelegate.x, pDelegate.y)
                          pDragItem.parent = mainView
                          pDragItem.x = posnInWindow.x - (pDragItem.width / 4)
                          pDragItem.y = posnInWindow.y - (pDragItem.height / 4)
                          pDragItem.z = 10

                          pmSelector.selectItem(index, pListView.model, mouse.modifiers)

                          if ((mouse.modifiers & Qt.ControlModifier) == 0)
                              pDragItem.itemsList = []

                          // workaround array length notification
                          var arr = pDragItem.itemsList
                          arr.push(pDelegate)
                          pDragItem.itemsList = arr
                      }
                      onDoubleClicked:
                      {
                          var paletteType = pDelegate.cRef.type
                          console.log("Palette type: " + paletteType)
                          toolLoader.paletteID = pDelegate.cRef.id

                          switch (paletteType)
                          {
                              case QLCPalette.Dimmer:
                                  toolLoader.source = "qrc:/IntensityTool.qml"
                              break
                              case QLCPalette.Color:
                                  toolLoader.source = "qrc:/ColorTool.qml"
                              break
                              case QLCPalette.Pan:
                              case QLCPalette.Tilt:
                              case QLCPalette.PanTilt:
                                  toolLoader.source = "qrc:/PositionTool.qml"
                              break
                          }
                      }

                      onDragActiveChanged:
                      {
                          if (dragActive)
                          {
                              pDragItem.itemLabel = pEntryItem.tLabel
                              pDragItem.itemIcon = pEntryItem.iSrc
                              pListView.dragActive = true
                          }
                          else
                          {
                              pDragItem.Drag.drop()
                              pDragItem.parent = pListView
                              pDragItem.x = 0
                              pDragItem.y = 0
                              pListView.dragActive = false
                          }
                      }

                      Rectangle
                      {
                          id: pDelegate
                          width: pListView.width
                          height: UISettings.listItemHeight
                          color: "transparent"

                          property QLCPalette cRef: paletteManager.getPalette(model.paletteID)
                          property int itemType: App.PaletteDragItem

                          Rectangle
                          {
                              anchors.fill: parent
                              radius: 3
                              color: UISettings.highlight
                              visible: model.isSelected
                          }

                          IconTextEntry
                          {
                              id: pEntryItem
                              width: parent.width
                              height: parent.height
                              iSrc: pDelegate.cRef.iconResource(true)
                              tLabel: pDelegate.cRef ? pDelegate.cRef.name : ""
                          }

                          // items divider
                          Rectangle
                          {
                              width: parent.width
                              height: 1
                              y: parent.height - 1
                              color: UISettings.bgLight
                          }
                      }
                  }
              }
          GenericMultiDragItem
          {
              id: pDragItem

              visible: pListView.dragActive

              Drag.active: pListView.dragActive
              Drag.source: pDragItem
              Drag.keys: [ "palette" ]
          }
      } // ListView
    } // ColumnLayout

    Loader
    {
        id: toolLoader
        anchors.fill: parent

        property int paletteID

        function dismiss()
        {
            source = ""
        }

        onLoaded:
        {
            item.width = toolLoader.width
            item.height = toolLoader.height
            item.loadPalette(paletteID)
        }
    }
}
