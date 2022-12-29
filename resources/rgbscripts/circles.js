/*
  Q Light Controller Plus
  circles.js

  Copyright (c) Massimo Callegari
  with Additions by Branson Matheson 

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

// Development tool access
var testAlgo;

(
  function()
  {
    var algo = new Object;
    algo.apiVersion = 2;
    algo.name = "Circles";
    algo.author = "Massimo Callegari+Branson Matheson";
    algo.acceptColors = 2;
    algo.properties = new Array();
    algo.circlesAmount = 3;
    algo.properties.push("name:circlesAmount|type:range|display:Amount|values:1,10|write:setAmount|read:getAmount");
    algo.circlesSize = 0;
    algo.properties.push("name:circlesSize|type:range|display:Diameter(0=max(h,v))|values:0,32|write:setSize|read:getSize");
    algo.fadeMode = 0;
    algo.properties.push("name:fadeMode|type:list|display:Fade Mode|values:Don't Fade,Fade In,Fade Out,Pulse|write:setFade|read:getFade");
    algo.fillCircles = 0;
    algo.properties.push("name:fillCircles|type:list|display:Fill circles|values:No,Yes|write:setFill|read:getFill");

    var util = new Object;
    util.pixelMap = new Array();
    util.initialized = false;
    util.circlesMaxSize = 0;

    var circles = new Array();

    function Circle(x, y, step, rgb)
    {
      this.xCenter = x;
      this.yCenter = y;
      this.step = step;
      this.rgb = rgb;
    }

    algo.setAmount = function(_amount)
    {
      algo.circlesAmount = _amount;
      util.initialized = false;
    };

    algo.getAmount = function()
    {
      return algo.circlesAmount;
    };

    algo.setSize = function(_size)
    {
      algo.circlesSize = _size;
      util.initialized = false;
    };

    algo.getSize = function()
    {
      return algo.circlesSize;
    };

    algo.setFade = function(_fade)
    {
      if (_fade === "Fade In") { algo.fadeMode = 1; }
      else if (_fade === "Fade Out") { algo.fadeMode = 2; }
      else if (_fade === "Pulse") { algo.fadeMode = 3; }
      else { algo.fadeMode = 0; }
    };

    algo.getFade = function()
    {
      if (algo.fadeMode === 1) { return "Fade In"; }
      else if (algo.fadeMode === 2) { return "Fade Out"; }
      else if (algo.fadeMode === 3) { return "Pulse"; }
      else { return "Don't Fade"; }
    };

    algo.setFill = function (_fill) {
      if (_fill === "Yes") {
        algo.fillCircles = 1;
      } else {
        algo.fillCircles = 0;
      }
    };

    algo.getFill = function () {
      if (algo.fillCircles === 1) {
        return "Yes";
      } else {
        return "No";
      }
    };

    util.initialize = function(size, rgb)
    {
      if (size > 0) {
        util.circlesMaxSize = size;
      }

      circles = new Array();
      for (var i = 0; i < algo.circlesAmount; i++) {
        circles[i] = new Circle(-1, -1, 0, rgb);
      }

      util.initialized = true;
    };

    util.getStepColor = function(step, rgb)
    {
      if (algo.fadeMode === 0)
      {
        return rgb;
      }
      else
      {
        var r = (rgb >> 16) & 0x00FF;
        var g = (rgb >> 8) & 0x00FF;
        var b = rgb & 0x00FF;

        var stepCount = Math.floor(util.circlesMaxSize / 2);
        var fadeStep = step;
        if ( algo.fadeMode === 2 ) {
          fadeStep = stepCount - step;
        } else if (algo.fadeMode == 3 && step >= stepCount/2) {
          fadeStep = stepCount - step + (stepCount/2) -1;
        } else if (algo.fadeMode == 3 && step < stepCount/2) {
          fadeStep = step + (stepCount/2) -1;
        }
        var newR = Math.round((r / stepCount) * fadeStep);
        var newG = Math.round((g / stepCount) * fadeStep);
        var newB = Math.round((b / stepCount) * fadeStep);
        var newRGB = (newR << 16) + (newG << 8) + newB;
        //console.log("mode" + fadeMode + " rgb: " + r + g + b + " nrgb: " + newR+newG+newB +" StepCount: " + stepCount + " step:" + step + " fadeStep:" + fadeStep )
        return newRGB;
      }
    };

    // from https://www.redblobgames.com/grids/circle-drawing/ 
    util.drawCircle = function(cx, cy, color, width, height, r) {
      ctop = Math.max(0, cy - r)
      cbottom = Math.min(height, cy + r )
      cleft = Math.max(0, cx - r)
      cright = Math.min(width, cx + r )
      for (py = ctop; py <= cbottom; py++) {
        for (px = cleft; px <= cright; px++ ) {
          dx = cx - px;
          dy = cy - py;
          distance_squared = (dx * dx) + (dy * dy);
          if ( distance_squared <= r*r-1 ) {  // -1 here so edges are clean
            util.drawPixel(px, py, color, width, height);
          }
        }
      }
    }

    util.drawPixel = function(cx, cy, color, width, height)
    {
      //cx = cx.toFixed(0);
      //cy = cy.toFixed(0);
      cx = Math.round(cx);
      cy = Math.round(cy);
      if (cx >= 0 && cx < width && cy >= 0 && cy < height) {
        util.pixelMap[cy][cx] = color;
      }
    };

    util.getNextStep = function(width, height, rgb)
    {
      var x = 0;
      var y = 0;
      // create an empty, black pixelMap
      util.pixelMap = new Array(height);
      for (y = 0; y < height; y++)
      {
        util.pixelMap[y] = new Array(width);
        for (x = 0; x < width; x++) {
          util.pixelMap[y][x] = 0;
        }
      }

      for (var i = 0; i < algo.circlesAmount; i++)
      {
        if (circles[i].xCenter === -1)
        {
          circles[i].rgb = rgb;
        }
        var color = util.getStepColor(circles[i].step, circles[i].rgb);
        //alert("Circle " + i + " xCenter: " + circles[i].xCenter + " color: " + color.toString(16));
        if (circles[i].xCenter === -1)
        {
          var seed = Math.floor(Math.random()*100);
          if (seed > 50) { continue; }
          circles[i].xCenter = Math.floor(Math.random() * width);
          circles[i].yCenter = Math.floor(Math.random() * height);
          util.pixelMap[circles[i].yCenter][circles[i].xCenter] = color;
        }
        else
        {
          var l = circles[i].step * Math.cos(Math.PI / 4);
          var radius2 = circles[i].step * circles[i].step;
          l = l.toFixed(0);

          if ( algo.fillCircles == 0 ) {
            for (x = 0; x <= l; x++)
            {
              y = Math.sqrt(radius2 - (x * x));

              util.drawPixel(circles[i].xCenter + x, circles[i].yCenter + y, color, width, height);
              util.drawPixel(circles[i].xCenter + x, circles[i].yCenter - y, color, width, height);
              util.drawPixel(circles[i].xCenter - x, circles[i].yCenter + y, color, width, height);
              util.drawPixel(circles[i].xCenter - x, circles[i].yCenter - y, color, width, height);

              util.drawPixel(circles[i].xCenter + y, circles[i].yCenter + x, color, width, height);
              util.drawPixel(circles[i].xCenter + y, circles[i].yCenter - x, color, width, height);
              util.drawPixel(circles[i].xCenter - y, circles[i].yCenter + x, color, width, height);
              util.drawPixel(circles[i].xCenter - y, circles[i].yCenter - x, color, width, height);
            }
          } else {
            util.drawCircle(circles[i].xCenter, circles[i].yCenter, color, width, height, circles[i].step)
          }
        } 

        circles[i].step++;
        if (circles[i].step >= (util.circlesMaxSize / 2))
        {
          circles[i].xCenter = -1;
          circles[i].yCenter = -1;
          circles[i].step = 0;
        }
      }

      return util.pixelMap;
    };

    algo.rgbMap = function(width, height, rgb, step)
    {
      if (util.initialized === false)
      {
        if ( algo.circlesSize > 0 ) {
          util.initialize(algo.circlesSize, rgb);
        } else if (height < width) {
          util.initialize(height, rgb);
        } else {
          util.initialize(width, rgb);
        }
      }

      return util.getNextStep(width, height, rgb);
    };

    algo.rgbMapStepCount = function(width, height)
    {
      return 2;
    };

    // Development tool access
    testAlgo = algo;

    return algo;
    }
)();
