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

import QtQuick
import QtQuick.Layouts
import QtQuick.Window
import QtQuick.Controls.Basic
import "."

ComboBox
{
    id: control

    /*! model: provides a data model for the popup.
        A model can be either a string list (QStringList) or a named model
        to provide icons and values (QVariant)
        In case of a QStringList, textRole should be set to ""
        A QML model with icons should look like this:
        [
            { mLabel: qsTr("Foo"), mIcon: "qrc:/foo.svg", mValue: 0 },
            { mLabel: qsTr("Bar"), mIcon: "qrc:/bar.svg", mValue: 1 },
            { mLabel: qsTr("Joe"), faIcon: FontAwesome.fa_egg, mValue: 2 }
        ]
     */

    textRole: "mLabel"
    valueRole: "mValue"
    wheelEnabled: true
    currentIndex: 0

    property string currentIcon
    property string currentTextIcon
    property string currentFAIcon
    property var currValue: undefined

    property int delegateHeight: UISettings.listItemHeight
    property bool isUpdating: false
    property int contentsMaxWidth: 0

    signal valueChanged(int value)

    function initSelection()
    {
        if (!model)
            return
        if (typeof currValue !== 'undefined' && currValue !== null)
            updateFromValue()
        else
            updateFromIndex()
    }

    Component.onCompleted: initSelection()
    onModelChanged: initSelection()
    onCurrValueChanged: updateFromValue()
    onCurrentIndexChanged: updateFromIndex()

    Connections {
        target: (model && typeof model.count !== "undefined") ? model : null
        function onCountChanged() {
            initSelection()
        }
    }

    function updateFromIndex()
    {
        if (!model || isUpdating)
            return

        isUpdating = true
        var item = model.length === undefined ? model.get(currentIndex) : model[currentIndex]
        displayText = item && item.mLabel ? item.mLabel : (item !== undefined ? item : "")
        if (item && item.mIcon)
            currentIcon = item.mIcon
        if (item && item.mTextIcon)
            currentTextIcon = item.mTextIcon
        if (item && item.faIcon)
            currentFAIcon = item.faIcon

        var valueToEmit = (item && item.mValue !== undefined) ? item.mValue : currentIndex
        if (valueToEmit !== currValue)
        {
            currValue = valueToEmit
            control.valueChanged(valueToEmit)
        }
        isUpdating = false
    }


    function updateFromValue()
    {
        if (!model || isUpdating)
            return

        isUpdating = true
        var iCount = model.length === undefined ? model.count : model.length
        for (var i = 0; i < iCount; i++) {
            var item = model.length === undefined ? model.get(i) : model[i]
            var matches = (item && item.mValue !== undefined) ? (item.mValue === currValue) : (i === currValue)
            if (matches) {
                displayText = item && item.mLabel ? item.mLabel : (item !== undefined ? item : "")
                if (item && item.mIcon)
                    currentIcon = item.mIcon
                if (item && item.faIcon)
                    currentFAIcon = item.faIcon
                if (item && item.mTextIcon)
                    currentTextIcon = item.mTextIcon
                if (currentIndex !== i)
                    currentIndex = i
                break
            }
        }
        isUpdating = false
        updateFromIndex()
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
            width: ListView.view.width
            implicitHeight: delegateHeight
            highlighted: control.highlightedIndex === index
            hoverEnabled: control.hoverEnabled
            padding: 0
            leftPadding: 3

            required property var model
            required property var modelData
            required property int index

            property int currentIdx: control.currentIndex
            text: control.textRole ? (Array.isArray(control.model) ? modelData[control.textRole] : model[control.textRole]) : modelData
            property string itemIcon: model.mIcon ? model.mIcon : (typeof modelData !== 'undefined' ? modelData.mIcon ? modelData.mIcon : "" : "")
            property string itemFAIcon: model.faIcon ? model.faIcon : (typeof modelData !== 'undefined' ? (modelData.faIcon ? modelData.faIcon : "") : "")
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

                    Text
                    {
                        visible: itemFAIcon ? true : false
                        height: delegateHeight
                        width: height
                        verticalAlignment: Text.AlignVCenter
                        font.family: UISettings.fontAwesomeFontName
                        font.pixelSize: delegateHeight * 0.7
                        color: UISettings.fgMain
                        text: itemFAIcon
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
                currentFAIcon = itemFAIcon

                if (itemValue !== undefined)
                    control.valueChanged(itemValue)
            }

            Rectangle { height: 1; width: parent.width; y: parent.height - 1 }
        }

    indicator:
        Text
        {
            x: control.mirrored ? control.padding : control.width - width - control.padding - 5
            y: control.topPadding + (control.availableHeight - height) / 2
            color: UISettings.fgLight
            opacity: enabled ? 1 : 0.3
            font.family: UISettings.fontAwesomeFontName
            font.pixelSize: UISettings.iconSizeMedium * 0.45
            text: FontAwesome.fa_chevron_down
        }

    contentItem:
        Rectangle
        {
            id: cRect
            height: control.height
            //width: tField.width + ((iconImg.visible || iconFa.visible) ? iconImg.width + 5 : 0)
            color: "transparent"

            Image
            {
                id: iconImg
                visible: currentIcon ? true : false
                x: 3
                y: 2
                height: control.height - 4
                width: height
                source: currentIcon ? currentIcon : ""
                sourceSize: Qt.size(width, height)
                opacity: control.enabled ? 1 : 0.3
            }

            Text
            {
                id: iconFa
                visible: currentFAIcon ? true : false
                x: 3
                height: control.height - 4
                width: height
                verticalAlignment: Text.AlignVCenter
                font.family: UISettings.fontAwesomeFontName
                font.pixelSize: control.height * 0.7
                color: UISettings.fgMain
                opacity: control.enabled ? 1 : 0.3
                text: currentFAIcon
            }

            TextField
            {
                id: tField
                x: (iconImg.visible || iconFa.visible) ? iconImg.width + 5 : 0
                height: control.height
                width: cRect.width //control ? (control.width - control.indicator.width - parent.leftPadding) : 100
                enabled: control.editable
                opacity: control.enabled ? 1 : 0.3
                font.family: UISettings.robotoFontName
                font.pixelSize: UISettings.textSizeDefault
                text: control.editable ? control.editText : control.displayText
                color: UISettings.fgMain
                selectedTextColor: UISettings.fgMain
                selectionColor: UISettings.highlightPressed
                selectByMouse: true

                background: Rectangle {
                    visible: control.enabled && control.editable && !control.flat
                    border.width: parent && parent.activeFocus ? 2 : 1
                    border.color: parent && parent.activeFocus ? UISettings.highlight : "transparent"
                    color: "transparent"
                }
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
