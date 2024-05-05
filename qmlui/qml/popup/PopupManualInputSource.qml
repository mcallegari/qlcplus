/*
  Q Light Controller Plus
  PopupManualInputSource.qml

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
    title: qsTr("Manual input source selection")
    standardButtons: Dialog.Cancel | Dialog.Ok

    property var wRef: null
    property int universe
    property int channel

    onOpened:
    {
        channelListView.model = virtualConsole.inputChannelsModel()
        uniCombo.model = virtualConsole.universeListModel()
    }
    //onClosed: channelListView.model = null

    onAccepted:
    {
        if (manualCheck.checked)
        {
            universe = uniCombo.currentValue
            channel = channelSpin.value - 1
        }

        virtualConsole.createAndAddInputSource(wRef, universe, channel)
    }

    contentItem:
        GridLayout
        {
            columnSpacing: UISettings.iconSizeMedium

            columns: 2
            rows: 2

            ButtonGroup { id: checkGroup }

            // row 1
            Row
            {
                Layout.columnSpan: 2
                Layout.fillWidth: true
                spacing: 5

                CustomCheckBox
                {
                    id: profileCheck
                    implicitHeight: UISettings.listItemHeight
                    implicitWidth: height
                    checked: true
                    ButtonGroup.group: checkGroup
                }
                RobotoText
                {
                    height: UISettings.listItemHeight
                    Layout.fillWidth: true
                    label: qsTr("Input profiles")
                }
            }

            // row 2
            ListView
            {
                id: channelListView
                Layout.columnSpan: 2
                width: mainView.width / 3
                height: mainView.height * 0.6
                clip: true
                boundsBehavior: Flickable.StopAtBounds

                Rectangle
                {
                    anchors.fill: parent
                    z: 3
                    color: "black"
                    opacity: 0.6
                    visible: manualCheck.checked
                }

                //model: null

                delegate:
                  Component
                  {
                    Loader
                    {
                        width: channelListView.width - (cListScrollBar.visible ? cListScrollBar.width : 0)
                        source: hasChildren ? "qrc:/TreeNodeDelegate.qml" : ""
                        onLoaded:
                        {
                            //console.log("[groupEditor] Item " + label + " has children: " + hasChildren)
                            item.cRef = classRef
                            item.textLabel = label
                            item.isSelected = Qt.binding(function() { return isSelected })

                            if (hasChildren)
                            {
                                item.itemIcon = "qrc:/group.svg"
                                if (type)
                                {
                                    item.itemType = type
                                    if (type == App.UniverseDragItem)
                                        isExpanded = true
                                }
                                item.nodePath = path
                                item.isExpanded = isExpanded
                                item.childrenDelegate = "qrc:/InputChannelDelegate.qml"
                                item.nodeChildren = childrenModel
                            }
                        }
                        Connections
                        {
                            target: item

                            function onMouseEvent(type, iID, iType, qItem, mouseMods)
                            {
                                switch (type)
                                {
                                    case App.Clicked:
                                        if (qItem === item)
                                        {
                                            model.isSelected = (mouseMods & Qt.ControlModifier) ? 2 : 1
                                            if (model.hasChildren)
                                                model.isExpanded = item.isExpanded
                                        }
                                        if (qItem.itemType === App.ChannelDragItem)
                                        {
                                            popupRoot.universe = qItem.itemID >> 16
                                            popupRoot.channel = qItem.itemID & 0x0000FFFF
                                        }
                                    break;
                                }
                            }
                        }
                    } // Loader
                  } // Component
                ScrollBar.vertical: CustomScrollBar { id: cListScrollBar }
            } // ListView

            // row 3
            Row
            {
                Layout.columnSpan: 2
                Layout.fillWidth: true
                spacing: 5

                CustomCheckBox
                {
                    id: manualCheck
                    implicitHeight: UISettings.listItemHeight
                    implicitWidth: height
                    ButtonGroup.group: checkGroup
                }
                RobotoText
                {
                    height: UISettings.listItemHeight
                    Layout.fillWidth: true
                    label: qsTr("Manual selection")
                }
            }

            // row 4
            RobotoText
            {
                height: UISettings.listItemHeight
                label: qsTr("Universe")
            }
            CustomComboBox
            {
                id: uniCombo
                enabled: manualCheck.checked
                Layout.fillWidth: true
                height: UISettings.listItemHeight
                onValueChanged: popupRoot.universe = currentValue
            }

            // row 5
            RobotoText
            {
                height: UISettings.listItemHeight
                label: qsTr("Channel")
            }
            CustomSpinBox
            {
                id: channelSpin
                enabled: manualCheck.checked
                Layout.fillWidth: true
                height: UISettings.listItemHeight
                from: 1
                to: 65535
                onValueChanged: popupRoot.channel = value
            }
        } // GridLayout
}
