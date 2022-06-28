/*
  Q Light Controller Plus
  lines.js

  Copyright (c) Branson Matheson
  Based on work by Massimo Callegari

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
    algo.author = "Branson Matheson";
    algo.acceptColors = 2;
    algo.properties = new Array();
    algo.linesAmount = 3;
    algo.properties.push("name:linesAmount|type:range|display:Amount|values:1,200|write:setAmount|read:getAmount");
    algo.linesSize = 10;
    algo.properties.push("name:linesSize|type:range|display:Size|values:1,32|write:setLinesSize|read:getLinesSize");
    algo.linesVariability = 0;
    algo.properties.push("name:linesVariability|type:range|display:Size Variablity|values:0,100|write:setVariability|read:getVariability");
    algo.linesType = 0;
    algo.properties.push("name:linesType|type:list|display:Type|values:Horizontal,Vertical,Plus,X,Star,Left,Right,Up,Down,UpRight,UpLeft,DownRight,DownLeft|write:setType|read:getType");
    algo.linesBias = 0;
    algo.properties.push("name:linesBias|type:list|display:Starting Bias|values:Random,Top,Bottom,Left,Right,TopLeft,TopRight,BottomLeft,BottomRight|write:setBias|read:getBias");
    algo.linesSlide = 0;
    algo.properties.push("name:linesSlide|type:list|display:Slide|values:None,Up,Down,Left,Right|write:setSlide|read:getSlide");
    algo.linesRollover = 0;
    algo.properties.push("name:linesRollover|type:list|display:Rollover|values:Yes,No|write:setRollover|read:getRollover");
    algo.fadeMode = 0;
    algo.properties.push("name:fadeMode|type:list|display:Fade Mode|values:Don't Fade,Fade In,Fade Out|write:setFade|read:getFade");

    var util = new Object;
    util.pixelMap = new Array();
    util.initialized = false;
    util.linesMaxSize = 0;

    var lines = new Array();

    function Line(x, y, step) {
      this.xCenter = x;
      this.yCenter = y;
      this.step = step;
      this.color = 0;
    };

    algo.setLinesSize = function(_size)
    {
	  // Only set if the input is valid.
      if (!(parseInt(_size) === NaN) && parseInt(_size) > 0) {
        algo.linesSize = parseInt(_size);
        util.initialized = false;
      }
    };

    algo.getLinesSize = function()
    {
      return algo.linesSize;
    };

    algo.setVariability = function(_var)
    {
	  // Only set if the input is valid.
      if (!(parseInt(_var) === NaN) && parseInt(_var) > 0) {
        algo.linesVariability = _var;
        util.initialized = false;
      }
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

    algo.setType = function(_type)
    {
      if (_type === "Vertical") { algo.linesType = 1; }
      else if (_type === "Plus") { algo.linesType = 2; }
      else if (_type === "X") { algo.linesType = 3; }
      else if (_type === "Star") { algo.linesType = 4; }
      else if (_type === "Left") { algo.linesType = 5; }
      else if (_type === "Right") { algo.linesType = 6; }
      else if (_type === "Up") { algo.linesType = 7; }
      else if (_type === "Down") { algo.linesType = 8; }
      else if (_type === "UpRight") { algo.linesType = 9; }
      else if (_type === "UpLeft") { algo.linesType = 10; }
      else if (_type === "DownRight") { algo.linesType = 11; }
      else if (_type === "DownLeft") { algo.linesType = 12; }
      else { algo.linesType = 0; }
    };

    algo.getType = function()
    {
      if (algo.linesType === 1) { return "Vertical"; }
      else if (algo.linesType === 2) { return "Plus"; }
      else if (algo.linesType === 3) { return "X"; }
      else if (algo.linesType === 4) { return "Star"; }
      else if (algo.linesType === 5) { return "Left"; }
      else if (algo.linesType === 6) { return "Right"; }
      else if (algo.linesType === 7) { return "Up"; }
      else if (algo.linesType === 8) { return "Down"; }
      else if (algo.linesType === 9) { return "UpRight"; }
      else if (algo.linesType === 10) { return "UpLeft"; }
      else if (algo.linesType === 11) { return "DownRight"; }
      else if (algo.linesType === 12) { return "DownLeft"; }
      else { return "Horizontal"; }
    };

    algo.setSlide = function(_slide)
    {
      if (_slide === "Up") { algo.linesSlide = 1; }
      else if (_slide === "Down") { algo.linesSlide = 2; }
      else if (_slide === "Left") { algo.linesSlide = 3; }
      else if (_slide === "Right") { algo.linesSlide = 4; }
      else { algo.linesSlide = 0; }
    };

    algo.getSlide = function()
    {
      if (algo.linesSlide === 1) { return "Up"; }
      else if (algo.linesSlide === 2) { return "Down"; }
      else if (algo.linesSlide === 3) { return "Left"; }
      else if (algo.linesSlide === 4) { return "Right"; }
      else { return "None"; }
    };

    algo.setBias = function(_bias)
    {
      if (_bias === "Top") { algo.linesBias = 1; }
      else if (_bias === "Bottom") { algo.linesBias = 2; }
      else if (_bias === "Left") { algo.linesBias = 3; }
      else if (_bias === "Right") { algo.linesBias = 4; }
      else if (_bias === "TopLeft") { algo.linesBias = 5; }
      else if (_bias === "TopRight") { algo.linesBias = 6; }
      else if (_bias === "BottomLeft") { algo.linesBias = 7; }
      else if (_bias === "BottomRight") { algo.linesBias = 8; }
      else { algo.linesBias = 0; }
    };

    algo.getBias = function()
    {
      if (algo.linesBias === 1) { return "Top"; }
      else if (algo.linesBias === 2) { return "Bottom"; }
      else if (algo.linesBias === 3) { return "Left"; }
      else if (algo.linesBias === 4) { return "Right"; }
      else if (algo.linesBias === 5) { return "TopLeft"; }
      else if (algo.linesBias === 6) { return "TopRight"; }
      else if (algo.linesBias === 7) { return "BottomLeft"; }
      else if (algo.linesBias === 8) { return "BottomRight"; }
      else { return "Random"; }
    };

    algo.setRollover = function(_rollover)
    {
      if (_rollover === "Yes") { algo.linesRollover = 1; }
      else { algo.linesRollover = 0; }
    };

    algo.getRollover = function()
    {
      if (algo.linesRollover === 1) { return "Yes"; }
      else { return "No"; }
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
        var factor = fadeStep / stepCount;
        var newR = Math.round(r * factor);
        var newG = Math.round(g * factor);
        var newB = Math.round(b * factor);

        var newRGB = (newR << 16) + (newG << 8) + newB;
        return newRGB;
      }
    };

    util.drawPixel = function(cx, cy, color, width, height)
    {
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
        if (lines[i].xCenter === -1)
        {
          // Randomize the initialization of a new line.
          var seed = Math.floor(Math.random() * 100);
          if (seed > 50)
            continue;

          // apply the current step color
          lines[i].color = rgb;

          // if biased .. move the start points to the cardinal ends of the space
          if (algo.linesBias == 1 || algo.linesBias == 5 || algo.linesBias == 6)
          {
            lines[i].yCenter = Math.round(Math.random() * height / 5);
          } 
          else if (algo.linesBias == 2 || algo.linesBias == 7 || algo.linesBias == 8)
          {
            lines[i].yCenter = (height - 1) - Math.round(Math.random() * height	 / 5);
          } 
          else
          {
            lines[i].yCenter = Math.round(Math.random() * (height - 1));
          }

          if (algo.linesBias == 3 || algo.linesBias == 5 || algo.linesBias == 7)
          {
            lines[i].xCenter = Math.round(Math.random() * width / 5);
          }
          else if (algo.linesBias == 4 || algo.linesBias == 6 || algo.linesBias == 8)
          {
            lines[i].xCenter = (width - 1) - Math.round(Math.random() * width / 5);
          } 
          else
          {
            lines[i].xCenter = Math.round(Math.random() * (width - 1));
          }

          util.pixelMap[lines[i].yCenter][lines[i].xCenter] = lines[i].color;
        } else {
          var color = util.getColor(lines[i].step, lines[i].color);
          //alert("Line " + i + " xCenter: " + lines[i].xCenter + " color: " + color.toString(16));

          var l = lines[i].step * Math.cos(Math.PI / 4);
          var radius2 = lines[i].step * lines[i].step;
          l = l.toFixed(0);

          for (x = 0; x <= l; x++)
          {
            y = Math.sqrt(radius2 - (x * x));

            if (algo.linesType == 0 || algo.linesType == 2 || algo.linesType == 4 || algo.linesType == 5) {
              util.drawPixel(lines[i].xCenter - x, lines[i].yCenter, color, width, height);
            } 
            if (algo.linesType == 0 || algo.linesType == 2 || algo.linesType == 4 || algo.linesType == 6) {
              util.drawPixel(lines[i].xCenter + x, lines[i].yCenter, color, width, height);
            }
            if (algo.linesType == 1 || algo.linesType == 2 || algo.linesType == 4 || algo.linesType == 7) {
              util.drawPixel(lines[i].xCenter, lines[i].yCenter - x, color, width, height);
            }
            if (algo.linesType == 1 || algo.linesType == 2 || algo.linesType == 4 || algo.linesType == 8) {
              util.drawPixel(lines[i].xCenter, lines[i].yCenter + x, color, width, height);
            }

            if (algo.linesType == 3 || algo.linesType == 4 ||  algo.linesType == 9) {
              util.drawPixel(lines[i].xCenter + x, lines[i].yCenter - x, color, width, height);
            } 
            if (algo.linesType == 3 || algo.linesType == 4 || algo.linesType == 10) {
              util.drawPixel(lines[i].xCenter - x, lines[i].yCenter - x, color, width, height);
            } 
            if (algo.linesType == 3 || algo.linesType == 4 || algo.linesType == 11) {
              util.drawPixel(lines[i].xCenter + x, lines[i].yCenter + x, color, width, height);
            } 
            if (algo.linesType == 3 || algo.linesType == 4 || algo.linesType == 12) {
              util.drawPixel(lines[i].xCenter - x, lines[i].yCenter + x, color, width, height);
            } 
          }
        }

        lines[i].step++;
        if (algo.linesSlide > 0) {
          if ( algo.linesSlide == 1 ) { lines[i].yCenter-- ;} 
          else if ( algo.linesSlide == 2 ) { lines[i].yCenter++ ;}
          else if ( algo.linesSlide == 3 ) { lines[i].xCenter-- ;}
          else if ( algo.linesSlide == 4 ) { lines[i].xCenter++ ;}

          if ( algo.linesRollover == 1 ) {
            if ( lines[i].xCenter == 0 ) { lines[i].xCenter = width; }
            if ( lines[i].yCenter == 0 ) { lines[i].yCenter = height; }
            if ( lines[i].xCenter >= width ) { lines[i].xCenter = 0; }
            if ( lines[i].yCenter >= height) { lines[i].yCenter = 0; }
          }
        }
        if (lines[i].step >= util.linesMaxSize || Math.floor(Math.random() * 100 + (lines[i].step * 2) ) < algo.linesVariability )
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
      return 2;
    };

    // Development tool access
    testAlgo = algo;

    return algo;
    }
)();
