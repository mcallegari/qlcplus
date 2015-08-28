/*
  Q Light Controller Plus
  VCWidgetProperties.qml

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

import com.qlcplus.classes 1.0
import "."

Rectangle
{
    id: wPropsRoot
    anchors.fill: parent
    color: "transparent"

    property VCWidget wObj: virtualConsole.selectedWidget

    RobotoText
    {
        visible: wObj ? false : true
        anchors.centerIn: parent
        label: qsTr("Select a widget first")
    }

    Rectangle
    {
        id: commonPropsBox
        x: 3
        width: parent.width - 6
        height: isExpanded ? (cPropsHeader.height + cPropsGrid.height) : cPropsHeader.height
        color: "transparent"
        clip: true
        visible: wObj ? true : false

        property bool isExpanded: true

        Rectangle
        {
            id: cPropsHeader
            width: parent.width
            height: 38
            color: headerMouseArea.containsMouse ? UISettings.highlight : UISettings.sectionHeader

            RobotoText
            {
                label: qsTr("Basic properties")
            }
            Text
            {
                x: parent.width - 34
                anchors.verticalCenter: parent.verticalCenter
                font.family: "FontAwesome"
                font.pointSize: 24
                text: commonPropsBox.isExpanded ? FontAwesome.fa_minus_square : FontAwesome.fa_plus_square
                color: "white"
            }

            MouseArea
            {
                id: headerMouseArea
                anchors.fill: parent
                hoverEnabled: true

                onClicked: commonPropsBox.isExpanded = !commonPropsBox.isExpanded
            }
        }

        GridLayout
        {
            id: cPropsGrid
            y: cPropsHeader.height
            width: parent.width
            columns: 2
            columnSpacing: 5
            rowSpacing: 4

            // row 1
            RobotoText
            {
                fontSize: 14
                label: qsTr("Label")
            }
            CustomTextEdit
            {
                inputText: wObj ? wObj.caption : ""
                Layout.fillWidth: true
                onTextChanged:
                {
                    if (wObj)
                        wObj.caption = text
                }
            }

            // row 2
            RobotoText
            {
                fontSize: 14
                label: qsTr("Background color")
            }
            Rectangle
            {
                width: 80
                height: 38
                color: wObj ? wObj.backgroundColor : "black"

                ColorTool
                {
                    id: bgColTool
                    parent: wPropsRoot
                    x: wPropsRoot.width
                    y: 100
                    visible: false

                    onColorChanged:
                    {
                        if(wObj)
                            wObj.backgroundColor = Qt.rgba(r, g, b, 1.0)
                    }
                }

                MouseArea
                {
                    anchors.fill: parent
                    onClicked: bgColTool.visible = !bgColTool.visible
                }
            }

            // row 3
            RobotoText
            {
                fontSize: 14
                label: qsTr("Foreground color")
            }
            Rectangle
            {
                width: 80
                height: 38
                color: wObj ? wObj.foregroundColor : "black"

                ColorTool
                {
                    id: fgColTool
                    parent: wPropsRoot
                    x: wPropsRoot.width
                    y: 100
                    visible: false

                    onColorChanged:
                    {
                        if(wObj)
                            wObj.foregroundColor = Qt.rgba(r, g, b, 1.0)
                    }
                }

                MouseArea
                {
                    anchors.fill: parent
                    onClicked: fgColTool.visible = !fgColTool.visible
                }
            }
        }
    }
}
