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

    property int patchesNumber: 0

    onPatchesNumberChanged: requestPaint()

    onPaint:
    {
        var ctx = wireBox.getContext('2d')
        var nodeSize = 8
        var vCenter = (height / 2) - (nodeSize / 2)
        ctx.strokeStyle = "yellow"
        ctx.fillStyle = "yellow"
        ctx.lineWidth = 2
        ctx.beginPath()
        ctx.clearRect(0, 0, width, height)

        var yDelta = height / patchesNumber

        if (patchesNumber > 0)
        {
            var yPos = (yDelta / 2) - (nodeSize / 2)
            var targetX = width - nodeSize - ctx.lineWidth

            ctx.ellipse(ctx.lineWidth, vCenter - (nodeSize / 2), nodeSize, nodeSize)
            ctx.fill()

            ctx.beginPath()
            ctx.moveTo(ctx.lineWidth, vCenter)
            ctx.lineTo(targetX / 2, vCenter)
            ctx.stroke()

            for (var i = 0; i < patchesNumber; i++)
            {
                ctx.beginPath()
                ctx.ellipse(targetX, yPos - (nodeSize / 2), nodeSize, nodeSize)
                ctx.fill()

                ctx.beginPath()
                ctx.moveTo(targetX / 2, vCenter)
                ctx.lineTo(targetX / 2, yPos)
                ctx.lineTo(targetX, yPos)
                ctx.stroke()

                yPos += yDelta
            }
        }
    }
}
