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

import QtQuick 2.0
import "."

Rectangle
{
    id: cbRoot
    implicitHeight: UISettings.listItemHeight
    width: 150
    color: cbMouseArea.containsMouse ? UISettings.bgLight : UISettings.bgMedium
    border.width: 1
    border.color: "#222"
    radius: 3

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
    property alias model: popupRepeater.model
    property alias currentIndex: popupRepeater.currentIndex
    property string currentText
    property string currentIcon
    property int currentValue

    signal valueChanged(int value)

    //onModelChanged: popupRepeater.currentIndex = 0

    onVisibleChanged:
    {
        if (visible == false && dropDownMenu)
            dropDownMenu.visible = false
    }

    function positionMenu()
    {
        var posnInWindow = cbRoot.mapToItem(mainView, 0, 0);
        var totalHeight = popupRepeater.count * popupRepeater.itemHeight
        console.log("Total height: " + totalHeight + ", mainview: " + mainView.height)
        dropDownMenu.height = Math.min(totalHeight, mainView.height - 50)
        if (posnInWindow.y + cbRoot.height + dropDownMenu.height > mainView.height)
          dropDownMenu.y = Math.max(posnInWindow.y - totalHeight, 25)
        else
          dropDownMenu.y = posnInWindow.y + cbRoot.height
        dropDownMenu.x = posnInWindow.x
    }

    Row
    {
        x: 4
        Image
        {
            id: mainIcon
            visible: currentIcon ? true : false
            height: cbRoot.height - 4
            width: height
            y: 2
            source: currentIcon ? currentIcon : ""
            sourceSize: Qt.size(width, height)
        }
        RobotoText
        {
            height: cbRoot.height
            width: cbRoot.width - 4 - arrowButton.width - (mainIcon.visible ? mainIcon.width : 0)
            label: currentText
            fontSize: UISettings.textSizeDefault
            fontBold: true
        }
        Rectangle
        {
            id: arrowButton
            width: UISettings.iconSizeMedium
            height: cbRoot.height
            color: "transparent" //"#404040"

            Image
            {
                anchors.centerIn: parent
                source: "qrc:/arrow-down.svg"
                sourceSize: Qt.size(UISettings.iconSizeMedium * 0.7, UISettings.iconSizeMedium * 0.35)
            }
        }
    }
    MouseArea
    {
        id: cbMouseArea
        anchors.fill: parent
        hoverEnabled: true

        onClicked:
        {
            positionMenu()
            dropDownMenu.visible = !dropDownMenu.visible
            dropDownMenu.forceActiveFocus()
        }
        onWheel:
        {
            var newIdx
            if (wheel.angleDelta.y > 0)
                newIdx = Math.max(0, popupRepeater.currentIndex - 1)
            else
                newIdx = Math.min(popupRepeater.currentIndex + 1, popupRepeater.count - 1)

            if (newIdx !== popupRepeater.currentIndex)
            {
                popupRepeater.currentIndex = newIdx
                console.log("Wheel event. Index: " + popupRepeater.currentIndex)
            }
        }
    }

    Rectangle
    {
        id: dropDownMenu
        y: cbRoot.height
        z: 99
        width: cbRoot.width
        color: UISettings.bgLight
        border.width: 1
        border.color: UISettings.bgLighter
        parent: mainView
        visible: false
        clip: true

        Keys.onEscapePressed: visible = false

        Flickable
        {
            id: popupFlickable
            width: parent.width
            height: parent.height
            boundsBehavior: Flickable.StopAtBounds
            contentHeight: popupRepeater.height
            contentWidth: parent.width

            Repeater
            {
                id: popupRepeater
                width: parent.width
                height: count * itemHeight

                property int itemHeight: UISettings.listItemHeight
                property int currentIndex: 0

                delegate:
                    Rectangle
                    {
                        id: delegateRoot
                        width: dropDownMenu.width
                        height: popupRepeater.itemHeight
                        y: index * height
                        color: "transparent"

                        property int currentIdx: popupRepeater.currentIndex
                        property int currentVal: cbRoot.currentValue
                        property string itemText: model.mLabel ? model.mLabel : (modelData.mLabel ? modelData.mLabel : modelData)
                        property string itemIcon: model.mIcon ? model.mIcon : (modelData.mIcon ? modelData.mIcon : "")
                        property int itemValue: (model.mValue !== undefined) ? model.mValue : ((modelData.mValue !== undefined) ? modelData.mValue : index)

                        onCurrentIdxChanged:
                        {
                            if (index == currentIdx)
                            {
                                currentText = itemText
                                currentIcon = itemIcon
                                if (itemValue !== undefined)
                                {
                                    cbRoot.currentValue = itemValue
                                    cbRoot.valueChanged(itemValue)
                                }
                            }
                        }

                        onCurrentValChanged:
                        {
                            if (itemValue == currentVal)
                            {
                                currentText = itemText
                                currentIcon = itemIcon
                            }
                        }

                        Component.onCompleted:
                        {
                            if (index === currentIdx)
                            {
                                currentText = itemText
                                currentIcon = itemIcon
                            }
                            if (currentValue && itemValue === currentValue)
                            {
                                currentText = itemText
                                currentIcon = itemIcon
                                popupRepeater.currentIndex = index
                            }
                        }

                        Row
                        {
                            x: 2
                            spacing: 2
                            Image
                            {
                                id: iconItem
                                visible: itemIcon ? true : false
                                height: delegateRoot.height - 4
                                width: height
                                y: 2
                                source: itemIcon
                                sourceSize: Qt.size(width, height)
                            }

                            RobotoText
                            {
                                id: textitem
                                label: itemText
                                height: delegateRoot.height
                                fontSize: UISettings.textSizeDefault
                            }
                        }

                        Rectangle { height: 1; width: parent.width; y: parent.height - 1 }

                        MouseArea
                        {
                            anchors.fill: parent
                            hoverEnabled: true
                            onEntered: delegateRoot.color = UISettings.highlight
                            onExited: delegateRoot.color = "transparent"
                            onClicked:
                            {
                                popupRepeater.currentIndex = index
                                currentText = itemText
                                currentIcon = itemIcon
                                dropDownMenu.visible = false

                                if (itemValue !== undefined)
                                    cbRoot.valueChanged(itemValue)
                            }
                        }
                    }
            } // Repeater
        } // Flickable
        CustomScrollBar { z: 2; flickable: popupFlickable }
    }
}
