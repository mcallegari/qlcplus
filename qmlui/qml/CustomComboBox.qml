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
    height: 30
    width: 150
    color: UISettings.bgMedium
    border.width: 1
    border.color: "#222"
    radius: 3

    property alias currentIndex: menuListView.currentIndex

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
    property alias model: menuListView.model
    property string currentText
    property string currentIcon

    onModelChanged: menuListView.currentIndex = 0

    onVisibleChanged:
    {
        if (visible == false)
            dropDownMenu.visible = false
    }

    function positionMenu()
    {
        var posnInWindow = cbRoot.mapToItem(mainView, 0, 0);
        var totalHeight = menuListView.count * 35
        //console.log("Total height: " + totalHeight)
        if (posnInWindow.y + cbRoot.height + totalHeight > mainView.height)
          dropDownMenu.y = posnInWindow.y - totalHeight
        else
          dropDownMenu.y = posnInWindow.y + cbRoot.height
        dropDownMenu.x = posnInWindow.x
        dropDownMenu.height = totalHeight
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
            fontSize: 12
            fontBold: true
        }
        Rectangle
        {
            id: arrowButton
            width: 30
            height: cbRoot.height
            color: "transparent" //"#404040"

            Image
            {
                anchors.centerIn: parent
                source: "qrc:/arrow-down.svg"
                sourceSize: Qt.size(20, 12)
            }
        }
    }
    MouseArea
    {
        anchors.fill: parent
        onClicked:
        {
            positionMenu()
            dropDownMenu.visible = !dropDownMenu.visible
        }
        onWheel:
        {
            var newIdx
            if (wheel.angleDelta.y > 0)
                newIdx = Math.max(0, menuListView.currentIndex - 1)
            else
                newIdx = Math.min(menuListView.currentIndex + 1, menuListView.count - 1)

            if (newIdx !== menuListView.currentIndex)
            {
                menuListView.currentIndex = newIdx
                console.log("Wheel event. Index: " + menuListView.currentIndex)
            }
        }
    }

    Rectangle
    {
        id: dropDownMenu
        y: cbRoot.height
        z: 99
        width: cbRoot.width
        color: UISettings.bgStrong
        border.width: 1
        border.color: UISettings.bgLight
        parent: mainView
        visible: false

        ListView
        {
            id: menuListView
            anchors.fill: parent
            currentIndex: 0
            boundsBehavior: Flickable.StopAtBounds

            delegate:
                Rectangle
                {
                    id: delegateRoot
                    width: menuListView.width
                    height: 35
                    color: "transparent"

                    property int currentIdx: menuListView.currentIndex

                    onCurrentIdxChanged:
                    {
                        if (index == menuListView.currentIndex)
                        {
                            if (modelData.mLabel)
                                currentText = modelData.mLabel
                            else
                                currentText = modelData
                            if (mIcon)
                                currentIcon = modelData.mIcon
                        }
                    }

                    Component.onCompleted:
                    {
                        if (index == menuListView.currentIndex)
                        {
                            if (modelData.mLabel)
                                currentText = modelData.mLabel
                            else
                                currentText = modelData
                            if (modelData.mIcon)
                                currentIcon = modelData.mIcon
                        }
                    }

                    Row
                    {
                        x: 2
                        spacing: 2
                        Image
                        {
                            id: iconItem
                            visible: modelData.mIcon ? true : false
                            height: delegateRoot.height - 4
                            width: height
                            y: 2
                            source: modelData.mIcon ? modelData.mIcon : ""
                            sourceSize: Qt.size(width, height)
                        }

                        RobotoText
                        {
                            id: textitem
                            label: modelData.mLabel ? modelData.mLabel : modelData
                            height: delegateRoot.height
                            fontSize: 12
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
                            if (modelData.mLabel)
                                currentText = modelData.mLabel
                            else
                                currentText = modelData
                            if (modelData.mIcon)
                                currentIcon = modelData.mIcon

                            menuListView.currentIndex = index
                            dropDownMenu.visible = false
                        }
                    }
                }
        }
    }
}
