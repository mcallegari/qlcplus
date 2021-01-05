/*
  Q Light Controller Plus
  InputChannelDelegate.qml

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
    id: chDelegate
    width: 100
    height: UISettings.listItemHeight

    color: "transparent"

    property QLCInputChannel cRef
    property string textLabel: cRef ? cRef.name : ""
    property int itemType: App.ChannelDragItem
    property int itemID
    property bool isSelected: false
    property bool isCheckable: false
    property bool isChecked: false
    property Item dragItem

    signal mouseEvent(int type, int iID, int iType, var qItem, int mouseMods)

    function typeToIcon()
    {
        if (!cRef)
            return ""

        switch (cRef.type)
        {
            case QLCInputChannel.Slider: return "qrc:/slider.svg"
            case QLCInputChannel.Button: return "qrc:/button.svg"
            case QLCInputChannel.Knob:
            case QLCInputChannel.Encoder:
                return "qrc:/knob.svg"
            case QLCInputChannel.NextPage: return "qrc:/forward.svg"
            case QLCInputChannel.PrevPage: return "qrc:/back.svg"
            case QLCInputChannel.PageSet: return "qrc:/star.svg"
            default: return ""
        }
    }

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

        IconTextEntry
        {
            Layout.fillWidth: true
            height: chDelegate.height
            tLabel: chDelegate.textLabel
            iSrc: cRef ? typeToIcon() : ""

            MouseArea
            {
                anchors.fill: parent
                hoverEnabled: true

                onClicked: chDelegate.mouseEvent(App.Clicked, cRef.id, cRef.type, chDelegate, mouse.modifiers)
                onDoubleClicked: chDelegate.mouseEvent(App.DoubleClicked, cRef.id, cRef.type, chDelegate, -1)
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

