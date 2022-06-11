/*
  Q Light Controller Plus
  circular.js

  Copyright (c) Massimo Callegari
  with Additions by Hans-Jürgen Tappe

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

    algo.segmentsCount = 1;
    algo.properties.push("name:circlesSize|type:range|display:Circle Segments|values:1,32|write:setSegments|read:getSegments");
    algo.fadeMode = 0;
    algo.properties.push("name:fadeMode|type:list|display:Fade Mode|values:Don't Fade,Fade|write:setFade|read:getFade");
    algo.fillMatrix = 0;
    algo.properties.push("name:fillMatrix|type:list|display:Fill Matrix|values:No,Yes|write:setFill|read:getFill");
    algo.circularMode = 0;
    algo.properties.push("name:circularMode|type:list|display:Mode|values:Radar Line,In,Out|write:setMode|read:getMode");

    var util = new Object;
    util.initialized = false;
    util.circleRadius = 0;
    util.map = new Array();
    util.centerX = 0;
    util.centerY = 0;
    util.progstep = 0;
    
    let geometryCalc = new Object;

    // calculate the angle from 0 to 2 pi starting north and counting clockwise
    geometryCalc.getAngle = function(offx, offy)
    {
      let angle = 0;
      // catch offx == 0
      if (offx == 0) {
        // This where the asymptote goes
        if (offy < 0) {
          angle = -1 * Math.PI / 2;
        } else {
          angle = Math.PI / 2;
        }
      } else {
        let gradient = offy / offx;
        angle = Math.atan(gradient);
      }
      angle += Math.PI / 2;
      if (offx < 0) {
        angle += Math.PI;  
      }
      return angle;
    }

    algo.setSegments = function(_amount)
    {
      algo.segmentsCount = _amount;
      util.initialized = false;
    };

    algo.getSegments = function()
    {
      return algo.segmentsCount;
    };

    algo.setFade = function(_fade)
    {
      if (_fade === "Fade") { algo.fadeMode = 1; }
      else { algo.fadeMode = 0; }
    };

    algo.getFade = function()
    {
      if (algo.fadeMode === 1) { return "Fade In"; }
      else { return "Don't Fade"; }
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

    algo.setMode = function(_fade)
    {
      if (_fade === "In") { algo.circularMode = 1; }
      else if (_fade === "Out") { algo.circularMode = 2; }
      else { algo.circularMode = 0; }
    };

    algo.getMode = function()
    {
      if (algo.circularMode === 1) { return "In"; }
      else if (algo.circularMode === 2) { return "Out"; }
      else { return "Radar Line"; }
    };

    util.initialize = function(width, height)
    {
      util.centerX = width / 2;
      util.centerY = height / 2;
      util.twoPi = 2 * Math.PI;

      if (algo.fillMatrix === 1) {
        util.circleRadius = 0;
      } else if (height > width) {
        util.circleRadius = width;
      } else {
        util.circleRadius = height;
      } 

      util.map = new Array(height);
      for (let y = 0; y < height; y++) {
        util.map[y] = new Array(width);
        util.map[y].map(pxl => {
          return 0;
        });
      }
      
      if (algo.fadeMode === 1) {
        util.stepFade = algo.rgbMapStepCount(width, height) / algo.segmentsCount;
      } else {
        util.stepFade = 1.0;
      }

      util.width = width;
      util.height = height;
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
      let pointr = (mRgb >> 16) & 0x00FF;
      let pointg = (mRgb >> 8) & 0x00FF;
      let pointb = mRgb & 0x00FF;

      // add the color to the algo.mapped location
      pointr += r;
      pointg += g;
      pointb += b;

      // set algo.mapped point
      return util.mergeRgb(pointr, pointg, pointb);
    }

    util.getMapPixelColor = function(ry, rx, rgb)
    {
      // TODO: Optimize multiple calculations
      let r = (rgb >> 16) & 0x00FF;
      let g = (rgb >> 8) & 0x00FF;
      let b = rgb & 0x00FF;

      let factor = 1.0;
      // calculate the offset difference of algo.map location to the float
      // location of the object
      let offx = rx - util.centerX;
      let offy = ry - util.centerY;
      
      let radius = Math.sqrt(offx * offx + offy * offy);
      let angle = geometryCalc.getAngle(offx, offy);
      angle = angle + (util.twoPi * (util.progstep / algo.rgbMapStepCount(util.width, util.height)));
      angle = angle * algo.segmentsCount;
      angle = (angle + util.twoPi) % (util.twoPi);

      let virtualx = Math.sin(angle) * radius;
      let virtualy = Math.cos(angle) * radius;

      // atan(1 - x - y) + atan(y - x + 1) + atan(x - y + 1) - 2
      let endfade = Math.atan(2.5 - virtualx - virtualy);
      let sidefade1 = Math.atan((virtualy - virtualx) / algo.segmentsCount + 1);
      let sidefade2 = Math.atan((virtualx - virtualy) / algo.segmentsCount + 1);
      factor = Math.min(1, Math.max(0, endfade + sidefade1 + sidefade2 -2));
      
      return util.getColor(r * factor, g * factor, b * factor, util.map[ry][rx]);
    }

    algo.rgbMap = function(width, height, rgb, step)
    {
      if (util.initialized === false || width != util.width || height != util.height)
      {
          util.initialize(width, height);
      }
      
      util.progstep = step;

      // Dim or clear algo.map data
      if (algo.fadeMode === 1) {
        // Dim current map data
        util.map = util.map.map(col => {
          return col.map(pxl => {
            return util.dimColor(pxl, util.stepFade);
          })
        });
      } else {
        util.map = util.map.map(col => {
          return col.map(pxl => {
            return 0;
          })
        });
      }
      
      // Draw the current map
      for (ry = 0; ry < height; ry++) {
        for (rx = 0; rx < width; rx++) {
          util.map[ry][rx] = util.getMapPixelColor(ry, rx, rgb);
        }
      }

      return util.map;
    };

    algo.rgbMapStepCount = function(width, height)
    {
      // Be sure to have sufficient steps for seamless circling
      return (2 * (width + height));
    };

    // Development tool access
    testAlgo = algo;

    return algo;
    }
)();
