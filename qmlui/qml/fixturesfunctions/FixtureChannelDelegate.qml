/*
  Q Light Controller Plus
  FixtureChannelDelegate.qml

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
import QtQuick.Layouts 1.0
import QtQuick.Controls 2.14

import org.qlcplus.classes 1.0
import "."

Rectangle
{
    id: chDelegate
    width: 100
    height: UISettings.listItemHeight

    color: "transparent"

    property string textLabel
    property string itemIcon: ""
    property int itemType: App.ChannelDragItem
    property bool isSelected: false
    property bool isCheckable: false
    property bool isChecked: false
    property bool showFlags: false
    property int itemID
    property int chIndex
    property int itemFlags
    property bool canFade: true
    property alias precedence: precedenceCombo.currValue
    property string modifier
    property Item dragItem

    signal mouseEvent(int type, int iID, int iType, var qItem, int mouseMods)

    Rectangle
    {
        anchors.fill: parent
        radius: 3
        color: UISettings.highlight
        visible: isSelected
    }

    RowLayout
    {
        height: UISettings.listItemHeight
        width: chDelegate.width

        CustomCheckBox
        {
            id: chCheckBox
            visible: isCheckable
            implicitWidth: UISettings.listItemHeight
            implicitHeight: implicitWidth
            checked: isChecked
            onCheckedChanged: chDelegate.mouseEvent(App.Checked, chIndex, checked, chDelegate, 0)
        }

        IconTextEntry
        {
            height: parent.height
            Layout.fillWidth: true
            tLabel: "" + (chIndex + 1) + ": " + chDelegate.textLabel
            iSrc: chDelegate.itemIcon

            MouseArea
            {
                anchors.fill: parent

                onPressed: chDelegate.mouseEvent(App.Pressed, chIndex, -1, chDelegate, mouse.modifiers)
                onClicked: chDelegate.mouseEvent(App.Clicked, chIndex, -1, chDelegate, mouse.modifiers)
                onDoubleClicked: chDelegate.mouseEvent(App.DoubleClicked, chIndex, -1, chDelegate, -1)
            }
        }

        // divider
        Rectangle
        {
            visible: showFlags
            width: 1
            height: parent.height
        }

        // flags stub
        Rectangle
        {
            visible: showFlags
            width: UISettings.chPropsFlagsWidth
            height: parent.height
            color: "transparent"
        }

        // divider
        Rectangle
        {
            visible: showFlags
            width: 1
            height: parent.height
        }

        // can fade
        Rectangle
        {
            visible: showFlags
            width: UISettings.chPropsCanFadeWidth
            height: parent.height
            color: "transparent"

            CustomCheckBox
            {
                anchors.centerIn: parent
                implicitHeight: UISettings.listItemHeight
                implicitWidth: implicitHeight
                checked: canFade
                onToggled: fixtureManager.setItemRoleData(itemID, chIndex, "canFade", checked)
            }
        }

        // divider
        Rectangle
        {
            visible: showFlags
            width: 1
            height: parent.height
        }

        // precedence combo
        CustomComboBox
        {
            id: precedenceCombo
            visible: showFlags
            implicitWidth: UISettings.chPropsPrecedenceWidth
            height: parent.height - 2

            ListModel
            {
                id: precModel
                ListElement { mLabel: qsTr("Auto (HTP)"); mValue: FixtureManager.AutoHTP }
                ListElement { mLabel: qsTr("Auto (LTP)"); mValue: FixtureManager.AutoLTP }
                ListElement { mLabel: qsTr("Forced HTP"); mValue: FixtureManager.ForcedHTP }
                ListElement { mLabel: qsTr("Forced LTP"); mValue: FixtureManager.ForcedLTP }
            }
            model: precModel
            onValueChanged: fixtureManager.setItemRoleData(itemID, chIndex, "precedence", value)
        }

        // divider
        Rectangle
        {
            visible: showFlags
            width: 1
            height: parent.height
        }

        // modifier
        Button
        {
            id: cmBtn
            visible: showFlags
            implicitWidth: UISettings.chPropsModifierWidth
            implicitHeight: parent.height
            padding: 0
            text: modifier === "" ? "..." : modifier
            hoverEnabled: true

            ToolTip.delay: 1000
            ToolTip.timeout: 5000
            ToolTip.visible: hovered
            ToolTip.text: text

            onClicked: fixtureManager.showModifierEditor(itemID, chIndex)

            contentItem:
                Text
                {
                    text: cmBtn.text
                    color: UISettings.fgMain
                    font.family: UISettings.robotoFontName
                    font.pixelSize: UISettings.textSizeDefault
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    elide: Text.ElideRight
                }

            background:
                Rectangle
                {
                    implicitWidth: cmBtn.implicitWidth
                    height: parent.height
                    border.width: 2
                    border.color: UISettings.bgStrong
                    color: cmBtn.hovered ? (cmBtn.down ? UISettings.highlightPressed : UISettings.highlight) : UISettings.bgControl
                }
        }
    }

    Rectangle
    {
        width: parent.width
        height: 1
        y: parent.height - 1
        color: UISettings.bgLight
    }
}
