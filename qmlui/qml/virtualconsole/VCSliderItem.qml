/*
  Q Light Controller Plus
  VCSliderItem.qml

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

import com.qlcplus.classes 1.0

VCWidgetItem
{
    id: sliderRoot
    property VCSlider sliderObj: null

    radius: 2

    onSliderObjChanged:
    {
        setCommonProperties(sliderObj)
    }

    ColumnLayout
    {
        anchors.fill: parent
        spacing: 2

        // value text box
        Text
        {
            anchors.horizontalCenter: parent.horizontalCenter
            height: 32
            font: sliderObj ? sliderObj.font : ""
            text: sliderObj ? /*sliderObj.sliderValue*/ "0" : ""

            color: sliderObj ? sliderObj.foregroundColor : "white"
        }

        // the central fader
        QLCPlusFader
        {
            anchors.horizontalCenter: parent.horizontalCenter
            Layout.fillHeight: true
            width: parent.width
        }

        // widget name text box
        Text
        {
            x: 2
            width: sliderRoot.width - 4
            height: 32
            font: sliderObj ? sliderObj.font : ""
            text: sliderObj ? sliderObj.caption : ""
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
            color: sliderObj ? sliderObj.foregroundColor : "white"
        }
    }
}
