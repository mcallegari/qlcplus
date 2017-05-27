/*
  Q Light Controller Plus
  VCLabelItem.qml

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

VCWidgetItem
{
    id: labelRoot
    property VCLabel labelObj: null
    clip: true

    onLabelObjChanged:
    {
        setCommonProperties(labelObj)
    }

    Text
    {
        x: 2
        width: parent.width - 4
        height: parent.height
        font: labelObj ? labelObj.font : ""
        text: labelObj ? labelObj.caption : ""
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignHCenter
        wrapMode: Text.Wrap
        lineHeight: 0.8
        color: labelObj ? labelObj.foregroundColor : "#111"
    }
}
