/*
  Q Light Controller Plus
  PopupChannelWizard.qml

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
import QtQuick.Controls 2.2

import org.qlcplus.classes 1.0
import "."

CustomPopupDialog
{
    id: popupRoot
    width: mainView.width / 2
    title: qsTr("Fixture Editor Wizard")

    property EditorRef editorView: null
    property ChannelEdit chEdit: null
    property bool capabilityWizard: false
    property var itemsList: []

    function updateItemsList(create)
    {
        pListModel.clear()

        for (var i = 0; i < amountSpin.value; i++)
        {
            var nStr = nameInputBox.text.replace(/#/g, i + 1)

            if (capabilityWizard)
            {
                var addrMin = startSpin.value + (widthSpin.value * i)
                var addrMax = addrMin + widthSpin.value - 1

                if (create)
                {
                    chEdit.addCapability(addrMin, addrMax, nStr)
                }
                else
                {
                    nStr = "[" + addrMin + " - " + addrMax + "] " + nStr
                    pListModel.append({"name": nStr})
                }
            }
            else
            {
                var chType = chTypesCombo.currValue
                var compNum = 1
                var compTypes = [ chType ]
                var compNames = [ nStr ]

                switch (chType)
                {
                    case EditorRef.RGBChannel:
                        compNum = 3
                        compTypes = [ QLCChannel.Red, QLCChannel.Green, QLCChannel.Blue ]
                        compNames = [ "Red", "Green", "Blue" ]
                    break
                    case EditorRef.RGBWChannel:
                        compNum = 4
                        compTypes = [ QLCChannel.Red, QLCChannel.Green, QLCChannel.Blue, QLCChannel.White ]
                        compNames = [ "Red", "Green", "Blue", "White" ]
                    break
                    case EditorRef.RGBAWChannel:
                        compNum = 5
                        compTypes = [ QLCChannel.Red, QLCChannel.Green, QLCChannel.Blue, QLCChannel.Amber, QLCChannel.White ]
                        compNames = [ "Red", "Green", "Blue", "Amber", "White" ]
                    break
                }

                for (var j = 0; j < compNum; j++)
                {
                    var str = (compNum == 1) ? nStr : compNames[j] + " " + (i + 1)
                    var type = (compNum == 1) ? chType : compTypes[j]

                    if (create)
                    {
                        editorView.addPresetChannel(str, type)
                    }
                    else
                    {
                        pListModel.append({"name": str})
                    }
                }
            }
        }
    }

    onOpened: updateItemsList(false)

    onAccepted:
    {
        updateItemsList(true)
    }

    contentItem:
        GridLayout
        {
            columns: 1
            columnSpacing: 5

            GroupBox
            {
                title: qsTr("Properties")
                Layout.fillWidth: true
                font.family: UISettings.robotoFontName
                font.pixelSize: UISettings.textSizeDefault
                palette.windowText: UISettings.fgMain

                RowLayout
                {
                    RobotoText
                    {
                        height: UISettings.listItemHeight
                        visible: capabilityWizard
                        label: qsTr("Start")
                    }

                    CustomSpinBox
                    {
                        id: startSpin
                        visible: capabilityWizard
                        from: 0
                        to: 254
                        onValueChanged: updateItemsList(false)
                    }

                    RobotoText
                    {
                        height: UISettings.listItemHeight
                        visible: capabilityWizard
                        label: qsTr("Width")
                    }

                    CustomSpinBox
                    {
                        id: widthSpin
                        visible: capabilityWizard
                        from: 1
                        to: 255
                        onValueChanged: updateItemsList(false)
                    }

                    RobotoText
                    {
                        height: UISettings.listItemHeight
                        label: qsTr("Amount")
                    }

                    CustomSpinBox
                    {
                        id: amountSpin
                        from: 1
                        to: 1000
                        onValueChanged: updateItemsList(false)
                    }

                    RobotoText
                    {
                        visible: !capabilityWizard
                        height: UISettings.listItemHeight
                        label: qsTr("Type")
                    }

                    CustomComboBox
                    {
                        id: chTypesCombo
                        visible: !capabilityWizard

                        ListModel
                        {
                            id: chTypesModel
                            ListElement { mLabel: qsTr("Red"); mIcon: "qrc:/red.svg"; mValue: QLCChannel.Red }
                            ListElement { mLabel: qsTr("Green"); mIcon: "qrc:/green.svg"; mValue: QLCChannel.Green }
                            ListElement { mLabel: qsTr("Blue"); mIcon: "qrc:/blue.svg"; mValue: QLCChannel.Blue }
                            ListElement { mLabel: qsTr("White"); mIcon: "qrc:/white.svg"; mValue: QLCChannel.White }
                            ListElement { mLabel: qsTr("Amber"); mIcon: "qrc:/amber.svg"; mValue: QLCChannel.Amber }
                            ListElement { mLabel: qsTr("UV"); mIcon: "qrc:/uv.svg"; mValue: QLCChannel.UV }
                            ListElement { mLabel: qsTr("RGB"); mIcon: "qrc:/color.svg"; mValue: EditorRef.RGBChannel }
                            ListElement { mLabel: qsTr("RGBW"); mIcon: "qrc:/color.svg"; mValue: EditorRef.RGBWChannel }
                            ListElement { mLabel: qsTr("RGBAW"); mIcon: "qrc:/color.svg"; mValue: EditorRef.RGBAWChannel }
                            ListElement { mLabel: qsTr("Dimmer"); mIcon: "qrc:/dimmer.svg"; mValue: QLCChannel.Intensity }
                            ListElement { mLabel: qsTr("Pan"); mIcon: "qrc:/pan.svg"; mValue: QLCChannel.Pan }
                            ListElement { mLabel: qsTr("Tilt"); mIcon: "qrc:/tilt.svg"; mValue: QLCChannel.Tilt }
                            ListElement { mLabel: qsTr("Color Macro"); mIcon: "qrc:/colorwheel.svg"; mValue: QLCChannel.Colour }
                            ListElement { mLabel: qsTr("Shutter"); mIcon: "qrc:/shutter.svg"; mValue: QLCChannel.Shutter }
                            ListElement { mLabel: qsTr("Beam"); mIcon: "qrc:/beam.svg"; mValue: QLCChannel.Beam }
                            ListElement { mLabel: qsTr("Effect"); mIcon: "qrc:/star.svg"; mValue: QLCChannel.Effect }

                        }
                        model: capabilityWizard ? null : chTypesModel
                        currValue: capabilityWizard ? 0 : QLCChannel.Red
                        onValueChanged:
                        {
                            currValue = value
                            updateItemsList(false)
                        }
                    }
                } // RowLayout
            }

            GroupBox
            {
                title: qsTr("Label")
                Layout.fillWidth: true
                font.family: UISettings.robotoFontName
                font.pixelSize: UISettings.textSizeDefault
                palette.windowText: UISettings.fgMain

                CustomTextEdit
                {
                    id: nameInputBox
                    Layout.fillWidth: true
                    text: capabilityWizard ? qsTr("Capability #") : qsTr("Channel #")
                    onAccepted: popupRoot.accept()
                    onTextChanged: updateItemsList(false)
                }
            }

            GroupBox
            {
                title: qsTr("Preview")
                Layout.fillWidth: true
                font.family: UISettings.robotoFontName
                font.pixelSize: UISettings.textSizeDefault
                palette.windowText: UISettings.fgMain

                ListView
                {
                    id: previewList
                    width: parent.width
                    implicitHeight: UISettings.bigItemHeight * 2
                    clip: true
                    boundsBehavior: Flickable.StopAtBounds
                    model: ListModel { id: pListModel }

                    delegate:
                        RobotoText
                        {
                            height: UISettings.listItemHeight
                            width: previewList.width
                            label: modelData
                        }

                    ScrollBar.vertical: CustomScrollBar { }
                }
            }
        } // GridLayout
}
