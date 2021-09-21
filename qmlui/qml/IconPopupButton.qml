/*
  Q Light Controller Plus
  IconPopupButton.qml

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
import QtQuick.Controls 2.1

import "."

CustomComboBox
{
    id: control

    /*! model: provides a data model for the popup.
        A model with icons should look like this:
            ListModel
            {
                ListElement { mLabel: qsTr("Foo"); mIcon:"qrc:/foo.svg"; mValue: 0 }
                ListElement { mLabel: qsTr("Bar"); mIcon:"qrc:/bar.svg"; mValue: 1 }
            }
        A model with text icons should look like this:
            ListModel
            {
                ListElement { mLabel: qsTr("Foo"); mTextIcon: "F"; mValue: 0 }
                ListElement { mLabel: qsTr("Bar"); mTextIcon: "B"; mValue: 1 }
            }
     */

    delegateHeight: UISettings.iconSizeDefault

    onDisplayTextChanged: buttonBox.tooltip = displayText
    Component.onCompleted: updateFromValue()

    indicator: null

    contentItem:
        IconButton
        {
            id: buttonBox
            anchors.fill: parent
            imgSource: currentIcon

            RobotoText
            {
                height: parent.height * 0.75
                anchors.centerIn: parent
                label: currentTextIcon
                fontSize: parent.height * 0.75
                fontBold: true
            }

            onClicked:
            {
                control.popup.width = UISettings.bigItemHeight * 2
                control.popup.visible ? control.popup.close() : control.popup.open()
            }
        }

    background:
        Rectangle
        {
            color: "transparent"
            implicitWidth: UISettings.iconSizeDefault
            implicitHeight: UISettings.iconSizeDefault
        }
}
