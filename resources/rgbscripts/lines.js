/*
  Q Light Controller Plus
  lines.js

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

// Development tool access
var testAlgo;

(
  function()
  {
    var algo = new Object;
    algo.apiVersion = 2;
    algo.name = "Lines";
    algo.author = "Massimo Callegari+Branson Matheson";
    algo.acceptColors = 2;
    algo.properties = new Array();
    algo.linesAmount = 3;
    algo.properties.push("name:linesAmount|type:range|display:Amount|values:1,200|write:setAmount|read:getAmount");
    algo.linesSize = 10;
    algo.properties.push("name:linesSize|type:range|display:Size|values:1,32|write:setSize|read:getSize");
    algo.linesVariablity = 0;
    algo.properties.push("name:linesVariablity|type:range|display:Size Variablity|values:0,100|write:setVariability|read:getVariability");
    algo.linesDirection = 0;
    algo.properties.push("name:linesDirection|type:list|display:Direction|values:Horizontal,Vertical,Plus,X,Star,Left,Right,Up,Down,UpRight,UpLeft,DownRight,DownLeft|write:setDirection|read:getDirection");
    algo.fadeMode = 0;
    algo.properties.push("name:fadeMode|type:list|display:Fade Mode|values:Don't Fade,Fade In,Fade Out|write:setFade|read:getFade");

    var util = new Object;
    util.pixelMap = new Array();
    util.initialized = false;
    util.linesMaxSize = 0;

    var lines = new Array();

    function Line(x, y, step)
    {
      this.xCenter = x;
      this.yCenter = y;
      this.step = step;
    }

    algo.setSize = function(_size)
    {
      algo.linesSize = _size;
      util.initialized = false;
    };

    algo.getSize = function()
    {
      return algo.linesSize;
    };

    algo.setVariability = function(_var)
    {
      algo.linesVariability = _var;
      util.initialized = false;
    };

    algo.getVariability = function()
    {
      return algo.linesVariability;
    };

    algo.setAmount = function(_amount)
    {
      algo.linesAmount = _amount;
      util.initialized = false;
    };

    algo.getAmount = function()
    {
      return algo.linesAmount;
    };

    algo.setFade = function(_fade)
    {
      if (_fade === "Fade In") { algo.fadeMode = 1; }
      else if (_fade === "Fade Out") { algo.fadeMode = 2; }
      else { algo.fadeMode = 0; }
    };

    algo.getFade = function()
    {
      if (algo.fadeMode === 1) { return "Fade In"; }
      else if (algo.fadeMode === 2) { return "Fade Out"; }
      else { return "Don't Fade"; }
    };

    algo.setDirection = function(_direction)
    {
      if (_direction === "Vertical") { algo.linesDirection = 1; }
      else if (_direction === "Plus") { algo.linesDirection = 2; }
      else if (_direction === "X") { algo.linesDirection = 3; }
      else if (_direction === "Star") { algo.linesDirection = 4; }
      else if (_direction === "Left") { algo.linesDirection = 5; }
      else if (_direction === "Right") { algo.linesDirection = 6; }
      else if (_direction === "Up") { algo.linesDirection = 7; }
      else if (_direction === "Down") { algo.linesDirection = 8; }
      else if (_direction === "UpRight") { algo.linesDirection = 9; }
      else if (_direction === "UpLeft") { algo.linesDirection = 10; }
      else if (_direction === "DownRight") { algo.linesDirection = 11; }
      else if (_direction === "DownLeft") { algo.linesDirection = 12; }
      else { algo.linesDirection = 0; }
    };

    algo.getDirection = function()
    {
      if (algo.linesDirection === 1) { return "Vertical"; }
      else if (algo.linesDirection === 2) { return "Plus"; }
      else if (algo.linesDirection === 3) { return "X"; }
      else if (algo.linesDirection === 4) { return "Star"; }
      else if (algo.linesDirection === 5) { return "Left"; }
      else if (algo.linesDirection === 6) { return "Right"; }
      else if (algo.linesDirection === 7) { return "Up"; }
      else if (algo.linesDirection === 8) { return "Down"; }
      else if (algo.linesDirection === 9) { return "UpRight"; }
      else if (algo.linesDirection === 10) { return "UpLeft"; }
      else if (algo.linesDirection === 11) { return "DownRight"; }
      else if (algo.linesDirection === 12) { return "DownLeft"; }
      else { return "Horizontal"; }
    };

    util.initialize = function(size)
    {
      if (size > 0) {
        util.linesMaxSize = size;
      }

      lines = new Array();
      for (var i = 0; i < algo.linesAmount; i++) {
        lines[i] = new Line(-1, -1, 0);
      }

      util.initialized = true;
    };

    util.getColor = function(step, rgb)
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

        var stepCount = Math.floor(util.linesMaxSize);
        var fadeStep = step;
        if (algo.fadeMode === 2) {
          fadeStep = stepCount - step;
        }
        var newR = (r / stepCount) * fadeStep;
        var newG = (g / stepCount) * fadeStep;
        var newB = (b / stepCount) * fadeStep;
        var newRGB = (newR << 16) + (newG << 8) + newB;
        return newRGB;
      }
    };

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
      var x, y;
      // create an empty, black pixelMap
      util.pixelMap = new Array(height);
      for (y = 0; y < height; y++)
      {
        util.pixelMap[y] = new Array(width);
        for (x = 0; x < width; x++) {
          util.pixelMap[y][x] = 0;
        }
      }

      for (var i = 0; i < algo.linesAmount; i++)
      {
        var color = util.getColor(lines[i].step, rgb);
        //alert("Line " + i + " xCenter: " + lines[i].xCenter + " color: " + color.toString(16));
        if (lines[i].xCenter === -1)
        {
          var seed = Math.floor(Math.random()*100);
          if (seed > 50) { continue; }
          lines[i].xCenter = Math.floor(Math.random() * width);
          lines[i].yCenter = Math.floor(Math.random() * height);
          util.pixelMap[lines[i].yCenter][lines[i].xCenter] = color;
        }
        else
        {
          var l = lines[i].step * Math.cos(Math.PI / 4);
          var radius2 = lines[i].step * lines[i].step;
          l = l.toFixed(0);

          for (x = 0; x <= l; x++)
          {
            y = Math.sqrt(radius2 - (x * x));

            if (algo.linesDirection == 0 || algo.linesDirection == 2 || algo.linesDirection == 4 || algo.linesDirection == 5) {
              util.drawPixel(lines[i].xCenter - x, lines[i].yCenter, color, width, height);
            } 
            if (algo.linesDirection == 0 || algo.linesDirection == 2 || algo.linesDirection == 4 || algo.linesDirection == 6) {
              util.drawPixel(lines[i].xCenter + x, lines[i].yCenter, color, width, height);
            }
            if (algo.linesDirection == 1 || algo.linesDirection == 2 || algo.linesDirection == 4 || algo.linesDirection == 7) {
              util.drawPixel(lines[i].xCenter, lines[i].yCenter - x, color, width, height);
            }
            if (algo.linesDirection == 1 || algo.linesDirection == 2 || algo.linesDirection == 4 || algo.linesDirection == 8) {
              util.drawPixel(lines[i].xCenter, lines[i].yCenter + x, color, width, height);
            }

            if (algo.linesDirection == 3 || algo.linesDirection == 4 ||  algo.linesDirection == 9) {
              util.drawPixel(lines[i].xCenter + x, lines[i].yCenter - x, color, width, height);
            } 
            if (algo.linesDirection == 3 || algo.linesDirection == 4 || algo.linesDirection == 10) {
              util.drawPixel(lines[i].xCenter - x, lines[i].yCenter - x, color, width, height);
            } 
            if (algo.linesDirection == 3 || algo.linesDirection == 4 || algo.linesDirection == 11) {
              util.drawPixel(lines[i].xCenter + x, lines[i].yCenter + x, color, width, height);
            } 
            if (algo.linesDirection == 3 || algo.linesDirection == 4 || algo.linesDirection == 12) {
              util.drawPixel(lines[i].xCenter - x, lines[i].yCenter + x, color, width, height);
            } 

          }
        }

        lines[i].step++;
        if (lines[i].step >= util.linesMaxSize || Math.floor(Math.random() * 100 + (lines[i].step * 2) ) < algo.linesVariability  )
        {
          lines[i].xCenter = -1;
          lines[i].yCenter = -1;
          lines[i].step = 0;
        }
      }

      return util.pixelMap;
    };

    algo.rgbMap = function(width, height, rgb, step)
    {
      if (util.initialized === false)
      {
          util.initialize(algo.linesSize);
      }

      return util.getNextStep(width, height, rgb);
    };

    algo.rgbMapStepCount = function(width, height)
    {
      return algo.linesSize;
    };

    // Development tool access
    testAlgo = algo;

    return algo;
    }
)();
