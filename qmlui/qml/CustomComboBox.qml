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
import QtQuick.Window 2.3
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
    wheelEnabled: true

    property string currentIcon
    property int currentValue

    signal valueChanged(int value)

    onCurrentValueChanged:
    {
        if (!model)
            return

        //console.log("Value changed:" + currentValue + ", model count: " + model.length)
        for (var i = 0; i < model.length; i++)
        {
            var item = model[i]
            if (item.mValue === currentValue)
            {
                displayText = item.mLabel
                if (item.mIcon)
                    currentIcon = item.mIcon
                currentIndex = i
                return
            }
        }
    }

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
            property string itemIcon: model.mIcon ? model.mIcon : (typeof modelData !== 'undefined' ? modelData.mIcon ? modelData.mIcon : "" : "")
            property int itemValue: (model.mValue !== undefined) ? model.mValue : ((modelData.mValue !== undefined) ? modelData.mValue : index)

            Component.onCompleted:
            {
                //console.log("Combo item completed index: " + index + ", label: " + text + ", value: " + itemValue)

                if (index === control.currentIndex)
                {
                    displayText = text
                    currentIcon = itemIcon
                    if (itemValue !== undefined && itemValue != currentValue)
                    {
                        currentValue = itemValue
                        control.valueChanged(itemValue)
                    }
                }
            }

            onCurrentIdxChanged:
            {
                if (index == currentIdx)
                {
                    displayText = text
                    currentIcon = itemIcon
                    console.log("Index changed:" + index + ", value: " + itemValue)
                    if (itemValue !== undefined)
                    {
                        currentValue = itemValue
                        control.valueChanged(itemValue)
                    }
                }
            }

            contentItem:
                Row
                {
                    spacing: 2
                    leftPadding: 3

                    Image
                    {
                        visible: itemIcon ? true : false
                        height: UISettings.listItemHeight - 4
                        width: height
                        y: 2
                        source: itemIcon
                        sourceSize: Qt.size(width, height)
                    }

                    RobotoText
                    {
                        label: text
                        height: UISettings.listItemHeight
                        fontSize: UISettings.textSizeDefault
                    }
                }

            background:
                Rectangle
                {
                    width: control.width
                    height: UISettings.listItemHeight
                    visible: control.down || control.highlighted || control.visualFocus
                    color: highlighted ? UISettings.highlight : hovered ? UISettings.bgControl : "transparent"
                }

            onClicked:
            {
                currentIndex = index
                displayText = text
                currentIcon = itemIcon

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
            spacing: 2
            leftPadding: 3

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
            color: control.hovered ? UISettings.bgLight : UISettings.bgControl
            border.width: 1
            border.color: UISettings.bgStrong
            radius: 3
        }

    popup:
        Popup
        {
            y: control.height
            width: control.width
            height: Math.min(contentItem.implicitHeight, control.Window.height - topMargin - bottomMargin)
            topMargin: 0
            bottomMargin: 0
            padding: 0

            contentItem:
                ListView
                {
                    id: popupList
                    clip: true
                    implicitHeight: contentHeight
                    model: control.popup.visible ? control.delegateModel : null
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
