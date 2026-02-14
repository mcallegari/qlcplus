/*
  Q Light Controller Plus
  PopupFolderBrowser.qml

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
    width: mainView.width / 2

    property alias currentFolder: folderBrowser.currentPath
    property var nameFilters
    //property string selectedFile: folderBrowser.currentPath + folderBrowser.separator() + fileNameInput.text
    property alias selectedFile: fileNameInput.text

    FolderBrowser
    {
        id: folderBrowser
    }

    function folderSeparator()
    {
        return folderBrowser.separator()
    }

    onOpened: folderList.selectedIndex = -1

    contentItem:
        GridLayout
        {
            columns: 2

            // row 1 - path breadcrumb
            ListView
            {
                Layout.columnSpan: 2
                Layout.fillWidth: true
                implicitHeight: UISettings.iconSizeDefault
                orientation: ListView.Horizontal
                boundsBehavior: Flickable.StopAtBounds
                clip: true
                spacing: 5
                model: folderBrowser.pathModel

                delegate:
                    RobotoText
                    {
                        id: delegateRoot
                        //width: UISettings.bigItemHeight * 1.3
                        //height: parent.height
                        color: prMouseArea.pressed ? UISettings.bgLight : UISettings.bgMedium
                        border.width: 1
                        border.color: UISettings.bgLight
                        rightMargin: 5
                        leftMargin: 5
                        label: modelData.name

                        property string absPath: modelData.absPath

                        MouseArea
                        {
                            id: prMouseArea
                            anchors.fill: parent
                            hoverEnabled: true

                            onClicked: folderBrowser.currentPath = modelData.absPath
                        }
                }
            }

            // row 2
            SplitView
            {
                Layout.columnSpan: 2
                Layout.fillWidth: true
                height: UISettings.listItemHeight * 8

                handle: Rectangle
                {
                    implicitWidth: screenPixelDensity * UISettings.scalingFactor * 0.9
                    color: SplitHandle.hovered || SplitHandle.pressed ? UISettings.highlight : UISettings.bgLighter
                }

                // List of drives/home
                ListView
                {
                    id: drivesList
                    SplitView.preferredWidth: popupRoot.width / 3
                    implicitHeight: UISettings.listItemHeight * 8

                    model: folderBrowser.drivesModel

                    property int selectedIndex: -1

                    delegate:
                        Rectangle
                        {
                            width: parent.width
                            height: UISettings.listItemHeight
                            color: index === drivesList.selectedIndex ? UISettings.highlight : "transparent"

                            IconTextEntry
                            {
                                x: 5
                                width: parent.width
                                height: UISettings.listItemHeight
                                tLabel: modelData.name
                                faSource: FontAwesome.fa_hard_drive
                                faColor: UISettings.fgMain
                            }

                            MouseArea
                            {
                                anchors.fill: parent
                                onClicked:
                                {
                                    drivesList.selectedIndex = index
                                    folderBrowser.currentPath = modelData.path
                                }
                            }

                            Rectangle
                            {
                                y: parent.height - 1
                                width: parent.width
                                height: 1
                                color: UISettings.bgLighter
                            }
                        }
                }

                // list of files and folders
                ListView
                {
                    id: folderList
                    SplitView.fillWidth: true
                    implicitHeight: UISettings.listItemHeight * 8
                    boundsBehavior: Flickable.StopAtBounds
                    clip: true

                    model: folderBrowser.folderModel

                    property int selectedIndex: -1

                    delegate:
                        Rectangle
                        {
                            width: folderList.width
                            height: UISettings.listItemHeight
                            color: index === folderList.selectedIndex ? UISettings.highlight : "transparent"

                            IconTextEntry
                            {
                                x: 5
                                width: parent.width
                                height: UISettings.listItemHeight
                                iSrc: modelData.isFolder ? "qrc:/folder.svg" : ""
                                tLabel: modelData.name
                                faSource: modelData.isFolder ? "" : FontAwesome.fa_file
                                faColor: UISettings.fgMain
                            }

                            MouseArea
                            {
                                anchors.fill: parent
                                onClicked:
                                {
                                    folderList.selectedIndex = index
                                    if (modelData.isFolder === false)
                                        fileNameInput.text = modelData.name
                                }
                                onDoubleClicked:
                                {
                                    if (modelData.isFolder === true)
                                    {
                                        folderList.selectedIndex = -1
                                        folderBrowser.currentPath = folderBrowser.currentPath + folderBrowser.separator() + modelData.name
                                    }
                                    else
                                        popupRoot.accept()
                                }
                            }

                            Rectangle
                            {
                                y: parent.height - 1
                                width: parent.width
                                height: 1
                                color: UISettings.bgLighter
                            }
                        }
                }
            }

            // row 3
            RobotoText
            {
                label: qsTr("File name:")
                height: UISettings.iconSizeMedium
            }

            Rectangle
            {
                Layout.fillWidth: true
                height: UISettings.iconSizeMedium
                color: UISettings.bgControl

                TextInput
                {
                    id: fileNameInput
                    anchors.fill: parent
                    color: UISettings.fgMain
                    clip: true
                    verticalAlignment: TextInput.AlignVCenter
                    font.family: UISettings.robotoFontName
                    font.pixelSize: UISettings.textSizeDefault
                    selectByMouse: true
                    selectionColor: UISettings.highlightPressed
                    selectedTextColor: UISettings.fgMain
                }
            }

            // row 4
            RobotoText
            {
                label: qsTr("Files of type:")
                height: UISettings.iconSizeMedium
            }
            CustomComboBox
            {
                Layout.fillWidth: true
                height: UISettings.iconSizeMedium
                textRole: ""
                model: nameFilters

                Component.onCompleted: folderBrowser.selectedNameFilter = currentText
                onActivated: folderBrowser.selectedNameFilter = currentText
            }
        }
}
