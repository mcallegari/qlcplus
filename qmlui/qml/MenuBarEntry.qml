/*
  Q Light Controller Plus
  MenuBarEntry.qml

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
import QtQuick.Controls 1.2

import "."

Rectangle
{
    id: menuEntry
    implicitWidth: entryContents.width + 10
    height: parent.height
    gradient: (checked || mouseArea1.containsMouse) ? selGradient : bgGradient

    property color checkedColor: UISettings.toolbarSelectionMain

    property bool checkable: false
    property bool editable: false
    property string imgSource: ""
    property string entryText: ""
    property real mFontSize: UISettings.textSizeDefault * 0.70
    property bool checked: false
    property Gradient bgGradient: defBgGradient
    property Gradient selGradient: defSelectionGradient
    property ExclusiveGroup exclusiveGroup: null

    onExclusiveGroupChanged:
    {
        if (exclusiveGroup)
            exclusiveGroup.bindCheckable(menuEntry)
    }

    signal clicked
    signal rightClicked
    signal toggled
    signal textChanged(var text)

    Gradient
    {
        id: defBgGradient
        GradientStop { position: 0 ; color: "transparent" }
        //GradientStop { position: 1 ; color: "#111" }
    }
    Gradient
    {
        id: defSelectionGradient
        GradientStop { position: 0 ; color: "#444" }
        GradientStop { position: 1 ; color: "#171717" }
    }

    Rectangle
    {
        anchors.fill: parent
        color: "#33ffffff"
        visible: mouseArea1.pressed
    }

    Row
    {
        id: entryContents
        height: parent.height
        spacing: 2
        //anchors.fill: parent
        //anchors.leftMargin: 3

        Image
        {
            id: btnIcon
            height: imgSource == "" ? 0 : parent.height - 4
            width: height
            x: 2
            y: 2
            source: imgSource
            sourceSize: Qt.size(width, height)
        }

        Rectangle
        {
            height: parent.height
            width: tbLoader.width
            color: "transparent"

            Loader
            {
                id: tbLoader
                height: parent.height
                //width: item.width

                source: menuEntry.editable ? "qrc:/EditableTextBox.qml" : "qrc:/RobotoText.qml"

                onLoaded:
                {
                    if (menuEntry.editable == true)
                    {
                        item.color = "transparent"
                        item.inputText = Qt.binding(function() { return entryText })
                        item.maximumHeight = parent.height
                        item.wrapText = false
                    }
                    else
                    {
                        item.label = Qt.binding(function() { return entryText })
                        item.height = parent.height
                        item.fontSize = mFontSize
                        item.fontBold = true
                    }
                    width = Qt.binding(function() { return item.width })
                }

                Connections
                {
                    ignoreUnknownSignals: true
                    target: tbLoader.item
                    onTextChanged: menuEntry.textChanged(text)
                }
            }

            Rectangle
            {
                id: selRect
                radius: 2
                color: checked ? checkedColor : "transparent"
                height: UISettings.listItemHeight * 0.1
                width: tbLoader.width
                y: parent.height - height - 1
            }
        }
    }

    MouseArea
    {
        id: mouseArea1
        anchors.fill: parent
        hoverEnabled: true
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        onClicked:
        {
            if (mouse.button === Qt.LeftButton)
            {
                if (checkable == true)
                {
                    if (checked == false)
                        checked = true
                    menuEntry.toggled(checked)
                }
                else
                    menuEntry.clicked()
            }
            else
                menuEntry.rightClicked()
        }
        onDoubleClicked:
        {
            if (menuEntry.editable)
            {
                tbLoader.item.enableEditing()
            }
        }
    }
}
