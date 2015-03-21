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
//import com.qlcplus.app 1.0

Rectangle {
        objectName: "fixturesAndFunctions"
        width: 600
        height: 400
        anchors.fill: parent
        color: "transparent"

        property string currentViewQML: "qrc:/2DView.qml"
        // string holding the current view. Used by the C++ for dynamic objects
        // creation
        property string currentView: "2D"
        property bool docLoaded: qlcplus.docLoaded

        onCurrentViewChanged: contextManager.activateContext(currentView)

        onDocLoadedChanged: {
            console.log("Doc loaded !! ----")
            viewUniverseCombo.model = ioManager.universes
            var tmpView = currentViewQML
            // clear the previously loaded view
            currentViewQML = ""
            // restore the previous view with new data
            currentViewQML = tmpView
        }

        LeftPanel {
            id: leftPanel
            x: 0
            z: 5
            height: parent.height
        }

        RightPanel {
            id: rightPanel
            x: parent.width - width
            z: 5
            height: parent.height
        }

        Rectangle {
            id: centerView
            width: parent.width - leftPanel.width - rightPanel.width
            x: leftPanel.width
            height: parent.height
            color: "transparent"

            Rectangle {
                id: viewToolbar
                width: parent.width
                height: 32
                z: 10
                gradient: Gradient {
                    id: ffMenuGradient
                    GradientStop { position: 0 ; color: "#222" }
                    GradientStop { position: 1 ; color: "#111" }
                }

                RowLayout {
                    id: rowLayout1
                    //anchors.horizontalCenter: parent.horizontalCenter
                    anchors.left: parent.left
                    anchors.leftMargin: 10
                    anchors.bottom: parent.bottom
                    anchors.top: parent.top

                    spacing: 5
                    ExclusiveGroup { id: menuBarGroup2 }

                    MenuBarEntry {
                        id: uniView
                        imgSource: "uniview.svg"
                        entryText: qsTr("Universe View")
                        checkable: true
                        checkedColor: "yellow"
                        bgGradient: ffMenuGradient
                        exclusiveGroup: menuBarGroup2
                        onCheckedChanged: {
                            if (checked == true)
                            {
                                currentViewQML = "qrc:/UniverseGridView.qml"
                                currentView = "UniverseGrid"
                            }
                        }
                    }
                    MenuBarEntry {
                        id: dmxView
                        imgSource: "dmxview.svg"
                        entryText: qsTr("DMX View")
                        checkable: true
                        //checked: true
                        checkedColor: "yellow"
                        bgGradient: ffMenuGradient
                        exclusiveGroup: menuBarGroup2
                        onCheckedChanged: {
                            if (checked == true)
                            {
                                currentViewQML = "qrc:/DMXView.qml"
                                currentView = "DMX"
                            }
                        }
                    }
                    MenuBarEntry {
                        id: twodView
                        imgSource: "2dview.svg"
                        entryText: qsTr("2D View")
                        checkable: true
                        checked: true
                        checkedColor: "yellow"
                        bgGradient: ffMenuGradient
                        exclusiveGroup: menuBarGroup2
                        onCheckedChanged: {
                            if (checked == true)
                            {
                                currentViewQML = "qrc:/2DView.qml"
                                currentView = "2D"
                            }
                        }
                    }
                    MenuBarEntry {
                        id: threedView
                        imgSource: "3dview.svg"
                        entryText: qsTr("3D View")
                        checkable: true
                        checkedColor: "yellow"
                        bgGradient: ffMenuGradient
                        exclusiveGroup: menuBarGroup2
                        onCheckedChanged: {
                            if (checked == true)
                            {
                                currentViewQML = "qrc:/3DView.qml"
                                currentView = "3D"
                            }
                        }
                    }

                    Rectangle {
                        Layout.fillWidth: true
                    }

                    CustomComboBox {
                        id: viewUniverseCombo
                        width: 100
                        height: 20
                        anchors.margins: 4
                        model: ioManager.universes

                        onCurrentIndexChanged: {
                            var tmpview = currentViewQML;
                            // clear the previously loaded view
                            currentViewQML = ""
                            // restore the previous view with new data
                            currentViewQML = tmpview
                        }
                    }
                }
            }

            Loader {
                id: previewLoader
                z: 0
                //objectName: "editorLoader"
                anchors.top: viewToolbar.bottom
                width: centerView.width
                height: parent.height - viewToolbar.height
                source: currentViewQML
            }
        }
}
