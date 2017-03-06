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
        yFactor = eHeight / eWidth;
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
        yFactor = eHeight / eWidth;
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
