/*
  Q Light Controller Plus
  PopupImportRDMFixtures.qml

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

CustomPopupDialog
{
    id: popupRoot
    width: mainView.width * 0.75
    height: mainView.height * 0.75
    title: qsTr("Import RDM fixtures")
    standardButtons: Dialog.Cancel | Dialog.Apply

    property bool autoAddressing: false

    ModelSelector
    {
        id: itemSelector
    }

    contentItem:
        Item
        {
            id: contentBox
            width: popupRoot.width
            height: popupRoot.height

            RobotoText
            {
                visible: rdmManager.discoveryStatus === RDMManager.Running
                anchors.horizontalCenter: parent.horizontalCenter
                label: qsTr("Detecting fixtures...")
            }
            Text
            {
                visible: rdmManager.discoveryStatus === RDMManager.Running
                anchors.centerIn: parent
                font.family: UISettings.fontAwesomeFontName
                font.pixelSize: UISettings.iconSizeDefault
                color: UISettings.fgMain
                text: FontAwesome.fa_spinner

                SequentialAnimation on rotation
                {
                    PropertyAnimation { to: 360; duration: 1500 }
                    running: true
                    loops: Animation.Infinite
                }
            }
            RobotoText
            {
                visible: rdmManager.discoveryStatus === RDMManager.Running
                anchors.horizontalCenter: parent.horizontalCenter
                y: parent.height * 0.75
                label: qsTr("Fixtures found" + ": " + rdmManager.fixturesFound)
            }

            ListView
            {
                id: fxListView
                z: 1
                visible: rdmManager.discoveryStatus === RDMManager.Finished
                width: contentBox.width
                height: contentBox.height
                boundsBehavior: Flickable.StopAtBounds
                headerPositioning: ListView.OverlayHeader
                clip: true
                model: rdmManager.fixtureList

                header:
                    RowLayout
                    {
                        z: 2
                        width: fxListView.width
                        height: UISettings.listItemHeight

                        RobotoText
                        {
                            width: UISettings.bigItemHeight
                            height: UISettings.listItemHeight
                            label: qsTr("Universe")
                            color: UISettings.sectionHeader
                        }
                        Rectangle { width: 1; height: UISettings.listItemHeight; color: UISettings.fgMedium }

                        RobotoText
                        {
                            width: UISettings.bigItemHeight * 1.5
                            height: UISettings.listItemHeight
                            label: qsTr("Manufacturer")
                            color: UISettings.sectionHeader
                        }
                        Rectangle { width: 1; height: UISettings.listItemHeight; color: UISettings.fgMedium }

                        RobotoText
                        {
                            Layout.fillWidth: true
                            height: UISettings.listItemHeight
                            label: qsTr("Model")
                            color: UISettings.sectionHeader
                        }
                        Rectangle { width: 1; height: UISettings.listItemHeight; color: UISettings.fgMedium }

                        RobotoText
                        {
                            width: UISettings.bigItemHeight
                            height: UISettings.listItemHeight
                            label: qsTr("DMX Address")
                            color: UISettings.sectionHeader
                        }
                        Rectangle { width: 1; height: UISettings.listItemHeight; color: UISettings.fgMedium }

                        RobotoText
                        {
                            width: UISettings.bigItemHeight * 1.5
                            height: UISettings.listItemHeight
                            label: qsTr("Personality")
                            color: UISettings.sectionHeader
                        }
                        Rectangle { width: 1; height: UISettings.listItemHeight; color: UISettings.fgMedium }

                        RobotoText
                        {
                            width: UISettings.bigItemHeight * 1.2
                            height: UISettings.listItemHeight
                            label: qsTr("UID")
                            color: UISettings.sectionHeader
                        }
                    }

                delegate:
                    Item
                    {
                        width: fxListView.width
                        height: UISettings.listItemHeight

                        property string fxUID: UID
                        property int fxAddress: dmxAddress
                        property int fxChannels: dmxChannels

                        MouseArea
                        {
                            anchors.fill: parent
                            onClicked: (mouse) => itemSelector.selectItem(index, fxListView.model, mouse.modifiers)
                        }

                        Rectangle
                        {
                            anchors.fill: parent
                            radius: 3
                            color: UISettings.highlight
                            visible: isSelected
                        }

                        RowLayout
                        {
                            id: delegateRow
                            anchors.fill: parent

                            RobotoText
                            {
                                width: UISettings.bigItemHeight
                                height: UISettings.listItemHeight
                                label: (universe + 1)
                            }
                            Rectangle { width: 1; height: UISettings.listItemHeight; color: UISettings.fgMedium }

                            RobotoText
                            {
                                width: UISettings.bigItemHeight * 1.5
                                height: UISettings.listItemHeight
                                label: manufacturer
                            }
                            Rectangle { width: 1; height: UISettings.listItemHeight; color: UISettings.fgMedium }

                            RobotoText
                            {
                                Layout.fillWidth: true
                                height: UISettings.listItemHeight
                                label: model
                            }
                            Rectangle { width: 1; height: UISettings.listItemHeight; color: UISettings.fgMedium }

                            CustomSpinBox
                            {
                                enabled: !autoAddressing
                                width: UISettings.bigItemHeight
                                height: UISettings.listItemHeight
                                from: 1
                                to: 511
                                value: fxAddress
                                onValueModified:
                                {
                                    rdmManager.requestDMXAddress(UID, value)
                                }
                            }
                            Rectangle { width: 1; height: UISettings.listItemHeight; color: UISettings.fgMedium }

                            RobotoText
                            {
                                width: UISettings.bigItemHeight * 1.5
                                height: UISettings.listItemHeight
                                label: personality
                            }
                            Rectangle { width: 1; height: UISettings.listItemHeight; color: UISettings.fgMedium }

                            RobotoText
                            {
                                width: UISettings.bigItemHeight * 1.2
                                height: UISettings.listItemHeight
                                label: fxUID
                            }
                        }
                    }
            }

            RowLayout
            {
                visible: rdmManager.discoveryStatus === RDMManager.Finished
                y: contentBox.height - height
                z: 3
                width: contentBox.width
                height: UISettings.listItemHeight

                CustomCheckBox
                {
                    implicitWidth: height
                    implicitHeight: UISettings.listItemHeight
                    checked: !autoAddressing
                    onClicked: autoAddressing = false
                }
                RobotoText
                {
                    height: UISettings.listItemHeight
                    fontBold: true
                    label: qsTr("Manual addresses")
                }
                CustomCheckBox
                {
                    implicitWidth: height
                    implicitHeight: UISettings.listItemHeight
                    checked: autoAddressing
                    onClicked: autoAddressing = true
                }
                RobotoText
                {
                    height: UISettings.listItemHeight
                    fontBold: true
                    label: qsTr("Auto-addresses")
                }
                RobotoText
                {
                    height: UISettings.listItemHeight
                    label: qsTr("From:")
                }
                CustomSpinBox
                {
                    id: aaStartAddrSpin
                    enabled: autoAddressing
                    height: UISettings.listItemHeight
                    from: 1
                    to: 511
                    value: 1
                }
                RobotoText
                {
                    height: UISettings.listItemHeight
                    label: qsTr("Gap:")
                }
                CustomSpinBox
                {
                    id: aaGapSpin
                    enabled: autoAddressing
                    height: UISettings.listItemHeight
                    from: 0
                    to: 511
                    value: 0
                }
                GenericButton
                {
                    implicitHeight: UISettings.listItemHeight
                    label: qsTr("Apply")
                    onClicked:
                    {
                        if (autoAddressing)
                        {
                            var selIndices = itemSelector.itemsList()
                            var currAddress = aaStartAddrSpin.value

                            for (var s = 0; s < selIndices.length; s++)
                            {
                                var item = fxListView.itemAtIndex(s)
                                item.fxAddress = currAddress
                                rdmManager.requestDMXAddress(item.fxUID, currAddress)
                                currAddress = currAddress + item.fxChannels + aaGapSpin.value
                            }
                        }
                        rdmManager.executeOperations()
                    }
                }
            }
        }
}
