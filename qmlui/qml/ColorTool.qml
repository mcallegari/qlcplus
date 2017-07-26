/*
  Q Light Controller Plus
  ColorTool.qml

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

import QtQuick 2.3
import QtQuick.Controls 2.1
import QtQuick.Layouts 1.1
import "."

Rectangle
{
    id: colorToolBox
    width: UISettings.bigItemHeight * 3
    height: UISettings.bigItemHeight * 3.5
    color: UISettings.bgMedium
    border.color: "#666"
    border.width: 2

    property bool closeOnSelect: false
    property int colorsMask: 0
    property color currentRGB
    property color currentWAUV
    property string colorToolQML: "qrc:/ColorToolBasic.qml"

    signal colorChanged(real r, real g, real b, real w, real a, real uv)

    Rectangle
    {
        id: colorToolBar
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

        RowLayout
        {
            id: rowLayout1
            anchors.fill: parent
            spacing: 5
            ButtonGroup { id: ctMenuBarGroup }

            MenuBarEntry
            {
                id: basicView
                entryText: qsTr("Basic")
                checked: true
                checkedColor: "green"
                bgGradient: cBarGradient
                ButtonGroup.group: ctMenuBarGroup
                mFontSize: UISettings.textSizeDefault
                onCheckedChanged:
                {
                    if (checked == true)
                        colorToolQML = "qrc:/ColorToolBasic.qml"
                }
            }

            MenuBarEntry
            {
                id: rgbView
                entryText: qsTr("Full")
                checkedColor: "green"
                bgGradient: cBarGradient
                ButtonGroup.group: ctMenuBarGroup
                mFontSize: UISettings.textSizeDefault
                onCheckedChanged:
                {
                    if (checked == true)
                        colorToolQML = "qrc:/ColorToolFull.qml"
                }
            }
            MenuBarEntry
            {
                id: filtersView
                entryText: qsTr("Filters")
                checkedColor: "green"
                bgGradient: cBarGradient
                ButtonGroup.group: ctMenuBarGroup
                mFontSize: UISettings.textSizeDefault
                onCheckedChanged:
                {
                    if (checked == true)
                        colorToolQML = "qrc:/ColorToolFilters.qml"
                }
            }
            // allow the tool to be dragged around
            // by holding it on the title bar
            MouseArea
            {
                Layout.fillWidth: true
                height: colorToolBar.height
                drag.target: colorToolBox
            }
        }
    }

    Loader
    {
        id: toolLoader
        z: 0
        //objectName: "editorLoader"
        anchors.top: colorToolBar.bottom
        width: colorToolBox.width
        height: parent.height - colorToolBar.height
        source: colorToolQML

        onLoaded:
        {
            item.width = width
            item.colorsMask = Qt.binding(function() { return colorToolBox.colorsMask })
            if (item.hasOwnProperty("currentRGB"))
                item.currentRGB = colorToolBox.currentRGB
            if (item.hasOwnProperty("currentWAUV"))
                item.currentWAUV = colorToolBox.currentWAUV
        }

        Connections
        {
             target: toolLoader.item
             ignoreUnknownSignals: true
             onColorChanged:
             {
                 currentRGB = Qt.rgba(r, g, b, 1.0)
                 currentWAUV = Qt.rgba(w, a, uv, 1.0)
                 colorToolBox.colorChanged(r, g, b, w, a, uv)
             }
             onReleased: if (closeOnSelect) colorToolBox.visible = false
        }
    }
}
