/*
  Q Light Controller Plus
  EFXPreview.qml

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

import "CanvasDrawFunctions.js" as DrawFuncs
import "."

Rectangle
{
    id: efxBox
    width: 400
    height: Math.min(width, maximumHeight)

    color: "transparent"

    property variant efxData
    property bool sphereView: false
    property int maximumHeight: 0

    property int sphereRadius: 20
    property int sphereRings: 16
    property int sphereSlices: 16

    onEfxDataChanged: patternLayer.requestPaint()

    /** The EFX preview can be switched between 'legacy' 2D plain and 'fake' 3D sphere view
      * Therefore there are 3 Canvas layers here, to avoid wasting an awful amount of CPU:
      * 1- background: in 2D it is a white rectangle, in 3D it's a wireframe sphere
      * 2- EFX path: the plain or projected path of the selected EFX pattern
      * 3- Heads: numbered circles moving across the EFX path
      */
    Canvas
    {
        id: bgLayer
        width: height
        height: parent.height
        anchors.horizontalCenter: parent.horizontalCenter
        antialiasing: true
        x: 0
        y: 0
        z: 0

        onPaint:
        {
            var ctx = bgLayer.getContext('2d')
            ctx.globalAlpha = 1.0
            ctx.fillStyle = efxBox.sphereView ? "black" : "white"
            ctx.lineWidth = 1

            ctx.clearRect(0, 0, width, height)
            ctx.fillRect(0, 0, width, height)

            if (efxBox.sphereView == false)
            {
                ctx.strokeStyle = UISettings.bgLight
                ctx.beginPath()
                ctx.moveTo(width / 2, 0)
                ctx.lineTo(width / 2, height)
                ctx.stroke()
                ctx.closePath()

                ctx.beginPath()
                ctx.moveTo(0, height / 2)
                ctx.lineTo(width, height / 2)
                ctx.stroke()
                ctx.closePath()
            }
            else
            {
                var sphere = new DrawFuncs.Sphere3D(sphereRadius, sphereRings, sphereSlices)
                var rotation = new DrawFuncs.Vertex3D()
                var distance = height * 2
                var lastX = -1
                var lastY = -1
                var i, x, y
                var vertex

                //console.log("Number of vertices: " + sphere.numberOfVertices)

                // draw each vertex to get the first sphere skeleton
                for(i = 0; i < sphere.numberOfVertices; i++)
                {
                    vertex = sphere.vertices[i]
                    DrawFuncs.rotateX(vertex, rotation.x)
                    DrawFuncs.rotateY(vertex, rotation.y)
                    DrawFuncs.rotateZ(vertex, rotation.z)

                    x = DrawFuncs.projection(vertex.x, vertex.z, width/2.0, 100.0, distance)
                    y = DrawFuncs.projection(vertex.y, vertex.z, height/2.0, 100.0, distance)

                    if (lastX == -1 && lastY == -1)
                    {
                        lastX = x
                        lastY = y
                        continue
                    }

                    if(x >= 0 && x < width && y >= 0 && y < height)
                    {
                        if(vertex.z < 0) {
                            ctx.strokeStyle = "gray"
                        } else {
                            ctx.strokeStyle = "white"
                        }
                        ctx.beginPath()
                        ctx.moveTo(lastX, lastY)
                        ctx.lineTo(x,y)
                        ctx.stroke()
                        ctx.closePath()
                    }

                    lastX = x
                    lastY = y
                }

                // now walk through rings to draw the slices
                for (i = 0; i < sphere.slices + 1; i++)
                {
                    for (var j = 0; j < sphere.rings + 1; j++)
                    {
                        vertex = sphere.vertices[i + (j * (sphere.slices + 1))]
                        DrawFuncs.rotateX(vertex, rotation.x)
                        DrawFuncs.rotateY(vertex, rotation.y)
                        DrawFuncs.rotateZ(vertex, rotation.z)

                        x = DrawFuncs.projection(vertex.x, vertex.z, width/2.0, 100.0, distance)
                        y = DrawFuncs.projection(vertex.y, vertex.z, height/2.0, 100.0, distance)

                        if (lastX == -1 && lastY == -1)
                        {
                            lastX = x
                            lastY = y
                            continue
                        }

                        if(x >= 0 && x < width && y >= 0 && y < height)
                        {
                            if(vertex.z < 0) {
                                ctx.strokeStyle = "gray"
                            } else {
                                ctx.strokeStyle = "white"
                            }
                            ctx.beginPath()
                            ctx.moveTo(lastX, lastY)
                            ctx.lineTo(x,y)
                            ctx.stroke()
                            ctx.closePath()
                        }

                        lastX = x
                        lastY = y
                    }
                }
            }
        }
    }

    Canvas
    {
        id: patternLayer
        width: height
        height: parent.height
        anchors.horizontalCenter: parent.horizontalCenter
        antialiasing: true
        x: 0
        y: 0
        z: 1

        onPaint:
        {
            var ctx = patternLayer.getContext('2d')
            ctx.globalAlpha = 1.0
            ctx.fillStyle = "transparent"
            ctx.lineWidth = 1

            ctx.clearRect(0, 0, width, height)
            ctx.fillRect(0, 0, width, height)

            var i;
            var x1, y1, x2, y2
            var scaleFactor

            if (efxBox.sphereView == false)
            {
                ctx.strokeStyle = "black"

                var halfWidth = width / 2
                var halfHeight = height / 2

                scaleFactor = height / 255

                ctx.beginPath()

                for (i = 0; i < efxData.length - 2; i+=2)
                {
                    x1 = efxData[i] * scaleFactor
                    y1 = efxData[i + 1] * scaleFactor
                    x2 = efxData[i + 2] * scaleFactor
                    y2 = efxData[i + 3] * scaleFactor
                    ctx.moveTo(x1, y1)
                    ctx.lineTo(x2, y2)
                    //console.log("Drawing: " + x1 + "," + y1 + " to " + x2 + "," + y2)
                    //console.log("Drawing: " + efxData[i] + "," + efxData[i + 1] + " to " + efxData[i + 2] + "," + efxData[i + 3])
                }

                ctx.stroke()
                ctx.closePath()
            }
            else
            {
                /** The 3D projection of the EFX path needs some math to retrieve the x,y,z coordinates:
                  * 1- scale from 0 - 255 to 0 - 360 degrees
                  * 2- convert from degrees to radians
                  * 3- perform the spherical point calculation
                  */

                var rotation = new DrawFuncs.Vertex3D()
                var distance = height * 2
                var vertex1, vertex2
                scaleFactor = 360 / 255 // to convert 0 - 255 to 0 - 360
                ctx.lineWidth = 2
/*
                ctx.beginPath()
                ctx.fillStyle = "yellow"
                vertex1 = DrawFuncs.getSphereVertex(90, 90, sphereRadius)
                console.log("Drawing: " + vertex1.x + "," + vertex1.y + "," + vertex1.z)
                x1 = DrawFuncs.projection(vertex1.x, vertex1.z, width/2.0, 100.0, distance)
                y1 = DrawFuncs.projection(vertex1.y, vertex1.z, height/2.0, 100.0, distance)
                ctx.arc(x1, y1, 10, 0, 2 * Math.PI, false);
                ctx.fill()
*/

                for (i = 0; i < efxData.length - 2; i+=2)
                {
                    vertex1 = DrawFuncs.getSphereVertex(efxData[i] * scaleFactor, efxData[i + 1] * scaleFactor, sphereRadius)
                    vertex2 = DrawFuncs.getSphereVertex(efxData[i + 2] * scaleFactor, efxData[i + 3] * scaleFactor, sphereRadius)

                    //console.log("Drawing: " + vertex1.x + "," + vertex1.y + "," + vertex1.z)

                    x1 = DrawFuncs.projection(vertex1.x, vertex1.z, width/2.0, 100.0, distance)
                    y1 = DrawFuncs.projection(vertex1.y, vertex1.z, height/2.0, 100.0, distance)

                    x2 = DrawFuncs.projection(vertex2.x, vertex2.z, width/2.0, 100.0, distance)
                    y2 = DrawFuncs.projection(vertex2.y, vertex2.z, height/2.0, 100.0, distance)

                    if (vertex2.z < 0)
                        ctx.strokeStyle = "#878700"
                    else
                        ctx.strokeStyle = "yellow"

                    ctx.beginPath()
                    ctx.moveTo(x1, y1)
                    ctx.lineTo(x2, y2)
                    ctx.stroke()
                    ctx.closePath()
                }
            }
        }
    }

    MouseArea
    {
        anchors.fill: parent
        onClicked:
        {
            sphereView = !sphereView
            bgLayer.requestPaint()
            patternLayer.requestPaint()
        }
    }
}
