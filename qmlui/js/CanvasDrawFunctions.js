/*
  Q Light Controller Plus
  CanvasDrawFunctions.js

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

function drawEllipse (ctx, eX, eY, eWidth, eHeight)
{
    var step = 2*Math.PI/30;
    var r = eWidth / 2;
    var xFactor = 1.0;
    var yFactor = 1.0;
    if (eWidth > eHeight)
    {
        yFactor = eHeight / eWidth;
    }
    if (eHeight > eWidth)
    {
        xFactor = eWidth / eHeight;
        r = eHeight / 2;
    }

    ctx.beginPath();
    for(var theta = 0; theta < 2*Math.PI; theta+=step)
    {
       var x = eX + xFactor * r * Math.cos(theta);
       var y = eY - yFactor * r * Math.sin(theta);
       ctx.lineTo(x,y);
    }

    ctx.closePath(); //close the end to the start point
    ctx.stroke();
}

function degToRad(degrees)
{
    return degrees * (Math.PI / 180);
}

function drawCursor(ctx, eX, eY, eWidth, eHeight, degrees, radius)
{
    var r = eWidth / 2;
    var xFactor = 1.0;
    var yFactor = 1.0;
    if (eWidth > eHeight)
    {
        yFactor = eHeight / eWidth;
    }
    if (eHeight > eWidth)
    {
        xFactor = eWidth / eHeight;
        r = eHeight / 2;
    }

    var radPos = degToRad(degrees);
    var x = eX + xFactor * r * Math.cos(radPos);
    var y = eY + yFactor * r * Math.sin(radPos);

    ctx.beginPath();
    ctx.ellipse(x - (radius / 2), y - (radius / 2), radius, radius);
    ctx.fill();
    ctx.closePath();    //close the end to the start point
    ctx.stroke();
}

function drawBasement(ctx, eWidth, eHeight)
{
    ctx.fillStyle = "#222";
    ctx.strokeStyle = "#333";
    ctx.beginPath();
    var halfWidth = eWidth / 2;
    var thirdWidth = eWidth / 3;
    var baseHeight = eHeight / 7;
    ctx.moveTo(halfWidth - thirdWidth, eHeight);
    ctx.lineTo(halfWidth - (thirdWidth * 0.75), eHeight - baseHeight);
    ctx.lineTo(halfWidth + (thirdWidth * 0.75), eHeight - baseHeight);
    ctx.lineTo(halfWidth + thirdWidth, eHeight);
    ctx.lineTo(halfWidth - thirdWidth, eHeight);

    ctx.fill();
    ctx.closePath();    //close the end to the start point
    ctx.stroke();
}

function Vertex3D()
{
    this.x = 0;
    this.y = 0;
    this.z = 0;
}

function Sphere3D(radius, rings, slices)
{
    this.vertices = [];
    this.radius = radius;
    this.rings = rings;
    this.slices = slices;
    this.numberOfVertices = 0;

    var M_PI_2 = Math.PI / 2;
    var dTheta = (Math.PI * 2) / this.slices;
    var dPhi = Math.PI / this.rings;

    // Iterate over latitudes (rings)
    for (var lat = 0; lat < this.rings + 1; ++lat)
    {
        var phi = M_PI_2 - lat * dPhi;
        var cosPhi = Math.cos(phi);
        var sinPhi = Math.sin(phi);

        // Iterate over longitudes (slices)
        for (var lon = 0; lon < this.slices + 1; ++lon)
        {
            var theta = lon * dTheta;
            var cosTheta = Math.cos(theta);
            var sinTheta = Math.sin(theta);

            this.vertices[this.numberOfVertices] = new Vertex3D();
            var p = this.vertices[this.numberOfVertices];

            p.x = this.radius * cosTheta * cosPhi;
            p.y = this.radius * sinPhi;
            p.z = this.radius * sinTheta * cosPhi;
            this.numberOfVertices++;
        }
    }
}

function rotateX(point, radians)
{
    var y = point.y;
    point.y = (y * Math.cos(radians)) + (point.z * Math.sin(radians) * -1.0);
    point.z = (y * Math.sin(radians)) + (point.z * Math.cos(radians));
}

function rotateY(point, radians)
{
    var x = point.x;
    point.x = (x * Math.cos(radians)) + (point.z * Math.sin(radians) * -1.0);
    point.z = (x * Math.sin(radians)) + (point.z * Math.cos(radians));
}

function rotateZ(point, radians)
{
    var x = point.x;
    point.x = (x * Math.cos(radians)) + (point.y * Math.sin(radians) * -1.0);
    point.y = (x * Math.sin(radians)) + (point.y * Math.cos(radians));
}

function projection(xy, z, xyOffset, zOffset, distance)
{
    return ((distance * xy) / (z - zOffset)) + xyOffset;
}

function getSphereVertex(xDegrees, yDegrees, radius)
{
    var v = new Vertex3D();
    var theta = degToRad(xDegrees);
    var phi = (Math.PI / 2) - degToRad(yDegrees);
    v.x = radius * Math.cos(theta) * Math.cos(phi);
    v.y = radius * Math.sin(phi);
    v.z = radius * Math.sin(theta) * Math.cos(phi);
    return v;
}

