/*
  Q Light Controller Plus
  FixtureGroupsBar.qml

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
import QtQuick.Controls

import org.qlcplus.classes 1.0
import "."

/**
  * A horizontal bar displaying a button for every Fixture Group of the project.
  * Clicking on a button selects all the fixtures of the represented group.
  * It is meant to be anchored at the bottom of the 2D/3D views.
  */
Rectangle
{
    id: barRoot
    height: UISettings.bigItemHeight * 0.55
    color: UISettings.bgMedium
    border.width: 1
    border.color: UISettings.bgStrong
    clip: true

    /** The ID of the group currently highlighted in this bar, or -1 if none */
    property int currentGroupID: -1

    /** Flag raised while this bar is changing the fixture selection itself,
      * to tell our own selection changes apart from the external ones */
    property bool isSelecting: false

    /** Drop the highlight as soon as the fixture selection is changed
      * by other means (single fixture click, rubber band, empty space click) */
    Connections
    {
        target: contextManager

        function onSelectedFixturesChanged()
        {
            if (barRoot.isSelecting === false)
                barRoot.currentGroupID = -1
        }
    }

    // don't let mouse events fall through to the view behind, otherwise
    // clicking on the bar empty space would reset the fixture selection
    // or zoom the view in/out
    MouseArea
    {
        anchors.fill: parent
        acceptedButtons: Qt.AllButtons
        onWheel: (wheel) => { wheel.accepted = true }
    }

    ListView
    {
        id: groupsListView
        anchors.fill: parent
        anchors.margins: 1
        orientation: ListView.Horizontal
        boundsBehavior: Flickable.StopAtBounds
        spacing: 2
        clip: true

        model: fixtureGroupEditor ? fixtureGroupEditor.groupsListModel : null

        // a group might have been deleted or renamed. Start over
        onModelChanged: barRoot.currentGroupID = -1

        delegate:
            Rectangle
            {
                id: groupButton
                width: UISettings.bigItemHeight
                height: groupsListView.height
                color: barRoot.currentGroupID === modelData.mValue ? UISettings.highlight :
                       (gMouseArea.containsMouse ? UISettings.bgLighter : UISettings.bgControl)
                border.width: 1
                border.color: UISettings.bgStrong

                // the group icon, as a watermark on the left of the cell background
                Image
                {
                    anchors.left: parent.left
                    anchors.leftMargin: 2
                    anchors.verticalCenter: parent.verticalCenter
                    height: parent.height * 0.75
                    width: height
                    source: modelData.mIcon
                    sourceSize: Qt.size(width, height)
                    opacity: 0.30
                }
/*
                // the number of fixtures, on the right of the cell background
                RobotoText
                {
                    anchors.right: parent.right
                    anchors.rightMargin: 4
                    anchors.verticalCenter: parent.verticalCenter
                    label: modelData.mCount
                    fontSize: UISettings.textSizeDefault * 0.85
                    labelColor: UISettings.fgLight
                    fontBold: true
                }
*/
                // the group name, spanning the whole cell
                RobotoText
                {
                    anchors.fill: parent
                    anchors.margins: 3
                    label: modelData.mLabel
                    fontSize: UISettings.textSizeDefault * 0.8
                    fontBold: true
                    //labelColor: UISettings.bgStrong
                    wrapText: true
                    textHAlign: Text.AlignHCenter
                }

                MouseArea
                {
                    id: gMouseArea
                    anchors.fill: parent
                    hoverEnabled: true

                    onClicked:
                    {
                        // clicking the highlighted group again deselects it
                        var deselect = barRoot.currentGroupID === modelData.mValue

                        barRoot.isSelecting = true
                        contextManager.resetFixtureSelection()

                        if (deselect === false)
                            contextManager.setFixtureGroupSelection(modelData.mValue, true, false)

                        barRoot.isSelecting = false
                        barRoot.currentGroupID = deselect ? -1 : modelData.mValue
                    }
                }
            }

        ScrollBar.horizontal: CustomScrollBar { orientation: Qt.Horizontal }
    }

    RobotoText
    {
        anchors.centerIn: parent
        visible: groupsListView.count === 0
        label: qsTr("No fixture group available")
        labelColor: UISettings.fgMedium
    }
}
