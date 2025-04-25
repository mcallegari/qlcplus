/*
  Q Light Controller Plus
  VCAnimationItem.qml

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
import QtQuick.Controls 2.2

import org.qlcplus.classes 1.0
import "."

VCWidgetItem
{
    id: animationRoot
    property VCAnimation animationObj: null

    clip: true

    onAnimationObjChanged:
    {
        setCommonProperties(animationObj)
    }

    GridLayout
    {
        id: itemsLayout
        anchors.fill: parent

        columns: levelFader.visible ? 3 : 2

        QLCPlusFader
        {
            id: levelFader
            Layout.fillHeight: true
            Layout.rowSpan: 3
            from: 0
            to: 255
            visible: animationObj ? animationObj.visibilityMask & VCAnimation.Fader : true
            value: animationObj ? animationObj.faderLevel : 0
            onValueChanged: if (animationObj) animationObj.faderLevel = value
        }

        Text
        {
            Layout.columnSpan: 2
            Layout.fillWidth: true
            visible: animationObj ? animationObj.visibilityMask & VCAnimation.Label : true
            font: animationObj ? animationObj.font : ""
            text: animationObj ? animationObj.caption : ""
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.Wrap
            lineHeight: 0.8
            color: animationObj ? animationObj.foregroundColor : "#111"
        }

        Rectangle
        {
            id: col1Button
            width: UISettings.iconSizeDefault * 1.5
            height: width
            radius: 5
            border.color: scMouseArea.containsMouse ? "white" : UISettings.bgLight
            border.width: 2
            color: animationObj ? animationObj.color1 : "transparent"
            visible: animationObj ? animationObj.visibilityMask & VCAnimation.Color1 : true

            MouseArea
            {
                id: scMouseArea
                anchors.fill: parent
                hoverEnabled: true
                onClicked: col1Tool.visible = !col1Tool.visible
            }

            ColorTool
            {
                id: col1Tool
                parent: animationRoot.parent
                x: animationRoot.x // -width - (UISettings.iconSizeDefault * 1.25)
                y: animationRoot.y // UISettings.bigItemHeight
                visible: false
                closeOnSelect: true
                currentRGB: animationObj ? animationObj.color1 : "black"

                onColorChanged:
                {
                    col1Button.color = Qt.rgba(r, g, b, 1.0)
                    animationObj.color1 = col1Button.color
                }
                onClose: visible = false
            }
        }
        Rectangle
        {
            id: col2Button
            width: UISettings.iconSizeDefault * 1.5
            height: width
            radius: 5
            border.color: ecMouseArea.containsMouse ? "white" : UISettings.bgLight
            border.width: 2
            color: animationObj ? animationObj.color2 : "transparent"
            visible: animationObj ? animationObj.visibilityMask & VCAnimation.Color2 : true

            MouseArea
            {
                id: ecMouseArea
                anchors.fill: parent
                hoverEnabled: true
                onClicked: col2Tool.visible = !col2Tool.visible
            }

            ColorTool
            {
                id: col2Tool
                parent: animationRoot.parent
                x: animationRoot.x // -width - (UISettings.iconSizeDefault * 1.25)
                y: animationRoot.y // UISettings.bigItemHeight
                visible: false
                closeOnSelect: true
                currentRGB: animationObj ? animationObj.color2 : "black"

                onColorChanged: animationObj.color2 = Qt.rgba(r, g, b, 1.0)
                onClose: visible = false
            }
        }
        Rectangle
        {
            id: col3Button
            width: UISettings.iconSizeDefault * 1.5
            height: width
            radius: 5
            border.color: ecMouseArea.containsMouse ? "white" : UISettings.bgLight
            border.width: 2
            color: animationObj ? animationObj.color3 : "transparent"
            visible: animationObj ? animationObj.visibilityMask & VCAnimation.Color3 : true

            MouseArea
            {
                id: ecMouseArea
                anchors.fill: parent
                hoverEnabled: true
                onClicked: col3Tool.visible = !col3Tool.visible
            }

            ColorTool
            {
                id: col3Tool
                parent: animationRoot.parent
                x: animationRoot.x // -width - (UISettings.iconSizeDefault * 1.25)
                y: animationRoot.y // UISettings.bigItemHeight
                visible: false
                closeOnSelect: true
                currentRGB: animationObj ? animationObj.color3 : "black"

                onColorChanged: animationObj.color3 = Qt.rgba(r, g, b, 1.0)
                onClose: visible = false
            }
        }
        Rectangle
        {
            id: col4Button
            width: UISettings.iconSizeDefault * 1.5
            height: width
            radius: 5
            border.color: ecMouseArea.containsMouse ? "white" : UISettings.bgLight
            border.width: 2
            color: animationObj ? animationObj.color4 : "transparent"
            visible: animationObj ? animationObj.visibilityMask & VCAnimation.Color4 : true

            MouseArea
            {
                id: ecMouseArea
                anchors.fill: parent
                hoverEnabled: true
                onClicked: col4Tool.visible = !col4Tool.visible
            }

            ColorTool
            {
                id: col4Tool
                parent: animationRoot.parent
                x: animationRoot.x // -width - (UISettings.iconSizeDefault * 1.25)
                y: animationRoot.y // UISettings.bigItemHeight
                visible: false
                closeOnSelect: true
                currentRGB: animationObj ? animationObj.color4 : "black"

                onColorChanged: animationObj.color4 = Qt.rgba(r, g, b, 1.0)
                onClose: visible = false
            }
        }
        Rectangle
        {
            id: col5Button
            width: UISettings.iconSizeDefault * 1.5
            height: width
            radius: 5
            border.color: ecMouseArea.containsMouse ? "white" : UISettings.bgLight
            border.width: 2
            color: animationObj ? animationObj.color5 : "transparent"
            visible: animationObj ? animationObj.visibilityMask & VCAnimation.Color5 : true

            MouseArea
            {
                id: ecMouseArea
                anchors.fill: parent
                hoverEnabled: true
                onClicked: col5Tool.visible = !col5Tool.visible
            }

            ColorTool
            {
                id: col5Tool
                parent: animationRoot.parent
                x: animationRoot.x // -width - (UISettings.iconSizeDefault * 1.25)
                y: animationRoot.y // UISettings.bigItemHeight
                visible: false
                closeOnSelect: true
                currentRGB: animationObj ? animationObj.color5 : "black"

                onColorChanged: animationObj.color5 = Qt.rgba(r, g, b, 1.0)
                onClose: visible = false
            }
        }

        CustomComboBox
        {
            id: algoCombo
            Layout.columnSpan: 2
            Layout.fillWidth: true
            height: UISettings.listItemHeight
            visible: animationObj ? animationObj.visibilityMask & VCAnimation.PresetCombo : true
            textRole: ""
            model: animationObj ? animationObj.algorithms : null
            currentIndex: animationObj ? animationObj.algorithmIndex : 0
            onCurrentIndexChanged:
            {
                if (animationObj)
                    animationObj.algorithmIndex = currentIndex
            }
        }
    }

    DropArea
    {
        id: dropArea
        anchors.fill: parent
        z: 2 // this area must be above the VCWidget resize controls
        keys: [ "function" ]

        onDropped:
        {
            // attach function here
            if (drag.source.hasOwnProperty("fromFunctionManager"))
                animationObj.functionID = drag.source.itemsList[0]
        }

        states: [
            State
            {
                when: dropArea.containsDrag
                PropertyChanges
                {
                    target: animationRoot
                    color: UISettings.activeDropArea
                }
            }
        ]
    }
}
