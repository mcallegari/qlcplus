/*
  Q Light Controller Plus
  FixtureDelegate.qml

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
import QtQuick.Layouts 1.0

import org.qlcplus.classes 1.0
import "."

Rectangle
{
    id: fxDelegate
    width: 100
    height: UISettings.listItemHeight

    color: "transparent"

    property Fixture cRef
    property string textLabel: cRef ? cRef.name : ""
    property int itemType: App.FixtureDragItem
    property bool isSelected: false
    property bool isCheckable: false
    property bool isChecked: false
    property Item dragItem

    signal mouseEvent(int type, int iID, int iType, var qItem, int mouseMods)

    Rectangle
    {
        anchors.fill: parent
        radius: 3
        color: UISettings.highlight
        visible: isSelected
    }

    RowLayout
    {
        anchors.fill: parent

        CustomCheckBox
        {
            id: chCheckBox
            visible: isCheckable
            implicitWidth: UISettings.listItemHeight
            implicitHeight: implicitWidth
            checked: isChecked
            onCheckedChanged: fxDelegate.mouseEvent(App.Checked, cRef.id, checked, fxDelegate, 0)
        }

        IconTextEntry
        {
            Layout.fillWidth: true
            height: fxDelegate.height
            tLabel: fxDelegate.textLabel
            iSrc: cRef ? cRef.iconResource(true) : ""

            MouseArea
            {
                anchors.fill: parent
                hoverEnabled: true

                onClicked: fxDelegate.mouseEvent(App.Clicked, cRef.id, cRef.type, fxDelegate, mouse.modifiers)
                onDoubleClicked: fxDelegate.mouseEvent(App.DoubleClicked, cRef.id, cRef.type, fxDelegate, -1)
            }
        }
    }

    Rectangle
    {
        width: parent.width
        height: 1
        y: parent.height - 1
        color: UISettings.bgLight
    }
}
