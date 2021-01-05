/*
  Q Light Controller Plus
  UsageList.qml

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

import org.qlcplus.classes 1.0
import "."

Rectangle
{
    anchors.fill: parent
    color: "transparent"

    property int functionID: -1
    property QLCFunction func: null

    signal requestView(int ID, string qmlSrc)

    onFunctionIDChanged:
    {
        func = functionManager.getFunction(functionID)
    }

    Flickable
    {
        anchors.fill: parent

        Column
        {
            id: editorColumn
            width: parent.width
            spacing: 2

            RowLayout
            {
                width: parent.width
                height: UISettings.iconSizeMedium
                z: 2

                IconButton
                {
                    width: UISettings.iconSizeMedium
                    height: width
                    rotation: 180
                    bgColor: UISettings.bgMedium
                    imgSource: "qrc:/arrow-right.svg"
                    tooltip: qsTr("Go back to the Function Manager")
                    onClicked: requestView(0, "qrc:/FunctionManager.qml")
                }

                RobotoText
                {
                    height: UISettings.iconSizeMedium
                    label: qsTr("Usage of")
                }

                IconTextEntry
                {
                    Layout.fillWidth: true
                    height: parent.height
                    tLabel: func ? func.name : ""
                    functionType: func ? func.type : -1
                }
            }

            SectionBox
            {
                sectionLabel: qsTr("Used by function list")
                sectionContents:
                    ListView
                    {
                        id: funcList
                        height: contentHeight
                        model: functionID >= 0 ? functionManager.usageList(functionID) : null

                        property int selectedIndex: -1

                        delegate:
                            FunctionDelegate
                            {
                                width: editorColumn.width
                                cRef: modelData.classRef ? modelData.classRef : null
                                textLabel: modelData.label
                                isSelected: index == funcList.selectedIndex

                                onMouseEvent:
                                {
                                    if (type == App.Clicked)
                                    {
                                        funcList.selectedIndex = index
                                    }
                                    else if (type == App.DoubleClicked)
                                    {
                                        functionManager.setEditorFunction(cRef.id, false, false)
                                        requestView(cRef.id, functionManager.getEditorResource(cRef.id))
                                    }
                                }
                            }
                    }
            } // SectionBox

            SectionBox
            {
                sectionLabel: qsTr("Used by widget list")
                sectionContents:
                    ListView
                    {
                        id: widgetList
                        height: contentHeight
                        model: functionID >= 0 ? virtualConsole.usageList(functionID) : null

                        property int selectedIndex: -1

                        delegate:
                            WidgetDelegate
                            {
                                width: editorColumn.width
                                cRef: modelData.classRef ? modelData.classRef : null
                                textLabel: modelData.label
                                isSelected: index == widgetList.selectedIndex

                                onMouseEvent:
                                {
                                    if (type == App.Clicked)
                                    {
                                        widgetList.selectedIndex = index
                                    }
                                    else if (type == App.DoubleClicked)
                                    {
                                        virtualConsole.editMode = true
                                        virtualConsole.setWidgetSelection(cRef.id, null, true, false)
                                        mainView.switchToContext("VC", "")
                                    }
                                }
                            }
                    }
            } // SectionBox
        }
    }
}
