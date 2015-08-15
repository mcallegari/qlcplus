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
import QtQuick.Controls 1.2
import QtQuick.Layouts 1.1

import "DetachWindow.js" as WinLoader

Rectangle
{
    objectName: "fixturesAndFunctions"
    anchors.fill: parent
    color: "transparent"

    property string currentViewQML: "qrc:/2DView.qml"

    // string holding the current view. Used by the C++ code
    // for dynamic items creation
    property string currentView: "2D"
    property bool docLoaded: qlcplus.docLoaded

    function enableContext(ctx)
    {
        if (ctx === "UniverseGrid")
            uniView.visible = true
        else if (ctx === "DMX")
            dmxView.visible = true
        else if (ctx === "2D")
            twodView.visible = true
    }

    onDocLoadedChanged:
    {
        // a new Doc has been loaded. Do here all the operations to
        // reset/restore the view (active contexts are updated in C++)
        viewUniverseCombo.model = ioManager.universeNames
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
            height: 34
            z: 10
            gradient: Gradient
            {
                id: ffMenuGradient
                GradientStop { position: 0 ; color: "#222" }
                GradientStop { position: 1 ; color: "#111" }
            }

            RowLayout
            {
                id: rowLayout1
                anchors.fill: parent
                spacing: 5
                ExclusiveGroup { id: menuBarGroup2 }

                MenuBarEntry
                {
                    id: uniView
                    imgSource: "uniview.svg"
                    entryText: qsTr("Universe View")
                    checkable: true
                    checkedColor: "yellow"
                    bgGradient: ffMenuGradient
                    exclusiveGroup: menuBarGroup2
                    onCheckedChanged:
                    {
                        if (checked == true)
                        {
                            currentViewQML = "qrc:/UniverseGridView.qml"
                            currentView = "UniverseGrid"
                        }
                    }
                    onRightClicked:
                    {
                        uniView.visible = false
                        WinLoader.createWindow("qrc:///UniverseGridView.qml")
                    }
                }
                MenuBarEntry
                {
                    id: dmxView
                    imgSource: "dmxview.svg"
                    entryText: qsTr("DMX View")
                    checkable: true
                    //checked: true
                    checkedColor: "yellow"
                    bgGradient: ffMenuGradient
                    exclusiveGroup: menuBarGroup2
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
                        WinLoader.createWindow("qrc:///DMXView.qml")
                    }
                }
                MenuBarEntry
                {
                    id: twodView
                    imgSource: "2dview.svg"
                    entryText: qsTr("2D View")
                    checkable: true
                    checked: true
                    checkedColor: "yellow"
                    bgGradient: ffMenuGradient
                    exclusiveGroup: menuBarGroup2
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
                        WinLoader.createWindow("qrc:///2DView.qml")
                    }
                }
                MenuBarEntry
                {
                    id: threedView
                    imgSource: "3dview.svg"
                    entryText: qsTr("3D View")
                    checkable: true
                    checkedColor: "yellow"
                    bgGradient: ffMenuGradient
                    exclusiveGroup: menuBarGroup2
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
                        WinLoader.createWindow("qrc:///3DView.qml")
                    }
                }

                CustomComboBox
                {
                    id: viewUniverseCombo
                    width: 100
                    height: 20
                    anchors.margins: 4
                    model: ioManager.universeNames

                    onCurrentIndexChanged:
                    {
                        // set the universe filter here
                    }
                }

                Rectangle { Layout.fillWidth: true }

                IconButton
                {
                    id: settingsButton
                    height: viewToolbar.height - 2
                    checkable: true
                    imgSource: "qrc:/configure.svg"
                    onToggled: previewLoader.item.showSettings(checked)
                }

                IconButton
                {
                    height: viewToolbar.height - 2
                    faSource: fontawesome.fa_search_minus
                    onClicked: previewLoader.item.setZoom(-0.5)
                }
                IconButton
                {
                    height: viewToolbar.height - 2
                    faSource: fontawesome.fa_search_plus
                    onClicked: previewLoader.item.setZoom(0.5)
                }
            }
        }

        Loader
        {
            id: previewLoader
            z: 0
            //objectName: "editorLoader"
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
