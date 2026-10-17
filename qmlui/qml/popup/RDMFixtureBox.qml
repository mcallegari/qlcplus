/*
  Q Light Controller Plus
  RDMFixtureBox.qml

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

import "."

/*
 * A RDM fixture, shown as a card with its information. It can be dragged
 * with the mouse: the dragged item is a shared proxy (dragController)
 * which is never destroyed, even if the models are rebuilt while dropping.
 * While hovering the card, an overlaid button switches on the fixture identify
 * function for as long as it is kept pressed.
 */
Item
{
    id: boxRoot
    width: UISettings.bigItemHeight * 2.1
    height: UISettings.bigItemHeight * 1.1

    property string fxUID: ""
    property string fxManufacturer: ""
    property string fxModel: ""
    property string fxPersonality: ""
    property string fxIP: ""
    property string fxDefMode: ""
    property int fxAddress: 0
    property int fxChannels: 0
    property int fxUniverse: 0
    property bool fxHasDef: false

    /** a reduced card, used in the layout grid cells */
    property bool compact: false

    /** true when the card is in the layout grid */
    property bool inGrid: false

    /** the drag proxy: begin(), moveTo() and drop() functions */
    property Item dragController: null

    readonly property color okColor: "#4CAF50"
    readonly property color warnColor: "#FFA000"
    readonly property real fontScale: compact ? 0.65 : 0.8

    Rectangle
    {
        anchors.fill: parent
        anchors.margins: 2
        radius: 4
        color: UISettings.bgControl
        border.width: 2
        border.color: fxHasDef ? okColor : UISettings.fgMedium

        Column
        {
            anchors.fill: parent
            anchors.margins: compact ? 3 : 4
            spacing: 1

            Text
            {
                width: parent.width - UISettings.textSizeDefault * 1.2
                elide: Text.ElideRight
                font.family: UISettings.robotoFontName
                font.pixelSize: UISettings.textSizeDefault * boxRoot.fontScale
                font.bold: true
                color: UISettings.fgMain
                text: compact ? fxModel : fxManufacturer + " " + fxModel
            }
            Text
            {
                visible: !compact
                width: parent.width
                elide: Text.ElideRight
                font.family: UISettings.robotoFontName
                font.pixelSize: UISettings.textSizeDefault * 0.7
                color: UISettings.fgLight
                text: "UID " + fxUID
            }
            Text
            {
                width: parent.width
                elide: Text.ElideRight
                font.family: UISettings.robotoFontName
                font.pixelSize: UISettings.textSizeDefault * boxRoot.fontScale
                color: UISettings.fgMain
                text: compact ? qsTr("U%1 - DMX %2").arg(fxUniverse + 1).arg(fxAddress) :
                                qsTr("Universe %1 - DMX %2 (%3 ch)").arg(fxUniverse + 1).arg(fxAddress).arg(fxChannels)
            }
            Text
            {
                visible: fxIP !== ""
                width: parent.width
                elide: Text.ElideRight
                font.family: UISettings.robotoFontName
                font.pixelSize: UISettings.textSizeDefault * boxRoot.fontScale
                color: UISettings.fgMain
                text: "IP " + fxIP
            }
            Text
            {
                visible: !compact
                width: parent.width
                elide: Text.ElideRight
                font.family: UISettings.robotoFontName
                font.pixelSize: UISettings.textSizeDefault * 0.7
                color: UISettings.fgLight
                text: fxPersonality
            }
        }

        // fixture definition library marker
        Text
        {
            anchors.top: parent.top
            anchors.right: parent.right
            anchors.margins: compact ? 3 : 4
            font.family: UISettings.fontAwesomeFontName
            font.pixelSize: UISettings.textSizeDefault * (compact ? 0.8 : 1.0)
            color: fxHasDef ? okColor : warnColor
            text: fxHasDef ? FontAwesome.fa_circle_check : FontAwesome.fa_circle_question
        }
    }

    // the hover state is tracked by a handler, which doesn't steal
    // the events from the card and from the identify button
    HoverHandler
    {
        id: cardHover
    }

    MouseArea
    {
        id: dragArea
        anchors.fill: parent

        // the card drag must not be taken over by the Flickable/GridView containing the card
        preventStealing: true

        property point pressPoint
        property bool dragging: false

        ToolTip.visible: cardHover.hovered && !dragging && !identifyArea.pressed
        ToolTip.delay: 600
        ToolTip.text: "UID " + fxUID + "\n" +
                      fxManufacturer + " " + fxModel + "\n" +
                      fxPersonality + "\n" +
                      (fxHasDef ? qsTr("Found in the fixtures library (%1)").arg(fxDefMode) :
                                  qsTr("Not found in the fixtures library: it will be added as a generic dimmer"))

        onPressed: (mouse) =>
        {
            pressPoint = Qt.point(mouse.x, mouse.y)
            dragging = false
        }

        onPositionChanged: (mouse) =>
        {
            if (!pressed || dragController === null)
                return

            if (!dragging && (Math.abs(mouse.x - pressPoint.x) > 10 || Math.abs(mouse.y - pressPoint.y) > 10))
            {
                dragging = true
                dragController.begin(fxUID, fxManufacturer + " " + fxModel,
                                     boxRoot.width, boxRoot.height, pressPoint.x, pressPoint.y, inGrid)
            }

            if (dragging)
            {
                var p = dragArea.mapToItem(dragController.parent, mouse.x, mouse.y)
                dragController.moveTo(p.x - pressPoint.x, p.y - pressPoint.y)
            }
        }

        onReleased:
        {
            if (dragging)
            {
                dragging = false
                dragController.drop()
            }
        }

        onCanceled:
        {
            if (dragging)
            {
                dragging = false
                dragController.drop()
            }
        }
    }

    // identify: on while pressed, off when released
    Rectangle
    {
        id: identifyButton
        visible: (cardHover.hovered || identifyArea.pressed) && !dragArea.dragging
        z: 2
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 4
        width: UISettings.textSizeDefault * (compact ? 1.6 : 2.0)
        height: width
        radius: width / 2
        color: identifyArea.pressed ? "#FFD600" : Qt.rgba(0, 0, 0, 0.65)
        border.width: 1
        border.color: UISettings.fgMain

        Text
        {
            anchors.centerIn: parent
            font.family: UISettings.fontAwesomeFontName
            font.pixelSize: parent.width * 0.55
            color: identifyArea.pressed ? "black" : UISettings.fgMain
            text: FontAwesome.fa_lightbulb
        }

        MouseArea
        {
            id: identifyArea
            anchors.fill: parent
            preventStealing: true

            onPressed: rdmManager.setIdentify(fxUID, true)
            onReleased: rdmManager.setIdentify(fxUID, false)
            onCanceled: rdmManager.setIdentify(fxUID, false)

            // don't leave a fixture identifying if the card goes away while pressed
            Component.onDestruction:
            {
                if (pressed)
                    rdmManager.setIdentify(fxUID, false)
            }
        }
    }
}
