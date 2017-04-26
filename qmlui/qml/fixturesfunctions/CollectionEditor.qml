/*
  Q Light Controller Plus
  CollectionEditor.qml

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
import QtQuick.Controls 1.2

import com.qlcplus.classes 1.0
import "."

Rectangle
{
    id: ceContainer
    anchors.fill: parent
    color: "transparent"

    property int functionID: -1

    signal requestView(int ID, string qmlSrc)

    ModelSelector
    {
        id: ceSelector
        //onItemsCountChanged: console.log("Collection Editor selected items changed !")
    }

    SplitView
    {
        anchors.fill: parent
        Loader
        {
            id: funcMgrLoader
            visible: width
            width: 0
            height: ceContainer.height
            source: ""

            Rectangle
            {
                width: 2
                height: parent.height
                x: parent.width - 2
                color: UISettings.bgLighter
            }
        }

        Column
        {
            Layout.fillWidth: true

            Rectangle
            {
                color: UISettings.bgMedium
                //width: funcMgrLoader.width ? ceContainer.width / 2 : ceContainer.width
                width: parent.width
                height: UISettings.iconSizeMedium
                z: 2

                Rectangle
                {
                    id: backBox
                    width: UISettings.iconSizeMedium
                    height: width
                    color: "transparent"

                    Image
                    {
                        id: leftArrow
                        anchors.fill: parent
                        rotation: 180
                        source: "qrc:/arrow-right.svg"
                        sourceSize: Qt.size(width, height)
                    }
                    MouseArea
                    {
                        anchors.fill: parent
                        hoverEnabled: true
                        onEntered: backBox.color = "#666"
                        onExited: backBox.color = "transparent"
                        onClicked:
                        {
                            if (funcMgrLoader.width)
                            {
                                funcMgrLoader.source = "";
                                funcMgrLoader.width = 0;
                                rightSidePanel.width = rightSidePanel.width / 2
                            }

                            functionManager.setEditorFunction(-1, false)
                            requestView(-1, "qrc:/FunctionManager.qml")
                        }
                    }
                }
                TextInput
                {
                    id: cNameEdit
                    x: leftArrow.width + 5
                    height: UISettings.iconSizeMedium
                    width: ceContainer.width - backBox.width - addFunc.width - removeFunc.width
                    color: UISettings.fgMain
                    clip: true
                    text: collectionEditor.functionName
                    verticalAlignment: TextInput.AlignVCenter
                    font.family: UISettings.robotoFontName
                    font.pixelSize: UISettings.textSizeDefault
                    selectByMouse: true
                    Layout.fillWidth: true
                    onTextChanged: collectionEditor.functionName = text
                }

                IconButton
                {
                    id: addFunc
                    x: parent.width - (UISettings.iconSizeMedium * 2) - 10
                    width: height
                    height: UISettings.iconSizeMedium
                    imgSource: "qrc:/add.svg"
                    checkable: true
                    tooltip: qsTr("Add a function")
                    onCheckedChanged:
                    {
                        if (checked)
                        {
                            rightSidePanel.width += 350
                            funcMgrLoader.width = 350
                            funcMgrLoader.source = "qrc:/FunctionManager.qml"
                        }
                        else
                        {
                            rightSidePanel.width = rightSidePanel.width - funcMgrLoader.width
                            funcMgrLoader.source = ""
                            funcMgrLoader.width = 0
                        }
                    }
                }

                IconButton
                {
                    id: removeFunc
                    x: parent.width - UISettings.iconSizeMedium - 5
                    width: height
                    height: UISettings.iconSizeMedium
                    imgSource: "qrc:/remove.svg"
                    tooltip: qsTr("Remove the selected function")
                    onClicked:
                    {
                        actionManager.requestActionPopup(ActionManager.DeleteEditorItems,
                                                         qsTr("Are you sure you want to remove the selected functions ?"),
                                                         ActionManager.OK | ActionManager.Cancel,
                                                         ceSelector.itemsList())
                    }
                }
            }

            ListView
            {
                id: cFunctionList
                width: parent.width //ceContainer.width
                height: ceContainer.height - UISettings.iconSizeMedium
                y: UISettings.iconSizeMedium
                boundsBehavior: Flickable.StopAtBounds

                property int dragInsertIndex: -1

                model: collectionEditor.functionsList
                delegate:
                    Item
                    {
                        width: cFunctionList.width
                        height: UISettings.listItemHeight

                        MouseArea
                        {
                            id: delegateRoot
                            width: cFunctionList.width
                            height: parent.height

                            drag.target: cfDelegate
                            drag.threshold: height / 2

                            onClicked: ceSelector.selectItem(index, cFunctionList.model, mouse.modifiers & Qt.ControlModifier)

                            onReleased:
                            {
                                if (cfDelegate.Drag.target === cfDropArea)
                                {
                                    cfDelegate.Drag.drop()
                                }
                                else
                                {
                                    // return the dragged item to its original position
                                    parent = delegateRoot
                                    cfDelegate.x = 0
                                    cfDelegate.y = 0
                                }
                            }

                            CollectionFunctionDelegate
                            {
                                id: cfDelegate
                                width: cFunctionList.width
                                functionID: model.funcID
                                isSelected: model.isSelected
                                indexInList: index
                                highlightIndex: cFunctionList.dragInsertIndex

                                Drag.active: delegateRoot.drag.active
                                Drag.source: cfDelegate
                                Drag.keys: [ "function" ]
                            }
                        }
                    }

                DropArea
                {
                    id: cfDropArea
                    anchors.fill: parent
                    // accept only functions
                    keys: [ "function" ]

                    onDropped:
                    {
                        console.log("Item dropped here. x: " + drag.x + " y: " + drag.y)

                        /* Check if the dragging was started from a Function Manager */
                        if (drag.source.hasOwnProperty("fromFunctionManager"))
                        {
                            var insertIndex = cFunctionList.dragInsertIndex
                            if (insertIndex == -1)
                                insertIndex = 0

                            for (var i = 0; i < drag.source.itemsList.length; i++)
                            {
                                console.log("Adding function with ID: " + drag.source.itemsList[i])
                                collectionEditor.addFunction(drag.source.itemsList[i], insertIndex + i)
                            }
                        }
                        else if (drag.source.hasOwnProperty("functionID"))
                        {
                            console.log("Item fID: " + drag.source.functionID)
                            collectionEditor.moveFunction(drag.source.functionID, cFunctionList.dragInsertIndex)
                        }

                        cFunctionList.dragInsertIndex = -1
                    }
                    onPositionChanged:
                    {
                        var idx = cFunctionList.indexAt(drag.x, drag.y)
                        //console.log("Item index:" + idx)
                        cFunctionList.dragInsertIndex = idx
                    }
                }
            } // end of ListView
        } // end of Column
    } // end of SplitView
}
