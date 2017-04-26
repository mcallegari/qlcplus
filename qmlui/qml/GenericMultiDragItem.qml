/*
  Q Light Controller Plus
  FunctionDragItem.qml

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

import com.qlcplus.classes 1.0
import "."

Item
{
    property string itemLabel
    property string itemIcon
    /** Generic list of items that this component represents */
    property var itemsList: []
    property bool multipleItems: itemsList.length > 1 ? true : false

    Rectangle
    {
        id: topItem
        width: UISettings.bigItemHeight * 1.5
        height: UISettings.listItemHeight
        z: 10
        border.width: 1
        border.color: UISettings.fgMain
        opacity: 0.8
        color: UISettings.bgMedium

        IconTextEntry
        {
            id: funcEntry
            width: parent.width
            height: parent.height
            tLabel: itemLabel
            iSrc: itemIcon
        }
    }
    Rectangle
    {
        visible: multipleItems
        width: topItem.width
        height: topItem.height
        x: topItem.height / 5
        y: topItem.height / 5
        z: 9
        border.color: UISettings.fgMedium
        opacity: 0.8
        color: UISettings.bgMedium
    }
    Rectangle
    {
        visible: multipleItems
        width: topItem.width
        height: topItem.height
        x: (topItem.height / 5) * 2
        y: (topItem.height / 5) * 2
        z: 8
        border.color: UISettings.fgMedium
        opacity: 0.8
        color: UISettings.bgMedium
    }
}

