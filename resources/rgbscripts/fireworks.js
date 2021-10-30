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
    var algo = new Object;
    algo.apiVersion = 2;
    algo.name = "Fireworks";
    algo.author = "Hans-Jürgen Tappe";
    algo.acceptColors = 2;
    algo.properties = new Array();
    algo.initialized = false;

    algo.rocketsCount = 3;
    algo.properties.push("name:rocketCount|type:range|display:Count|values:1,50|write:setCount|read:getCount");
    algo.randomColor = 1;
    algo.properties.push("name:randomColor|type:list|display:Random Color|values:No,Yes|write:setRandom|read:getRandom");
    algo.triggerPoint = 3;
    algo.properties.push("name:triggerPoint|type:range|display:Trigger Point|values:0,5|write:setTrigger|read:getTrigger");

    algo.particleCount = 10;

    algo.setCount = function(_count)
    {
      algo.rocketsCount = _count;
      algo.initialized = false;
    };

    algo.getCount = function()
    {
      return algo.rocketsCount;
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

    algo.setTrigger = function(_trigger)
    {
      algo.triggerPoint = _trigger;
    };

    algo.getTrigger = function()
    {
      return algo.triggerPoint;
    };

    algo.rgbMap = function(width, height, rgb, progstep)
    {
      if (algo.initialized === false) {
        util.initialize(width, height);
      }

      // Dim current map data
      util.map = util.map.map(col => {
	      return col.map(pxl => {
		      return util.dimColor(pxl, 0.8);
        })
      });

      // Initialize each rocket displayed
      for (var i = 0; i < algo.rocketsCount; i++) {
        // create a new position for the rocket if the rocket has reached map borders
        if (algo.rockets[i].y > height ||
            algo.rockets[i].y < 0 ||
            algo.rockets[i].x > width ||
            algo.rockets[i].x < 0) {
          algo.rockets[i].initialized = false;
        }
        
        // Initialize the rocket
        if (!algo.rockets[i].initialized) {
          util.initializeRocket(i, width, height, rgb);
        }

        // Trigger the rocket
        if (! algo.rockets[i].triggered &&
            Math.floor(algo.rockets[i].y) <= Math.floor(algo.rockets[i].triggerPoint)) {
          // switch to particles mode
          algo.rockets[i].triggered = true;
          // initialize the particles
          for (var j = 0; j < algo.particleCount; j++) {
            algo.rockets[i].particle[j].x = algo.rockets[i].x;
            algo.rockets[i].particle[j].y = algo.rockets[i].y;
          }
        }

        // Draw the rocket
        if (! algo.rockets[i].triggered) {
          util.drawObject(algo.rockets[i].x, algo.rockets[i].y,
            width, height,
            algo.rockets[i].r, algo.rockets[i].g, algo.rockets[i].b);
        } else {
          // draw the particles
          for (var j = 0; j < algo.particleCount; j++) {
          }
        }

        // set rocket's next center location
        algo.rockets[i].x += algo.rockets[i].xDirection;
        algo.rockets[i].y -= algo.rockets[i].yDirection;
        // Carry away the rocket centers
        algo.rockets[i].xDirection = algo.rockets[i].xDirection *
          (1 + algo.rockets[i].yDirection / 30);
      }
      return util.map;
    };

    algo.rgbMapStepCount = function(width, height)
    {
      return 1024; // This make no difference to the script
    };

    // --------------------------------
    // Helper Utilities & Functions
    var util = new Object;

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

    util.drawObject = function(x, y, w, h, r, g, b) {
      // workout closest map location for rocket
      var mx = Math.floor(x);
      var my = Math.floor(y);
      for (var ry = my; ry < my + 2; ry++) {
        for (var rx = mx; rx < mx + 2; rx++) {
          // Draw only if edges are on the map
          if (rx < w && rx > -1 && ry < h && ry > -1) {
            // Draw the box for debugging.
            //util.map[ry][rx] = util.getColor(45, 45, 45, 0);
            var offx = rx - x;
            var offy = ry - y;
            var hyp = Math.max(0, 1 - Math.abs(Math.sqrt( (offx * offx) + (offy * offy))));
            var pointr = Math.round(r * hyp);
            var pointg = Math.round(g * hyp);
            var pointb = Math.round(b * hyp);
            util.map[ry][rx] = util.getColor(pointr, pointg, pointb, util.map[ry][rx]);
          }
        }
      }
    }
    
    util.initializeRocket = function(i, w, h, rgb) {
      // reset height and set a start x location
      algo.rockets[i].x = util.getNewNumberRange(Math.round(w / 5), Math.round(4 * w / 5));
      algo.rockets[i].y = h;
      // determine the x,y speed
      algo.rockets[i].yDirection = util.getNewNumberRange(3, 5) / 3;
      algo.rockets[i].xDirection = (algo.rockets[i].x - (w / 2)) / (w * h) *
          (algo.rockets[i].yDirection);
      // Set the rocket to raise mode, untriggered.
      algo.rockets[i].triggered = false;
      // Initialize the rocket trigger point
      if (algo.triggerPoint == 0) {
        algo.rockets[i].triggerPoint = h;
      } else {
        var min = Math.floor(h / algo.triggerPoint);
        var max = Math.floor(h - (h / algo.triggerPoint));
        algo.rockets[i].triggerPoint = util.getNewNumberRange(min, max);
      }
      // initialize the rocket color
      if (algo.randomColor === 0) {
        do {
          // Chose random colour for each rocket
          algo.rockets[i].r = Math.round(Math.random() * 255);
          algo.rockets[i].g = Math.round(Math.random() * 255);
          algo.rockets[i].b = Math.round(Math.random() * 255);
          // try again if it is too dim
        } while ((algo.rockets[i].r + algo.rockets[i].g + algo.rockets[i].b) < 125);
      } else {
        algo.rockets[i].r = (rgb >> 16) & 0x00FF;
        algo.rockets[i].g = (rgb >> 8) & 0x00FF;
        algo.rockets[i].b = rgb & 0x00FF;
      }
      // initialize particles
      algo.rockets[i].particle = new Array();
      for (var j = 0; j < algo.particleCount; j++) {
        algo.rockets[i].particle[j] = new Object();
      }
      
      // Set rocket status to initialized
      algo.rockets[i].initialized = true;
    }

    util.initialize = function(width, height)
    {
      algo.rockets = new Array(algo.rocketsCount);
      for (var i = 0; i < algo.rocketsCount; i++) {
        algo.rockets[i] = new Object();
        algo.rockets[i].initialized = false;
      }

      // Clear map data
      util.map = new Array(height);
      for (var y = 0; y < height; y++) {
        util.map[y] = new Array();
        for (var x = 0; x < width; x++) {
          util.map[y][x] = 0;
        }
      }

      for (var i = 0; i < algo.rocketsCount; i++) {
        algo.rockets[i] = {
          x: 0,
          y: height,
          xDirection: 0,
          yDirection: height,
          triggerPoint: 0,
          triggered: false,
          r: 0,
          g: 0,
          b: 0,
        };
      }

      algo.initialized = true;
      return;
    };

    // Development tool access
    testAlgo = algo;

    return algo;
  }
)();
