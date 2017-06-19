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

import QtQuick 2.9
import QtQuick.Controls 2.2
import "."

ComboBox
{
    id: control

    /*! model: provides a data model for the popup.
        A model can be either a string list (QStringList) or a named model
        to provide icons and values (QVariant)
        A QML model with icons should look like this:
        ListModel
        {
            ListElement { mLabel: qsTr("Foo"); mIcon:"qrc:/foo.svg"; mValue: 0 }
            ListElement { mLabel: qsTr("Bar"); mIcon:"qrc:/bar.svg"; mValue: 1 }
        }
     */

    textRole: "mLabel"

    property string currentIcon
    property int currentValue

    signal valueChanged(int value)

    delegate:
        ItemDelegate
        {
            width: parent.width
            implicitHeight: UISettings.listItemHeight
            highlighted: control.highlightedIndex === index
            hoverEnabled: control.hoverEnabled
            padding: 0
            leftPadding: 3

            property int currentIdx: control.currentIndex
            text: model.mLabel ? model.mLabel : (modelData.mLabel ? modelData.mLabel : modelData)
            property string itemIcon: model.mIcon ? model.mIcon : (modelData && modelData.mIcon ? modelData.mIcon : "")
            property int itemValue: (model.mValue !== undefined) ? model.mValue : ((modelData.mValue !== undefined) ? modelData.mValue : index)

            Component.onCompleted:
            {
                if (index === control.currentIndex)
                {
                    displayText = text
                    currentIcon = itemIcon
                }
                if (control.currentValue && itemValue === control.currentValue)
                {
                    displayText = text
                    currentIcon = itemIcon
                    currentIndex = index
                }
                //console.log("Combo item completed index: " + index)
            }

            onCurrentIdxChanged:
            {
                if (index == currentIdx)
                {
                    displayText = text
                    currentIcon = itemIcon
                    if (itemValue !== undefined)
                    {
                        control.currentValue = itemValue
                        control.valueChanged(itemValue)
                    }
                }
            }

            contentItem:
                Row
                {
                    spacing: 2
                    Image
                    {
                        id: iconItem
                        visible: itemIcon ? true : false
                        height: UISettings.listItemHeight - 4
                        width: height
                        y: 2
                        source: itemIcon
                        sourceSize: Qt.size(width, height)
                    }

                    RobotoText
                    {
                        id: textitem
                        label: text
                        height: UISettings.listItemHeight
                        fontSize: UISettings.textSizeDefault
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
                displayText = text
                currentIcon = itemIcon
                currentIndex = index

                if (itemValue !== undefined)
                    control.valueChanged(itemValue)
            }

            Rectangle { height: 1; width: parent.width; y: parent.height - 1 }
        }

    indicator:
        Image
        {
            x: control.mirrored ? control.padding : control.width - width - control.padding
            y: control.topPadding + (control.availableHeight - height) / 2
            source: "qrc:/arrow-down.svg"
            sourceSize: Qt.size(UISettings.iconSizeMedium * 0.7, UISettings.iconSizeMedium * 0.35)
            opacity: enabled ? 1 : 0.3
        }

    contentItem:
        Row
        {
            x: 2
            spacing: 2
            Image
            {
                visible: currentIcon ? true : false
                height: control.height - 4
                width: height
                y: 2
                source: currentIcon ? currentIcon : ""
                sourceSize: Qt.size(width, height)
                opacity: control.enabled ? 1 : 0.3
            }

            RobotoText
            {
                label: control.displayText
                height: control.height
                fontSize: UISettings.textSizeDefault
                opacity: control.enabled ? 1 : 0.3
            }
        }

    background:
        Rectangle
        {
            visible: !control.flat || control.down
            implicitWidth: 150
            implicitHeight: UISettings.listItemHeight
            color: control.hovered ? UISettings.bgLight : UISettings.bgMedium
            border.width: 1
            border.color: UISettings.bgStrong
            radius: 3

            MouseArea
            {
                anchors.fill: parent

                onClicked: control.popup.visible ? control.popup.close() : control.popup.open()

                onWheel:
                {
                    var newIdx
                    if (wheel.angleDelta.y > 0)
                        newIdx = Math.max(0, currentIndex - 1)
                    else
                        newIdx = Math.min(currentIndex + 1, count - 1)

                    if (newIdx !== currentIndex)
                    {
                        currentIndex = newIdx
                        console.log("Wheel event. Index: " + currentIndex)
                    }
                }
            }
        }

    popup:
        Popup
        {
            y: control.height
            width: control.width
            implicitHeight: contentItem.implicitHeight
            padding: 0

            contentItem:
                ListView
                {
                    id: popupList
                    clip: true
                    implicitHeight: contentHeight
                    model: control.delegateModel
                    currentIndex: control.highlightedIndex
                    boundsBehavior: Flickable.StopAtBounds
                    highlightRangeMode: ListView.ApplyRange
                    highlightMoveDuration: 0

                    CustomScrollBar { flickable: popupList }
                }

            background:
                Rectangle
                {
                    color: UISettings.bgLight
                    border.width: 1
                    border.color: UISettings.bgLighter
                }
        }
}
