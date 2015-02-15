/*
  Q Light Controller Plus
  CustomComboBox.qml

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

import QtQuick 2.3
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.2

ComboBox {
    implicitHeight: 24
    implicitWidth: 150
    style: ComboBoxStyle {
        background: Rectangle {
            anchors.fill: parent
            radius: 3
            color: "#333333"
            border.width: 1
            border.color: "#222"

            Rectangle {
                width: 30
                height: parent.height
                anchors.right: parent.right
                //border.width: 2
                //border.color: "#222"
                color: "transparent" //"#404040"

                Image {
                    anchors.centerIn: parent
                    source: "arrowdown.png"
                }
            }
         }
        label: RobotoText {
            //verticalAlignment: Qt.AlignVCenter
            //anchors.left: parent.left
            //anchors.leftMargin: 5
            label: control.currentText
            anchors.fill: parent
            fontSize: 12
            fontBold: true
        }
        dropDownButtonWidth: 30
    }
}
