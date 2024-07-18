/*
  Q Light Controller Plus
  IntensityTool.qml

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
import QtQuick.Controls 2.0

import org.qlcplus.classes 1.0
import "."

Rectangle
{
    id: intRoot
    width: UISettings.bigItemHeight * (paletteBox.checked ? 2 : 1.5)
    height: (UISettings.bigItemHeight * 3) + paletteBox.height
    color: UISettings.bgMedium
    //border.color: UISettings.bgLight
    //border.width: 2

    property bool dmxValues: true
    property bool closeOnSelect: false
    property var dragTarget: null
    property alias showPalette: paletteBox.visible

    property alias currentValue: spinBox.value
    property real previousValue: 0
    property bool relativeValue: false

    signal valueChanged(int value)
    signal close()

    onCurrentValueChanged:
    {
        paletteBox.updateValue(dmxValues ? currentValue : currentValue * 2.55)

        if (paletteBox.isEditing || paletteBox.checked)
        {
            paletteBox.updatePreview()
        }
        else
        {
            var val = relativeValue ? currentValue - previousValue : currentValue
            if (intRoot.visible)
                intRoot.valueChanged(dmxValues ? val : val * 2.55)
            //if (closeOnSelect)
            //    intRoot.close()
        }
        previousValue = currentValue
    }

    onVisibleChanged:
    {
        if (!visible)
            paletteBox.checked = false
    }

    function show(value)
    {
        previousValue = 0
        if (value === -1)
        {
            relativeValue = true
            currentValue = 0
        }
        else
        {
            relativeValue = false
            currentValue = dmxValues ? Math.round(value) : Math.round(value / 2.55)
        }
        visible = true
    }

    function loadPalette(pId)
    {
        var palette = paletteManager.getPalette(pId)
        if (palette)
        {
            dragTopBar.visible = false
            paletteToolbar.visible = true
            paletteToolbar.text = palette.name
            paletteBox.editPalette(palette)
            intRoot.currentValue = dmxValues ? palette.intValue1 : palette.intValue1 / 2.55
        }
    }

    MouseArea
    {
        anchors.fill: parent
        onWheel: { return false }
    }

    Column
    {
        width: parent.width
        height: parent.height
        spacing: 5

        // draggable topbar
        Rectangle
        {
            id: dragTopBar
            width: parent.width
            height: UISettings.listItemHeight
            z: 10
            gradient:
                Gradient
                {
                    GradientStop { position: 0; color: UISettings.toolbarStartSub }
                    GradientStop { position: 1; color: UISettings.toolbarEnd }
                }

            RobotoText
            {
                height: parent.height
                anchors.horizontalCenter: parent.horizontalCenter
                label: qsTr("Intensity")
                fontSize: UISettings.textSizeDefault
                fontBold: true
            }
            // allow the tool to be dragged around
            // by holding it on the title bar
            MouseArea
            {
                anchors.fill: parent
                drag.target: intRoot.dragTarget ? intRoot.dragTarget : intRoot
            }

            GenericButton
            {
                width: height
                height: parent.height
                anchors.right: parent.right
                border.color: UISettings.bgMedium
                useFontawesome: true
                label: FontAwesome.fa_times
                onClicked: intRoot.close()
            }
        }

        EditorTopBar
        {
            id: paletteToolbar
            visible: false
            onBackClicked: intRoot.parent.dismiss()
            onTextChanged: paletteBox.setName(text)
        }

        // main control 'widget'
        Rectangle
        {
            id: intControl
            color: "transparent"
            x: (parent.width - width) / 2
            width: intRoot.width * 0.75
            height: intRoot.height - (UISettings.listItemHeight * 2) - (showPalette ? paletteBox.height : 0) - 20

            Image
            {
                id: intBackgroundImg
                anchors.fill: parent
                source: "qrc:/dimmer-back.svg"
                sourceSize: Qt.size(width, height)
            }

            Rectangle
            {
                id: rectMask
                color: "transparent"
                anchors.bottom: parent.bottom
                width: parent.width
                height: {
                    var range = relativeValue ? 512 : (dmxValues ? 256.0 : 100.0)
                    var multValue = relativeValue ? currentValue + 255 : currentValue
                    return Math.round((parent.height * multValue) / range)
                }
                clip: true

                Image
                {
                    anchors.bottom: parent.bottom
                    width: intBackgroundImg.width
                    height: intBackgroundImg.height
                    source: "qrc:/dimmer-fill.svg"
                    sourceSize: Qt.size(width, height)
                }
            }

            Slider
            {
                anchors.fill: parent
                orientation: Qt.Vertical
                from: relativeValue ? (dmxValues ? -255 : -100) : 0
                to: dmxValues ? 255 : 100
                stepSize: 1.0
                background: Rectangle { color: "transparent" }
                handle: Rectangle { color: "transparent" }
                wheelEnabled: true
                value: currentValue

                onPositionChanged: currentValue = valueAt(position)
            }
        }

        RowLayout
        {
            x: 5
            height: UISettings.listItemHeight
            width: intRoot.width - 10
            spacing: 5

            CustomSpinBox
            {
                id: spinBox
                Layout.fillWidth: true
                height: UISettings.listItemHeight
                from: relativeValue ? (dmxValues ? -255 : -100) : 0
                suffix: dmxValues ? "" : "%"
                to: dmxValues ? 255 : 100

                onValueModified: currentValue = value
            }

            DMXPercentageButton
            {
                height: UISettings.listItemHeight
                width: intRoot.width / 3
                dmxMode: dmxValues
                onClicked:
                {
                    var slVal = currentValue
                    var newVal
                    dmxValues = !dmxValues
                    if (dmxValues == false)
                        newVal = Math.round((slVal / 255.0) * 100.0)
                    else
                        newVal = Math.round((slVal / 100.0) * 255.0)

                    currentValue = newVal
                }
            }
        }

        PaletteFanningBox
        {
            id: paletteBox
            x: 5
            width: intRoot.width - 10
            paletteType: QLCPalette.Dimmer
        }
    } // Column
}

