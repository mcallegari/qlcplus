/*
  Q Light Controller Plus
  FixtureEditor.qml

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

import QtQuick 2.2
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.1

import org.qlcplus.classes 1.0
import "."

Rectangle
{
    id: mainView
    visible: true
    width: 800
    height: 600
    anchors.fill: parent
    color: UISettings.bgMain

    FontLoader
    {
        source: "qrc:/RobotoCondensed-Regular.ttf"
    }

    // Load the "FontAwesome" font for the monochrome icons
    FontLoader
    {
        source: "qrc:/FontAwesome.otf"
    }

    Rectangle
    {
        id: mainToolbar
        width: parent.width
        height: UISettings.iconSizeDefault
        z: 50
        gradient: Gradient
        {
            GradientStop { position: 0; color: UISettings.toolbarStartMain }
            GradientStop { position: 1; color: UISettings.toolbarEnd }
        }

        RowLayout
        {
            spacing: 5
            anchors.fill: parent

            ButtonGroup { id: menuBarGroup }

            MenuBarEntry
            {
                imgSource: "qrc:/back.svg"
                entryText: qsTr("Back to QLC+")
                iconRotation: 180
                onPressed: qlcplus.closeFixtureEditor()
                autoExclusive: false
                checkable: false
            }
            MenuBarEntry
            {
                imgSource: "qrc:/filenew.svg"
                entryText: qsTr("New")
                //onPressed: qlcplus.closeFixtureEditor()
                autoExclusive: false
                checkable: false
            }
            MenuBarEntry
            {
                imgSource: "qrc:/filesave.svg"
                entryText: qsTr("Save")
                //onPressed: qlcplus.closeFixtureEditor()
                autoExclusive: false
                checkable: false
            }
            MenuBarEntry
            {
                imgSource: "qrc:/filesaveas.svg"
                entryText: qsTr("Save as...")
                //onPressed: qlcplus.closeFixtureEditor()
                autoExclusive: false
                checkable: false
            }
        }
    }

    /* Rectangle covering the whole window to
     * have a dimmered background for popups */
    Rectangle
    {
        id: dimScreen
        anchors.fill: parent
        visible: false
        z: 99
        color: Qt.rgba(0, 0, 0, 0.5)
    }
}
