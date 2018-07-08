/*
  Q Light Controller Plus
  SettingsViewDMX.qml

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

import org.qlcplus.classes 1.0
import "."

Rectangle
{
    id: settingsRoot
    width: mainView.width / 5
    height: parent.height

    color: UISettings.bgMedium
    border.width: 1
    border.color: "#222"

    property bool showAddresses: ViewDMX.showAddresses
    property bool relativeAddresses: ViewDMX.relativeAddresses

    GridLayout
    {
        width: settingsRoot.width
        columns: 2
        columnSpacing: 5
        rowSpacing: 5

        // row 1
        Rectangle
        {
            height: UISettings.listItemHeight
            Layout.fillWidth: true
            color: UISettings.sectionHeader
            Layout.columnSpan: 2

            RobotoText
            {
                x: 5
                anchors.verticalCenter: parent.verticalCenter
                label: qsTr("Channels")
            }
        }

        // row 2
        RobotoText { label: qsTr("Show addresses") }
        CustomCheckBox
        {
            implicitHeight: UISettings.listItemHeight
            implicitWidth: implicitHeight
            checked: showAddresses
            onToggled: ViewDMX.showAddresses = checked
        }

        // row 2
        RobotoText { label: qsTr("Relative addresses") }
        CustomCheckBox
        {
            implicitHeight: UISettings.listItemHeight
            implicitWidth: implicitHeight
            checked: relativeAddresses
            onToggled: ViewDMX.relativeAddresses = checked
        }
    }
}
