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
    property bool dmxMode: true

    SplitView
    {
        anchors.fill: parent
        orientation: Qt.Vertical

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

                    Rectangle { Layout.fillWidth: true; color: "transparent" }

                    RobotoText
                    {
                        label: qsTr("Universe")
                    }

                    CustomComboBox
                    {
                        id: viewUniverseCombo
                        implicitWidth: UISettings.bigItemHeight * 2
                        //height: viewToolbar.height - 4
                        anchors.margins: 1
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
                        onClicked: simpleDesk.resetUniverse(viewUniverseCombo.currentIndex)
                    }
                }
            }

            ListView
            {
                y: viewToolbar.height
                height: parent.height - y
                width: parent.width
                orientation: ListView.Horizontal
                model: simpleDesk.channelList
                boundsBehavior: Flickable.StopAtBounds
                highlightFollowsCurrentItem: false
                currentIndex: -1

                delegate:
                    Rectangle
                    {
                        id: chRoot
                        implicitWidth: UISettings.iconSizeDefault
                        height: parent.height - sbar.height
                        color: {
                            if (isOverride)
                                return "red";
                            else
                            {
                                switch(chDisplay)
                                {
                                    case SimpleDesk.None: return "transparent"
                                    case SimpleDesk.Odd: return "#414b41"
                                    case SimpleDesk.Even: return "#42444b"
                                }
                            }
                        }
                        border.width: 1
                        border.color: UISettings.borderColorDark

                        property Fixture fixtureObj: model.cRef
                        property alias dmxValue: chValueSpin.value
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
                                to: dmxMode ? 255 : 100
                                value: dmxValue
                                onMoved: {
                                    model.isOverride = true
                                    model.chValue = valueAt(position)
                                    simpleDesk.setValue(index, dmxValue)
                                }
                            }

                            // channel value
                            CustomSpinBox
                            {
                                id: chValueSpin
                                implicitWidth: UISettings.iconSizeDefault
                                height: UISettings.listItemHeight * 0.75
                                from: 0
                                to: dmxMode ? 255 : 100
                                suffix: dmxMode ? "" : "%"
                                showControls: false
                                padding: 0
                                horizontalAlignment: Qt.AlignHCenter
                                value: model.chValue
                                onValueModified: {
                                    model.isOverride = true
                                    model.chValue = value
                                    simpleDesk.setValue(index, dmxValue)
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
                                label: index
                            }

                            // channel reset button
                            IconButton
                            {
                                faSource: FontAwesome.fa_remove
                                faColor: UISettings.bgControl
                                tooltip: qsTr("Reset the channel")
                                onClicked: simpleDesk.resetChannel(index)
                            }
                        }
                    }

                ScrollBar.horizontal:
                    CustomScrollBar {
                        id: sbar
                        orientation: Qt.Horizontal
                    }
            }

        }

        Rectangle
        {
            color: "transparent"
            SplitView.fillHeight: true

            KeyPad
            {
                id: keypad
                height: parent.height

                onExecuteCommand:
                {
                    simpleDesk.sendKeypadCommand(cmd)
                    keypad.commandString = ""
                }
            }

            RobotoText
            {
                id: historyHeader
                x: keypad.width + 10
                z: 2
                width: (parent.width - keypad.width) / 2
                color: UISettings.bgControl
                label: qsTr("Commands history")

            }

            ListView
            {
                id: keypadHistory
                x: keypad.width + 10
                y: historyHeader.height
                z: 1
                height: parent.height
                width: (parent.width - keypad.width) / 2
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
            }
        }
    }
}
