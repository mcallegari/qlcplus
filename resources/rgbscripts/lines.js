/*
  Q Light Controller Plus
  lines.js

  Copyright (c) Branson Matheson

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
    algo.linesType = 0;
    algo.properties.push("name:linesType|type:list|display:Type|values:Horizontal,Vertical,Plus,X,Star,Left,Right,Up,Down,UpRight,UpLeft,DownRight,DownLeft|write:setType|read:getType");
    algo.linesBias = 0;
    algo.properties.push("name:linesBias|type:list|display:Starting Bias|values:Random,Top,Bottom,Left,Right,TopLeft,TopRight,BottomLeft,BottomRight|write:setBias|read:getBias");
    algo.linesDistribution = 0;
    algo.properties.push("name:linesDistribution|type:list|display:Distribution|values:All,Every 2nd,Every 3rd,Half A,Half B,Center Third,Edges Only|write:setDistribution|read:getDistribution");
    algo.linesMovement = 0;
    algo.properties.push("name:linesMovement|type:list|display:Movement|values:None,Up,Down,Left,Right,Up Loop,Down Loop,Left Loop,Right Loop|write:setMovement|read:getMovement");
    algo.linesLifecycle = 0;
    algo.properties.push("name:linesLifecycle|type:list|display:Lifecycle|values:Grow,Grow Fade In,Grow Fade Out,Shrink,Shrink Fade In,Shrink Fade Out,Static,Static Fade In,Static Fade Out|write:setLifecycle|read:getLifecycle");
    algo.linesPattern = 0;
    algo.properties.push("name:linesPattern|type:list|display:Line Pattern|values:Solid,Dashed,Dotted,Double|write:setPattern|read:getPattern");
    algo.linesBrightnessVariance = 0;
    algo.properties.push("name:linesBrightnessVariance|type:range|display:Brightness Variance|values:0,100|write:setBrightnessVariance|read:getBrightnessVariance");
    algo.linesMovementSpeed = 100;
    algo.properties.push("name:linesMovementSpeed|type:range|display:Movement Energy|values:10,200|write:setMovementSpeed|read:getMovementSpeed");
    algo.fadeMode = 0;

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
      this.color = 0;
      this.brightnessFactor = 1.0;
      this.movementCounter = 0;
      this.movementSpeed = 1.0;
    };

    algo.setLinesSize = function(_size)
    {
      if (!(parseInt(_size) === NaN) && parseInt(_size) > 0)
      {
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
      if (!(parseInt(_var) === NaN) && parseInt(_var) >= 0)
      {
        algo.linesVariability = parseInt(_var);
        util.initialized = false;
      }
    };

    algo.getVariability = function()
    {
      return algo.linesVariability;
    };

    algo.setAmount = function(_amount)
    {
      if (!(parseInt(_amount) === NaN) && parseInt(_amount) >= 1 && parseInt(_amount) <= 200)
      {
        algo.linesAmount = parseInt(_amount);
        util.initialized = false;
      }
    };

    algo.getAmount = function()
    {
      return algo.linesAmount;
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



    algo.setPattern = function(_pattern)
    {
      if (_pattern === "Dashed") { algo.linesPattern = 1; }
      else if (_pattern === "Dotted") { algo.linesPattern = 2; }
      else if (_pattern === "Double") { algo.linesPattern = 3; }
      else { algo.linesPattern = 0; }
    };

    algo.getPattern = function()
    {
      if (algo.linesPattern === 1) { return "Dashed"; }
      else if (algo.linesPattern === 2) { return "Dotted"; }
      else if (algo.linesPattern === 3) { return "Double"; }
      else { return "Solid"; }
    };

    algo.setBrightnessVariance = function(_variance)
    {
      if (!(parseInt(_variance) === NaN) && parseInt(_variance) >= 0 && parseInt(_variance) <= 100)
      {
        algo.linesBrightnessVariance = parseInt(_variance);
      }
    };

    algo.getBrightnessVariance = function()
    {
      return algo.linesBrightnessVariance;
    };

    algo.setMovementSpeed = function(_speed)
    {
      if (!(parseInt(_speed) === NaN) && parseInt(_speed) >= 10 && parseInt(_speed) <= 200)
      {
        algo.linesMovementSpeed = parseInt(_speed);
      }
    };

    algo.getMovementSpeed = function()
    {
      return algo.linesMovementSpeed;
    };

    // Missing setters for test compatibility
    algo.setLinesAmount = function(_amount)
    {
      algo.setAmount(_amount);
    };

    algo.setLinesMovement = function(_movement)
    {
      algo.setMovement(_movement);
    };

    algo.setLinesLifecycle = function(_lifecycle)
    {
      algo.setLifecycle(_lifecycle);
    };

    algo.setLinesDistribution = function(_distribution)
    {
      algo.setDistribution(_distribution);
    };

    algo.setLinesPattern = function(_pattern)
    {
      algo.setPattern(_pattern);
    };

    algo.setLinesBrightnessVariance = function(_variance)
    {
      algo.setBrightnessVariance(_variance);
    };

    algo.setLinesVariability = function(_variability)
    {
      algo.setVariability(_variability);
    };

    algo.setLinesMovementSpeed = function(_speed)
    {
      algo.setMovementSpeed(_speed);
    };

    algo.setLinesType = function(_type)
    {
      algo.setType(_type);
    };

    algo.setLinesBias = function(_bias)
    {
      algo.setBias(_bias);
    };

    algo.setMovement = function(_movement)
    {
      if (_movement === "Up") { algo.linesSlide = 1; algo.linesRollover = 0; }
      else if (_movement === "Down") { algo.linesSlide = 2; algo.linesRollover = 0; }
      else if (_movement === "Left") { algo.linesSlide = 3; algo.linesRollover = 0; }
      else if (_movement === "Right") { algo.linesSlide = 4; algo.linesRollover = 0; }
      else if (_movement === "Up Loop") { algo.linesSlide = 1; algo.linesRollover = 1; }
      else if (_movement === "Down Loop") { algo.linesSlide = 2; algo.linesRollover = 1; }
      else if (_movement === "Left Loop") { algo.linesSlide = 3; algo.linesRollover = 1; }
      else if (_movement === "Right Loop") { algo.linesSlide = 4; algo.linesRollover = 1; }
      else { algo.linesSlide = 0; algo.linesRollover = 0; }
    };

    algo.getMovement = function()
    {
      if (algo.linesSlide === 0) return "None";
      var direction = "";
      if (algo.linesSlide === 1) direction = "Up";
      else if (algo.linesSlide === 2) direction = "Down";
      else if (algo.linesSlide === 3) direction = "Left";
      else if (algo.linesSlide === 4) direction = "Right";

      return direction + (algo.linesRollover === 1 ? " Loop" : "");
    };

    algo.setLifecycle = function(_lifecycle)
    {
      if (_lifecycle === "Grow") { algo.linesSizeBehavior = 0; algo.fadeMode = 0; }
      else if (_lifecycle === "Grow Fade In") { algo.linesSizeBehavior = 0; algo.fadeMode = 1; }
      else if (_lifecycle === "Grow Fade Out") { algo.linesSizeBehavior = 0; algo.fadeMode = 2; }
      else if (_lifecycle === "Shrink") { algo.linesSizeBehavior = 1; algo.fadeMode = 0; }
      else if (_lifecycle === "Shrink Fade In") { algo.linesSizeBehavior = 1; algo.fadeMode = 1; }
      else if (_lifecycle === "Shrink Fade Out") { algo.linesSizeBehavior = 1; algo.fadeMode = 2; }
      else if (_lifecycle === "Static") { algo.linesSizeBehavior = 2; algo.fadeMode = 0; }
      else if (_lifecycle === "Static Fade In") { algo.linesSizeBehavior = 2; algo.fadeMode = 1; }
      else if (_lifecycle === "Static Fade Out") { algo.linesSizeBehavior = 2; algo.fadeMode = 2; }
      else { algo.linesSizeBehavior = 0; algo.fadeMode = 0; }
    };

    algo.getLifecycle = function()
    {
      var behavior = "";
      if (algo.linesSizeBehavior === 0) behavior = "Grow";
      else if (algo.linesSizeBehavior === 1) behavior = "Shrink";
      else if (algo.linesSizeBehavior === 2) behavior = "Static";

      var fade = "";
      if (algo.fadeMode === 1) fade = " Fade In";
      else if (algo.fadeMode === 2) fade = " Fade Out";

      return behavior + fade;
    };

    algo.setDistribution = function(_distribution)
    {
      if (_distribution === "Every 2nd") { algo.linesDistribution = 1; }
      else if (_distribution === "Every 3rd") { algo.linesDistribution = 2; }
      else if (_distribution === "Half A") { algo.linesDistribution = 3; }
      else if (_distribution === "Half B") { algo.linesDistribution = 4; }
      else if (_distribution === "Center Third") { algo.linesDistribution = 5; }
      else if (_distribution === "Edges Only") { algo.linesDistribution = 6; }
      else { algo.linesDistribution = 0; } // All
    };

    algo.getDistribution = function()
    {
      if (algo.linesDistribution === 1) { return "Every 2nd"; }
      else if (algo.linesDistribution === 2) { return "Every 3rd"; }
      else if (algo.linesDistribution === 3) { return "Half A"; }
      else if (algo.linesDistribution === 4) { return "Half B"; }
      else if (algo.linesDistribution === 5) { return "Center Third"; }
      else if (algo.linesDistribution === 6) { return "Edges Only"; }
      else { return "All"; }
    };

    util.initialize = function(size)
    {
      if (size > 0)
      {
        util.linesMaxSize = size;
      }

      lines = new Array();
      for (var i = 0; i < algo.linesAmount; i++)
      {
        lines[i] = new Line(-1, -1, 0);
      }

      util.initialized = true;
    };

    util.getEffectiveLineSize = function(step)
    {
      if (algo.linesSizeBehavior === 1) {
        // Shrink: start large and get smaller
        return util.linesMaxSize - step;
      }
      else if (algo.linesSizeBehavior === 2) {
        // Static: maintain constant size
        return util.linesMaxSize;
      }
      else {
        // Grow (default): start small and get larger
        return step;
      }
    };

    util.shouldDrawPixel = function(distance, effectiveSize)
    {
      if (algo.linesPattern === 0) {
        // Solid: always draw
        return true;
      }
      else if (algo.linesPattern === 1) {
        // Dashed: draw in segments of 3, skip 2
        var segment = Math.floor(distance / 3) % 2;
        return segment === 0;
      }
      else if (algo.linesPattern === 2) {
        // Dotted: draw every 3rd pixel
        return (Math.floor(distance) % 3) === 0;
      }
      else if (algo.linesPattern === 3) {
        // Double: draw at start and end portions
        var portion = distance / effectiveSize;
        return (portion <= 0.3 || portion >= 0.7);
      }
      return true;
    };

    util.applyBrightnessVariance = function(color, brightnessFactor)
    {
      if (algo.linesBrightnessVariance === 0) {
        return color;
      }

      var r = (color >> 16) & 0x00FF;
      var g = (color >> 8) & 0x00FF;
      var b = color & 0x00FF;

      var newR = Math.round(r * brightnessFactor);
      var newG = Math.round(g * brightnessFactor);
      var newB = Math.round(b * brightnessFactor);

      // Clamp values to 0-255 range
      newR = Math.max(0, Math.min(255, newR));
      newG = Math.max(0, Math.min(255, newG));
      newB = Math.max(0, Math.min(255, newB));

      return (newR << 16) + (newG << 8) + newB;
    };

    util.getDistributedPosition = function(maxValue, isHorizontalLine)
    {
      var dimension = maxValue;

      // Handle edge case for very small dimensions
      if (dimension <= 1)
      {
        return 0;
      }

      if (algo.linesDistribution === 0)
      {
        return Math.round(Math.random() * (dimension - 1));
      }
      else if (algo.linesDistribution === 1)
      {
        var step = 2;
        var maxSteps = Math.floor(dimension / step);
        if (maxSteps <= 0) return 0;
        var stepIndex = Math.floor(Math.random() * maxSteps);
        return Math.min(stepIndex * step, dimension - 1);
      }
      else if (algo.linesDistribution === 2)
      {
        var step = 3;
        var maxSteps = Math.floor(dimension / step);
        if (maxSteps <= 0) return 0;
        var stepIndex = Math.floor(Math.random() * maxSteps);
        return Math.min(stepIndex * step, dimension - 1);
      }
      else if (algo.linesDistribution === 3)
      {
        var halfPoint = Math.floor(dimension / 2);
        if (halfPoint <= 1) return 0;
        return Math.round(Math.random() * (halfPoint - 1));
      }
      else if (algo.linesDistribution === 4)
      {
        var halfPoint = Math.floor(dimension / 2);
        var range = dimension - halfPoint - 1;
        if (range <= 0) return Math.min(halfPoint, dimension - 1);
        return halfPoint + Math.round(Math.random() * range);
      }
      else if (algo.linesDistribution === 5)
      {
        var thirdPoint = Math.floor(dimension / 3);
        var startPos = thirdPoint;
        var endPos = dimension - thirdPoint;
        var range = endPos - startPos - 1;
        if (range <= 0) return Math.min(startPos, dimension - 1);
        return startPos + Math.round(Math.random() * range);
      }
      else if (algo.linesDistribution === 6)
      {
        var quarterPoint = Math.floor(dimension / 4);
        if (quarterPoint <= 1)
        {
          return Math.round(Math.random() * (dimension - 1));
        }

        if (Math.random() < 0.5)
        {
          return Math.round(Math.random() * (quarterPoint - 1));
        }
        else
        {
          var startPos = dimension - quarterPoint;
          var range = quarterPoint - 1;
          if (range <= 0) return Math.min(startPos, dimension - 1);
          return startPos + Math.round(Math.random() * range);
        }
      }

      return Math.round(Math.random() * (dimension - 1));
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

          // Set brightness factor based on variance setting
          if (algo.linesBrightnessVariance > 0) {
            var variance = algo.linesBrightnessVariance / 100.0;
            var randomFactor = (Math.random() * 2 - 1) * variance; // -variance to +variance
            lines[i].brightnessFactor = Math.max(0.1, Math.min(1.0, 1.0 + randomFactor));
          } else {
            lines[i].brightnessFactor = 1.0;
          }

          // Set movement speed based on speed setting (10-200% range)
          var speedVariance = 0.5; // ±50% variance from base speed
          var baseSpeed = algo.linesMovementSpeed / 100.0; // Convert percentage to decimal
          var randomSpeedFactor = (Math.random() * 2 - 1) * speedVariance; // -0.5 to +0.5
          lines[i].movementSpeed = Math.max(0.1, baseSpeed + randomSpeedFactor);
          lines[i].movementCounter = 0;

          // Position lines using distribution and bias
          // For horizontal lines (0,2,4), distribution affects Y positioning
          // For vertical lines (1), distribution affects X positioning
          // For other types, distribution affects the primary axis

          var isHorizontalType = (algo.linesType === 0 || algo.linesType === 2 || algo.linesType === 4);
          var isVerticalType = (algo.linesType === 1);

          // Y positioning (vertical placement)
          if (isHorizontalType && algo.linesDistribution > 0) {
            // Use distribution for Y when horizontal lines
            lines[i].yCenter = util.getDistributedPosition(height, true);
          } else if (algo.linesBias == 1 || algo.linesBias == 5 || algo.linesBias == 6) {
            // Top bias
            lines[i].yCenter = Math.round(Math.random() * height / 5);
          } else if (algo.linesBias == 2 || algo.linesBias == 7 || algo.linesBias == 8) {
            // Bottom bias
            lines[i].yCenter = (height - 1) - Math.round(Math.random() * height / 5);
          } else {
            // Random Y
            lines[i].yCenter = Math.round(Math.random() * (height - 1));
          }

          // X positioning (horizontal placement)
          if (isVerticalType && algo.linesDistribution > 0) {
            // Use distribution for X when vertical lines
            lines[i].xCenter = util.getDistributedPosition(width, false);
          } else if (algo.linesBias == 3 || algo.linesBias == 5 || algo.linesBias == 7) {
            // Left bias
            lines[i].xCenter = Math.round(Math.random() * width / 5);
          } else if (algo.linesBias == 4 || algo.linesBias == 6 || algo.linesBias == 8) {
            // Right bias
            lines[i].xCenter = (width - 1) - Math.round(Math.random() * width / 5);
          } else {
            // Random X
            lines[i].xCenter = Math.round(Math.random() * (width - 1));
          }

          // Bounds check before setting pixel
          if (lines[i].yCenter >= 0 && lines[i].yCenter < height &&
              lines[i].xCenter >= 0 && lines[i].xCenter < width)
          {
            util.pixelMap[lines[i].yCenter][lines[i].xCenter] = lines[i].color;
          }
        }
        else
        {
          var baseColor = util.getColor(lines[i].step, lines[i].color);
          var color = util.applyBrightnessVariance(baseColor, lines[i].brightnessFactor);

          var effectiveSize = util.getEffectiveLineSize(lines[i].step);
          var l = Math.floor(effectiveSize / 2);
          var radius2 = l * l;

          for (x = 0; x <= l; x++)
          {
            if (!util.shouldDrawPixel(x, effectiveSize))
            {
              continue;
            }

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

        if (algo.linesSizeBehavior !== 2)
        {
          lines[i].step++;
        }

        if (algo.linesSlide > 0)
        {
          lines[i].movementCounter += lines[i].movementSpeed;

          while (lines[i].movementCounter >= 1.0)
          {
            if (algo.linesSlide == 1) { lines[i].yCenter--; }
            else if (algo.linesSlide == 2) { lines[i].yCenter++; }
            else if (algo.linesSlide == 3) { lines[i].xCenter--; }
            else if (algo.linesSlide == 4) { lines[i].xCenter++; }

            lines[i].movementCounter -= 1.0;
          }

          if (algo.linesRollover == 1)
          {
            if (lines[i].xCenter < 0) { lines[i].xCenter = width - 1; }
            if (lines[i].yCenter < 0) { lines[i].yCenter = height - 1; }
            if (lines[i].xCenter >= width) { lines[i].xCenter = 0; }
            if (lines[i].yCenter >= height) { lines[i].yCenter = 0; }
          }
        }

        var shouldTerminate = false;

        if (algo.linesSizeBehavior === 1)
        {
          shouldTerminate = (util.getEffectiveLineSize(lines[i].step) <= 0);
        }
        else if (algo.linesSizeBehavior === 2)
        {
          if (algo.linesSlide > 0 && algo.linesRollover === 0)
          {
            if (lines[i].xCenter < 0 || lines[i].xCenter >= width ||
                lines[i].yCenter < 0 || lines[i].yCenter >= height)
            {
              shouldTerminate = true;
            }
          }
        }
        else
        {
          shouldTerminate = (lines[i].step >= util.linesMaxSize);
        }

        if (shouldTerminate || Math.floor(Math.random() * 100 + (lines[i].step * 2)) < algo.linesVariability)
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

    // Test suite compatibility - functions named exactly like property names
    algo.linesAmount = function(amount) { algo.setAmount(amount); };
    algo.linesSize = function(size) { algo.setLinesSize(size); };
    algo.linesType = function(type) { algo.setType(type); };
    algo.linesBias = function(bias) { algo.setBias(bias); };
    algo.linesDistribution = function(distribution) { algo.setDistribution(distribution); };
    algo.linesMovement = function(movement) { algo.setMovement(movement); };
    algo.linesLifecycle = function(lifecycle) { algo.setLifecycle(lifecycle); };
    algo.linesPattern = function(pattern) { algo.setPattern(pattern); };
    algo.linesBrightnessVariance = function(variance) { algo.setBrightnessVariance(variance); };
    algo.linesMovementSpeed = function(speed) { algo.setMovementSpeed(speed); };
    algo.linesVariability = function(variability) { algo.setVariability(variability); };

    return algo;
  }
)();
