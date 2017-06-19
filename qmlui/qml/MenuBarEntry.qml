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
import QtQuick.Controls 2.1

import "."

Button
{
    id: control
    //implicitWidth: contentItem.width + 10
    implicitHeight: parent.height

    hoverEnabled: true
    checkable: true
    padding: 0
    topPadding: 0
    bottomPadding: 0

    property string imgSource: ""
    property string entryText: ""
    property real mFontSize: UISettings.textSizeDefault * 0.70

    property Gradient bgGradient: defBgGradient
    property Gradient selGradient: defSelectionGradient
    property color checkedColor: UISettings.toolbarSelectionMain

    signal rightClicked

    Gradient
    {
        id: defBgGradient
        GradientStop { position: 0 ; color: "transparent" }
    }
    Gradient
    {
        id: defSelectionGradient
        GradientStop { position: 0 ; color: "#444" }
        GradientStop { position: 1 ; color: "#171717" }
    }

    contentItem:
        Row
        {
            id: entryContents
            width: btnIcon.width + btnLabel.width

            Image
            {
                id: btnIcon
                height: control.imgSource == "" ? 0 : control.height - control.topPadding - control.bottomPadding
                width: height
                x: 2
                y: 2
                source: control.imgSource
                sourceSize: Qt.size(width, height)
            }

            RobotoText
            {
                id: btnLabel
                height: control.height
                label: control.entryText
                fontSize: control.mFontSize
                fontBold: true

                Rectangle
                {
                    id: selRect
                    y: parent.height - height - 2
                    height: UISettings.listItemHeight * 0.1
                    width: parent.width
                    radius: height / 2
                    color: checked ? checkedColor : "transparent"

                }
            }
        }

    background:
        Rectangle
        {
            gradient: (checked || hovered) ? selGradient : bgGradient
            opacity: enabled ? 1 : 0.3
        }


    MouseArea
    {
        anchors.fill: parent
        acceptedButtons: Qt.RightButton

        onClicked:
        {
            if (mouse.button === Qt.RightButton)
                control.rightClicked()
        }
    }
}

