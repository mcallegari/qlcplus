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
import QtQuick.Controls 2.14
import "."

ComboBox
{
    id: control

    /*! model: provides a data model for the popup.
        A model can be either a string list (QStringList) or a named model
        to provide icons and values (QVariant)
        In case of a QStringList, textRole should be set to ""
        A QML model with icons should look like this:
        ListModel
        {
            ListElement { mLabel: qsTr("Foo"); mIcon:"qrc:/foo.svg"; mValue: 0 }
            ListElement { mLabel: qsTr("Bar"); mIcon:"qrc:/bar.svg"; mValue: 1 }
        }
     */

    textRole: "mLabel"
    valueRole: "mValue"
    wheelEnabled: true
    currentIndex: 0

    property string currentIcon
    property string currentTextIcon
    property int currValue
    property int delegateHeight: UISettings.listItemHeight
    property bool isUpdating: false
    property int contentsMaxWidth: 0

    signal valueChanged(int value)

    onCurrValueChanged: updateFromValue()
    onCurrentIndexChanged: updateFromIndex()

    function updateFromIndex()
    {
        if (!model || isUpdating)
            return

        isUpdating = true
        var item = model.length === undefined ? model.get(currentIndex) : model[currentIndex]
        displayText = item.mLabel ? item.mLabel : item
        //console.log("Index changed: " + currentIndex + ", label: " + displayText)
        if (item.mIcon)
            currentIcon = item.mIcon
        if (item.mTextIcon)
            currentTextIcon = item.mTextIcon

        if (item.mValue !== undefined)
            control.valueChanged(item.mValue)
        isUpdating = false
    }

    function updateFromValue()
    {
        if (!model || isUpdating)
            return

        isUpdating = true
        var iCount = model.length === undefined ? model.count : model.length
        //console.log("Value changed:" + currValue + ", model count: " + iCount)

        for (var i = 0; i < iCount; i++)
        {
            var item = model.length === undefined ? model.get(i) : model[i]
            if (item.mValue === currValue)
            {
                displayText = item.mLabel
                if (item.mIcon)
                    currentIcon = item.mIcon
                if (item.mTextIcon)
                    currentTextIcon = item.mTextIcon
                currentIndex = i
                isUpdating = false
                return
            }
        }
        isUpdating = false
    }

    Rectangle
    {
        anchors.fill: parent
        z: 3
        color: "black"
        opacity: 0.4
        visible: !parent.enabled
    }

    delegate:
        ItemDelegate
        {
            width: parent.width
            implicitHeight: delegateHeight
            highlighted: control.highlightedIndex === index
            hoverEnabled: control.hoverEnabled
            padding: 0
            leftPadding: 3

            property int currentIdx: control.currentIndex
            text: control.textRole ? (Array.isArray(control.model) ? modelData[control.textRole] : model[control.textRole]) : modelData
            property string itemIcon: model.mIcon ? model.mIcon : (typeof modelData !== 'undefined' ? modelData.mIcon ? modelData.mIcon : "" : "")
            property int itemValue: (model.mValue !== undefined) ? model.mValue : ((modelData.mValue !== undefined) ? modelData.mValue : index)

            contentItem:
                Row
                {
                    spacing: 2
                    leftPadding: 3

                    Image
                    {
                        visible: itemIcon ? true : false
                        height: delegateHeight - 4
                        width: height
                        y: 2
                        source: itemIcon
                        sourceSize: Qt.size(width, height)
                    }

                    RobotoText
                    {
                        label: text
                        height: delegateHeight
                        fontSize: UISettings.textSizeDefault

                        onWidthChanged:
                        {
                            var w = width + (itemIcon ? delegateHeight : 0) + 15
                            if (w > contentsMaxWidth)
                                contentsMaxWidth = w
                        }
                    }
                }

            background:
                Rectangle
                {
                    width: contentItem.width
                    height: delegateHeight
                    visible: control.down || control.highlighted || control.visualFocus
                    color: highlighted ? UISettings.highlight : (hovered ? UISettings.bgControl : "transparent")
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
            leftPadding: 6
            clip: true

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
            implicitHeight: delegateHeight
            color: control.hovered ? UISettings.bgLight : UISettings.bgControl
            border.width: 1
            border.color: UISettings.bgStrong
            radius: 3
        }

    popup:
        Popup
        {
            y: control.height
            width: Math.min(UISettings.bigItemHeight * 3, Math.max(control.width, contentsMaxWidth))
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

                    ScrollBar.vertical: CustomScrollBar { }
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
