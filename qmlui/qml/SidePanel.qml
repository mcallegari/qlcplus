/*
  Q Light Controller Plus
  SidePanel.qml

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
    id: sidePanelRoot
    width: collapseWidth
    height: 500
    color: UISettings.bgStrong

    property int panelAlignment: Qt.AlignRight
    property bool isOpen: false
    property int collapseWidth: 50
    property int expandedWidth: 450
    property string loaderSource: ""
    property int iconSize: collapseWidth - 4

    property alias itemID: viewLoader.itemID

    signal contentLoaded(var item, int ID)

    function animatePanel(checked)
    {
        if (checked === isOpen)
            return

        if (isOpen == false)
        {
            animateOpen.start()
            isOpen = true
        }
        else
        {
            animateClose.start()
            isOpen = false
            loaderSource = ""
        }
    }

    Loader
    {
        id: viewLoader
        x: sidePanelRoot.panelAlignment === Qt.AlignRight ? collapseWidth : 0
        width: sidePanelRoot.width - collapseWidth
        height: parent.height
        source: loaderSource

        // this is a generic ID used by the Loader
        // content to target an object to edit/view
        property int itemID: -1

        onLoaded: contentLoaded(item, itemID)

        Connections
        {
            ignoreUnknownSignals: true
            target: viewLoader.item
            onRequestView:
            {
                console.log("SidePanel loader ID requested: " + ID)
                itemID = ID
                loaderSource = qmlSrc
            }
        }
    }


    NumberAnimation
    {
        id: animateOpen
        target: sidePanelRoot
        properties: "width"
        to: expandedWidth
        duration: 200
        onStopped: sidePanelRoot.width = expandedWidth
    }

    NumberAnimation
    {
        id: animateClose
        target: sidePanelRoot
        properties: "width"
        to: collapseWidth
        duration: 200
        onStopped: sidePanelRoot.width = collapseWidth
    }

    Rectangle
    {
        id: gradientBorder
        y: sidePanelRoot.panelAlignment == Qt.AlignRight ? 0 : width
        x: sidePanelRoot.panelAlignment == Qt.AlignRight ? height : parent.width - height
        height: collapseWidth
        color: "#141414"
        width: parent.height
        transformOrigin: Item.TopLeft
        rotation: sidePanelRoot.panelAlignment == Qt.AlignRight ? 90 : 270
        gradient: Gradient
        {
            GradientStop { position: 0; color: "#141414" }
            GradientStop { position: 0.21; color: UISettings.bgStrong }
            GradientStop { position: 0.79; color: UISettings.bgStrong }
            GradientStop { position: 1; color: "#141414" }
        }

        MouseArea
        {
            anchors.fill: parent
            z: 1
            x: parent.width - width
            hoverEnabled: true
            cursorShape: pressed ? Qt.ClosedHandCursor : Qt.OpenHandCursor
            drag.target: sidePanelRoot
            drag.axis: Drag.XAxis
            drag.minimumX: collapseWidth

            onPositionChanged:
            {
                if (drag.active == true)
                {
                    if (sidePanelRoot.panelAlignment == Qt.AlignRight)
                    {
                        sidePanelRoot.width = sidePanelRoot.parent.width - sidePanelRoot.x
                    }
                    else
                    {
                        var obj = mapToItem(null, mouseX, mouseY)
                        sidePanelRoot.width = obj.x + (collapseWidth / 2)
                    }
                }
            }
            //onClicked: animatePanel("")
        }
    }
}
