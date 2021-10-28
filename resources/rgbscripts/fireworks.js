/*
  Q Light Controller Plus
  fireworks.js

  Copyright (c) Hans-Jürgen Tappe
  derived from on balls.js, Copyright (c) Tim Cullingworth

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
    var util = new Object;

    var algo = new Object;
    algo.apiVersion = 2;
    algo.name = "Fireworks";
    algo.author = "Hans-Jürgen Tappe";
    algo.acceptColors = 2;
    algo.properties = new Array();
    algo.rstepcount = 0;
    algo.gstepcount = 50;
    algo.bstepcount = 100;
    algo.initialized = false;

    algo.presetSize = 5;
    algo.properties.push("name:presetSize|type:range|display:Size|values:1,20|write:setSize|read:getSize");
    algo.presetNumber = 3;
    algo.properties.push("name:presetNumber|type:range|display:Number|values:1,50|write:setNumber|read:getNumber");
    algo.randomColor = 1;
    algo.properties.push("name:randomColor|type:list|display:Random Colour|values:No,Yes|write:setRandom|read:getRandom");
    algo.sparkle = 0;
    algo.properties.push("name:sparkle|type:list|display:Sparkling|values:No,Yes|write:setSparkling|read:getSparkling");
    algo.triggerPoint = 3;
    algo.properties.push("name:triggerPoint|type:range|display:Trigger Point|values:0,5|write:setTrigger|read:getTrigger");

    algo.setSize = function(_size)
    {
      algo.presetSize = _size;
    };

    algo.getSize = function()
    {
      return algo.presetSize;
    };

    algo.setNumber = function(_step)
    {
      algo.presetNumber = _step;
      algo.initialized = false;
    };

    algo.getNumber = function()
    {
      return algo.presetNumber;
    };

    algo.setRandom = function(_random)
    {
      if (_random === "Yes") {
        algo.randomColor = 0;
      } else if (_random === "No") {
        algo.randomColor = 1;
      }
    };

    algo.getRandom = function()
    {
      if (algo.randomColor === 0) {
        return "Yes";
      } else if (algo.randomColor === 1) {
        return "No";
      }
    };

    algo.setSparkling = function(_sparkle)
    {
      if (_sparkle === "Yes") {
        algo.sparkling = 1;
      } else {
        algo.sparkling = 0;
      }
    };

    algo.getSparkling = function()
    {
      if (algo.sparkling === 1) {
        return "Yes";
      } else {
        return "No";
      }
    };

    algo.setTrigger = function(_trigger)
    {
      algo.triggerPoint = _trigger;
    };

    algo.getTrigger = function()
    {
      return algo.triggerPoint;
    };

    // random position function for new rocket
    util.getNewNumberRange = function(minVal, maxVal) {
      // Search in the range of min to max + 1
      // which will be reduced by random() excluding 1
      // and floor() reducing to lower number.
      return Math.floor(Math.random() * (maxVal + 1 - minVal)) + minVal;
    }

    // Combine RGB color from color channels
    util.mergeRgb = function(r, g, b) {
      r = Math.min(255, Math.round(r));
      g = Math.min(255, Math.round(g));
      b = Math.min(255, Math.round(b));
      return ((r << 16) + (g << 8) + b);
    }
    
    util.getColor = function(r, g, b, mRgb) {
      // split rgb in to components
      var pointr = (mRgb >> 16) & 0x00FF;
      var pointg = (mRgb >> 8) & 0x00FF;
      var pointb = mRgb & 0x00FF;
      // add the color to the mapped location
      pointr += r;
      pointg += g;
      pointb += b;
      // set mapped point
      return util.mergeRgb(pointr, pointg, pointb);
    }

    util.dimColor = function(mRgb, factor) {
      if (mRgb < 1) {
        return 0;
      }
      // split rgb in to components
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

    util.initialize = function(width, height)
    {
      algo.rocket = new Array(algo.presetNumber);

      // Clear map data
      util.map = new Array(height);
      for (var y = 0; y < height; y++) {
        util.map[y] = new Array();
        for (var x = 0; x < width; x++) {
          util.map[y][x] = 0;
        }
      }

      for (var i = 0; i < algo.presetNumber; i++) {
        algo.rocket[i] = {
          x: 0,
          y: height + algo.presetSize,
          xDirection: 0,
          yDirection: height,
          trigger: 0,
          r: 0,
          g: 0,
          b: 0,
        };
      }

      algo.initialized = true;
      return;
    };

    algo.rgbMap = function(width, height, rgb, progstep)
    {
      if (algo.initialized === false) {
        util.initialize(width, height);
      }

      // Dim map data
      for (var y = 0; y < height; y++) {
        for (var x = 0; x < width; x++) {
          util.map[y][x] = util.dimColor(util.map[y][x], 0.8);
        }
      }

      // for each rocket displayed
      for (var i = 0; i < algo.presetNumber; i++) {
        // create a new position for the rocket
        if (algo.rocket[i].y - Math.round(algo.presetSize / 2) > height ||
            algo.rocket[i].y + Math.round(algo.presetSize / 2) < 0 ||
            algo.rocket[i].x - Math.round(algo.presetSize / 2) > width ||
            algo.rocket[i].x + Math.round(algo.presetSize / 2) < 0) {
          algo.rocket[i].x = util.getNewNumberRange(Math.round(width / 5), Math.round(4 * width / 5));
          algo.rocket[i].y = height;
          algo.rocket[i].yDirection = util.getNewNumberRange(3, 5) / 3;
          algo.rocket[i].xDirection = (algo.rocket[i].x - (width / 2)) / (width * height) *
              (algo.rocket[i].yDirection);
          if (algo.triggerPoint == 0) {
            algo.rocket[i].trigger = height;
          } else {
            var min = Math.floor(height / algo.triggerPoint);
            var max = Math.floor(height - (height / algo.triggerPoint));
            algo.rocket[i].trigger = util.getNewNumberRange(min, max);
          }
          if (algo.randomColor === 0) {
            do {
              // Chose random colour for each rocket
              algo.rocket[i].r = Math.round(Math.random() * 255);
              algo.rocket[i].g = Math.round(Math.random() * 255);
              algo.rocket[i].b = Math.round(Math.random() * 255);
              // try again if it is too dim
            } while ((algo.rocket[i].r + algo.rocket[i].g + algo.rocket[i].b) < 125);
          } else {
            algo.rocket[i].r = (rgb >> 16) & 0x00FF;
            algo.rocket[i].g = (rgb >> 8) & 0x00FF;
            algo.rocket[i].b = rgb & 0x00FF;
          }
        }

        // workout closest map location for rocket
        var mx = Math.floor(algo.rocket[i].x);
        var my = Math.floor(algo.rocket[i].y);
        
        // area size to draw rocket
        var currentSize = 0.5;
        if (my <= Math.floor(algo.rocket[i].trigger)) {
          // Use the full size after trigger has been reached.
//          currentSize = algo.presetSize;
        }
        var boxRadius = boxRadius = Math.round(currentSize / 2);
        
        // area for faded edges
        for (var ry = my - boxRadius; ry < my + boxRadius + 2; ry++) {
          for (var rx = mx - boxRadius; rx < mx + boxRadius + 2; rx++) {
            // Draw only if edges are on the map
            if (rx < width && rx > -1 && ry < height && ry > -1) {
              // Draw the box for debugging.
              //util.map[ry][rx] = util.getColor(45, 45, 45, 0);
                var offx = rx - algo.rocket[i].x;
                var offy = ry - algo.rocket[i].y;
                var hyp = Math.max(0, 1 - Math.abs(Math.sqrt( (offx * offx) + (offy * offy)) / (boxRadius + 1)));
                var sparkle = 1;
                if (algo.sparkling === 1 &&
                    Math.floor(algo.rocket[i].trigger + 1) != ry &&
                    my <= Math.floor(algo.rocket[i].trigger)) {
                  sparkle = Math.random();
                  if (hyp > 0) {
                    hyp = 1;
                  }
                }
                var pointr = Math.round(algo.rocket[i].r * hyp * sparkle);
                var pointg = Math.round(algo.rocket[i].g * hyp * sparkle);
                var pointb = Math.round(algo.rocket[i].b * hyp * sparkle);
                util.map[ry][rx] = util.getColor(pointr, pointg, pointb, util.map[ry][rx]);
            }
          }
        }

        // set rocket's next center location
        algo.rocket[i].x += algo.rocket[i].xDirection;
        algo.rocket[i].y -= algo.rocket[i].yDirection;
        // Carry away the rocket centers
        algo.rocket[i].xDirection = algo.rocket[i].xDirection *
            (1 + algo.rocket[i].yDirection / 30);
      }
      return util.map;
    };

    algo.rgbMapStepCount = function(width, height)
    {
      return 1024; // This make no difference to the script
    };

    // Development tool access
    testAlgo = algo;

    return algo;
  }
)();
