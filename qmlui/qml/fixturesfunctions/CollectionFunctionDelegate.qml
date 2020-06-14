/*
  Q Light Controller Plus
  CollectionFunctionDelegate.qml

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
import org.qlcplus.classes 1.0
import "."

Rectangle
{
    id: funcDelegate
    width: 100
    height: UISettings.listItemHeight

    color: "transparent"

    property int functionID: -1
    property QLCFunction func
    property string textLabel
    property bool isSelected: false
    property int indexInList: -1
    property int highlightIndex: -1

    onFunctionIDChanged:
    {
        func = functionManager.getFunction(functionID)
        textLabel = func.name
        funcEntry.functionType = func.type
    }

    Rectangle
    {
        anchors.fill: parent
        radius: 3
        color: UISettings.highlight
        visible: isSelected
    }

    IconTextEntry
    {
        id: funcEntry
        width: parent.width
        height: parent.height
        tLabel: textLabel
    }

    // items divider
    Rectangle
    {
        width: parent.width
        height: 1
        y: parent.height - 1
        color: UISettings.bgLight
    }

    // top line drag highlight
    Rectangle
    {
        visible: highlightIndex == indexInList
        width: parent.width
        height: 2
        z: 1
        color: UISettings.selection
    }
}

