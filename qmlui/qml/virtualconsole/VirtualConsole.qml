/*
  Q Light Controller Plus
  VirtualConsole.qml

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

import "DetachWindow.js" as WinLoader
import "."

Rectangle
{
    id: vcContainer
    anchors.fill: parent
    color: "transparent"

    property string contextName: "VC"
    property int selectedPage: virtualConsole.selectedPage
    property bool docLoaded: qlcplus.docLoaded

    onDocLoadedChanged:
    {
        // force a reload of the selected page
        pageLoader.active = false
        pageLoader.active = true
    }

    onSelectedPageChanged:
    {
        pageLoader.source = ""
        if (selectedPage < 0)
            return;
        pageLoader.source = "qrc:/VCPageArea.qml"
    }

    VCRightPanel
    {
        id: vcRightPanel
        x: parent.width - width
        z: 5
        height: parent.height
    }

    Rectangle
    {
        id: centerView
        width: parent.width - vcRightPanel.width
        height: parent.height
        color: "transparent"
        clip: true

        Rectangle
        {
            id: vcToolbar
            width: parent.width
            height: 34
            z: 10
            gradient: Gradient
            {
                id: vcTbGradient
                GradientStop { position: 0; color: UISettings.toolbarStartSub }
                GradientStop { position: 1; color: UISettings.toolbarEnd }
            }

            RowLayout
            {
                id: rowLayout1
                anchors.fill: parent
                spacing: 5
                ExclusiveGroup { id: vcToolbarGroup }

                MenuBarEntry
                {
                    id: vcPage1
                    entryText: qsTr("Page 1")
                    checkable: true
                    checked: true
                    checkedColor: UISettings.toolbarSelectionSub
                    bgGradient: vcTbGradient
                    exclusiveGroup: vcToolbarGroup
                    onCheckedChanged:
                    {
                        if (checked == true)
                            virtualConsole.selectedPage = 0
                    }
                    onRightClicked:
                    {
                        vcPage1.visible = false
                        WinLoader.createVCWindow("qrc:/VCPageArea.qml", 0)
                    }
                }
                MenuBarEntry
                {
                    id: vcPage2
                    entryText: qsTr("Page 2")
                    checkable: true
                    checkedColor: UISettings.toolbarSelectionSub
                    bgGradient: vcTbGradient
                    exclusiveGroup: vcToolbarGroup
                    onCheckedChanged:
                    {
                        if (checked == true)
                            virtualConsole.selectedPage = 1
                    }
                    onRightClicked:
                    {
                        vcPage1.visible = false
                        WinLoader.createVCWindow("qrc:/VCPageArea.qml", 1)
                    }
                }

                Rectangle { Layout.fillWidth: true }
            }
        }

        Loader
        {
            id: pageLoader
            z: 0
            anchors.top: vcToolbar.bottom
            width: centerView.width
            height: parent.height - vcToolbar.height
            source: "qrc:/VCPageArea.qml"

            onLoaded:
            {
                // set the page
                pageLoader.item.page = selectedPage
            }
        }
    }
}
