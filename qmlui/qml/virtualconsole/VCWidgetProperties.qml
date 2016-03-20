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

import com.qlcplus.classes 1.0
import "."

Rectangle
{
    id: wPropsRoot
    anchors.fill: parent
    color: "transparent"

    property VCWidget wObj

    Component.onCompleted: wObj = Qt.binding(function() { return virtualConsole.selectedWidget })

    onWObjChanged:
    {
        wPropsLoader.active = false
        wPropsLoader.active = true
    }

    RobotoText
    {
        visible: wObj ? false : true
        anchors.centerIn: parent
        label: qsTr("Select a widget first")
    }

    Column
    {
        width: parent.width
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
                    fontSize: 14
                    label: qsTr("Label")
                }
                CustomTextEdit
                {
                    Layout.fillWidth: true
                    color: UISettings.bgStronger
                    inputText: wObj ? wObj.caption : ""

                    onTextChanged:
                    {
                        if (wObj)
                            wObj.caption = text
                    }
                }

                // row 2
                RobotoText
                {
                    fontSize: 14
                    label: qsTr("Background color")
                }
                Rectangle
                {
                    width: 80
                    height: 38
                    color: wObj ? wObj.backgroundColor : "black"

                    ColorTool
                    {
                        id: bgColTool
                        parent: wPropsRoot
                        x: wPropsRoot.width
                        y: 100
                        visible: false

                        onColorChanged:
                        {
                            if(wObj)
                                wObj.backgroundColor = Qt.rgba(r, g, b, 1.0)
                        }
                    }

                    MouseArea
                    {
                        anchors.fill: parent
                        onClicked: bgColTool.visible = !bgColTool.visible
                    }
                }

                // row 3
                RobotoText
                {
                    fontSize: 14
                    label: qsTr("Foreground color")
                }
                Rectangle
                {
                    width: 80
                    height: 38
                    color: wObj ? wObj.foregroundColor : "black"

                    ColorTool
                    {
                        id: fgColTool
                        parent: wPropsRoot
                        x: wPropsRoot.width
                        y: 100
                        visible: false

                        onColorChanged:
                        {
                            if(wObj)
                                wObj.foregroundColor = Qt.rgba(r, g, b, 1.0)
                        }
                    }

                    MouseArea
                    {
                        anchors.fill: parent
                        onClicked: fgColTool.visible = !fgColTool.visible
                    }
                }

                // row 4
                RobotoText
                {
                    fontSize: 14
                    label: qsTr("Font")
                }

                Rectangle
                {
                    Layout.fillWidth: true
                    height: 38
                    color: "transparent"

                    Text
                    {
                        anchors.fill: parent
                        font.family: wObj ? wObj.font.family : ""
                        font.bold: wObj ? wObj.font.bold : false
                        font.italic: wObj ? wObj.font.italic : false
                        font.pointSize: 12
                        text: wObj ? wObj.font.family : ""
                        color: "white"
                        verticalAlignment: Text.AlignVCenter
                    }

                    IconButton
                    {
                        x: parent.width - UISettings.iconSizeDefault
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
                                wObj.font = fontDialog.font
                            }
                        }
                    }
                }
             } // GridLayout
        } // SectionBox

        Loader
        {
            id: wPropsLoader
            width: parent.width
            visible: wObj ? true : false
            source: wObj ? wObj.propertiesResource : ""

            onLoaded: item.widgetRef = wObj
        }
    }
}
