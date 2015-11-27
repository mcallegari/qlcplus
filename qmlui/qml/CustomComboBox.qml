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
    implicitHeight: 30
    implicitWidth: 150
    color: UISettings.bgMedium
    border.width: 1
    border.color: "#222"
    radius: 3

    property alias currentIndex: menuListView.currentIndex
    property alias model: menuListView.model
    property string currentText

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
        console.log("Total height: " + totalHeight)
        if (posnInWindow.y + cbRoot.height + totalHeight > mainView.height)
          dropDownMenu.y = posnInWindow.y - totalHeight
        else
          dropDownMenu.y = posnInWindow.y + cbRoot.height
        dropDownMenu.x = posnInWindow.x
        dropDownMenu.height = totalHeight
    }

    Row
    {
        x: 2
        RobotoText
        {
            height: cbRoot.height
            width: cbRoot.width - 2 - arrowButton.width
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
    }

    Rectangle
    {
        id: dropDownMenu
        y: cbRoot.height
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

                    Component.onCompleted:
                    {
                        if (index == menuListView.currentIndex)
                            currentText = modelData
                    }

                    RobotoText
                    {
                        id: textitem
                        x: 3
                        label: modelData
                        height: parent.height
                        fontSize: 12
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
                            currentText = modelData
                            menuListView.currentIndex = index
                            dropDownMenu.visible = false
                        }
                    }
                }
        }
    }
}
