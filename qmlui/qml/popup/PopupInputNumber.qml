/*
  Q Light Controller Plus
  PopupInputNumber.qml

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

import QtQuick
import QtQuick.Layouts

import org.qlcplus.classes 1.0
import "."

CustomPopupDialog
{
    width: mainView.width / 3

    property alias label: inputLabel.label
    property alias from: numberSpin.from
    property alias to: numberSpin.to
    property alias suffix: numberSpin.suffix
    property alias value: numberSpin.value

    onOpened:
    {
        numberSpin.focus = true
    }

    contentItem:
        GridLayout
        {
            width: parent.width
            height: UISettings.iconSizeDefault * rows
            columns: 2
            columnSpacing: 5

            // Row 1
            RobotoText
            {
                id: inputLabel
                label: qsTr("Enter a number")
            }

            CustomSpinBox
            {
                id: numberSpin
                Layout.fillWidth: true
                from: 0
                to: 10000
            }
        }
}

