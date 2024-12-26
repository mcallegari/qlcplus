/*
  Q Light Controller Plus
  PopupCustomFeedback.qml

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

import QtQuick 2.14
import QtQuick.Layouts 1.14
import QtQuick.Controls 2.14

import org.qlcplus.classes 1.0
import "."

CustomPopupDialog
{
    id: popupRoot
    width: mainView.width / (colorTableList.visible ? 2 : 3)
    title: qsTr("Custom Feedback")
    standardButtons: Dialog.Cancel | Dialog.Ok

    property var widgetObjRef: null
    property var universe
    property var channel

    property alias lowerValue: lowerValueSpin.value
    property alias upperValue: upperValueSpin.value
    property alias monitorValue: monitorValueSpin.value

    property alias lowerColor: lowerColorBox.color
    property alias upperColor: upperColorBox.color
    property alias monitorColor: monitorColorBox.color

    property int selectedValue: -1

    property bool hasColorTable: false
    property bool hasMidiChannelTable: false

    onOpened:
    {
        if (!widgetObjRef)
            return

        var sourceInfo = widgetObjRef.inputSourceFullInfo(universe, channel)

        lowerValue = sourceInfo.lowerValue
        upperValue = sourceInfo.upperValue
        monitorValue = sourceInfo.monitorValue

        if (sourceInfo.hasColorTable)
        {
            if (sourceInfo.hasOwnProperty("lowerColor"))
                lowerColor = sourceInfo.lowerColor
            if (sourceInfo.hasOwnProperty("upperColor"))
                upperColor = sourceInfo.upperColor
            if (sourceInfo.hasOwnProperty("monitorColor"))
                monitorColor = sourceInfo.monitorColor

            hasColorTable = true
            colorTableList.model = sourceInfo.colorTable
        }
        else
            hasColorTable = false

        if (sourceInfo.hasMIDIChannelTable)
        {
            hasMidiChannelTable = true
            lowerChannelCombo.model = sourceInfo.midiChannelTable
            upperChannelCombo.model = sourceInfo.midiChannelTable
            monitorChannelCombo.model = sourceInfo.midiChannelTable

            if (sourceInfo.hasOwnProperty("lowerChannel"))
                lowerChannelCombo.currentIndex = sourceInfo.lowerChannel
            if (sourceInfo.hasOwnProperty("upperChannel"))
                upperChannelCombo.currentIndex = sourceInfo.upperChannel
            if (sourceInfo.hasOwnProperty("monitorChannel"))
                monitorChannelCombo.currentIndex = sourceInfo.monitorChannel
        }
        else
            hasMidiChannelTable = false
    }

    onAccepted:
    {
        if (!widgetObjRef)
            return

        widgetObjRef.updateInputSourceFeedbackValues(universe, channel, lowerValue, upperValue, monitorValue)

        if (hasMidiChannelTable)
            widgetObjRef.updateInputSourceExtraParams(universe, channel,
                    lowerChannelCombo.currentIndex, upperChannelCombo.currentIndex, monitorChannelCombo.currentIndex)
    }

    function setFeedbackValue(value, color)
    {
        if (selectedValue == 0)
        {
            lowerValue = value
            lowerColor = color
        }
        else if (selectedValue == 1)
        {
            upperValue = value
            upperColor = color
        }
        else if (selectedValue == 2)
        {
            monitorValue = value
            monitorColor = color
        }
    }

    contentItem:
        GridLayout
        {
            width: popupRoot.width
            columns: 2

            ColumnLayout
            {
                id: valuesColumn
                Layout.fillWidth: true

                GroupBox
                {
                    title: qsTr("Values")
                    width: parent.width
                    font.family: UISettings.robotoFontName
                    font.pixelSize: UISettings.textSizeDefault
                    palette.windowText: UISettings.fgMain

                    GridLayout
                    {
                        width: parent.width
                        columns: 3

                        // row 1
                        RobotoText
                        {
                            height: UISettings.listItemHeight
                            label: qsTr("Lower Value")
                        }

                        CustomSpinBox
                        {
                            id: lowerValueSpin
                            from: 0
                            to: 255
                        }

                        Rectangle
                        {
                            id: lowerColorBox
                            visible: popupRoot.hasColorTable
                            height: UISettings.listItemHeight
                            width: UISettings.bigItemHeight
                            color: "black"

                            border.width: 2
                            border.color: popupRoot.selectedValue === 0 ? UISettings.selection : UISettings.fgMain

                            MouseArea
                            {
                                anchors.fill: parent
                                onClicked:
                                {
                                    selectedValue = 0
                                    colorTableList.visible = true
                                }
                            }
                        }

                        // row 2
                        RobotoText
                        {
                            height: UISettings.listItemHeight
                            label: qsTr("Upper Value")
                        }

                        CustomSpinBox
                        {
                            id: upperValueSpin
                            from: 0
                            to: 255
                        }

                        Rectangle
                        {
                            id: upperColorBox
                            visible: popupRoot.hasColorTable
                            height: UISettings.listItemHeight
                            width: UISettings.bigItemHeight
                            color: "black"

                            border.width: 2
                            border.color: popupRoot.selectedValue === 1 ? UISettings.selection : UISettings.fgMain

                            MouseArea
                            {
                                anchors.fill: parent
                                onClicked:
                                {
                                    selectedValue = 1
                                    colorTableList.visible = true
                                }
                            }
                        }

                        // row 3
                        RobotoText
                        {
                            height: UISettings.listItemHeight
                            label: qsTr("Monitor Value")
                        }

                        CustomSpinBox
                        {
                            id: monitorValueSpin
                            from: 0
                            to: 255
                        }

                        Rectangle
                        {
                            id: monitorColorBox
                            visible: popupRoot.hasColorTable
                            height: UISettings.listItemHeight
                            width: UISettings.bigItemHeight
                            color: "black"

                            border.width: 2
                            border.color: popupRoot.selectedValue === 2 ? UISettings.selection : UISettings.fgMain

                            MouseArea
                            {
                                anchors.fill: parent
                                onClicked:
                                {
                                    selectedValue = 2
                                    colorTableList.visible = true
                                }
                            }
                        }
                    }
                } // GroupBox

                GroupBox
                {
                    visible: popupRoot.hasMidiChannelTable
                    title: qsTr("MIDI Channel")
                    width: parent.width
                    font.family: UISettings.robotoFontName
                    font.pixelSize: UISettings.textSizeDefault
                    palette.windowText: UISettings.fgMain

                    GridLayout
                    {
                        width: parent.width
                        columns: 2

                        // row 1
                        RobotoText
                        {
                            height: UISettings.listItemHeight
                            label: qsTr("Lower Channel")
                        }

                        CustomComboBox
                        {
                            id: lowerChannelCombo
                            Layout.fillWidth: true
                            height: UISettings.listItemHeight
                            textRole: ""
                        }

                        // row 2
                        RobotoText
                        {
                            height: UISettings.listItemHeight
                            label: qsTr("Upper Channel")
                        }

                        CustomComboBox
                        {
                            id: upperChannelCombo
                            Layout.fillWidth: true
                            height: UISettings.listItemHeight
                            textRole: ""
                        }

                        // row 3
                        RobotoText
                        {
                            height: UISettings.listItemHeight
                            label: qsTr("Monitor Channel")
                        }

                        CustomComboBox
                        {
                            id: monitorChannelCombo
                            Layout.fillWidth: true
                            height: UISettings.listItemHeight
                            textRole: ""
                        }
                    }
                } // GroupBox
            } // ColumnLayout

            ListView
            {
                id: colorTableList
                visible: false
                Layout.fillWidth: true
                height: valuesColumn.height

                clip: true
                boundsBehavior: Flickable.StopAtBounds
                headerPositioning: ListView.OverlayHeader

                header:
                    RowLayout
                    {
                        z: 2
                        width: colorTableList.width
                        height: UISettings.listItemHeight

                        RobotoText
                        {
                            width: UISettings.bigItemHeight * 0.7
                            height: UISettings.listItemHeight
                            label: qsTr("Value")
                            color: UISettings.sectionHeader
                        }
                        Rectangle { width: 1; height: UISettings.listItemHeight; color: UISettings.fgMedium }

                        RobotoText
                        {
                            Layout.fillWidth: true
                            height: UISettings.listItemHeight
                            label: qsTr("Label")
                            color: UISettings.sectionHeader
                        }
                        Rectangle { width: 1; height: UISettings.listItemHeight; color: UISettings.fgMedium }

                        RobotoText
                        {
                            width: UISettings.bigItemHeight
                            height: UISettings.listItemHeight
                            label: qsTr("Color")
                            color: UISettings.sectionHeader
                        }
                    }

                delegate:
                    RowLayout
                    {
                        width: colorTableList.width
                        height: UISettings.listItemHeight

                        RobotoText
                        {
                            width: UISettings.bigItemHeight
                            height: UISettings.listItemHeight
                            label: modelData.index
                        }
                        RobotoText
                        {
                            Layout.fillWidth: true
                            height: UISettings.listItemHeight
                            label: modelData.name
                        }
                        Rectangle
                        {
                            width: UISettings.bigItemHeight
                            height: UISettings.listItemHeight
                            color: modelData.color

                            MouseArea
                            {
                                anchors.fill: parent
                                onClicked:
                                {
                                    popupRoot.setFeedbackValue(modelData.index, modelData.color)
                                }
                            }
                        }
                    }
            }
        }
}
