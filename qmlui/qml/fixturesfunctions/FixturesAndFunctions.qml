/*
  Q Light Controller Plus
  FixturesAndFunctions.qml

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
    id: fixtureAndFunctions
    objectName: "fixturesAndFunctions"
    anchors.fill: parent
    color: "transparent"

    property string currentViewQML: "qrc:/2DView.qml"

    // string holding the current view. Used by the C++ code
    // for dynamic items creation
    property string currentView: "2D"
    //property bool docLoaded: qlcplus.docLoaded

    function enableContext(ctx, setChecked)
    {
        var item = null
        if (ctx === "UNIGRID")
            item = uniView
        else if (ctx === "DMX")
            item = dmxView
        else if (ctx === "2D")
            item = twodView
        else if (ctx === "3D")
            item = threedView

        if (item)
        {
            item.visible = true
            if (setChecked)
                item.checked = true
        }
    }

    LeftPanel
    {
        id: leftPanel
        x: 0
        z: 5
        height: parent.height - (bottomPanel.visible ? bottomPanel.height : 0)
    }

    RightPanel
    {
        id: rightPanel
        x: parent.width - width
        z: 5
        height: parent.height - (bottomPanel.visible ? bottomPanel.height : 0)
    }

    BottomPanel
    {
        id: bottomPanel
        objectName: "bottomPanelItem"
        y: parent.height - height
        z: 8
        visible: false
    }

    Rectangle
    {
        id: centerView
        width: parent.width - leftPanel.width - rightPanel.width
        x: leftPanel.width
        height: parent.height - (bottomPanel.visible ? bottomPanel.height : 0)
        color: "transparent"

        Rectangle
        {
            id: viewToolbar
            width: parent.width
            height: UISettings.iconSizeMedium
            z: 10
            gradient: Gradient
            {
                id: ffMenuGradient
                GradientStop { position: 0; color: UISettings.toolbarStartSub }
                GradientStop { position: 1; color: UISettings.toolbarEnd }
            }

            RowLayout
            {
                id: rowLayout1
                anchors.fill: parent
                spacing: 5
                ButtonGroup { id: ffMenuBarGroup }

                MenuBarEntry
                {
                    id: uniView
                    imgSource: "uniview.svg"
                    entryText: qsTr("Universe View")
                    checkedColor: UISettings.toolbarSelectionSub
                    bgGradient: ffMenuGradient
                    ButtonGroup.group: ffMenuBarGroup
                    onCheckedChanged:
                    {
                        if (checked == true)
                        {
                            currentViewQML = "qrc:/UniverseGridView.qml"
                            currentView = "UNIGRID"
                        }
                    }
                    onRightClicked:
                    {
                        uniView.visible = false
                        contextManager.detachContext("UNIGRID")
                    }
                }
                MenuBarEntry
                {
                    id: dmxView
                    imgSource: "dmxview.svg"
                    entryText: qsTr("DMX View")
                    checkedColor: UISettings.toolbarSelectionSub
                    bgGradient: ffMenuGradient
                    ButtonGroup.group: ffMenuBarGroup
                    onCheckedChanged:
                    {
                        if (checked == true)
                        {
                            currentViewQML = "qrc:/DMXView.qml"
                            currentView = "DMX"
                        }
                    }
                    onRightClicked:
                    {
                        dmxView.visible = false
                        contextManager.detachContext("DMX")
                    }
                }
                MenuBarEntry
                {
                    id: twodView
                    imgSource: "2dview.svg"
                    entryText: qsTr("2D View")
                    checked: true
                    checkedColor: UISettings.toolbarSelectionSub
                    bgGradient: ffMenuGradient
                    ButtonGroup.group: ffMenuBarGroup
                    onCheckedChanged:
                    {
                        if (checked == true)
                        {
                            currentViewQML = "qrc:/2DView.qml"
                            currentView = "2D"
                        }
                    }
                    onRightClicked:
                    {
                        twodView.visible = false
                        contextManager.detachContext("2D")
                    }
                }
                MenuBarEntry
                {
                    id: threedView
                    imgSource: "3dview.svg"
                    entryText: qsTr("3D View")
                    checkedColor: UISettings.toolbarSelectionSub
                    bgGradient: ffMenuGradient
                    ButtonGroup.group: ffMenuBarGroup
                    onCheckedChanged:
                    {
                        if (checked == true)
                        {
                            currentViewQML = "qrc:/3DView.qml"
                            currentView = "3D"
                        }
                    }
                    onRightClicked:
                    {
                        threedView.visible = false
                        contextManager.detachContext("3D")
                    }
                }

                CustomComboBox
                {
                    id: viewUniverseCombo
                    width: UISettings.bigItemHeight * 1.5
                    height: viewToolbar.height - 4
                    anchors.margins: 1
                    model: ioManager.universesListModel

                    onValueChanged:
                    {
                        // set the universe filter here
                        contextManager.universeFilter = value
                        fixtureManager.universeFilter = value
                    }
                }

                Rectangle { Layout.fillWidth: true; color: "transparent" }

                IconButton
                {
                    id: settingsButton
                    implicitHeight: viewToolbar.height - 2
                    checkable: true
                    imgSource: "qrc:/configure.svg"
                    onToggled: previewLoader.item.showSettings(checked)
                }

                ZoomItem
                {
                    width: UISettings.iconSizeMedium * 2
                    implicitHeight: viewToolbar.height - 2
                    fontColor: "#222"
                    onZoomOutClicked: previewLoader.item.setZoom(-0.5)
                    onZoomInClicked: previewLoader.item.setZoom(0.5)
                }
            }
        }

        Loader
        {
            id: previewLoader
            z: 0
            anchors.top: viewToolbar.bottom
            width: centerView.width
            height: parent.height - viewToolbar.height
            source: currentViewQML

            onLoaded:
            {
                settingsButton.visible = item.hasSettings()
            }
        }
    }
}
