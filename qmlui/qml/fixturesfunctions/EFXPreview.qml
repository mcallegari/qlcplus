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
    property int sphereRings: 17
    property int sphereSlices: 16
    property int sphereRotationX: 0
    property int sphereRotationY: 0
    property bool isRotating: false

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
            var ctx = bgLayer.getContext('2d');
            ctx.globalAlpha = 1.0;
            ctx.fillStyle = efxBox.sphereView ? "black" : "white";
            ctx.lineWidth = 1;

            ctx.clearRect(0, 0, width, height);
            ctx.fillRect(0, 0, width, height);

            if (efxBox.sphereView == false)
            {
                ctx.strokeStyle = UISettings.bgLight;
                ctx.beginPath();
                ctx.moveTo(width / 2, 0);
                ctx.lineTo(width / 2, height);
                ctx.stroke();
                ctx.closePath();

                ctx.beginPath();
                ctx.moveTo(0, height / 2);
                ctx.lineTo(width, height / 2);
                ctx.stroke();
                ctx.closePath();
            }
            else
            {
                var sphere = new DrawFuncs.Sphere3D(sphereRadius, sphereRings, sphereSlices);
                var rotation = new DrawFuncs.Vertex3D(DrawFuncs.degToRad(sphereRotationX), DrawFuncs.degToRad(sphereRotationY), 0);
                var distance = height * 2;
                var i, j, vertices;

                var lastP = new DrawFuncs.Vertex3D();
                var firstP = new DrawFuncs.Vertex3D();

                function startRenderingPortion()
                {
                    DrawFuncs.clearPoint(lastP);
                    DrawFuncs.clearPoint(firstP);
                }

                function closeRenderingPortion(ctx, width, height)
                {
                    strokeSegmentImpl(ctx, firstP.x, firstP.y, firstP.z, width, height);
                    DrawFuncs.clearPoint(lastP);
                    DrawFuncs.clearPoint(firstP);
                }

                function strokeSegmentImpl(ctx, x, y, z, width, height)
                {
                    if (x < 0 || x >= width || y < 0 || y >= height)
                        return;

                    var eps = 0.01;
                    if ((z < -eps && lastP.z < eps) || (z < eps && lastP.z < -eps))
                    {
                        ctx.strokeStyle = "gray";
                    }
                    else
                    {
                        ctx.strokeStyle = "white";
                    }

                    ctx.beginPath();
                    if (x === lastP.x && y === lastP.y)
                    {
                        // draw single point
                        ctx.moveTo(x, y);
                        ctx.lineTo(x + 1, y + 1);
                    }
                    else
                    {
                        ctx.moveTo(lastP.x, lastP.y);
                        ctx.lineTo(x, y);
                    }
                    ctx.stroke();
                    ctx.closePath();

                    lastP.x = x;
                    lastP.y = y;
                    lastP.z = z;
                }

                function strokeSegment(p0, ctx, width, height)
                {
                    var p = new DrawFuncs.Vertex3D(p0); // clone original point to not mess it up with rotation!
                    DrawFuncs.rotateX(p, rotation.x);
                    DrawFuncs.rotateY(p, rotation.y);
                    DrawFuncs.rotateZ(p, rotation.z);

                    var x, y;
                    x = DrawFuncs.projection(p.x, p.z, width / 2.0, 100.0, distance);
                    y = DrawFuncs.projection(p.y, p.z, height / 2.0, 100.0, distance);

                    if (lastP.x === DrawFuncs.EMPTY_VALUE && lastP.y === DrawFuncs.EMPTY_VALUE)
                    {
                        lastP = new DrawFuncs.Vertex3D(x, y, p.z);
                        DrawFuncs.fillPointFromPoint(firstP, lastP);
                        return;
                    }
                    strokeSegmentImpl(ctx, x, y, p.z, width, height);
                }

                // draw each vertex to get the first sphere skeleton
                for (i = 0; i < sphere.rings.length; i++)
                {
                    startRenderingPortion();
                    vertices = sphere.rings[i];
                    for (j = 0; j < vertices.length; j++)
                    {
                        strokeSegment(vertices[j], ctx, width, height);
                    }
                    closeRenderingPortion(ctx, width, height);
                }

                // now walk through rings to draw the slices
                for (i = 0; i < sphere.slicesCount; i++)
                {
                    startRenderingPortion();
                    for (j = 0; j < sphere.rings.length; j++)
                    {
                        vertices = sphere.rings[j];
                        var p = vertices[i % vertices.length];// for top and bottom vertices.length = 1
                        strokeSegment(p, ctx, width, height);
                    }
                    //closeRenderingPortion(ctx, width, height); // don't close back!
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

                var rotation = new DrawFuncs.Vertex3D(DrawFuncs.degToRad(sphereRotationX), DrawFuncs.degToRad(sphereRotationY), 0)
                var distance = height * 2
                var vertex1, vertex2
                scaleFactor = 360 / 255 // to convert 0 - 255 to 0 - 360
                ctx.lineWidth = 2

                if (efxData.length < 4)
                    return

                vertex1 = DrawFuncs.getSphereVertex(efxData[0] * scaleFactor, efxData[1] * scaleFactor, sphereRadius)
                DrawFuncs.rotateX(vertex1, rotation.x);
                DrawFuncs.rotateY(vertex1, rotation.y);
                DrawFuncs.rotateZ(vertex1, rotation.z);

                x1 = DrawFuncs.projection(vertex1.x, vertex1.z, width/2.0, 100.0, distance)
                y1 = DrawFuncs.projection(vertex1.y, vertex1.z, height/2.0, 100.0, distance)

                for (i = 2; i < efxData.length - 2; i+=2)
                {
                    vertex2 = DrawFuncs.getSphereVertex(efxData[i + 2] * scaleFactor, efxData[i + 3] * scaleFactor, sphereRadius)

                    DrawFuncs.rotateX(vertex2, rotation.x);
                    DrawFuncs.rotateY(vertex2, rotation.y);
                    DrawFuncs.rotateZ(vertex2, rotation.z);
                    //console.log("Drawing: " + vertex1.x + "," + vertex1.y + "," + vertex1.z)

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

                    vertex1 = vertex2
                    x1 = x2
                    y1 = y2
                }
            }
        }
    }

    MouseArea
    {
        anchors.fill: parent

        property int lastXPos
        property int lastYPos

        onPressed:
        {
            if (sphereView == false)
                return

            // initialize local variables to determine the selection orientation
            lastXPos = mouse.x
            lastYPos = mouse.y
        }
        onPositionChanged:
        {
            if (sphereView == false)
                return

            isRotating = true

            if (Math.abs(mouse.x - lastXPos) > Math.abs(mouse.y - lastYPos))
            {
                if (mouse.x < lastXPos)
                    sphereRotationY--
                else
                    sphereRotationY++
            }
            else
            {
                if (mouse.y < lastYPos)
                    sphereRotationX--
                else
                    sphereRotationX++
            }
            lastXPos = mouse.x
            lastYPos = mouse.y
            bgLayer.requestPaint()
            patternLayer.requestPaint()
        }
        onReleased:
        {
            if (isRotating == true)
            {
                isRotating = false
                return
            }

            sphereView = !sphereView
            bgLayer.requestPaint()
            patternLayer.requestPaint()
        }
    }
}
