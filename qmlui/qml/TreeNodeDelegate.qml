/*
  Q Light Controller Plus
  TreeNodeDelegate.qml

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

import QtQuick 2.2

import "."

Column
{
    id: nodeContainer
    width: 350
    //height: nodeLabel.height + nodeChildrenView.height

    property string textLabel
    property string nodePath
    property var folderChildren
    property bool isExpanded: false
    property bool isSelected: false
    property string nodeIcon: "qrc:/folder.svg"
    property string childrenDelegate: "qrc:/FunctionDelegate.qml"

    signal toggled(bool expanded, int newHeight)
    signal clicked(int ID, var qItem, int mouseMods)
    signal doubleClicked(int ID, int Type)
    signal pathChanged(string oldPath, string newPath)

    Rectangle
    {
        color: "transparent"
        width: nodeContainer.width
        height: 35

        // selection rectangle
        Rectangle
        {
            anchors.fill: parent
            radius: 3
            color: UISettings.highlight
            visible: isSelected
        }

        Image
        {
            width: 40
            height: 35
            source: nodeIcon
        }

        TextInput
        {
            id: nodeLabel
            x: 45
            z: 0
            width: parent.width - 45
            height: 35
            readOnly: true
            text: textLabel
            verticalAlignment: TextInput.AlignVCenter
            color: UISettings.fgMain
            font.family: "Roboto Condensed"
            font.pointSize: 12
            echoMode: TextInput.Normal
            selectByMouse: true
            selectionColor: "#4DB8FF"
            selectedTextColor: "#111"

            onEditingFinished:
            {
                z = 0
                select(0, 0)
                readOnly = true
                nodeContainer.pathChanged(nodePath, text)
            }
        }

        MouseArea
        {
            anchors.fill: parent
            height: 35
            onClicked:
            {
                isExpanded = !isExpanded
                //isSelected = true
                nodeContainer.clicked(-1, nodeContainer, mouse.modifiers)
            }
            onDoubleClicked:
            {
                nodeLabel.z = 5
                nodeLabel.readOnly = false
                nodeLabel.focus = true
                nodeLabel.cursorPosition = nodeLabel.text.length
                nodeLabel.cursorVisible = true
            }
        }
    }

    Repeater
    {
        id: nodeChildrenView
        visible: isExpanded
        width: nodeContainer.width - 20
        model: visible ? folderChildren : null
        delegate:
            Component
            {
                Loader
                {
                    id: childrenLoader
                    width: nodeChildrenView.width
                    x: 20
                    //height: 35
                    source: hasChildren ? "qrc:/TreeNodeDelegate.qml" : childrenDelegate
                    onLoaded:
                    {
                        item.textLabel = label
                        item.isSelected = Qt.binding(function() { return isSelected })

                        if (hasChildren)
                        {
                            item.nodePath = nodePath + "/" + path
                            item.isExpanded = isExpanded
                            item.folderChildren = childrenModel
                            item.nodeIcon = nodeContainer.nodeIcon
                            item.childrenDelegate = childrenDelegate

                            console.log("Item path: " + item.nodePath + ", label: " + label)
                        }
                        else
                        {
                            item.cRef = classRef
                        }
                    }
                    Connections
                    {
                        target: item
                        onClicked:
                        {
                            if (qItem == item)
                            {
                                model.isSelected = (mouseMods & Qt.ControlModifier) ? 2 : 1
                                if (model.hasChildren)
                                    model.isExpanded = item.isExpanded
                            }
                            nodeContainer.clicked(ID, qItem, mouseMods)
                        }
                    }
                    Connections
                    {
                        target: item
                        onDoubleClicked: nodeContainer.doubleClicked(ID, Type)
                    }
                    Connections
                    {
                        ignoreUnknownSignals: true
                        target: item
                        onPathChanged: nodeContainer.pathChanged(oldPath, newPath)
                    }
                }
        }
    }
}
