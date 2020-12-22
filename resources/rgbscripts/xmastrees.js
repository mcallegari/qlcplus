/*
  Q Light Controller Plus
  xmastrees.js

  Copyright (c) Hans-Jürgen Tappe
  based on balls.js, Copyright (c) Tim Cullingworth

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

(function ()
{
  var algo = new Object;
  algo.apiVersion = 2;
  algo.name = "Xmas Trees";
  algo.author = "Hans-Jürgen Tappe";
  algo.acceptColors = 2;
  algo.properties = new Array();
  algo.rstepcount = 0;
  algo.gstepcount = 50;
  algo.bstepcount = 100;

  algo.presetSize = 5;
  algo.properties
      .push("name:presetSize|type:range|display:Size|values:5,20|write:setSize|read:getSize");
  algo.presetNumber = 3;
  algo.properties
      .push("name:presetNumber|type:range|display:Number|values:1,50|write:setNumber|read:getNumber");
  algo.presetRandom = 1;
  algo.properties
      .push("name:presetRandom|type:list|display:Random Colour|values:No,Yes|write:setRandom|read:getRandom");
  algo.presetCollision = 1;
  algo.properties
      .push("name:presetCollision|type:list|display:Self Collision|values:No,Yes|write:setCollision|read:getCollision");
  var util = new Object;
  algo.initialized = false;

  algo.setSize = function (_size)
  {
    algo.presetSize = _size;
  };

  algo.getSize = function ()
  {
    return algo.presetSize;
  };

  algo.setNumber = function (_step)
  {
    algo.presetNumber = _step;
    algo.initialized = false;
  };

  algo.getNumber = function ()
  {
    return algo.presetNumber;
  };

  algo.setRandom = function (_random)
  {
    if (_random === "Yes") {
      algo.presetRandom = 0;
    } else if (_random === "No") {
      algo.presetRandom = 1;
    }
  };

  algo.getRandom = function ()
  {
    if (algo.presetRandom === 0) {
      return "Yes";
    } else if (algo.presetRandom === 1) {
      return "No";
    }
  };

  algo.setCollision = function (_collision)
  {
    if (_collision === "Yes") {
      algo.presetCollision = 0;
    } else if (_collision === "No") {
      algo.presetCollision = 1;
    }
  };

  algo.getCollision = function ()
  {
    if (algo.presetCollision === 0) {
      return "Yes";
    } else if (algo.presetCollision === 1) {
      return "No";
    }
  };

  util.initialize = function (width, height)
  {
    algo.tree = new Array(algo.presetNumber);
    algo.direction = new Array(algo.presetNumber);
    algo.colour = new Array(algo.presetNumber);

    for (var i = 0; i < algo.presetNumber; i++) {
      // set random start locations for trees
      var x = Math.random() * (width - 1);
      var y = Math.random() * (height - 1);
      algo.tree[i] = [ y, x ];
      // set random directions
      var yDirection = (Math.random() * 2) - 1;
      var xDirection = (Math.random() * 2) - 1;
      algo.direction[i] = [ yDirection, xDirection ];
      do {
        // Chose random colour for each tree
        var treeR = Math.round(Math.random() * 255);
        var treeG = Math.round(Math.random() * 255);
        var treeB = Math.round(Math.random() * 255);
        // try again if it is too dim
      } while ((treeR + treeG + treeB) < 125); // was: 356
      algo.colour[i] = (treeR << 16) + (treeG << 8) + treeB;
    }

    algo.initialized = true;
    return;
  };

  algo.rgbMap = function (width, height, rgb, progstep)
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

    // for each tree displayed
    for (var i = 0; i < algo.presetNumber; i++) {
      // use RGB or tree random colour
      if (algo.presetRandom === 0) {
        rgb = algo.colour[i];
      }
      // split colour
      var r = (rgb >> 16) & 0x00FF;
      var g = (rgb >> 8) & 0x00FF;
      var b = rgb & 0x00FF;

      // tree's location, as float
      var yx = algo.tree[i];
      // tree's direction / speed, as float
      var step = algo.direction[i];
      // workout closest map location for tree
      var my = Math.floor(yx[0]);
      var mx = Math.floor(yx[1]);
      // area size to draw tree
      var boxRadius = Math.round(algo.presetSize / 2);

      // area for faded edges
      for (var ry = my - boxRadius; ry < my + boxRadius + 2; ry++) {
        for (var rx = mx - boxRadius; rx < mx + boxRadius + 2; rx++) {
          // Draw only if edges are on the map
          if (rx < width && rx > -1 && ry < height && ry > -1) {
            var treer = r;
            var treeg = g;
            var treeb = b;
            // get curent colour on the map
            var pointRGB = map[ry][rx];
            // split rgb in to components
            var pointr = (pointRGB >> 16) & 0x00FF;
            var pointg = (pointRGB >> 8) & 0x00FF;
            var pointb = pointRGB & 0x00FF;
            // Debugging: Draw a grey area for the box
            // if (pointr === 0) {
            // pointr += 20;
            // }
            // if (pointg === 0) {
            // pointg += 20;
            // }
            // if (pointb === 0) {
            // pointb += 20;
            // }

            // calculate the offset difference of map location to the float
            // location of the tree
            var offx = rx - yx[1];
            var offy = ry - yx[0];

            // Calculate color intensity
            if (ry == my + boxRadius + 1) {
              // The tree foot:
              // offx remains the same.
              // offy is 0
              offy = 0;
            } else {
              // Offset to bottom
              offy += (algo.presetSize / 2);
            }
            var hyp = ((1 - (Math.sqrt((offx * offx * 1.8) + (offy * offy)))) * 2.5)
                - 0 + offy * 3;
            hyp = Math.min(1, Math.max(0, hyp));

            // dim mapped tree colours by the distance from the tree
            // add the tree color to the mapped location
            pointr += Math.round(treer * hyp);
            pointg += Math.round(treeg * hyp);
            pointb += Math.round(treeb * hyp);
            // if adding the colours over saturates reduce it to the maximum
            if (pointr > 255) {
              pointr = 255;
            }
            if (pointg > 255) {
              pointg = 255;
            }
            if (pointb > 255) {
              pointb = 255;
            }

            // combine colours
            pointRGB = (pointr << 16) + (pointg << 8) + pointb;
            // set mapped point
            map[ry][rx] = pointRGB;
          }
        }
      }
      // if collision detection is on
      if (algo.presetCollision === 0) {
        // tree collision detection
        // check all trees
        for (var ti = 0; ti < algo.presetNumber; ti++) {
          // but not the current one
          if (ti !== i) {
            // calculate distance to current tree
            var disy = (yx[0] + step[0]) - algo.tree[ti][0];
            var disx = (yx[1] + step[1]) - algo.tree[ti][1];
            var dish = Math.sqrt((disx * disx) + (disy * disy));
            // if to close
            if (dish < (1.414) * (algo.presetSize / 2)) {
              // swap speed / direction of current tree
              var stepy = step[0];
              // with tree that is too close
              var stepx = step[1];
              algo.direction[i][0] = algo.direction[ti][0];
              algo.direction[i][1] = algo.direction[ti][1];
              algo.direction[ti][0] = stepy;
              algo.direction[ti][1] = stepx;
            }
          }
        }
      }

      // edge collision detection
      if (yx[0] <= 0 && step[0] < 0) {
        // top edge and moving up
        step[0] *= -1;
      } else if (yx[0] >= height - 1 && step[0] > 0) {
        // bottom edge and moving down
        step[0] *= -1;
      }

      if (yx[1] <= 0 && step[1] < 0) {
        // left edge and moving left
        step[1] *= -1;
      } else if (yx[1] >= width - 1 && step[1] > 0) {
        // right edge and moving right
        step[1] *= -1;
      }

      // set tree's next location
      yx[0] += step[0];
      yx[1] += step[1];

      // update location
      algo.tree[i] = yx;
      // update direction
      algo.direction[i] = step;
    }

    return map;
  };

  algo.rgbMapStepCount = function (width, height)
  {
    return width * height; // This make no diferance to the script ;-)
  };

  // Development tool access
  testAlgo = algo;

  return algo;
})();
