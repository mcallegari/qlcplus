/*
  Q Light Controller Plus
  PatchWireBox.qml

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

Canvas
{
    id: wireBox
    width: 50
    height: 120
    contextType: "2d"

    property int patchesNumber: 0
    property bool showFeedback: false

    onPatchesNumberChanged: requestPaint()
    onShowFeedbackChanged: requestPaint()

    onPaint:
    {
        var nodeSize = 8
        var halfNode = nodeSize / 2
        var vCenter = (height / 2) - halfNode
        context.strokeStyle = "yellow"
        context.fillStyle = "yellow"
        context.lineWidth = 3
        context.beginPath()
        context.clearRect(0, 0, width, height)

        var yDelta = height / patchesNumber
        var yPos = (yDelta / 2) - halfNode
        var targetX = width - nodeSize

        if (patchesNumber > 0)
        {
            context.ellipse(0, vCenter - halfNode, nodeSize, nodeSize)
            context.fill()

            context.beginPath()
            context.moveTo(context.lineWidth, vCenter)
            context.lineTo(targetX / 2, vCenter)
            context.stroke()

            for (var i = 0; i < patchesNumber; i++)
            {
                context.beginPath()
                context.ellipse(targetX, yPos - halfNode, nodeSize, nodeSize)
                context.fill()

                context.beginPath()
                context.moveTo(targetX / 2, vCenter)
                context.lineTo(targetX / 2, yPos)
                context.lineTo(targetX, yPos)
                context.stroke()

                yPos += yDelta
            }

            if (showFeedback)
            {
                yPos = vCenter + (UISettings.bigItemHeight * 0.4) - (UISettings.iconSizeMedium * 0.4)
                context.strokeStyle = "green"
                context.fillStyle = "green"

                context.ellipse(0, yPos - halfNode, nodeSize, nodeSize)
                context.fill()

                context.beginPath()
                context.moveTo(context.lineWidth, yPos)
                context.lineTo(targetX, yPos)
                context.stroke()

                context.beginPath()
                context.ellipse(targetX, yPos - halfNode, nodeSize, nodeSize)
                context.fill()
            }
        }
    }
}
