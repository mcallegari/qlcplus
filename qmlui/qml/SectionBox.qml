/*
  Q Light Controller Plus
  SectionBox.qml

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
import "."

Rectangle
{
    id: boxRoot
    width: parent.width
    height: isExpanded ? (cPropsHeader.height + sectionLoader.height) : cPropsHeader.height
    color: "transparent"
    clip: true

    property bool isExpanded: true
    property string sectionLabel: ""
    property Component sectionContents

    Column
    {
        id: sectionColumn
        width: parent.width

        Rectangle
        {
            id: cPropsHeader
            width: parent.width
            height: UISettings.listItemHeight
            color: headerMouseArea.containsMouse ? UISettings.highlight : UISettings.sectionHeader

            RobotoText
            {
                anchors.centerIn: parent
                label: boxRoot.sectionLabel
                height: UISettings.listItemHeight
                fontSize: UISettings.textSizeDefault
            }
            Text
            {
                x: parent.width - UISettings.listItemHeight
                anchors.verticalCenter: parent.verticalCenter
                font.family: "FontAwesome"
                font.pixelSize: UISettings.textSizeDefault * 1.2
                text: boxRoot.isExpanded ? FontAwesome.fa_minus_square : FontAwesome.fa_plus_square
                color: "white"
            }

            MouseArea
            {
                id: headerMouseArea
                anchors.fill: parent
                hoverEnabled: true

                onClicked: boxRoot.isExpanded = !boxRoot.isExpanded
            }

            Rectangle
            {
                width: parent.width
                height: 1
                y: parent.height - 1
                color: UISettings.sectionHeaderDiv
            }
        }

        Loader
        {
            id: sectionLoader
            width: parent.width
            sourceComponent: isExpanded ? boxRoot.sectionContents : null
        }
    }
}
