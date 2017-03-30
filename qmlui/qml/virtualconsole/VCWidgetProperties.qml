/*
  Q Light Controller Plus
  VCWidgetProperties.qml

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
import QtQuick.Dialogs 1.1
import QtQuick.Controls 1.2

import com.qlcplus.classes 1.0
import "."

Rectangle
{
    id: wPropsRoot
    anchors.fill: parent
    color: "transparent"

    property VCWidget wObj: virtualConsole.selectedWidget
    property int selectedWidgetsCount: virtualConsole.selectedWidgetsCount

    //Component.onCompleted: wObj = Qt.binding(function() { return virtualConsole.selectedWidget })
    Component.onDestruction: virtualConsole.resetWidgetSelection()

    onWObjChanged:
    {
        wPropsLoader.active = false
        wPropsLoader.source = wObj ? wObj.propertiesResource : ""
        wPropsLoader.active = true
        vcRightPanel.width = vcRightPanel.width - sideLoader.width
        sideLoader.source = ""
        sideLoader.width = 0
    }

    onSelectedWidgetsCountChanged:
    {
        if (selectedWidgetsCount > 1)
            wPropsLoader.source = ""
        else
            wPropsLoader.source = wObj ? wObj.propertiesResource : ""
    }

    ColorTool
    {
        id: bgColTool
        parent: mainView
        x: vcRightPanel.x - width
        y: 100
        visible: false
        selectedColor: wObj ? wObj.backgroundColor : "black"

        onColorChanged:
        {
            if(wObj && selectedWidgetsCount < 2)
                wObj.backgroundColor = Qt.rgba(r, g, b, 1.0)
            else
                virtualConsole.setWidgetsBackgroundColor(Qt.rgba(r, g, b, 1.0))
        }
    }

    ColorTool
    {
        id: fgColTool
        parent: mainView
        x: vcRightPanel.x - width
        y: 100
        visible: false
        selectedColor: wObj ? wObj.foregroundColor : "black"

        onColorChanged:
        {
            if(wObj && selectedWidgetsCount < 2)
                wObj.foregroundColor = Qt.rgba(r, g, b, 1.0)
            else
                virtualConsole.setWidgetsForegroundColor(Qt.rgba(r, g, b, 1.0))
        }
    }

    SplitView
    {
        anchors.fill: parent
        Loader
        {
            id: sideLoader
            visible: width
            width: 0
            height: wPropsRoot.height
            source: ""

            property var modelProvider: null

            onLoaded:
            {
                if (modelProvider && item.hasOwnProperty('modelProvider'))
                    item.modelProvider = modelProvider
            }

            Rectangle
            {
                z: 1
                width: 2
                height: parent.height
                x: parent.width - 2
                color: UISettings.bgLighter
            }
        }

        Rectangle
        {
            Layout.fillWidth: true
            height: wPropsRoot.height
            color: "transparent"

            RobotoText
            {
                visible: wObj ? false : true
                anchors.centerIn: parent
                label: qsTr("Select a widget first")
            }

            Flickable
            {
              id: propsFlickable
              anchors.fill: parent
              boundsBehavior: Flickable.StopAtBounds
              contentHeight: propsContentsColumn.height

              Column
              {
                id: propsContentsColumn
                width: parent.width - (wpBar.visible ? wpBar.width : 0)
                spacing: 5

                SectionBox
                {
                    width: parent.width
                    visible: wObj ? true : false
                    sectionLabel: qsTr("Basic properties")

                    sectionContents:
                      GridLayout
                      {
                        id: cPropsGrid
                        width: parent.width
                        columns: 2
                        columnSpacing: 5
                        rowSpacing: 4

                        // row 1
                        RobotoText
                        {
                            height: UISettings.listItemHeight
                            label: qsTr("Label")
                        }
                        CustomTextEdit
                        {
                            Layout.fillWidth: true
                            height: UISettings.listItemHeight
                            color: UISettings.bgMedium
                            inputText: wObj ? wObj.caption : ""

                            onTextChanged:
                            {
                                if(wObj && selectedWidgetsCount < 2)
                                    wObj.caption = text
                                else
                                    virtualConsole.setWidgetsCaption(text)
                            }
                        }

                        // row 2
                        RobotoText
                        {
                            height: UISettings.listItemHeight
                            label: qsTr("Background color")
                        }
                        Rectangle
                        {
                            Layout.fillWidth: true
                            height: UISettings.listItemHeight
                            color: wObj ? wObj.backgroundColor : "black"

                            MouseArea
                            {
                                anchors.fill: parent
                                onClicked:
                                {
                                    fgColTool.visible = false
                                    bgColTool.visible = !bgColTool.visible
                                }
                            }
                        }

                        // row 3
                        RobotoText
                        {
                            height: UISettings.listItemHeight
                            label: qsTr("Foreground color")
                        }
                        Rectangle
                        {
                            Layout.fillWidth: true
                            height: UISettings.listItemHeight
                            color: wObj ? wObj.foregroundColor : "black"

                            MouseArea
                            {
                                anchors.fill: parent
                                onClicked:
                                {
                                    bgColTool.visible = false
                                    fgColTool.visible = !fgColTool.visible
                                }
                            }
                        }

                        // row 4
                        RobotoText
                        {
                            height: UISettings.listItemHeight
                            label: qsTr("Font")
                        }

                        Rectangle
                        {
                            Layout.fillWidth: true
                            height: UISettings.listItemHeight
                            color: "transparent"

                            Text
                            {
                                anchors.fill: parent
                                font.family: wObj ? wObj.font.family : ""
                                font.bold: wObj ? wObj.font.bold : false
                                font.italic: wObj ? wObj.font.italic : false
                                font.pixelSize: UISettings.textSizeDefault * 0.8
                                text: wObj ? wObj.font.family : ""
                                color: "white"
                                verticalAlignment: Text.AlignVCenter
                            }

                            IconButton
                            {
                                width: UISettings.iconSizeMedium
                                height: width
                                anchors.right: parent.right
                                imgSource: "qrc:/font.svg"
                                //bgColor: "#aaa"
                                //hoverColor: "#888"

                                onClicked: fontDialog.visible = true

                                FontDialog
                                {
                                    id: fontDialog
                                    title: qsTr("Please choose a font")
                                    font: wObj ? wObj.font : ""
                                    visible: false

                                    onAccepted:
                                    {
                                        console.log("Selected font: " + fontDialog.font)
                                        if(wObj && selectedWidgetsCount < 2)
                                            wObj.font = fontDialog.font
                                        else
                                            virtualConsole.setWidgetsFont(fontDialog.font)
                                    }
                                }
                            }
                        }

                        // row 5
                        RobotoText
                        {
                            height: UISettings.listItemHeight
                            label: qsTr("Background image")
                        }

                        Rectangle
                        {
                            Layout.fillWidth: true
                            height: UISettings.listItemHeight
                            color: "transparent"

                            RobotoText
                            {
                                width: parent.width - imgButton.width - 5
                                height: parent.height
                                label: wObj ? wObj.backgroundImage : ""
                                fontSize: UISettings.textSizeDefault * 0.8
                            }

                            IconButton
                            {
                                id: imgButton
                                width: UISettings.iconSizeMedium
                                height: width
                                anchors.right: parent.right
                                imgSource: "qrc:/background.svg"

                                onClicked: fileDialog.visible = true

                                FileDialog
                                {
                                    id: fileDialog
                                    visible: false
                                    title: qsTr("Select an image")
                                    nameFilters: [ "Image files (*.png *.bmp *.jpg *.jpeg *.gif)", "All files (*)" ]

                                    onAccepted:
                                    {
                                        if(wObj && selectedWidgetsCount < 2)
                                            wObj.backgroundImage = fileDialog.fileUrl
                                        else
                                            virtualConsole.setWidgetsBackgroundImage(fileDialog.fileUrl)
                                    }
                                }
                            }
                        }

                        // row 6
                        RobotoText
                        {
                            visible: selectedWidgetsCount > 1 ? true : false
                            label: qsTr("Alignment")
                        }

                        Row
                        {
                            Layout.fillWidth: true
                            visible: selectedWidgetsCount > 1 ? true: false

                            IconButton
                            {
                                id: alignLeftBtn
                                width: UISettings.iconSizeDefault
                                height: width
                                bgColor: UISettings.bgLighter
                                imgSource: "qrc:/align-left.svg"
                                tooltip: qsTr("Align the selected widgets to the left")
                                onClicked: virtualConsole.setWidgetsAlignment(wObj, Qt.AlignLeft)
                            }
                            IconButton
                            {
                                id: alignRightBtn
                                width: UISettings.iconSizeDefault
                                height: width
                                bgColor: UISettings.bgLighter
                                imgSource: "qrc:/align-right.svg"
                                tooltip: qsTr("Align the selected widgets to the right")
                                onClicked: virtualConsole.setWidgetsAlignment(wObj, Qt.AlignRight)
                            }
                            IconButton
                            {
                                id: alignTopBtn
                                width: UISettings.iconSizeDefault
                                height: width
                                bgColor: UISettings.bgLighter
                                imgSource: "qrc:/align-top.svg"
                                tooltip: qsTr("Align the selected widgets to the top")
                                onClicked: virtualConsole.setWidgetsAlignment(wObj, Qt.AlignTop)
                            }
                            IconButton
                            {
                                id: alignBottomBtn
                                width: UISettings.iconSizeDefault
                                height: width
                                bgColor: UISettings.bgLighter
                                imgSource: "qrc:/align-bottom.svg"
                                tooltip: qsTr("Align the selected widgets to the bottom")
                                onClicked: virtualConsole.setWidgetsAlignment(wObj, Qt.AlignBottom)
                            }
                        }
                     } // GridLayout
                } // SectionBox

                Loader
                {
                    id: wPropsLoader
                    width: parent.width
                    visible: wObj ? true : false
                    //source: wObj && virtualConsole.selectedWidgetsCount < 2 ? wObj.propertiesResource : ""

                    onLoaded: item.widgetRef = wObj
                }

                SectionBox
                {
                    width: parent.width
                    visible: wObj ? true : false
                    sectionLabel: qsTr("External Controls")
                    isExpanded: false

                    sectionContents:
                        ExternalControls
                        {
                            width: parent.width

                            objRef: wObj
                        }
                }
              } // end of properties column
            } // end of flickable
            CustomScrollBar { id: wpBar; flickable: propsFlickable }
        } // end of Rectangle
    } // end of SplitView
}
