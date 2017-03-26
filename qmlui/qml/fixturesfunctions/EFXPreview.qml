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
    height: Math.min(width < minimumHeight ? minimumHeight : width, maximumHeight)

    color: "transparent"

    property int minimumHeight: 0
    property int maximumHeight: 0

    property real maxPanDegrees: 360.0
    property real maxTiltDegrees: 270.0
    property real halfTiltDegrees: maxTiltDegrees / 2

    property variant efxData
    property variant fixturesData
    property int animationInterval: 0

    property bool sphereView: false
    property int sphereRadius: 20
    property int sphereRings: 17
    property int sphereSlices: 16
    property int sphereRotationX: 0
    property int sphereRotationY: 0
    property bool isRotating: false

    onEfxDataChanged: patternLayer.requestPaint()

    Timer
    {
        interval: animationInterval
        repeat: true
        running: true
        onTriggered: headsLayer.requestPaint()
    }

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
            ctx.fillStyle = sphereView ? "black" : "white";
            ctx.lineWidth = 1;

            ctx.clearRect(0, 0, width, height);
            ctx.fillRect(0, 0, width, height);

            if (sphereView == false)
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
                    // clone original point to not mess it up with rotation
                    var p = new DrawFuncs.Vertex3D(p0);
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
                        // for top and bottom vertices.length = 1
                        var p = vertices[i % vertices.length];
                        strokeSegment(p, ctx, width, height);
                    }
                    //closeRenderingPortion(ctx, width, height); // don't close back!
                }
            }
        }

        RobotoText
        {
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.margins: 2
            height: fontSize
            visible: !sphereView
            label: "Pan (" + maxPanDegrees + "°)"
            labelColor: UISettings.fgMedium
            fontSize: UISettings.textSizeDefault * 0.6
        }

        RobotoText
        {
            anchors.left: parent.left
            anchors.bottom: parent.bottom
            anchors.margins: 2
            height: fontSize
            visible: !sphereView
            label: "Tilt (" + maxTiltDegrees + "°)"
            labelColor: UISettings.fgMedium
            fontSize: UISettings.textSizeDefault * 0.6
        }
    } // bgLayer Canvas

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
            var ctx = patternLayer.getContext('2d');
            ctx.globalAlpha = 1.0;
            ctx.fillStyle = "transparent";
            ctx.lineWidth = 1;

            ctx.clearRect(0, 0, width, height);
            ctx.fillRect(0, 0, width, height);

            var i;
            var x1, y1, x2, y2;

            if (sphereView == false)
            {
                var scaleFactor = height / 255;
                ctx.strokeStyle = "black";

                ctx.beginPath();

                for (i = 0; i < efxData.length - 2; i+=2)
                {
                    x1 = efxData[i] * scaleFactor;
                    y1 = efxData[i + 1] * scaleFactor;
                    x2 = efxData[i + 2] * scaleFactor;
                    y2 = efxData[i + 3] * scaleFactor;
                    ctx.moveTo(x1, y1);
                    ctx.lineTo(x2, y2);
                    //console.log("Drawing: " + x1 + "," + y1 + " to " + x2 + "," + y2)
                    //console.log("Drawing: " + efxData[i] + "," + efxData[i + 1] + " to " + efxData[i + 2] + "," + efxData[i + 3])
                }

                // stroke the last segment to close the path
                x1 = x2;
                y1 = y2;
                x2 = efxData[0] * scaleFactor;
                y2 = efxData[1] * scaleFactor;
                ctx.moveTo(x1, y1);
                ctx.lineTo(x2, y2);

                ctx.stroke();
                ctx.closePath();
            }
            else
            {
                /** The 3D projection of the EFX path needs some math to retrieve the x,y,z coordinates:
                  * 1- scale from 0 - 255 to 0 - 360 degrees
                  * 2- convert from degrees to radians
                  * 3- perform the spherical point calculation
                  */
                var rotation = new DrawFuncs.Vertex3D(DrawFuncs.degToRad(sphereRotationX), DrawFuncs.degToRad(sphereRotationY), 0);
                var distance = height * 2;
                var vertex1, vertex2;
                var xScaleFactor = maxPanDegrees / 255;
                var yScaleFactor = maxTiltDegrees / 255;

                ctx.lineWidth = 2;

                if (efxData.length < 4)
                    return;

                vertex1 = DrawFuncs.getSphereVertex(efxData[0] * xScaleFactor, efxData[1] * yScaleFactor, halfTiltDegrees, sphereRadius, rotation);
                x1 = DrawFuncs.projection(vertex1.x, vertex1.z, width/2.0, 100.0, distance);
                y1 = DrawFuncs.projection(vertex1.y, vertex1.z, height/2.0, 100.0, distance);

                for (i = 2; i < efxData.length; i+=2)
                {
                    vertex2 = DrawFuncs.getSphereVertex(efxData[i] * xScaleFactor, efxData[i + 1] * yScaleFactor, halfTiltDegrees, sphereRadius, rotation);
                    x2 = DrawFuncs.projection(vertex2.x, vertex2.z, width/2.0, 100.0, distance);
                    y2 = DrawFuncs.projection(vertex2.y, vertex2.z, height/2.0, 100.0, distance);

                    //console.log("Drawing: " + vertex1.x + "," + vertex1.y + "," + vertex1.z)

                    if (vertex2.z < 0)
                        ctx.strokeStyle = "#878700";
                    else
                        ctx.strokeStyle = "yellow";

                    ctx.beginPath();
                    ctx.moveTo(x1, y1);
                    ctx.lineTo(x2, y2);
                    ctx.stroke();
                    ctx.closePath();

                    vertex1 = vertex2;
                    x1 = x2;
                    y1 = y2;
                }

                // stroke the last segment to close the path
                vertex2 = DrawFuncs.getSphereVertex(efxData[0] * xScaleFactor, efxData[1] * yScaleFactor, halfTiltDegrees, sphereRadius, rotation);
                x2 = DrawFuncs.projection(vertex2.x, vertex2.z, width/2.0, 100.0, distance);
                y2 = DrawFuncs.projection(vertex2.y, vertex2.z, height/2.0, 100.0, distance);

                if (vertex2.z < 0)
                    ctx.strokeStyle = "#878700";
                else
                    ctx.strokeStyle = "yellow";

                ctx.beginPath();
                ctx.moveTo(x1, y1);
                ctx.lineTo(x2, y2);
                ctx.stroke();
                ctx.closePath();
            }
        }
    } // patternLayer Canvas

    Canvas
    {
        id: headsLayer
        width: height
        height: parent.height
        anchors.horizontalCenter: parent.horizontalCenter
        antialiasing: true
        x: 0
        y: 0
        z: 2

        onPaint:
        {
            var ctx = headsLayer.getContext('2d');
            ctx.globalAlpha = 1.0;
            ctx.fillStyle = "transparent";
            ctx.lineWidth = 1;
            ctx.strokeStyle = "black";

            ctx.clearRect(0, 0, width, height);
            ctx.fillRect(0, 0, width, height);

            var x, y;
            var headRadius = height / 20;
            var halfHeadRadius = headRadius / 2;
            var fontSize = headRadius * 0.8;
            var xScaleFactor = sphereView ? (maxPanDegrees / 255) : (height / 255);
            var yScaleFactor = sphereView ? (maxTiltDegrees / 255) : (height / 255);
            var distance = height * 2;
            var efxDataHalfLen = efxData.length / 2;
            var rotation = new DrawFuncs.Vertex3D(DrawFuncs.degToRad(sphereRotationX), DrawFuncs.degToRad(sphereRotationY), 0);
            var fxCountDown = fixturesData.length / 2;

            ctx.font = fontSize + "px \"" + UISettings.robotoFontName + "\"";
            ctx.textAlign = "center";

            for (var i = 0; i < fixturesData.length; i+=2)
            {
                var idx = fixturesData[i];
                var direction = fixturesData[i + 1];

                x = efxData[idx * 2] * xScaleFactor;
                y = efxData[(idx * 2) + 1] * yScaleFactor;

                ctx.fillStyle = "white";

                if (sphereView == true)
                {
                    var vertex = DrawFuncs.getSphereVertex(x, y, halfTiltDegrees, sphereRadius, rotation);

                    x = DrawFuncs.projection(vertex.x, vertex.z, width/2.0, 100.0, distance);
                    y = DrawFuncs.projection(vertex.y, vertex.z, height/2.0, 100.0, distance);

                    if (vertex.z < 0)
                        ctx.fillStyle = "gray";
                }

                ctx.beginPath();
                ctx.ellipse(x - halfHeadRadius, y - halfHeadRadius, headRadius, headRadius);
                ctx.fill();
                ctx.closePath();
                ctx.stroke();

                ctx.beginPath();
                ctx.fillStyle = "black";
                ctx.fillText(fxCountDown, x, y + halfHeadRadius / 2);
                ctx.fill();
                ctx.closePath();

                fixturesData[i] += direction;

                if (fixturesData[i] >= efxDataHalfLen && direction > 0)
                    fixturesData[i] = 0;
                if (fixturesData[i] < 0 && direction < 0)
                    fixturesData[i] = efxDataHalfLen - 1;

                fxCountDown--;
            }
        }
    } // headsLayer Canvas

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
