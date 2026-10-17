/*
  Q Light Controller Plus
  PopupImportRDMFixtures.qml

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
import QtQuick.Controls

import org.qlcplus.classes 1.0
import "."

CustomPopupDialog
{
    id: popupRoot
    width: mainView.width * 0.8
    height: mainView.height * 0.85
    title: qsTr("Import RDM fixtures")
    standardButtons: Dialog.Cancel | Dialog.Ok

    onClicked: (role) =>
    {
        if (role === Dialog.Ok)
            rdmManager.applyLayout()

        close()
    }

    contentItem:
        Item
        {
            id: contentBox
            width: popupRoot.width
            height: popupRoot.height

            // detection in progress
            RobotoText
            {
                visible: rdmManager.discoveryStatus === RDMManager.Running
                anchors.horizontalCenter: parent.horizontalCenter
                label: qsTr("Detecting fixtures...")
            }
            Text
            {
                visible: rdmManager.discoveryStatus === RDMManager.Running
                anchors.centerIn: parent
                font.family: UISettings.fontAwesomeFontName
                font.pixelSize: UISettings.iconSizeDefault
                color: UISettings.fgMain
                text: FontAwesome.fa_spinner

                SequentialAnimation on rotation
                {
                    PropertyAnimation { to: 360; duration: 1500 }
                    running: true
                    loops: Animation.Infinite
                }
            }
            RobotoText
            {
                visible: rdmManager.discoveryStatus === RDMManager.Running
                anchors.horizontalCenter: parent.horizontalCenter
                y: parent.height * 0.75
                label: qsTr("Fixtures found" + ": " + rdmManager.fixturesFound)
            }

            // the item following the mouse while dragging a fixture
            Rectangle
            {
                id: dragGhost
                z: 100
                visible: false
                radius: 4
                opacity: 0.85
                color: UISettings.highlight
                border.width: 2
                border.color: UISettings.fgMain

                property string fxUID: ""
                property string fxLabel: ""
                property bool fromGrid: false

                Drag.keys: [ "rdmFixture" ]
                Drag.source: dragGhost

                Text
                {
                    anchors.fill: parent
                    anchors.margins: 4
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    wrapMode: Text.Wrap
                    elide: Text.ElideRight
                    font.family: UISettings.robotoFontName
                    font.pixelSize: UISettings.textSizeDefault * 0.8
                    font.bold: true
                    color: UISettings.fgMain
                    text: dragGhost.fxLabel
                }

                function begin(uid, label, w, h, hotX, hotY, inGrid)
                {
                    fxUID = uid
                    fxLabel = label
                    fromGrid = inGrid
                    width = w
                    height = h
                    Drag.hotSpot.x = hotX
                    Drag.hotSpot.y = hotY
                    visible = true
                    Drag.active = true
                }

                function moveTo(px, py)
                {
                    x = px
                    y = py
                }

                function drop()
                {
                    var action = Drag.drop()
                    Drag.active = false
                    visible = false

                    // a fixture dragged out of the grid and dropped nowhere goes back to the pool
                    if (action === Qt.IgnoreAction && fromGrid)
                        rdmManager.unplaceFixture(fxUID)
                }
            }

            ColumnLayout
            {
                visible: rdmManager.discoveryStatus === RDMManager.Finished
                anchors.fill: parent
                spacing: 4

                RobotoText
                {
                    Layout.fillWidth: true
                    height: UISettings.listItemHeight
                    fontBold: true
                    label: qsTr("Detected fixtures") + ": " + rdmManager.fixturesFound + "  -  " +
                           qsTr("drag them into the layout grid")
                }

                // fixtures not yet placed in the layout grid
                Rectangle
                {
                    id: fixturesArea
                    Layout.fillWidth: true
                    Layout.preferredHeight: contentBox.height * 0.28
                    color: UISettings.bgStrong
                    border.width: 1
                    border.color: fixturesDrop.containsDrag ? UISettings.activeDropArea : UISettings.bgLight
                    clip: true

                    GridView
                    {
                        id: fxGridView
                        anchors.fill: parent
                        anchors.margins: 2
                        boundsBehavior: Flickable.StopAtBounds
                        cellWidth: UISettings.bigItemHeight * 2.1
                        cellHeight: UISettings.bigItemHeight * 1.1
                        model: rdmManager.fixtureList

                        ScrollBar.vertical: ScrollBar { visible: fxGridView.contentHeight > fxGridView.height }

                        delegate:
                            RDMFixtureBox
                            {
                                dragController: dragGhost
                                fxUID: UID
                                fxManufacturer: manufacturer
                                fxModel: modelName
                                fxPersonality: personality
                                fxIP: ipAddress
                                fxDefMode: defMode
                                fxAddress: dmxAddress
                                fxChannels: dmxChannels
                                fxUniverse: universe
                                fxHasDef: hasDefinition
                            }
                    }

                    // dropping a fixture here takes it out of the layout grid
                    DropArea
                    {
                        id: fixturesDrop
                        anchors.fill: parent
                        keys: [ "rdmFixture" ]
                        onDropped: (drop) => rdmManager.unplaceFixture(dragGhost.fxUID)
                    }
                }

                // layout grid size and auto patching
                RowLayout
                {
                    Layout.fillWidth: true
                    height: UISettings.listItemHeight
                    spacing: 6

                    RobotoText
                    {
                        height: UISettings.listItemHeight
                        fontBold: true
                        label: qsTr("Layout grid") + ":"
                    }
                    CustomSpinBox
                    {
                        height: UISettings.listItemHeight
                        from: 1
                        to: 50
                        value: rdmManager.gridWidth
                        onValueModified: rdmManager.gridWidth = value
                    }
                    RobotoText
                    {
                        height: UISettings.listItemHeight
                        label: "x"
                    }
                    CustomSpinBox
                    {
                        height: UISettings.listItemHeight
                        from: 1
                        to: 50
                        value: rdmManager.gridHeight
                        onValueModified: rdmManager.gridHeight = value
                    }

                    Item { Layout.fillWidth: true; height: 1 }

                    RobotoText
                    {
                        height: UISettings.listItemHeight
                        label: qsTr("Start address") + ":"
                    }
                    CustomSpinBox
                    {
                        id: startAddrSpin
                        height: UISettings.listItemHeight
                        from: 1
                        to: 512
                        value: 1
                    }
                    RobotoText
                    {
                        height: UISettings.listItemHeight
                        label: qsTr("Gap") + ":"
                    }
                    CustomSpinBox
                    {
                        id: gapSpin
                        height: UISettings.listItemHeight
                        from: 0
                        to: 511
                        value: 0
                    }
                    GenericButton
                    {
                        implicitHeight: UISettings.listItemHeight
                        enabled: rdmManager.placedCount > 0
                        label: qsTr("Autopatch")
                        onClicked: rdmManager.autoPatch(startAddrSpin.value, gapSpin.value)
                    }
                }

                // the layout grid: fixtures order defines the addressing
                // and the position in the 2D view
                Rectangle
                {
                    id: layoutArea
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    color: UISettings.bgStrong
                    border.width: 1
                    border.color: UISettings.bgLight
                    clip: true

                    Flickable
                    {
                        id: layoutFlick
                        anchors.fill: parent
                        anchors.margins: 2
                        contentWidth: layoutGrid.width
                        contentHeight: layoutGrid.height
                        boundsBehavior: Flickable.StopAtBounds

                        ScrollBar.vertical: ScrollBar { visible: layoutFlick.contentHeight > layoutFlick.height }
                        ScrollBar.horizontal: ScrollBar { visible: layoutFlick.contentWidth > layoutFlick.width }

                        Grid
                        {
                            id: layoutGrid
                            columns: rdmManager.gridWidth

                            Repeater
                            {
                                model: rdmManager.gridList

                                delegate:
                                    Rectangle
                                    {
                                        width: UISettings.bigItemHeight * 1.5
                                        height: UISettings.bigItemHeight * 0.85
                                        color: cellDrop.containsDrag ? Qt.rgba(0.62, 1, 0.32, 0.25) : "transparent"
                                        border.width: 1
                                        border.color: cellDrop.containsDrag ? UISettings.activeDropArea : UISettings.bgLight

                                        Text
                                        {
                                            visible: !occupied
                                            anchors.centerIn: parent
                                            font.family: UISettings.robotoFontName
                                            font.pixelSize: UISettings.textSizeDefault
                                            color: UISettings.bgLight
                                            text: cellIndex + 1
                                        }

                                        Loader
                                        {
                                            anchors.fill: parent
                                            active: occupied
                                            sourceComponent:
                                                RDMFixtureBox
                                                {
                                                    width: parent.width
                                                    height: parent.height
                                                    compact: true
                                                    inGrid: true
                                                    dragController: dragGhost
                                                    fxUID: UID
                                                    fxManufacturer: manufacturer
                                                    fxModel: modelName
                                                    fxPersonality: personality
                                                    fxIP: ipAddress
                                                    fxDefMode: defMode
                                                    fxAddress: dmxAddress
                                                    fxChannels: dmxChannels
                                                    fxUniverse: universe
                                                    fxHasDef: hasDefinition
                                                }
                                        }

                                        DropArea
                                        {
                                            id: cellDrop
                                            anchors.fill: parent
                                            keys: [ "rdmFixture" ]
                                            onDropped: (drop) => rdmManager.placeFixture(dragGhost.fxUID, cellIndex)
                                        }
                                    }
                            }
                        }
                    }
                }
            }
        }
}
