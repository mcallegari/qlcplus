/*
  Q Light Controller Plus
  CustomSpinBox.qml

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
import QtQuick.Controls.Styles 1.3
import "."

SpinBox
{
    id: spinbox
    font.family: "Roboto Condensed"
    font.pointSize: 14
    width: 70
    implicitHeight: 30

    property bool showControls: true

    style:
        SpinBoxStyle
        {
            // taken from SpinBoxStyle.qml original code
            // fundamental for the whole spinbox layout !
            padding
            {
                top: 1
                left: 2
                right: controlWidth + 2
                bottom: 0
            }
            background:
                Rectangle
                {
                    color: UISettings.bgMedium
                    border.color: "#222"
                    radius: 3
                }
            textColor: UISettings.fgMain

            property int controlWidth: showControls ? Math.min(35, spinbox.width / 3) : 0

            incrementControl:
                Rectangle
                {
                    visible: showControls
                    implicitHeight: 10
                    implicitWidth: controlWidth
                    border.color: "#222"
                    radius: 3
                    color: styleData.upHovered && !styleData.upPressed
                        ? "#555" : (styleData.upPressed ? "#222" : "#444")
                    Image
                    {
                        anchors.centerIn: parent
                        source: "qrc:/arrow-up.svg"
                        height: parent.height - 8
                        sourceSize: Qt.size(parent.width, parent.height - 8)
                        //opacity: spinbox.enabled ? (styleData.upPressed ? 1 : 0.6) : 0.5
                    }
                }

            decrementControl:
                Rectangle
                {
                    visible: showControls
                    implicitHeight: 10
                    implicitWidth: controlWidth
                    border.color: "#222"
                    radius: 3
                    color: styleData.downHovered && !styleData.downPressed
                        ? "#555" : (styleData.downPressed ? "#222" : "#444")
                    Image
                    {
                        anchors.centerIn: parent
                        source: "qrc:/arrow-up.svg"
                        rotation: 180
                        height: parent.height - 8
                        sourceSize: Qt.size(parent.width, parent.height - 8)
                        //opacity: spinbox.enabled ? (styleData.downPressed ? 1 : 0.6) : 0.5
                    }
                }
        }
}
