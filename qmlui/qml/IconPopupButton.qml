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

    contentItem:
        IconButton
        {
            id: buttonBox
            anchors.fill: parent
            imgSource: currentIcon

            RobotoText
            {
                id: textIcon
                height: parent.height * 0.75
                anchors.centerIn: parent
                label: ""
                fontSize: parent.height * 0.75
                fontBold: true
            }

            onClicked:
            {
                control.popup.width = UISettings.bigItemHeight * 2
                control.popup.visible ? control.popup.close() : control.popup.open()
            }
        }

    delegate:
        ItemDelegate
        {
            width: UISettings.bigItemHeight * 2
            implicitHeight: UISettings.iconSizeDefault
            highlighted: control.highlightedIndex === index
            hoverEnabled: control.hoverEnabled
            padding: 0
            leftPadding: 3

            Component.onCompleted:
            {
                // check for corresponding index
                if (index === control.currentIndex)
                {
                    if (model.mIcon)
                        control.currentIcon = mIcon

                    if (model.mTextIcon)
                        textIcon.label = mTextIcon

                    buttonBox.tooltip = mLabel
                }
                // check for corresponding value
                if (currentValue && mValue === currentValue)
                {
                    if (model.mIcon)
                        control.currentIcon = mIcon

                    if (model.mTextIcon)
                        textIcon.label = mTextIcon

                    control.currentIndex = index
                    buttonBox.tooltip = mLabel
                }
            }

            contentItem:
                Row
                {
                    spacing: 2

                    Image
                    {
                        visible: model.mIcon ? true : false
                        height: control.height - 4
                        width: height
                        x: 3
                        y: 2
                        source: model.mIcon ? mIcon : ""
                        sourceSize: Qt.size(width, height)
                    }

                    Rectangle
                    {
                        visible: model.mTextIcon ? true : false
                        y: 2
                        height: control.height - 4
                        width: height
                        color: UISettings.bgLight
                        radius: 3
                        border.width: 1
                        border.color: UISettings.bgStrong

                        RobotoText
                        {
                            id: txtIcon
                            height: parent.height
                            anchors.centerIn: parent
                            label: model.mTextIcon ? mTextIcon : ""
                            fontSize: parent.height * 0.9
                        }
                    }

                    RobotoText
                    {
                        x: 3
                        label: mLabel
                        height: parent.height
                    }
                }

            background:
                Rectangle
                {
                    visible: control.down || control.highlighted || control.visualFocus
                    color: highlighted ? UISettings.highlight : hovered ? UISettings.bgMedium : "transparent"
                }

            onClicked:
            {
                displayText = mLabel
                if (model.mIcon)
                    control.currentIcon = mIcon
                if (model.mTextIcon)
                    textIcon.label = mTextIcon
                currentIndex = index
                buttonBox.tooltip = mLabel
                control.valueChanged(mValue)
            }

            Rectangle { height: 1; width: parent.width; y: parent.height - 1 }
        }

    background:
        Rectangle
        {
            color: "transparent"
            implicitWidth: UISettings.iconSizeDefault
            implicitHeight: UISettings.iconSizeDefault
        }
}
