/*
  Q Light Controller Plus
  flyingObjects.js

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

(function()
  {
    var algo = new Object;
    algo.apiVersion = 2;
    algo.name = "Flying Objects";
    algo.author = "Hans-Jürgen Tappe";
    algo.acceptColors = 2;
    algo.properties = new Array();

    algo.presetSize = 5;
    algo.properties.push("name:presetSize|type:range|display:Size|values:5,20|write:setSize|read:getSize");
    algo.presetNumber = 3;
    algo.properties.push("name:presetNumber|type:range|display:Number|values:1,50|write:setNumber|read:getNumber");
    algo.presetRandom = 1;
    algo.properties.push("name:presetRandom|type:list|display:Random Colour|values:No,Yes|write:setRandom|read:getRandom");
    algo.presetCollision = 1;
    algo.properties.push("name:presetCollision|type:list|display:Self Collision|values:No,Yes|write:setCollision|read:getCollision");
	  algo.selectedAlgo = "trees";
  	algo.properties.push("name:selectedAlgo|type:list|display:Objects|"
			+ "values:Balls,Snowman,Ufo,Xmas Stars,Xmas Trees|"
			+ "write:setSelectedAlgo|read:getSelectedAlgo");

    algo.initialized = false;
    algo.boxRadius = 1;
    algo.realsize = 1;

    var util = new Object;

    // Algorithms ----------------------------

    var ballsAlgo = new Object;
    ballsAlgo.getMapPixelColor = function(i, rx, ry, r, g, b)
    {
      // calculate the offset difference of algo.map location to the float
      // location of the object
      var offx = rx - algo.obj[i].x;
      var offy = ry - algo.obj[i].y;
      var factor = 1 - (Math.sqrt( (offx * offx) + (offy * offy))/((algo.presetSize/2)+1));
      if (factor < 0) {
        factor = 0;
      }
      return getColor(r * factor, g * factor, b * factor, algo.map[ry][rx]);
    }

    var snowmanAlgo = new Object;
    snowmanAlgo.getMapPixelColor = function(i, rx, ry, r, g, b)
    {
      // calculate the offset difference of algo.map location to the float
      // location of the object
      var offx = rx - algo.obj[i].x;
 
      var size = algo.presetSize * 2 / 5;
      var offy = ry - algo.obj[i].y + (algo.presetSize - size) / 2;

      var factor1 = 1 - (Math.sqrt((offx * offx) + (offy * offy)) / ((size / 2) + 1));
      // Set a bit towards background by dimming.
      if (factor1 < 0) {
        factor1 = 0;
      }

      size = algo.presetSize * 4 / 5;
      offy = ry - algo.obj[i].y - (algo.presetSize - size) / 2;

      var factor2 = 1 - (Math.sqrt((offx * offx) + (offy * offy)) / ((size / 2) + 1));
      if (factor2 < 0) {
        factor2 = 0;
      }

      // Merge the two balls
      var factor = Math.max(factor1, factor2);
      if (factor > 1) {
        factor = 1;
      }

      return getColor(r * factor, g * factor, b * factor, algo.map[ry][rx]);
    }

    var ufoAlgo = new Object;
    ufoAlgo.getMapPixelColor = function(i, rx, ry, r, g, b)
    {
      var offx = rx - algo.obj[i].x;
 
      var size = algo.presetSize * 2 / 5;
      var offy1 = ry - algo.obj[i].y;

      var factor1 = 1 - (Math.sqrt((offx * offx) + (offy1 * offy1)) / ((size / 2) + 1));
      // Set a bit towards background by dimming.
      if (factor1 < 0) {
        factor1 = 0;
      }
       // factor1 = 0;

      var factor2 = 0;
      var offy2 = ry - algo.obj[i].y - size / 4;
      if (offy2 <= 0) {
        size = algo.presetSize;
        factor2 = 1 - (Math.sqrt((offx * offx) + (offy2 * offy2) * (size / 1.5)) / ((size / 2) + 1));
        if (factor2 < 0) {
          factor2 = 0;
        }
      }

      var factor = factor1 + factor2;
      if (factor > 1) {
        factor = 1;
      }

      return getColor(r * factor, g * factor, b * factor, algo.map[ry][rx]);

    }

    var treesAlgo = new Object;
    treesAlgo.getMapPixelColor = function(i, rx, ry, r, g, b)
    {
      // calculate the offset difference of algo.map location to the float
      // location of the object
      var offx = rx - algo.obj[i].x;
      var offy = ry - algo.obj[i].y;

      // Calculate color intensity
      if (ry === Math.floor(algo.obj[i].y) + algo.boxRadius + 1) {
        // The tree foot:
        // offx remains the same.
        // offy is 0
        offy = 0;
      } else {
        // Offset to bottom
        offy += (algo.presetSize / 2);
      }
      var factor = ((1 - (Math.sqrt((offx * offx * 1.8) + (offy * offy)))) * 2.5)
          - 0 + offy * 3;
      factor = Math.min(1, Math.max(0, factor));
    
      // add the object color to the algo.mapped location
      return getColor(r * factor, g * factor, b * factor, algo.map[ry][rx]);
    };
      
    var starsAlgo = new Object;
    starsAlgo.getMapPixelColor = function(i, rx, ry, r, g, b)
    {
      // calculate the offset difference of algo.map location to the float
      // location of the object
      var offx = Math.abs(Math.round(rx - algo.obj[i].x));
      var offy = Math.abs(Math.round(ry - algo.obj[i].y));

      // Calculate color pixel positions
      if ( // The tips
        // Top and bottom tips
        (offy > Math.floor(algo.realsize / 4) && offy <= algo.realsize / 2 &&
          offx <= Math.floor(algo.realsize / 2) - offy) ||
        // Outer tips
        (offy >= 0 && offy <= Math.floor(algo.realsize / 4) &&
          offx <= offy + Math.round(algo.realsize / 4))
      ) {
        // add the color to the algo.mapped location
        return getColor(r, g, b, algo.map[ry][rx]);
      }
        return algo.map[ry][rx];
    }

    // Setters and Getters ------------------------

    algo.setSize = function(_size)
    {
      algo.presetSize = _size;
      algo.initialized = false;
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
      algo.initialized = false;
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

    algo.setSelectedAlgo= function(_selected)
    {
      if (_selected === "Xmas Stars") {
        algo.selectedAlgo = "stars";
      } else if (_selected === "Balls") {
        algo.selectedAlgo = "balls";
      } else if (_selected === "Snowman") {
        algo.selectedAlgo = "snowman";
      } else if (_selected === "Ufo") {
        algo.selectedAlgo = "ufo";
      } else {
        algo.selectedAlgo = "trees";
      }
    };

    algo.getSelectedAlgo = function()
    {
      if (algo.selectedAlgo === "stars") {
        return "Xmas Stars";
      } else if (algo.selectedAlgo === "balls") {
        return "Balls";
      } else if (algo.selectedAlgo === "snowman") {
        return "Snowman";
      } else if (algo.selectedAlgo === "ufo") {
        return "Ufo";
      } else {
        return "Xmas Trees";
      }
    };

    // General Purpose Functions ------------------

    // Combine RGB color from color channels
    function mergeRgb(r, g, b)
    {
      r = Math.min(255, Math.round(r));
      g = Math.min(255, Math.round(g));
      b = Math.min(255, Math.round(b));
      return ((r << 16) + (g << 8) + b);
    }

    function getColor(r, g, b, mRgb)
    {
      // split rgb in to components
      var pointr = (mRgb >> 16) & 0x00FF;
      var pointg = (mRgb >> 8) & 0x00FF;
      var pointb = mRgb & 0x00FF;
      // add the color to the algo.mapped location
      pointr += r;
      pointg += g;
      pointb += b;
      // set algo.mapped point
      return mergeRgb(pointr, pointg, pointb);
    }

    // Key Functions ------------------------------

    util.initialize = function(width, height)
    {
      algo.obj = new Array(algo.presetNumber);

      for (var i = 0; i < algo.presetNumber; i++) {
        algo.obj[i] = {
          // set random start locations for objects
          x: Math.random() * (width - 1),
          y: Math.random() * (height - 1),
        };
          // set random directions
        do {
          do {
            algo.obj[i].xDirection = (Math.random() * 2) - 1;
          } while (Math.abs(algo.obj[i].xDirection) < 0.1);
          do {
            algo.obj[i].yDirection = (Math.random() * 2) - 1;
          } while (Math.abs(algo.obj[i].yDirection) < 0.1);
        } while (Math.abs(algo.obj[i].xDirection + algo.obj[i].yDirection) < 0.3);
        do {
          // Chose random colour for each object
          algo.obj[i].r = Math.round(Math.random() * 255);
          algo.obj[i].g = Math.round(Math.random() * 255);
          algo.obj[i].b = Math.round(Math.random() * 255);
          // try again if it is too dim
        } while ((algo.obj[i].r + algo.obj[i].g + algo.obj[i].b) < 125);
      }

      // area size to draw object
      algo.boxRadius = Math.round(algo.presetSize / 2);
      algo.realsize = Math.floor(algo.presetSize / 2) * 2 + 1;

      algo.initialized = true;
      return;
    };

    algo.rgbMap = function(width, height, rgb, progstep)
    {
      if (algo.initialized === false) {
        util.initialize(width, height);
      }

      // Clear algo.map data
      algo.map = new Array(height);
      for (var y = 0; y < height; y++) {
        algo.map[y] = new Array();
        for (var x = 0; x < width; x++) {
          algo.map[y][x] = 0;
        }
      }

      // for each object displayed
      for (var i = 0; i < algo.presetNumber; i++) {
        // workout closest map location for object
        var mx = Math.floor(algo.obj[i].x);
        var my = Math.floor(algo.obj[i].y);

        // split colour
        var r = algo.obj[i].r;
        var g = algo.obj[i].g;
        var b = algo.obj[i].b;
        if (algo.presetRandom != 0) {
          r = (rgb >> 16) & 0x00FF;
          g = (rgb >> 8) & 0x00FF;
          b = rgb & 0x00FF;
        }

        for (var ry = my - algo.boxRadius; ry < my + algo.boxRadius + 2; ry++) {
          for (var rx = mx - algo.boxRadius; rx < mx + algo.boxRadius + 2; rx++) {
            // Draw only if edges are on the map
            if (rx < width && rx > -1 && ry < height && ry > -1) {
              // DEVELOPMENT: Draw the box for debugging.
              //algo.map[ry][rx] = getColor(20, 20, 20, algo.map[ry][rx]);

              // add the object color to the mapped location
              if (algo.selectedAlgo === "stars") {
                algo.map[ry][rx] = starsAlgo.getMapPixelColor(i, rx, ry, r, g, b);
              } else if (algo.selectedAlgo === "balls") {
                algo.map[ry][rx] = ballsAlgo.getMapPixelColor(i, rx, ry, r, g, b);
              } else if (algo.selectedAlgo === "snowman") {
                algo.map[ry][rx] = snowmanAlgo.getMapPixelColor(i, rx, ry, r, g, b);
              } else if (algo.selectedAlgo === "ufo") {
                algo.map[ry][rx] = ufoAlgo.getMapPixelColor(i, rx, ry, r, g, b);
              } else {
                algo.map[ry][rx] = treesAlgo.getMapPixelColor(i, rx, ry, r, g, b);
              }
            }
          }
        }

        // if collision detection is on
        if (algo.presetCollision === 0) {
          // object collision detection
          // check all objects
          for (var ti = 0; ti < algo.presetNumber; ti++) {
            // but not the current one
            if (ti !== i) {
              // calculate distance to current object
              var disx = (algo.obj[i].x + algo.obj[i].xDirection) - algo.obj[ti].x;
              var disy = (algo.obj[i].y + algo.obj[i].yDirection) - algo.obj[ti].y;
              var dish = Math.sqrt((disx * disx) + (disy * disy));
              // if to close
              if (dish < (1.414) * (algo.presetSize / 2)) {
                // swap speed / direction of current object
                var stepx = algo.obj[i].xDirection;
                var stepy = algo.obj[i].yDirection;
                algo.obj[i].xDirection = algo.obj[ti].xDirection;
                algo.obj[i].yDirection = algo.obj[ti].yDirection;
                algo.obj[ti].xDirection = stepx;
                algo.obj[ti].yDirection = stepy;
              }
            }
          }
        }

        // edge collision detection
        if (algo.obj[i].y <= 0 && algo.obj[i].yDirection < 0) {
          // top edge and moving up
          algo.obj[i].yDirection *= -1;
        } else if (algo.obj[i].y >= height - 1 && algo.obj[i].yDirection > 0) {
          // bottom edge and moving down
          algo.obj[i].yDirection *= -1;
        }

        if (algo.obj[i].x <= 0 && algo.obj[i].xDirection < 0) {
          // left edge and moving left
          algo.obj[i].xDirection *= -1;
        } else if (algo.obj[i].x >= width - 1 && algo.obj[i].xDirection > 0) {
          // right edge and moving right
          algo.obj[i].xDirection *= -1;
        }

        // set object's next location
        algo.obj[i].x += algo.obj[i].xDirection;
        algo.obj[i].y += algo.obj[i].yDirection;
      }

      return algo.map;
    };

    algo.rgbMapStepCount = function(width, height)
    {
      // This make no difference to the script ;-)
      return width * height;
    };

    // Development tool access
    testAlgo = algo;

    return algo;
  }
)();
