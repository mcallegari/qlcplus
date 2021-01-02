/*
  Q Light Controller Plus
  xmasstars.js

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
    algo.name = "Xmas Stars";
    algo.author = "Hans-Jürgen Tappe";
    algo.acceptColors = 2;
    algo.properties = new Array();
    algo.rstepcount = 0;
    algo.gstepcount = 50;
    algo.bstepcount = 100;

    algo.presetSize = 5;
    algo.properties.push("name:presetSize|type:range|display:Size|values:5,20|write:setSize|read:getSize");
    algo.presetNumber = 3;
    algo.properties.push("name:presetNumber|type:range|display:Number|values:1,50|write:setNumber|read:getNumber");
    algo.presetRandom = 1;
    algo.properties.push("name:presetRandom|type:list|display:Random Colour|values:No,Yes|write:setRandom|read:getRandom");
    algo.presetCollision = 1;
    algo.properties.push("name:presetCollision|type:list|display:Self Collision|values:No,Yes|write:setCollision|read:getCollision");
    var util = new Object;
    algo.initialized = false;

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
        algo.presetRandom = 0;
      } else if (_random === "No") {
        algo.presetRandom = 1;
      }
    };

    algo.getRandom = function()
    {
      if (algo.presetRandom === 0) {
        return "Yes";
      } else if (algo.presetRandom === 1) {
        return "No";
      }
    };

    algo.setCollision = function(_collision)
    {
      if (_collision === "Yes") {
        algo.presetCollision = 0;
      } else if (_collision === "No") {
        algo.presetCollision = 1;
      }
    };

    algo.getCollision = function()
    {
      if (algo.presetCollision === 0) {
        return "Yes";
      } else if (algo.presetCollision === 1) {
        return "No";
      }
    };

    // Combine RGB color from color channels
    function mergeRgb(r, g, b) {
      r = Math.min(255, Math.round(r));
      g = Math.min(255, Math.round(g));
      b = Math.min(255, Math.round(b));
      return ((r << 16) + (g << 8) + b);
    }
    
    function getColor(r, g, b, mRgb) {
      // split rgb in to components
      var pointr = (mRgb >> 16) & 0x00FF;
      var pointg = (mRgb >> 8) & 0x00FF;
      var pointb = mRgb & 0x00FF;
      // add the color to the mapped location
      pointr += r;
      pointg += g;
      pointb += b;
      // set mapped point
      return mergeRgb(pointr, pointg, pointb);
    }

    util.initialize = function(width, height)
    {
      algo.star = new Array(algo.presetNumber);

      for (var i = 0; i < algo.presetNumber; i++) {
        algo.star[i] = {
          x: Math.random() * (width - 1),
          y: Math.random() * (height - 1),
          xDirection: (Math.random() * 2) - 1,
          yDirection: (Math.random() * 2) - 1,
          r: 0,
          g: 0,
          b: 0,
        };
        do {
          // Chose random colour for each star
          algo.star[i].r = Math.round(Math.random() * 255);
          algo.star[i].g = Math.round(Math.random() * 255);
          algo.star[i].b = Math.round(Math.random() * 255);
          // try again if it is too dim
        } while ((algo.star[i].r + algo.star[i].g + algo.star[i].b) < 125);
      }

      algo.initialized = true;
      return;
    };

    algo.rgbMap = function(width, height, rgb, progstep)
    {
      if (algo.initialized === false) {
        util.initialize(width, height);
      }

      // Clear map data
      var map = new Array(height);
      for (var y = 0; y < height; y++) {
        map[y] = new Array();
        for (var x = 0; x < width; x++) {
          map[y][x] = 0;
        }
      }

      // area size to draw star
      var boxRadius = Math.round(algo.presetSize / 2);
      var realsize = Math.floor(algo.presetSize / 2) * 2;

      // for each star displayed
      for (var i = 0; i < algo.presetNumber; i++) {
        // workout closest map location for star
        var mx = Math.floor(algo.star[i].x);
        var my = Math.floor(algo.star[i].y);
        var realsize = Math.floor(algo.presetSize / 2) * 2 + 1;

        var r = algo.star[i].r;
        var g = algo.star[i].g;
        var b = algo.star[i].b;
        if (algo.presetRandom != 0) {
          r = (rgb >> 16) & 0x00FF;
          g = (rgb >> 8) & 0x00FF;
          b = rgb & 0x00FF;
        }

        // area for faded edges
        for (var ry = my - boxRadius; ry < my + boxRadius + 2; ry++) {
          for (var rx = mx - boxRadius; rx < mx + boxRadius + 2; rx++) {
            // Draw only if edges are on the map
            if (rx < width && rx > -1 && ry < height && ry > -1) {
              // Draw the box for debugging.
              // map[ry][rx] = getColor(20, 20, 20, map[ry][rx]);

              // calculate the offset difference of map location to the float
              // location of the tree
              var offx = Math.abs(Math.round(rx - algo.star[i].x));
              var offy = Math.abs(Math.round(ry - algo.star[i].y));

              // 9
              // - - - - X - - - -
              // - - - X - X - - -
              // X X X - - - X X X
              // - X - - - - - X -
              // - - X - - - X - -
              // - X - - - - - X -
              // X X X - - - X X X
              // - - - X - X - - -
              // - - - - X - - - -

              // 7
              // - - - X - - -
              // X - X - X - X
              // - X - - - X -
              // - - X - X - -
              // - X - - - X -
              // X - X - X - X
              // - - - X - - -

              // 5
              // - - X - -
              // X - - - X
              // - X - X -
              // X - - - X
              // - - X - -

              // Calculate color pixel positions
              if ( // The tips
                  // Top and bottom tips
                  (offy > Math.floor(realsize / 4) && offy <= realsize / 2 &&
                      offx <= Math.floor(realsize / 2) - offy) ||
                  // Outer tips
                  (offy >= 0 && offy <= Math.floor(realsize / 4) &&
                      offx <= offy + Math.round(realsize / 4))
                ) {
                // add the star color to the mapped location
                map[ry][rx] = getColor(r, g, b, map[ry][rx]);
              }
            }
          }
        }

        // if collision detection is on
        if (algo.presetCollision === 0) {
          // star collision detection
          // check all stars
          for (var ti = 0; ti < algo.presetNumber; ti++) {
            // but not the current one
            if (ti !== i) {
              // calculate distance to current star
              var disx = (algo.star[i].x + algo.star[i].xDirection) - algo.star[ti].x;
              var disy = (algo.star[i].y + algo.star[i].yDirection) - algo.star[ti].y;
              var dish = Math.sqrt((disx * disx) + (disy * disy));
              // if to close
              if (dish < (1.414) * (algo.presetSize / 2)) {
                // swap speed / direction of current star
                // with star that is too close
                var stepx = algo.star[i].xDirection;
                var stepy = algo.star[i].yDirection;
                algo.star[i].xDirection = algo.star[ti].xDirection;
                algo.star[i].yDirection = algo.star[ti].yDirection;
                algo.star[ti].xDirection = stepx;
                algo.star[ti].yDirection = stepy;
              }
            }
          }
        }

        // edge collision detection
        if (algo.star[i].y <= 0 && algo.star[i].yDirection < 0) {
          // top edge and moving up
          algo.star[i].yDirection *= -1;
        } else if (algo.star[i].y >= height - 1 && algo.star[i].yDirection > 0) {
          // bottom edge and moving down
          algo.star[i].yDirection *= -1;
        }

        if (algo.star[i].x <= 0 && algo.star[i].xDirection < 0) {
          // left edge and moving left
          algo.star[i].xDirection *= -1;
        } else if (algo.star[i].x >= width - 1 && algo.star[i].xDirection > 0) {
          // right edge and moving right
          algo.star[i].xDirection *= -1;
        }

        // set star's next location
        algo.star[i].x += algo.star[i].xDirection;
        algo.star[i].y += algo.star[i].yDirection;
      }

      return map;
    };

    algo.rgbMapStepCount = function(width, height)
    {
      return width * height; // This make no diferance to the script ;-)
    };

    // Development tool access
    testAlgo = algo;

    return algo;
  }
)();
