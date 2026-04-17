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

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import org.qlcplus.classes 1.0
import "."

VCWidgetItem
{
    id: animationRoot
    property VCAnimation animationObj: null

    clip: true

    function colorControlVisible(index)
    {
        return true
    }

    function toggleColorTool(colorTool, button)
    {
        if (!colorTool || !button)
            return

        var popupParent = virtualConsole ? virtualConsole.currentPageItem() : null
        if (!popupParent)
            popupParent = animationRoot.parent
        if (!popupParent)
            return

        for (var i = 0; i < colorsRepeater.count; i++)
        {
            var repItem = colorsRepeater.itemAt(i)
            if (!repItem || !repItem.colorTool || repItem.colorTool === colorTool)
                continue
            repItem.colorTool.visible = false
        }

        colorTool.parent = popupParent
        colorTool.z = 10000

        var posInPage = button.mapToItem(popupParent, 0, button.height)
        var toolWidth = Math.max(colorTool.width, colorTool.implicitWidth)
        var toolHeight = Math.max(colorTool.height, colorTool.implicitHeight)
        var belowY = posInPage.y
        var aboveY = posInPage.y - button.height - toolHeight
        var preferredY = (belowY + toolHeight <= popupParent.height) ? belowY : aboveY

        colorTool.x = Math.max(0, Math.min(posInPage.x, popupParent.width - toolWidth))
        colorTool.y = Math.max(0, Math.min(preferredY, popupParent.height - toolHeight))
        colorTool.visible = !colorTool.visible
    }

    onAnimationObjChanged:
    {
        setCommonProperties(animationObj)
    }

    GridLayout
    {
        id: itemsLayout
        anchors.fill: parent

        columns: levelFader.visible ? 2 : 1

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
            Layout.fillWidth: true
            visible: animationObj ? animationObj.visibilityMask & VCAnimation.Label : true
            font: animationObj ? animationObj.font : Qt.font({ family: UISettings.robotoFontName })
            text: animationObj ? animationObj.caption : ""
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.Wrap
            lineHeight: 0.8
            color: animationObj ? animationObj.foregroundColor : "#111"
        }

        RowLayout
        {
            Layout.fillWidth: true
            height: UISettings.iconSizeDefault

            Repeater
            {
                id: colorsRepeater
                model: animationObj ? animationObj.colorCount : 0

                delegate: Item
                {
                    required property int index
                    width: UISettings.iconSizeDefault
                    height: width
                    visible: animationRoot.colorControlVisible(index)
                    property alias colorTool: picker

                    Rectangle
                    {
                        id: colorButton
                        anchors.fill: parent
                        radius: 5
                        border.color: colorMouseArea.containsMouse ? "white" : UISettings.bgLight
                        border.width: 2
                        color: animationObj && animationObj.colors.length > index ? animationObj.colors[index] : "transparent"

                        MouseArea
                        {
                            id: colorMouseArea
                            anchors.fill: parent
                            hoverEnabled: true
                            onClicked: animationRoot.toggleColorTool(picker, colorButton)
                        }
                    }

                    ColorTool
                    {
                        id: picker
                        parent: animationRoot
                        z: 10000
                        visible: false
                        closeOnSelect: true
                        currentRGB: colorButton.color

                        onToolColorChanged:
                            function(r, g, b, w, a, uv)
                            {
                                if (animationObj)
                                    animationObj.setColorAt(index, Qt.rgba(r, g, b, 1.0))
                            }
                        onClose: visible = false
                    }
                }
            }
        } // RowLayout

        CustomComboBox
        {
            id: algoCombo
            Layout.fillWidth: true
            height: UISettings.listItemHeight
            visible: animationObj ? animationObj.visibilityMask & VCAnimation.PresetCombo : true
            textRole: ""
            model: animationObj ? animationObj.algorithms : null
            currentIndex: animationObj ? animationObj.algorithmIndex : 0
            onActivated: (index) =>
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
