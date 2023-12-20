/*
  Q Light Controller Plus
  ProfilesList.qml

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

Rectangle
{
    id: profilesContainer
    anchors.fill: parent
    color: "transparent"

    property int universeIndex: 0

    onUniverseIndexChanged:
    {
        profListView.model = ioManager.universeInputProfiles(universeIndex)
    }

    function loadSources(input)
    {
        profListView.model = ioManager.universeInputProfiles(universeIndex)
    }

    CustomPopupDialog
    {
        id: deletePopup
        standardButtons: Dialog.Ok | Dialog.Cancel
        title: qsTr("!! Warning !!")
        message: qsTr("Do you wish to permanently delete profile '" + profListView.selectedName + "'?")
        onAccepted:
        {
            ioManager.removeInputProfile(profListView.selectedName)
            profListView.model = ioManager.universeInputProfiles(universeIndex)
            profListView.selectedIndex = -1
        }
    }

    ColumnLayout
    {
        implicitWidth: profilesContainer.width
        height: profilesContainer.height
        z: 2
        spacing: 3

        Rectangle
        {
            id: topBar
            implicitWidth: profilesContainer.width
            implicitHeight: UISettings.iconSizeMedium
            z: 5
            gradient: Gradient
            {
                GradientStop { position: 0; color: UISettings.toolbarStartSub }
                GradientStop { position: 1; color: UISettings.toolbarEnd }
            }

            RowLayout
            {
                width: parent.width
                height: parent.height
                y: 1

                IconButton
                {
                    width: height
                    height: topBar.height - 2
                    visible: profEditor.isEditing
                    enabled: profEditor.isEditing && profileEditor.modified
                    imgSource: "qrc:/filesave.svg"
                    tooltip: qsTr("Save this profile")

                    onClicked:
                    {
                        ioManager.saveInputProfile()
                        profListView.model = ioManager.universeInputProfiles(universeIndex)
                    }
                }

                IconButton
                {
                    width: height
                    height: topBar.height - 2
                    visible: profEditor.isEditing
                    checkable: true
                    imgSource: "qrc:/wizard.svg"
                    tooltip: qsTr("Toggle the automatic detection procedure")

                    onToggled:
                    {
                        if (profEditor.isEditing)
                        {
                            if (checked)
                                profEditor.showWizard()
                            profileEditor.toggleDetection()
                        }
                    }
                }

                IconButton
                {
                    width: height
                    height: topBar.height - 2
                    imgSource: "qrc:/add.svg"
                    tooltip: profEditor.isEditing ? qsTr("Add a new channel") : qsTr("Create a new input profile")

                    onClicked:
                    {
                        if (profEditor.isEditing)
                        {
                            profEditor.addNewChannel()
                        }
                        else
                        {
                            ioManager.createInputProfile()
                            profEditor.isEditing = true
                        }
                    }
                }

                IconButton
                {
                    width: height
                    height: topBar.height - 2
                    imgSource: "qrc:/edit.svg"
                    tooltip: profEditor.isEditing ? qsTr("Edit the selected channel") : qsTr("Edit the selected input profile")
                    enabled: profEditor.isEditing ? profEditor.selectedChannel() >= 0 : profListView.selectedIndex >= 0

                    onClicked:
                    {
                        if (profEditor.isEditing)
                        {
                            profEditor.editSelectedChannel()
                        }
                        else
                        {
                            ioManager.editInputProfile(profListView.selectedName)
                            profEditor.isEditing = true
                            if (profListView.selectedIsUser == false)
                                profEditor.showWarning()
                        }
                    }
                }

                IconButton
                {
                    width: height
                    height: topBar.height - 2
                    imgSource: "qrc:/remove.svg"
                    tooltip: profEditor.isEditing ? qsTr("Delete the selected channel") : qsTr("Delete the selected input profile(s)")
                    enabled: profEditor.isEditing ? profEditor.selectedChannel() >= 0 : profListView.selectedIndex >= 0 && profListView.selectedIsUser

                    onClicked:
                    {
                        if (profEditor.isEditing)
                        {
                            profEditor.removeSelectedChannel()
                        }
                        else
                        {
                            if (profListView.selectedIsUser)
                                deletePopup.open()
                        }
                    }
                }

                Rectangle { Layout.fillWidth: true }

                GenericButton
                {
                    width: height
                    height: topBar.height - 2
                    visible: profEditor.isEditing
                    border.color: UISettings.bgMedium
                    useFontawesome: true
                    label: FontAwesome.fa_times
                    onClicked:
                    {
                        if (profileEditor.modified)
                        {
                            profEditor.showSaveFirst()
                        }
                        else
                        {
                            ioManager.finishInputProfile()
                            profEditor.isEditing = false
                        }
                    }
                }
            }
        }

        ListView
        {
            id: profListView
            implicitWidth: profilesContainer.width
            Layout.fillHeight: true
            z: 1
            boundsBehavior: Flickable.StopAtBounds
            visible: !profEditor.isEditing

            property int selectedIndex: -1
            property string selectedName
            property bool selectedIsUser: false

            delegate:
                Item
                {
                    id: itemRoot
                    height: UISettings.listItemHeight * 2
                    width: profilesContainer.width

                    MouseArea
                    {
                        id: delegateRoot
                        width: profilesContainer.width
                        height: parent.height

                        drag.target: profileItem
                        drag.threshold: 30

                        onClicked:
                        {
                            profListView.selectedIndex = index
                            profListView.selectedName = profileItem.lineName
                            profListView.selectedIsUser = modelData.isUser
                        }

                        onReleased:
                        {
                            profileItem.x = 3
                            profileItem.y = 0

                            if (profileItem.Drag.target !== null)
                            {
                                ioManager.setInputProfile(profileItem.pluginUniverse, profileItem.lineName)
                                profListView.model = ioManager.universeInputProfiles(universeIndex)
                            }
                            else
                            {
                                // return the dragged item to its original position
                                parent = itemRoot
                            }
                        }

                        PluginDragItem
                        {
                            id: profileItem
                            x: 3
                            height: UISettings.listItemHeight * 2
                            color: delegateRoot.pressed ? UISettings.highlightPressed :
                                (profListView.selectedIndex === index ? UISettings.highlight : "transparent")

                            pluginUniverse: modelData.universe
                            pluginName: modelData.plugin
                            lineName: modelData.name
                            pluginLine: modelData.line

                            Drag.active: delegateRoot.drag.active
                            Drag.source: delegateRoot
                            Drag.hotSpot.x: width / 2
                            Drag.hotSpot.y: height / 2
                            // this key must match the one in UniverseIOItem, to avoid dragging
                            // an input profile in the wrong place
                            Drag.keys: [ "profile-" + universeIndex ]

                            Text
                            {
                                visible: modelData.isUser
                                anchors.right: parent.right
                                anchors.rightMargin: 5
                                anchors.verticalCenter: parent.verticalCenter
                                color: UISettings.fgMain
                                font.family: "FontAwesome"
                                font.pixelSize: parent.height / 2
                                text: FontAwesome.fa_user
                            }

                            // line divider
                            Rectangle
                            {
                                width: parent.width - 6
                                height: 1
                                y: parent.height - 1
                                color: UISettings.bgLight
                            }
                        } // PluginDragItem
                    } // MouseArea
                } // Item
        } // ListView

        InputProfileEditor
        {
            id: profEditor
            width: profilesContainer.width
            Layout.fillHeight: true
            visible: isEditing
        }
    } // ColumnLayout
}
