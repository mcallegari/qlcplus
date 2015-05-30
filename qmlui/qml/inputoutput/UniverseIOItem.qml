/*
  Q Light Controller Plus
  UniverseIOItem.qml

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

import com.qlcplus.classes 1.0

Rectangle {
    id: uniItem
    width: parent.width
    height: 120
    color: isSelected ? "#2D444E" : "transparent"
    border.width: 2
    border.color: isSelected ? "yellow" : "transparent"

    property Universe universe
    property bool isSelected: false
    property int outputPatchesNumber: 0
    property int inputPatchesNumber: 0

    onUniverseChanged: {
        if (universe != null)
        {
            if (universe.inputPatch !== null)
                inputPatchesNumber = 1
            else
                inputPatchesNumber = 0

            if (universe.outputPatch !== null)
                outputPatchesNumber = 1
            else
                outputPatchesNumber = 0
        }
    }

    signal selected(int index)

    // area containing the input Patches
    Column {
        id: inputBox
        width: uniBox.x
    }

    // representation of the central Universe block
    Rectangle {
        id: uniBox
        anchors.centerIn: parent
        width: 200
        height: 100
        radius: 5
        //color: "#1C2255"
        gradient: Gradient {
            id: bgGradient
            GradientStop { position: 0 ; color: "#1C2255" }
            GradientStop { position: 1 ; color: "#2B3483" }
        }
        border.width: 2
        border.color: "#111"

        RobotoText {
            height: parent.height
            width: parent.width
            label: universe.name
            wrapText: true
            textAlign: Text.AlignHCenter
        }
    }

    // Output patches wires box
    Canvas {
        id: wireBox
        x: uniBox.x + uniBox.width - 6
        width: (uniItem.width - uniBox.width) / 8 // one quarter of a uniItem side
        height: uniItem.height
        z: 10

        onPaint: {
            var ctx = wireBox.getContext('2d');
            var vCenter = wireBox.height / 2;
            var nodeSize = 8
            ctx.strokeStyle = "yellow";
            ctx.fillStyle = "yellow";
            ctx.lineWidth = 2;
            ctx.save();
            ctx.clearRect(0, 0, wireBox.width, wireBox.height);
            if (outputPatchesNumber > 0)
            {
                ctx.ellipse(ctx.lineWidth, vCenter, nodeSize, nodeSize);
                ctx.lineTo(wireBox.width - nodeSize - ctx.lineWidth, vCenter + nodeSize / 2);
                ctx.ellipse(wireBox.width - nodeSize - ctx.lineWidth, vCenter, nodeSize, nodeSize);
                ctx.fill();
                ctx.stroke();
            }
            ctx.restore();
        }
    }

    // area containing the output Patches
    Column {
        id: outputBox
        x: wireBox.x + wireBox.width - 6
        anchors.verticalCenter: uniItem.verticalCenter

        Repeater {
            model: outputPatchesNumber
            delegate: OutputPatchItem {
                width: wireBox.width * 3

                universeID: universe.id
                patch: universe.outputPatch
            }
        }
    }

    // Global mouse area to select this Universe item
    MouseArea {
        anchors.fill: parent
        onClicked: {
            if (isSelected == false)
            {
                isSelected = true
                ioManager.setSelectedItem(uniItem, universe.id)
                uniItem.selected(universe.id);
            }
        }
    }

    // items divider
    Rectangle {
        width: parent.width
        height: 2
        y: parent.height - 2
        color: isSelected ? "yellow" : "#666"
    }
}

