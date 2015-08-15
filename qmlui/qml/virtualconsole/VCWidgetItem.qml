/*
  Q Light Controller Plus
  VCWidgetItem.qml

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

Rectangle
{
    width: 100
    height: 100
    color: "darkgray"
    border.width: 1
    border.color: "#111"

    function setCommonProperties(obj)
    {
        if (obj === null)
            return;

        x = obj.geometry.x
        y = obj.geometry.y
        width = obj.geometry.width
        height = obj.geometry.height
        color = obj.backgroundColor
    }

    // resize area
    Rectangle
    {

    }
}
