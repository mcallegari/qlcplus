/*
  Q Light Controller Plus
  PopupMonitor.qml

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
import QtQuick.Controls 2.2

import org.qlcplus.classes 1.0
import "."

CustomPopupDialog
{
    id: popupRoot
    width: mainView.width / 3
    title: qsTr("2D Point of view selection")
    standardButtons: Dialog.Ok

    property int selectedPov: MonitorProperties.FrontView

    contentItem:
        GridLayout
        {
            columns: 2
            rowSpacing: 5
            columnSpacing: 5

            // row 1
            RobotoText
            {
                Layout.columnSpan: 2
                Layout.fillWidth: true
                height: UISettings.listItemHeight
                label: qsTr("Please select the initial point of view for your 2D preview")
            }

            // row 2
            CustomCheckBox
            {
                id: topViewCheck
                implicitHeight: UISettings.listItemHeight
                implicitWidth: height
                autoExclusive: true
                onCheckedChanged: if (checked) selectedPov = MonitorProperties.TopView
            }
            RobotoText
            {
                height: UISettings.listItemHeight
                Layout.fillWidth: true
                label: qsTr("Top view")
            }

            // row 3
            CustomCheckBox
            {
                id: frontViewCheck
                implicitHeight: UISettings.listItemHeight
                implicitWidth: height
                checked: true
                autoExclusive: true
                onCheckedChanged: if (checked) selectedPov = MonitorProperties.FrontView
            }
            RobotoText
            {
                height: UISettings.listItemHeight
                Layout.fillWidth: true
                label: qsTr("Front view")
            }

            // row 4
            CustomCheckBox
            {
                id: rightViewCheck
                implicitHeight: UISettings.listItemHeight
                implicitWidth: height
                autoExclusive: true
                onCheckedChanged: if (checked) selectedPov = MonitorProperties.RightSideView
            }
            RobotoText
            {
                height: UISettings.listItemHeight
                Layout.fillWidth: true
                label: qsTr("Right side view")
            }

            // row 5
            CustomCheckBox
            {
                id: leftViewCheck
                implicitHeight: UISettings.listItemHeight
                implicitWidth: height
                autoExclusive: true
                onCheckedChanged: if (checked) selectedPov = MonitorProperties.LeftSideView
            }
            RobotoText
            {
                height: UISettings.listItemHeight
                Layout.fillWidth: true
                label: qsTr("Left side view")
            }
        }
}
