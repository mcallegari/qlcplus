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

import "."

Rectangle
{
    id: ipbRoot
    width: UISettings.iconSizeDefault
    height: UISettings.iconSizeDefault
    color: "transparent"

    /*! iconFromPopup: if true, the button icon will be the one of the currently selected item.
                       if false, the icon will never change, and will be $iconSource */
    property bool iconFromPopup: true

    /*! iconSource: the icon displayed on this button. It is set automatically when selecting
                    a popup entry if $iconFromPopup is true, otherwise it can be specified
                    by the parent item */
    property string iconSource: ""

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
    property alias model: menuListView.model

    property int popupPosition: Qt.AlignBottom

    /*! currentIndex: the index of the currently selected entry of the popup */
    property alias currentIndex: menuListView.currentIndex

    /*! currentValue: the value of the currently selected entry of the popup */
    property int currentValue

    signal valueChanged(var value)

    //onModelChanged: menuListView.currentIndex = 0

    onVisibleChanged:
    {
        if (visible == false)
            dropDownMenu.visible = false
    }

    function positionMenu()
    {
        var posnInWindow = ipbRoot.mapToItem(mainView, 0, 0);
        var totalHeight = menuListView.count * UISettings.listItemHeight
        //console.log("Total height: " + totalHeight)
        if (posnInWindow.y + ipbRoot.height + totalHeight > mainView.height)
          dropDownMenu.y = posnInWindow.y - totalHeight
        else
          dropDownMenu.y = posnInWindow.y + ipbRoot.height
        dropDownMenu.x = posnInWindow.x
        dropDownMenu.height = totalHeight
    }

    IconButton
    {
        id: buttonBox
        anchors.fill: parent
        imgSource: iconSource

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
            positionMenu()
            dropDownMenu.visible = !dropDownMenu.visible
        }
    }

    Rectangle
    {
        id: dropDownMenu
        y: ipbRoot.height
        width: UISettings.bigItemHeight * 2
        color: UISettings.bgMedium
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
                    height: UISettings.listItemHeight
                    color: "transparent"

                    Component.onCompleted:
                    {
                        // check for corresponding index
                        if (index == menuListView.currentIndex &&
                            iconFromPopup == true)
                        {
                            if (model.mIcon)
                                ipbRoot.iconSource = mIcon

                            if (model.mTextIcon)
                                textIcon.label = mTextIcon

                            buttonBox.tooltip = mLabel
                        }
                        // check for corresponding value
                        if (currentValue && mValue === currentValue)
                        {
                            if (model.mIcon)
                                ipbRoot.iconSource = mIcon

                            if (model.mTextIcon)
                                textIcon.label = mTextIcon

                            menuListView.currentIndex = index
                            buttonBox.tooltip = mLabel
                        }
                    }

                    Row
                    {
                        x: 2
                        spacing: 2

                        Image
                        {
                            id: btnIcon
                            visible: model.mIcon ? true : false
                            height: delegateRoot.height - 4
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
                            height: delegateRoot.height - 4
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
                            id: textitem
                            x: 3
                            label: mLabel
                            height: parent.height
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
                            if (iconFromPopup == true)
                            {
                                if (model.mIcon)
                                    ipbRoot.iconSource = mIcon
                                if (model.mTextIcon)
                                    textIcon.label = mTextIcon
                                buttonBox.tooltip = mLabel
                            }
                            menuListView.currentIndex = index
                            dropDownMenu.visible = false
                            ipbRoot.valueChanged(mValue)
                        }
                    }
                }
        }
    }
}
