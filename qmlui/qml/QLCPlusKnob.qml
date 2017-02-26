/*
  Q Light Controller Plus
  QLCPlusKnob.qml

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
import QtQuick.Controls 2.0

import "CanvasDrawFunctions.js" as DrawFuncs
import "."

Dial
{
    id: control
    implicitHeight: height
    implicitWidth: width
    height: 50
    width: height

    from: 0
    to: 255

    onPositionChanged: kCanvas.requestPaint()
    onHeightChanged: kCanvas.requestPaint()

    background: Canvas {
        id: kCanvas
        width: control.width
        height: control.height
        antialiasing: true

        onPaint:
        {
            var startAngle = 90 + 40
            var arcWidth = 4
            var ctx = kCanvas.getContext('2d');
            ctx.globalAlpha = 1.0

            ctx.clearRect(0, 0, width, height)

            var outerGrad = ctx.createLinearGradient(0, 0, 0, height)
            var innerGrad = ctx.createLinearGradient(0, 0, 0, height)
            outerGrad.addColorStop(0, '#777')
            outerGrad.addColorStop(1, '#888')
            innerGrad.addColorStop(0, '#888')
            innerGrad.addColorStop(1, '#777')

            ctx.beginPath()
            ctx.fillStyle = outerGrad
            ctx.arc(width / 2, height / 2, (width * 0.5) - arcWidth, 0, 2 * Math.PI)
            ctx.fill()
            ctx.closePath()

            ctx.beginPath()
            ctx.fillStyle = innerGrad
            ctx.arc(width / 2, height / 2, (width * 0.35), 0, 2 * Math.PI)
            ctx.fill()
            ctx.closePath()

            ctx.beginPath()
            ctx.strokeStyle = "#00FF00"
            ctx.lineWidth = arcWidth
            ctx.arc(width / 2, height / 2, (width / 2) - (arcWidth / 2), DrawFuncs.degToRad(startAngle),
                    DrawFuncs.degToRad(startAngle + (control.position * 280)))
            ctx.stroke()
            ctx.closePath()
        }
    }

    handle: Rectangle {
        id: handleItem
        x: background.x + background.width / 2 - handle.width / 2
        y: background.y + background.height / 2 - handle.height / 2
        width: control.width / 7
        height: width
        color: control.pressed ? "#17a81a" : "#21be2b"
        border.width: 1
        border.color: UISettings.bgStrong
        radius: height / 2
        antialiasing: true
        opacity: control.enabled ? 1 : 0.3
        transform: [
            Translate {
                y: -Math.min(control.background.width, control.background.height) * 0.35 + handleItem.height / 2
            },
            Rotation {
                angle: control.angle
                origin.x: handleItem.width / 2
                origin.y: handleItem.height / 2
            }
        ]
    }
}
