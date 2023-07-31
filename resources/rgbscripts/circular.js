/*
  Q Light Controller Plus
  circular.js

  Copyright (c) Hans-Jürgen Tappe

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
    algo.name = "Circular";
    algo.author = "Hans-Jürgen Tappe";
    algo.acceptColors = 1;
    algo.properties = new Array();

    algo.circularMode = 0;
    algo.properties.push("name:circularMode|type:list|display:Mode|values:Radar,Propellor,Spiral Right,Spiral Left,S-Curve Right,S-Curve Left,Rings Spreading,Rings Rotating|write:setMode|read:getMode");
    algo.width = 1;
    algo.properties.push("name:width|type:range|display:Line Weight|values:1,100|write:setWidth|read:getWidth");
    algo.fillMatrix = 0;
    algo.properties.push("name:fillMatrix|type:list|display:Fill Matrix|values:No,Yes|write:setFill|read:getFill");
    algo.segmentsCount = 1;
    algo.properties.push("name:circlesSize|type:range|display:Segments|values:1,32|write:setSegments|read:getSegments");
    algo.divisor = 1;
    algo.properties.push("name:divisor|type:range|display:Algorithm Factor|values:1,10|write:setDivisor|read:getDivisor");
    algo.centerRadius = 0;
    algo.properties.push("name:centerRadius|type:range|display:Center Rotation|values:-10,10|write:setCenterRotation|read:getCenterRotation");
    algo.centerGap = 0;
    algo.properties.push("name:centerGap|type:range|display:Center Gap|values:0,100|write:setCenterGap|read:getCenterGap");
    algo.fadeMode = 0;
    algo.properties.push("name:fadeMode|type:list|display:Radar Fade Mode|values:Don't Fade,Fade Left,Fade Right|write:setFade|read:getFade");

    var util = new Object;
    util.initialized = false;
    util.circleRadius = 0;
    util.map = new Array();
    util.centerX = 0;
    util.centerY = 0;
    util.progstep = 0;
    util.circleFactor = 0;
    util.stepPercent = 0;
    util.stepAngle = 0;

    var geometryCalc = new Object;

    // calculate the angle from 0 to 2 pi starting north and counting clockwise
    geometryCalc.getAngle = function(offx, offy)
    {
      var angle = 0;
      // catch offx == 0
      if (offx == 0) {
        // This where the asymptote goes
        if (offy < 0) {
          angle = -1 * Math.PI / 2;
        } else {
          angle = Math.PI / 2;
        }
      } else {
        var gradient = offy / offx;
        angle = Math.atan(gradient);
      }
      angle += Math.PI / 2;
      if (offx < 0) {
        angle += Math.PI;  
      }
      return angle;
    }

    algo.setWidth = function(barWidth)
    {
      algo.width = barWidth;
    };

    algo.getWidth = function()
    {
      return parseInt(algo.width, 10);
    };

    algo.setCenterRotation = function(_amount)
    {
      algo.centerRadius = _amount;
      util.initialized = false;
    };

    algo.getCenterRotation = function()
    {
      return algo.centerRadius;
    };

    algo.setSegments = function(_amount)
    {
      algo.segmentsCount = _amount;
      util.initialized = false;
    };

    algo.getSegments = function()
    {
      return algo.segmentsCount;
    };

    algo.setCenterGap = function(centerGap)
    {
      algo.centerGap = centerGap;
    };

    algo.getCenterGap = function()
    {
      return parseInt(algo.centerGap, 10);
    };

    algo.setFade = function(_fade)
    {
      if (_fade === "Fade Left") { algo.fadeMode = 1; }
      else if (_fade === "Fade Right") { algo.fadeMode = 2; }
      else { algo.fadeMode = 0; }
    };

    algo.getFade = function()
    {
      if (algo.fadeMode === 1) { return "Fade Left"; }
      else if (algo.fadeMode === 2) { return "Fade Right"; }
      else { return "Don't Fade"; }
    };

    algo.setDivisor = function(_value)
    {
      algo.divisor = _value;
      util.initialized = false;
    };

    algo.getDivisor = function()
    {
      return algo.divisor;
    };

    algo.setFill = function (_fill) {
      if (_fill === "Yes") {
        algo.fillMatrix = 1;
      } else {
        algo.fillMatrix = 0;
      }
    };

    algo.getFill = function () {
      if (algo.fillMatrix === 1) {
        return "Yes";
      } else {
        return "No";
      }
    };

    algo.setMode = function(_mode)
    {
      if (_mode === "Spiral Right") { algo.circularMode = 1; }
      else if (_mode === "Spiral Left") { algo.circularMode = 2; }
      else if (_mode === "S-Curve Right") { algo.circularMode = 3; }
      else if (_mode === "S-Curve Left") { algo.circularMode = 4; }
      else if (_mode === "Rings Spreading") { algo.circularMode = 5; }
      else if (_mode === "Rings Rotating") { algo.circularMode = 6; }
      else if (_mode === "Propellor") { algo.circularMode = 7; }
      else { algo.circularMode = 0; }
    };

    algo.getMode = function()
    {
      if (algo.circularMode === 1) { return "Spiral Right"; }
      else if (algo.circularMode === 2) { return "Spiral Left"; }
      else if (algo.circularMode === 3) { return "S-Curve Right"; }
      else if (algo.circularMode === 4) { return "S-Curve Left"; }
      else if (algo.circularMode === 5) { return "Rings Spreading"; }
      else if (algo.circularMode === 6) { return "Rings Rotating"; }
      else if (algo.circularMode === 7) { return "Propellor"; }
      else { return "Radar"; }
    };

    util.initialize = function(width, height)
    {
      util.centerX = width / 2 - 0.5;
      util.centerY = height / 2 - 0.5;
      util.vCenterX = util.centerX;
      util.vCenterY = util.centerY;

      util.twoPi = 2 * Math.PI;
      util.halfPi = Math.PI / 2;

      if (algo.fillMatrix === 1) {
        util.circleRadius = 0;
      } else if (height > width) {
        util.circleRadius = width;
      } else {
        util.circleRadius = height;
      }

      util.map = new Array(height);
      for (var y = 0; y < height; y++) {
        util.map[y] = new Array(width);
        for (var x = 0; x < width; x ++) {
          util.map[y][x] = 0;
        }
      }
      
      util.stepFade = algo.rgbMapStepCount(width, height) / algo.segmentsCount;
      util.circleFactor = algo.rgbMapStepCount(width, height) / 3;

      util.width = width;
      util.height = height;

      util.blindoutRadius = Math.min(width, height) / 2;
      util.sOffsetFactor = Math.round(Math.min(width, height) / Math.PI / algo.divisor) * Math.PI;

      util.initialized = true;
    };

    // Combine RGB color from color channels
    util.mergeRgb = function(r, g, b) {
      r = Math.min(255, Math.round(r));
      g = Math.min(255, Math.round(g));
      b = Math.min(255, Math.round(b));
      return ((r << 16) + (g << 8) + b);
    }

    util.dimColor = function(mRgb, factor) {
      if (mRgb < 1) {
        return 0;
      }
      // split rgb into components
      var pointr = (mRgb >> 16) & 0x00FF;
      var pointg = (mRgb >> 8) & 0x00FF;
      var pointb = mRgb & 0x00FF;
      // add the color to the mapped location
      pointr *= factor;
      pointg *= factor;
      pointb *= factor;
      // set mapped point
      return util.mergeRgb(pointr, pointg, pointb);
    }

    util.getColor = function(r, g, b, mRgb)
    {
      // Stay within boundaries for the input values (do not overshoot in calculation)
      r = Math.max(0, Math.min(255, Math.round(r)));
      g = Math.max(0, Math.min(255, Math.round(g)));
      b = Math.max(0, Math.min(255, Math.round(b)));

      // split rgb in to components
      var pointr = (mRgb >> 16) & 0x00FF;
      var pointg = (mRgb >> 8) & 0x00FF;
      var pointb = mRgb & 0x00FF;

      // add the color to the algo.mapped location
      pointr += r;
      pointg += g;
      pointb += b;

      // set algo.mapped point
      return util.mergeRgb(pointr, pointg, pointb);
    }

    // Blind out towards 0 percent
    util.blindoutPercent = function(percent, sharpness)
    {
      if (undefined == sharpness) {
        sharpness = 1;
      }
      if (percent <= 0) {
        return 0;
      }
      // Normalize input
      percent = Math.min(1, percent);
      // asec consumes values > 1. asec(x) = acos(1/x)
      var factor = Math.min(1, Math.acos(1 /
        (Math.sqrt(sharpness * percent * percent) + 1)
      ) * util.halfPi);
      return factor;
    }

    util.step = function(width, height, rgb, step)
    {
      // clear algo.map data
      for (var y = 0; y < height; y++) {
        for (var x = 0; x < width; x ++) {
          util.map[y][x] = 0;
        }
      }
      
      util.progstep = step;
      util.stepPercent = util.progstep / algo.rgbMapStepCount(util.width, util.height);
      if (algo.circularMode === 6) {
        util.stepAngle = util.twoPi * util.stepPercent;
      }

      if (algo.centerRadius !== 0) {
        var offsAngle = util.twoPi * util.stepPercent;
        var direction = 1;
        if (algo.centerRadius < 0) {
          direction = -1;
        }
        var offsFactor = Math.min(util.width, util.height) * algo.centerRadius / 20;
        util.vCenterX = util.centerX + Math.sin(offsAngle) * offsFactor;
        util.vCenterY = util.centerY + direction * Math.cos(offsAngle) * offsFactor;
      }


      // Optimize multiple calculations
      var r = (rgb >> 16) & 0x00FF;
      var g = (rgb >> 8) & 0x00FF;
      var b = rgb & 0x00FF;

      // Draw the current map
      for (ry = 0; ry < height; ry++) {
        for (rx = 0; rx < width; rx++) {
          util.map[ry][rx] = util.getMapPixelColor(ry, rx, r, g, b);
        }
      }
    }

    util.getMapPixelColor = function(ry, rx, r, g, b)
    {
      var factor = 1.0;

      // calculate the offset difference of algo.map location to the float
      // location of the object
      var offx = rx - util.vCenterX;
      var offy = ry - util.vCenterY;
      var pointRadius = Math.sqrt(offx * offx + offy * offy);
      var pointAngle = geometryCalc.getAngle(offx, offy);
      // Progress the pointAngle by the step specific angle
      var stepAngle = util.twoPi * (1 - util.stepPercent);
      var angle = pointAngle + stepAngle;
      // Repeat the trigonometry based on segments to be used
      angle = angle * algo.segmentsCount;
      // Normalize the angle
      angle = (angle + util.twoPi) % util.twoPi;

      if (algo.circularMode === 1) {
        // Right Spiral
        factor = Math.atan(util.halfPi * (1 - (angle / util.twoPi)));
        factor = 1 - 0.1 / (algo.divisor / 10)
            + Math.sin(pointRadius / (0.5 + algo.getWidth()) * util.halfPi
            + util.twoPi * factor - Math.PI);
      } else if (algo.circularMode === 2) {
        // Left Spiral
        factor = Math.atan(util.halfPi * (angle / util.twoPi));
        factor = 1 - 0.1 / (algo.divisor / 10)
            + Math.sin(pointRadius / (0.5 + algo.getWidth()) * util.halfPi
            + util.twoPi * factor - Math.PI);
      } else if (algo.circularMode === 3) {
        // Right S-Curve
//        angle = util.twoPi / (1 - 0.1 / (algo.getWidth() / 10));
        var pRadius = Math.sqrt(offx * offx / algo.getWidth() + offy * offy / algo.getWidth());
        var virtualx = Math.sin(angle) * pRadius;
        var virtualy = offy;
        if (angle < Math.PI) {
          virtualy = Math.cos(angle) * pRadius + util.sOffsetFactor;
        } else {
          virtualy = Math.cos(angle) * pRadius - util.sOffsetFactor;
        }
        var sRadius = Math.sqrt(virtualx * virtualx + virtualy * virtualy);
        factor = Math.min(1.0, algo.getWidth() / 10 - 0.1) + Math.cos(sRadius);
      } else if (algo.circularMode === 4) {
        // Left S-Curve
        var pRadius = Math.sqrt(offx * offx / algo.getWidth() + offy * offy / algo.getWidth());
        var virtualx = Math.sin(angle) * pRadius;
        var virtualy = offy;
        if (angle < Math.PI) {
          virtualy = Math.cos(angle) * pRadius - util.sOffsetFactor;
        } else {
          virtualy = Math.cos(angle) * pRadius + util.sOffsetFactor;
        }
        var sRadius = Math.sqrt(virtualx * virtualx + virtualy * virtualy);
        factor = Math.min(1.0, algo.getWidth() / 10 - 0.1) + Math.cos(sRadius);
      } else if (algo.circularMode === 5) {
        // Rings Spreading
        var pRadius = Math.sqrt(offx * offx + offy * offy);
        factor = Math.min(1.0, algo.getWidth() / 10 - 0.1) +
            Math.cos(pRadius / algo.divisor - (util.twoPi * util.progstep / util.circleFactor));
      } else if (algo.circularMode === 6) {
        // Rings Rotating
        var virtualx = Math.sin(angle) * pointRadius + algo.divisor * Math.sin(util.stepAngle);
        var virtualy = Math.cos(angle) * pointRadius + algo.divisor * Math.cos(util.stepAngle);
        var vRadius = Math.sqrt(virtualx * virtualx + virtualy * virtualy);
        factor = Math.min(1.0, algo.getWidth() / 10 - 0.1) +
            Math.cos(vRadius);
      } else if (algo.circularMode === 7) {
        // Propellor
        // Calculate the relative angle to the next propellor blade
        var segmentAngle = util.twoPi / algo.segmentsCount; //algo.segmentsCount;
        var barAngle = (pointAngle + stepAngle + segmentAngle / 2) % segmentAngle - segmentAngle / 2;
        // Calculate the distance to the main angle
        var barDistance = pointRadius * Math.abs(Math.sin(barAngle));
        // Show the pixel if the distance to the main angle is in range.
        if (Math.cos(barAngle) >= 0 && barDistance <= algo.getWidth() / 2)
          factor = 1;
        else
          factor = 0;
      } else {
        // Radar
        var virtualx = Math.sin(angle) * pointRadius;
        var virtualy = Math.cos(angle) * pointRadius;

        var sidefade1 = Math.atan((algo.getWidth() - virtualx - virtualx) / algo.segmentsCount + 1);
        var sidefade2 = Math.atan((algo.getWidth() + virtualx + virtualx) / algo.segmentsCount + 1);
        var endfade = Math.atan(virtualy + 1.5);
  
        factor = endfade + sidefade1 + sidefade2 - 2;
  
        var fadeFactor = 0;
        if (algo.fadeMode === 1) {
          fadeFactor = algo.divisor * Math.atan(1.5 * (angle / util.twoPi)) - algo.divisor + 1;
        }
        else if (algo.fadeMode === 2) {
          fadeFactor = algo.divisor * Math.atan(1.5 * (1 - (angle / util.twoPi))) - algo.divisor + 1;
        }
        
        factor = Math.max(factor, fadeFactor)  
      }

      if (algo.fillMatrix === 0) {
        // circle
        var distance = Math.sqrt(offx * offx + offy * offy);
        var distPercent = distance / util.blindoutRadius;
        if (algo.circularMode === 7) {
          if (util.blindoutPercent(1 - distPercent, 5.0) < 0.3)
            factor = 0;
        } else {
          factor *= util.blindoutPercent(1 - distPercent, 5.0);
        }
      }

      if (algo.getCenterGap() !== 0) {
        var distance = Math.sqrt(offx * offx + offy * offy);
        var distPercent = (algo.getCenterGap() / 2) / distance;
        if (algo.circularMode === 7) {
          if (util.blindoutPercent(1 - distPercent, 5.0) < 0.3)
            factor = 0;
        } else {
          factor *= util.blindoutPercent(1 - distPercent, 5.0);
        }
      }

      // Normalize the factor      
      factor = Math.min(1, Math.max(0, factor));
      return util.getColor(r * factor, g * factor, b * factor, util.map[ry][rx]);
    }

    algo.rgbMap = function(width, height, rgb, step)
    {
      if (util.initialized === false || width != util.width || height != util.height)
      {
          util.initialize(width, height);
      }
      
      util.step(width, height, rgb, step);

      return util.map;
    };

    algo.rgbMapStepCount = function(width, height)
    {
      // Be sure to have sufficient steps for seamless circling
      // Fix the value to align circle times between different matrix sizes
      // 100 steps at a smooth 100ms timing rotate in 10s.
      return 100;
    };

    // Development tool access
    testAlgo = algo;

    return algo;
    }
)();
