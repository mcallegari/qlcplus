/*
  Q Light Controller Plus
  SimpleDesk.qml

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
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.13

import org.qlcplus.classes 1.0
import "."

Rectangle
{
    id: sDeskContainer
    anchors.fill: parent
    color: "transparent"

    property string contextName: "SDESK"
    property bool dmxValues: true
    property real maxValue: 255

    ChannelToolLoader
    {
        id: channelToolLoader
        z: 2

        onValueChanged: simpleDesk.setValue(fixtureID, channelIndex, value)
    }

    SplitView
    {
        anchors.fill: parent
        orientation: Qt.Vertical
        z: 1

        // Top view (faders)
        Rectangle
        {
            SplitView.minimumHeight: sDeskContainer.height * 0.2
            SplitView.preferredHeight: sDeskContainer.height * 0.6
            SplitView.maximumHeight: sDeskContainer.height * 0.8

            color: "transparent"

            Rectangle
            {
                id: viewToolbar
                width: parent.width
                height: UISettings.iconSizeDefault
                z: 10
                gradient: Gradient
                {
                    GradientStop { position: 0; color: UISettings.toolbarStartSub }
                    GradientStop { position: 1; color: UISettings.toolbarEnd }
                }

                RowLayout
                {
                    anchors.fill: parent
                    spacing: 5

                    RobotoText
                    {
                        label: qsTr("Universe")
                    }

                    CustomComboBox
                    {
                        id: viewUniverseCombo
                        implicitWidth: UISettings.bigItemHeight * 2
                        padding: 0
                        model: simpleDesk.universesListModel
                        currValue: simpleDesk.universeFilter
                        onValueChanged: simpleDesk.universeFilter = value
                    }

                    // universe reset button
                    IconButton
                    {
                        faSource: FontAwesome.fa_remove
                        faColor: UISettings.bgControl
                        tooltip: qsTr("Reset the whole universe")
                        onClicked:
                        {
                            simpleDesk.resetUniverse(viewUniverseCombo.currentIndex)
                        }
                    }

                    Rectangle { Layout.fillWidth: true; color: "transparent" }

                    // DMX/Percentage button
                    DMXPercentageButton
                    {
                        height: parent.height - 2
                        dmxMode: dmxValues
                        onClicked:
                        {
                            if (dmxValues)
                            {
                                dmxValues = false
                                maxValue = 100
                            }
                            else
                            {
                                maxValue = 255
                                dmxValues = true
                            }
                        }
                    }
                }
            }

            // The actual channel list view
            ListView
            {
                id: channelView
                y: viewToolbar.height
                height: parent.height - y
                width: parent.width
                orientation: ListView.Horizontal
                model: simpleDesk.channelList
                boundsBehavior: Flickable.StopAtBounds
                highlightFollowsCurrentItem: false
                currentIndex: -1

                function scrollToItem(chIdx)
                {
                    console.log("[scrollToItem] chIdx: " + chIdx)
                    positionViewAtIndex(chIdx, ListView.Beginning)
                    currentIndex = chIdx
                }

                delegate:
                    Rectangle
                    {
                        id: chRoot
                        implicitWidth: UISettings.iconSizeDefault
                        height: channelView.height - sbar.height
                        color: {
                            if (isOverride)
                            {
                                return "red"
                            }
                            else
                            {
                                switch(chDisplay)
                                {
                                    case SimpleDesk.None: return "transparent"
                                    case SimpleDesk.Odd: return UISettings.bgFixtureOdd
                                    case SimpleDesk.Even: return UISettings.bgFixtureEven
                                }
                            }
                        }
                        border.width: 1
                        border.color: UISettings.borderColorDark

                        property Fixture fixtureObj: model.cRef
                        property int chDisplay: model.chDisplay
                        property bool isOverride: model.isOverride

                        ColumnLayout
                        {
                            width: parent.width - 2
                            height: parent.height

                            // channel icon
                            IconButton
                            {
                                width: UISettings.iconSizeMedium
                                Layout.alignment: Qt.AlignHCenter
                                height: width
                                border.width: 0
                                tooltip: fixtureObj ? fixtureManager.channelName(fixtureObj.id, model.chIndex) : ""
                                imgSource: fixtureObj ? fixtureManager.channelIcon(fixtureObj.id, model.chIndex) : ""
                                visible: fixtureObj ? true : false

                                onClicked:
                                {
                                    if (fixtureObj)
                                        channelToolLoader.loadChannelTool(this, fixtureObj.id, model.chIndex, model.chValue)
                                }
                            }

                            // channel fader
                            QLCPlusFader
                            {
                                padding: 0
                                width: parent.width * 0.95
                                Layout.alignment: Qt.AlignHCenter
                                Layout.fillHeight: true
                                from: 0
                                to: 255
                                value: model.chValue
                                onMoved:
                                {
                                    model.isOverride = true
                                    model.chValue = valueAt(position)
                                    simpleDesk.setValue(fixtureObj ? fixtureObj.id : -1, fixtureObj ? model.chIndex : index, model.chValue)
                                }
                            }

                            // channel value
                            CustomSpinBox
                            {
                                id: chValueSpin
                                implicitWidth: UISettings.iconSizeDefault
                                height: UISettings.listItemHeight * 0.75
                                from: 0
                                to: maxValue
                                suffix: dmxValues ? "" : "%"
                                showControls: false
                                padding: 0
                                horizontalAlignment: Qt.AlignHCenter
                                value: dmxValues ? model.chValue : (model.chValue / 255.0) * 100.0
                                onValueModified:
                                {
                                    model.isOverride = true
                                    model.chValue = value * (dmxValues ? 1.0 : 2.55)
                                    simpleDesk.setValue(fixtureObj ? fixtureObj.id : -1, fixtureObj ? model.chIndex : index, model.chValue)
                                }
                            }

                            // DMX address
                            RobotoText
                            {
                                Layout.alignment: Qt.AlignHCenter
                                height: UISettings.listItemHeight * 0.75
                                fontSize: UISettings.textSizeDefault
                                labelColor: UISettings.fgMain
                                fontBold: true
                                label: index + 1
                            }

                            // channel reset button
                            IconButton
                            {
                                faSource: FontAwesome.fa_remove
                                faColor: UISettings.bgControl
                                tooltip: qsTr("Reset the channel")
                                onClicked:
                                {
                                    var channel = index - (fixtureObj ? fixtureObj.address : 0)
                                    simpleDesk.unsetDumpValue(fixtureObj ? fixtureObj.id : -1, channel)
                                    simpleDesk.resetChannel(index)
                                }
                            }
                        }
                    }

                ScrollBar.horizontal:
                    CustomScrollBar {
                        id: sbar
                        orientation: Qt.Horizontal
                    }
            }
        } // Rectangle

        // Bottom view (fixture list, keypad history, keypad)
        Rectangle
        {
            color: "transparent"
            SplitView.fillHeight: true

            Column
            {
                width: parent.width / 2
                height: parent.height

                RobotoText
                {
                    id: fixtureHeader
                    z: 2
                    width: parent.width
                    color: UISettings.sectionHeader
                    label: qsTr("Fixture List")
                    leftMargin: 5
                }

                ListView
                {
                    id: fixtureList
                    z: 1
                    height: parent.height - fixtureHeader.height
                    width: parent.width
                    boundsBehavior: Flickable.StopAtBounds
                    model: simpleDesk.fixtureList

                    delegate:
                        Rectangle
                        {
                            id: fixtureDelegate
                            width: fixtureList.width
                            height: UISettings.listItemHeight
                            color: fxiMa.pressed ? UISettings.highlight : "transparent"

                            property Fixture fixtureObj: modelData

                            IconTextEntry
                            {
                                width: parent.width
                                height: UISettings.listItemHeight

                                tLabel: fixtureObj.name
                                iSrc: fixtureObj.iconResource(true)

                                MouseArea
                                {
                                    id: fxiMa
                                    anchors.fill: parent

                                    onClicked: channelView.scrollToItem(fixtureObj.address)
                                }
                            }

                            RobotoText
                            {
                                x: parent.width - width
                                height: UISettings.listItemHeight
                                rightMargin: 5
                                label: (fixtureObj.address + 1) + " - " + (fixtureObj.address + fixtureObj.channels)
                            }
                        }
                }
            }

            Column
            {
                x: parent.width / 2
                width: parent.width / 2
                height: parent.height

                RobotoText
                {
                    id: historyHeader
                    z: 2
                    width: parent.width - keypad.width
                    leftMargin: 7
                    color: UISettings.sectionHeader
                    label: qsTr("Commands history")

                    Rectangle
                    {
                        width: 2
                        height: parent.height
                        color: UISettings.bgLight
                    }
                }

                ListView
                {
                    id: keypadHistory
                    z: 1
                    height: parent.height - historyHeader.height
                    width: parent.width - keypad.width
                    boundsBehavior: Flickable.StopAtBounds
                    model: simpleDesk.commandHistory

                    delegate:
                        RobotoText
                        {
                            label: modelData
                            width: keypadHistory.width

                            Rectangle
                            {
                                y: parent.height - 1
                                width: parent.width
                                height: 1
                                color: UISettings.bgLight
                            }

                            MouseArea
                            {
                                anchors.fill: parent
                                onDoubleClicked: keypad.commandString = modelData
                            }
                        }

                    Rectangle
                    {
                        width: 2
                        height: parent.height
                        color: UISettings.bgLight
                    }
                }
            }

            KeyPad
            {
                id: keypad
                x: parent.width - width
                height: parent.height

                onExecuteCommand:
                {
                    simpleDesk.sendKeypadCommand(cmd)
                    keypad.commandString = ""
                }
            }
        }
    }
}
