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

Rectangle
{
    id: nodeContainer
    width: 350
    height: nodeLabel.height + nodeChildrenView.height

    color: "transparent"

    property string textLabel
    property string nodePath
    property var folderChildren
    property bool isExpanded: false
    property bool isSelected: false
    property int childrenHeight: 0
    property int variableHeight: 0
    property string nodeIcon: "qrc:/folder.svg"
    property string childrenDelegate: "qrc:/FunctionDelegate.qml"

    signal toggled(bool expanded, int newHeight)
    signal clicked(var qItem)
    signal doubleClicked(int ID, int Type)
    signal pathChanged(string oldPath, string newPath)

    Rectangle
    {
        width: parent.width
        height: 35
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
        font.family: "RobotoCondensed"
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
        width: parent.width
        height: 35
        onClicked:
        {
            isExpanded = !isExpanded
            nodeContainer.toggled(isExpanded, childrenHeight)
            isSelected = true
            nodeContainer.clicked(nodeContainer)
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

    function nodeToggled(expanded, height)
    {
        if (expanded)
            variableHeight += height;
        else
            variableHeight -= height;
    }

    ListView
    {
        id: nodeChildrenView
        visible: isExpanded
        x: 30
        y: nodeLabel.height
        height: isExpanded ? (childrenHeight + variableHeight) : 0
        model: folderChildren
        delegate:
            Component
            {
                Loader
                {
                    width: nodeContainer.width
                    //height: 35
                    source: hasChildren ? "qrc:/TreeNodeDelegate.qml" : childrenDelegate
                    onLoaded:
                    {
                        item.textLabel = label
                        if (hasChildren)
                        {
                            item.nodePath = nodePath + "/" + path
                            item.folderChildren = childrenModel
                            item.nodeIcon = nodeContainer.nodeIcon
                            item.childrenDelegate = childrenDelegate
                            item.childrenHeight = (childrenModel.rowCount() * 35)

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
                         onToggled: nodeToggled(item.isExpanded, item.childrenHeight)
                    }
                    Connections
                    {
                        target: item
                        onClicked: if (hasChildren) nodeContainer.clicked(item)
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
