/*
  Q Light Controller Plus
  snowbubbles.js

  Copyright (c) Hans-Jürgen Tappe
  Derived from starfield.js, Copyright (c) Doug Puckett

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
  function () {
    var algo = {};
    algo.apiVersion = 2;
    algo.name = "Snow or Bubbles";
    algo.author = "Hans-Jürgen Tappe";
    algo.properties = [];
    algo.acceptColors = 1;
    algo.presetColor = 0x000000;
    // number of flakes on screen at one time (default)
    algo.presetflakes = 20;
    algo.properties.push("name:presetflakes|type:range|display:Number of Flakes (10-255)|values:10,255|write:setAmount|read:getAmount");
    algo.windchill = 0;
    algo.properties.push("name:windchill|type:list|display:Windchill|values:No,Yes|write:setWindchill|read:getWindchill");
    algo.reverse = 0;
    algo.properties.push("name:reverse|type:list|display:Bottom-to-top|values:No,Yes|write:setReverse|read:getReverse");
    // Multicolor flakes off defaultly (1 = flakes will be randomly colored)
    algo.multiColor = 0;
    algo.properties.push("name:multiColor|type:list|display:MultiColored flakes?|values:No,Yes|write:setMulti|read:getMulti");
    // depth - best not to change
    var depth = 5;
    // main flake position array
    var flakes = new Array(255);
    algo.speedX = 0;

    algo.setAmount = function (_amount) {
      algo.presetflakes = _amount;
    };

    algo.getAmount = function () {
      return algo.presetflakes;
    };

    algo.setMulti = function(_multic)
    {
      if (_multic === "Yes") {
        // Random Colored flakes
        algo.multiColor = 1;
      } else {
        // flakes are chosen color
        algo.multiColor = 0;
      }
    };

    algo.getMulti = function()
    {
      if (algo.multiColor === 1) {
        return "Yes";
      } else {
        return "No";
      }
    };

    algo.setWindchill = function(_wind)
    {
      if (_wind === "Yes") {
        // Random Colored flakes
        algo.windchill = 1;
      } else {
        // flakes are chosen color
        algo.windchill = 0;
      }
    };

    algo.getWindchill = function()
    {
      if (algo.windchill === 1) {
        return "Yes";
      } else {
        return "No";
      }
    };

    algo.setReverse = function(_reverse)
    {
      if (_reverse === "Yes") {
        // Random Colored flakes
        algo.reverse = 1;
      } else {
        // flakes are chosen color
        algo.reverse = 0;
      }
    };

    algo.getReverse = function()
    {
      if (algo.reverse === 1) {
        return "Yes";
      } else {
        return "No";
      }
    };

    var util = new Object;
    algo.initialized = false;

    // random position function for new flake
    function getNewNumberRange(minVal, maxVal) {
      // Search in the range of min to max + 1
      // which will be reduced by random() excluding 1
      // and floor() reducing to lower number.
      return Math.floor(Math.random() * (maxVal + 1 - minVal)) + minVal;
    }

    // set color of flake - if multicolor, choose random color. If not
    // random, return user chosen color
    function getNewColor(isMultiColor, zColor) {
      if (isMultiColor === 1) {
        // random red level
        var tr = Math.round(Math.random() * 255);
        // random green level
        var tg = Math.round(Math.random() * 255);
        // random blue level
        var tb = Math.round(Math.random() * 255);
        // returned combined color
        return (tr << 16) + (tg << 8) + tb;
      }
      else {
        // If not multicolor, return chosen color
        return zColor;
      }
    }

    // update x speed of flakes
    function updateFlakeSpeedx(currentSpeed) {
      if (algo.windchill === 1) {
        var r = Math.random() - 0.5;
        var factor = Math.sqrt(Math.abs(r));
        if (r < 0) {
          factor = -1 * factor;
        }
        return Math.min(0.4, Math.max(-0.4, currentSpeed + factor));
      } else {
        return 0;
      }
    }

    // get y speed of flake
    function getFlakeSpeedy(depth) {
      var s = (1 / (depth + 1))
      if (algo.reverse === 0) {
        return s;
      } else {
        return (-1 * s);
      }
    }

    // initialize the flakes and load random positions
    util.initialize = function (width, height) {
       for (var i = 0; i < flakes.length; i++) {
        flakes[i] = {
          x: getNewNumberRange(0, width - 1),
          y: getNewNumberRange(0, height - 1),
          z: getNewNumberRange(0, depth - 1),
          c: getNewColor(algo.multiColor, algo.presetColor),
          s: 1,
        };
        flakes[i].s = getFlakeSpeedy(flakes[i].z);
      }

      algo.initialized = true;
      return;
    };

    // main QLC+ routine where the work is done
    algo.rgbMap = function (width, height, rgb, step) {
      if (algo.initialized === false) {
        util.initialize(width, height);
      }

      // Clear map data for a blank map
      var map = new Array(height);
      for (var y = 0; y < height; y++) {
        map[y] = new Array();
        for (var x = 0; x < width; x++) {
          map[y][x] = 0;
        }
      }
      
      algo.speedX = updateFlakeSpeedx(algo.speedX);

      // Start moving the flakes by looping through this routine,
      // addressing each flake individually (i is the flake number)
      for (var i = 0; i < algo.presetflakes; i++) {
        // create a new position for the flake
        if (flakes[i].y > height || flakes[i].y < 0) {
          flakes[i].x = getNewNumberRange(0, width - 1);
          if (algo.reverse === 0) {
            flakes[i].y = 0;
          } else {
            flakes[i].y = height - 1;
          }
          flakes[i].z = getNewNumberRange(0, depth - 1);
          flakes[i].c = getNewColor(algo.multiColor,rgb);
          flakes[i].s = getFlakeSpeedy(flakes[i].z);
        }

        // calculate the flakes next position
        var px = flakes[i].x;
        var py = flakes[i].y;

        // make sure flake is on the screen
        if (px >= 0 && px < width && py >= 0 && py < height) {
          // get rid of x and y position fractions
          px = Math.floor(px);
          py = Math.floor(py);

          // parse out individual colors of current flake color
          var r = (flakes[i].c >> 16) & 0x00FF;
          var g = (flakes[i].c >> 8) & 0x00FF;
          var b = flakes[i].c & 0x00FF;

          // if flake is far away, then it should be darker
          var colorLevel = 1 - (flakes[i].z / depth);
          
          // Get the current channel values
          var pRGB = map[py][px];
          var pr = (pRGB >> 16) & 0x00FF;
          var pg = (pRGB >> 8) & 0x00FF;
          var pb = pRGB & 0x00FF;
          
          // Adjust flake brightness level based on how far away it is
          // and by chosen flake color
          r = Math.floor(r * colorLevel);
          g = Math.floor(g * colorLevel);
          b = Math.floor(b * colorLevel);
          
          // The higher value wins
          r = Math.max(r, pr);
          g = Math.max(g, pg);
          b = Math.max(b, pb);

          // put all the individual rgb colors back together
          pRGB = (r << 16) + (g << 8) + b;

          // store the flake's combined color in the map
          map[py][px] = pRGB;
        }
        
        // new position of flake
        flakes[i].x += algo.speedX;
        flakes[i].y += flakes[i].s;
      }
      // return the map back to QLC+
      return map;
    };

    algo.rgbMapStepCount = function (width, height) {
      return 2;
    };

  // Development tool access
  testAlgo = algo;

  return algo;
}
)();
