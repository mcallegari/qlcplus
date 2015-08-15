/*
  Q Light Controller Plus
  VCFrameItem.qml

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
    id: frameRoot
    property VCFrame frameObj: null

    clip: true

    onFrameObjChanged:
    {
        setCommonProperties(frameObj)
    }

    // Frame header
    Rectangle
    {
        x: 2
        y: 2
        width: parent.width
        height: 32
        color: "transparent"
        visible: frameObj ? frameObj.showHeader : false

        RowLayout
        {
            height: 32
            width: parent.width - 4
            spacing: 2

            IconButton
            {
                width: 32
                height: 32
                tooltip: qsTr("Expand/Collapse this frame")
                faSource: checked ? "\uf065" /* fa_expand */ : "\uf066" /* fa_compress */
                faColor: "white"
                checkable: true
                //checkedColor: bgColor
                onToggled:
                {
                    if (checked)
                    {
                        frameRoot.width = 200
                        frameRoot.height = 36
                    }
                    else
                    {
                        frameRoot.width = frameObj.geometry.width
                        frameRoot.height = frameObj.geometry.height
                    }
                }
            }

            Rectangle
            {
                height: 32
                radius: 3
                gradient: Gradient
                {
                    GradientStop { position: 0 ; color: "#666666" }
                    GradientStop { position: 1 ; color: "#000000" }
                }
                Layout.fillWidth: true

                Text
                {
                    x: 2
                    width: parent.width - 4
                    height: parent.height
                    font: frameObj ? frameObj.font : null
                    text: frameObj ? frameObj.caption : ""
                    verticalAlignment: Text.AlignVCenter
                    color: frameObj ? frameObj.foregroundColor : "white"
                }
            }

            IconButton
            {
                width: 32
                height: 32
                checkable: true
                tooltip: qsTr("Enable/Disable this frame")
                imgSource: "qrc:/apply.svg"
                visible: frameObj ? frameObj.showEnable : true
            }
        }
    }
}
