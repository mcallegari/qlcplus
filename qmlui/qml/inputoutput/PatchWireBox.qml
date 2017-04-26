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

Canvas
{
    id: wireBox
    width: 50
    height: 120
    contextType: "2d"

    property int patchesNumber: 0

    onPatchesNumberChanged: requestPaint()

    onPaint:
    {
        var nodeSize = 8
        var vCenter = (height / 2) - (nodeSize / 2)
        context.strokeStyle = "yellow"
        context.fillStyle = "yellow"
        context.lineWidth = 2
        context.beginPath()
        context.clearRect(0, 0, width, height)

        var yDelta = height / patchesNumber

        if (patchesNumber > 0)
        {
            var yPos = (yDelta / 2) - (nodeSize / 2)
            var targetX = width - nodeSize - context.lineWidth

            context.ellipse(context.lineWidth, vCenter - (nodeSize / 2), nodeSize, nodeSize)
            context.fill()

            context.beginPath()
            context.moveTo(context.lineWidth, vCenter)
            context.lineTo(targetX / 2, vCenter)
            context.stroke()

            for (var i = 0; i < patchesNumber; i++)
            {
                context.beginPath()
                context.ellipse(targetX, yPos - (nodeSize / 2), nodeSize, nodeSize)
                context.fill()

                context.beginPath()
                context.moveTo(targetX / 2, vCenter)
                context.lineTo(targetX / 2, yPos)
                context.lineTo(targetX, yPos)
                context.stroke()

                yPos += yDelta
            }
        }
    }
}
