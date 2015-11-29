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

import com.qlcplus.classes 1.0
import "."

Rectangle
{
    id: fxDelegate
    width: 100
    height: 35

    color: "transparent"

    property Fixture cRef
    property string textLabel: cRef ? cRef.name : ""
    property bool isSelected: false

    signal clicked(int ID, var qItem, int mouseMods)
    signal doubleClicked(int fID, int fType)

    function iconFromType(type)
    {
        if (type === "Color Changer")
            return "qrc:/fixture.svg";
        else if (type === "Dimmer")
            return "qrc:/dimmer.svg";
        else if (type === "Moving Head")
            return "qrc:/movinghead.svg";
        else
            return "qrc:/fixture.svg";
    }

    Rectangle
    {
        anchors.fill: parent
        radius: 3
        color: UISettings.highlight
        visible: isSelected
    }

    IconTextEntry
    {
        id: fxEntry
        width: parent.width
        height: parent.height
        tLabel: textLabel
        iSrc: cRef ? iconFromType(cRef.type) : ""
    }
    Rectangle
    {
        width: parent.width
        height: 1
        y: parent.height - 1
        color: "#666"
    }

    MouseArea
    {
        id: fxMouseArea
        anchors.fill: parent
        hoverEnabled: true

        onClicked:
        {
            isSelected = true
            fxDelegate.clicked(cRef.id, fxDelegate, mouse.modifiers)
        }
        onDoubleClicked:
        {
            fxDelegate.doubleClicked(cRef.id, -1)
        }
    }
}
