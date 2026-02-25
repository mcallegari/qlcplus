/*
  Q Light Controller Plus
  VCWidgetsList.qml

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
    id: wlContainer
    anchors.fill: parent
    color: "transparent"

    property var modelProvider: null
    property bool allowEditing: false
    property string searchFilter: ""

    property var widgetFilters: [
        VCWidget.ButtonWidget,
        VCWidget.SliderWidget,
        VCWidget.SpeedWidget,
        VCWidget.CueListWidget
    ]

    function filteredWidgetsList()
    {
        if (!modelProvider)
            return []

        var allWidgets = virtualConsole.widgetsList(widgetFilters, modelProvider.id)
        if (!searchFilter.length)
            return allWidgets

        var needle = searchFilter.toLowerCase()
        var wList = []

        for (var i = 0; i < allWidgets.length; i++)
        {
            var widget = allWidgets[i]
            if (!widget.hasOwnProperty("id"))
                continue

            if (String(widget.label).toLowerCase().indexOf(needle) !== -1)
                wList.push(widget)
        }

        if (!wList.length)
            wList.push({ label: qsTr("<None>") })

        return wList
    }

    ColumnLayout
    {
        anchors.fill: parent
        spacing: 2

        Rectangle
        {
            Layout.fillWidth: true
            height: UISettings.iconSizeMedium
            color: UISettings.bgMedium
            radius: 5
            border.width: 2
            border.color: UISettings.borderColorDark

            Text
            {
                id: searchIcon
                x: 6
                width: height
                height: parent.height - 6
                anchors.verticalCenter: parent.verticalCenter
                color: "gray"
                font.family: UISettings.fontAwesomeFontName
                font.pixelSize: height - 6
                text: FontAwesome.fa_magnifying_glass
            }

            TextInput
            {
                x: searchIcon.width + 14
                y: 3
                height: parent.height - 6
                width: parent.width - x
                color: UISettings.fgMain
                text: searchFilter
                font.family: UISettings.robotoFontName
                font.pixelSize: height - 6
                selectionColor: UISettings.highlightPressed
                selectByMouse: true

                onTextEdited: searchFilter = text.trim()
            }
        }

        ListView
        {
            id: widgetsList
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            boundsBehavior: Flickable.StopAtBounds
            model: filteredWidgetsList()

            property int selectedIndex: -1
            property bool dragActive: false

            delegate:
                WidgetDelegate
                {
                    id: wItem
                    width: widgetsList.width
                    cRef: modelData.classRef ? modelData.classRef : null
                    textLabel: modelData.label
                    isSelected: index === widgetsList.selectedIndex
                    dragItem: wDragItem

                    onMouseEvent: (eventType, iID, iType, qItem, mouseMods) =>
                    {
                        if (!cRef)
                            return

                        switch (eventType)
                        {
                            case App.Pressed:
                            {
                                var posnInWindow = qItem.mapToItem(mainView, 0, 0)
                                wDragItem.parent = mainView
                                wDragItem.x = posnInWindow.x - (wDragItem.width / 4)
                                wDragItem.y = posnInWindow.y - (wDragItem.height / 4)
                            }
                            break;
                            case App.Clicked:
                                widgetsList.selectedIndex = index
                            break;
                            case App.DragStarted:
                            {
                                wDragItem.itemsList = [ cRef.id ]
                                wDragItem.itemLabel = cRef.caption ? cRef.caption : modelData.label
                                wDragItem.itemIcon = virtualConsole.widgetIcon(cRef.type)
                                widgetsList.dragActive = true
                            }
                            break;
                            case App.DragFinished:
                            {
                                wDragItem.Drag.drop()
                                wDragItem.parent = widgetsList
                                wDragItem.x = 0
                                wDragItem.y = 0
                                widgetsList.dragActive = false
                            }
                            break;
                        }
                    }
                }

            ScrollBar.vertical: CustomScrollBar { }

            GenericMultiDragItem
            {
                id: wDragItem

                property bool fromVCWidgetsList: true

                visible: widgetsList.dragActive

                Drag.active: widgetsList.dragActive
                Drag.source: wDragItem
                Drag.keys: [ "audiotriggerswidget" ]
            }
        }
    }
}
