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
    ctx.closePath();
    ctx.stroke();
}

function drawBasement(ctx, eWidth, eHeight)
{
    ctx.fillStyle = "#333";
    ctx.strokeStyle = "#444";
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

var EMPTY_VALUE = Number.MIN_VALUE;

function fillPointFromPoint(target, src)
{
    target.x = src.x;
    target.y = src.y;
    target.z = src.z;
}

function clearPoint(p)
{
    p.x = EMPTY_VALUE;
    p.y = EMPTY_VALUE;
    p.z = EMPTY_VALUE;
}

function Vertex3D(x, y, z)
{
    if (arguments.length === 3)
    {
        this.x = x;
        this.y = y;
        this.z = z;
    }
    else if (arguments.length === 1)
    {
        // 1 argument means point
        fillPointFromPoint(this, x);
    }
    else
    {
        // no arguments mean create empty
        clearPoint(this);
    }
}

function Sphere3D(radius, rings, slices)
{
    this.radius = radius;
    this.innerRingsCount = rings;
    this.slicesCount = slices;

    var M_PI_2 = Math.PI / 2;
    var dTheta = (Math.PI * 2) / this.slicesCount;
    var dPhi = Math.PI / this.innerRingsCount;

    this.rings = [];
    // always add both poles
    this.rings.push([new Vertex3D(0, this.radius, 0)]);

    // Iterate over latitudes (rings)
    for (var lat = 0; lat < this.innerRingsCount; ++lat)
    {
        var phi = M_PI_2 - lat * dPhi - dPhi / 2;
        var cosPhi = Math.cos(phi);
        var sinPhi = Math.sin(phi);
        //console.log("lat = " + lat + " phi = " + (phi / Math.PI) + " sinPhi = " + sinPhi);

        var vertices = [];
        // Iterate over longitudes (slices)
        for (var lon = 0; lon < this.slicesCount; ++lon)
        {
            var theta = lon * dTheta;
            var cosTheta = Math.cos(theta);
            var sinTheta = Math.sin(theta);
            var p = new Vertex3D();
            p.x = this.radius * cosTheta * cosPhi;
            p.y = this.radius * sinPhi;
            p.z = this.radius * sinTheta * cosPhi;
            vertices.push(p);
        }
        this.rings.push(vertices);
    }

    // always add both poles
    this.rings.push([new Vertex3D(0, -this.radius, 0)]);
}

function rotateX(vertex, radians)
{
    var y = vertex.y;
    vertex.y = (y * Math.cos(radians)) + (vertex.z * Math.sin(radians) * -1.0);
    vertex.z = (y * Math.sin(radians)) + (vertex.z * Math.cos(radians));
}

function rotateY(vertex, radians)
{
    var x = vertex.x;
    vertex.x = (x * Math.cos(radians)) + (vertex.z * Math.sin(radians) * -1.0);
    vertex.z = (x * Math.sin(radians)) + (vertex.z * Math.cos(radians));
}

function rotateZ(vertex, radians)
{
    var x = vertex.x;
    vertex.x = (x * Math.cos(radians)) + (vertex.y * Math.sin(radians) * -1.0);
    vertex.y = (x * Math.sin(radians)) + (vertex.y * Math.cos(radians));
}

function projection(xy, z, xyOffset, zOffset, distance)
{
    return ((distance * xy) / (z - zOffset)) + xyOffset;
}

function getSphereVertex(xDegrees, yDegrees, yDegOffset, radius, rotation)
{
    var v = new Vertex3D();
    var theta = degToRad(xDegrees);
    var phi = (Math.PI / 2) - degToRad(yDegrees - yDegOffset);
    v.x = radius * Math.cos(theta) * Math.cos(phi);
    v.y = radius * Math.sin(phi);
    v.z = radius * Math.sin(theta) * Math.cos(phi);

    rotateX(v, rotation.x);
    rotateY(v, rotation.y);
    rotateZ(v, rotation.z);

    return v;
}

