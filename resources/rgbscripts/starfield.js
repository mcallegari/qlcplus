/*
  Q Light Controller Plus
  starfield.js

  Copyright (c) Doug Puckett
  With Additons by Branson Matheson

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
  function() {
    var algo = {};
    algo.apiVersion = 2;
    algo.name = "3D Starfield";
    algo.author = "Doug Puckett+Branson Matheson";
    algo.properties = [];
    algo.acceptColors = 1;
    algo.presetColor = 0x000000;
    algo.properties.push("name:StarsAmount|type:range|display:Number of Stars (10-255)|values:10,255|write:setAmount|read:getAmount");
    algo.presetStars = 50;          // 50 stars on screen at one time (default)
    algo.properties.push("name:MultiColor|type:list|display:MultiColored Stars?|values:No,Yes|write:setMulti|read:getMulti");
    algo.multiColor = 0;            // Multicolor stars off defaultly (1 = stars will be randomly colored)
    algo.properties.push("name:InvertBrightness|type:list|display:Invert Brightness|values:Dim->Bright,Bright->Dim|write:setInvert|read:getInvert");
    algo.invertColor = 0;           // Reverse Brightness
    var depth = 128;                // depth - best not to change
    var stars = new Array(255);     // main star position array

    algo.setAmount = function(_amount) {
      algo.presetStars = _amount;
    };

    algo.getAmount = function() {
      return algo.presetStars;
    };

    algo.setMulti = function(_multic) {
      if (_multic === "Yes") {
        algo.multiColor = 1;    // Random Colored Stars
      } else {
        algo.multiColor = 0;    // Stars are chosen color
      }
    };

    algo.getMulti = function() {
      if (algo.multiColor === 1) {
        return "Yes";
      } else {
        return "No";
      }
    };

    algo.setInvert = function(_invert) {
      if (_invert === "Bright->Dim") {
        algo.invertColor = 1;    // Start Bright -> Dim
      } else {
        algo.invertColor = 0;    // Start Dim -> Bright
      }
    };

    algo.getInvert = function() {
      if (algo.invertColor === 1) {
        return "Bright->Dim";
      } else {
        return "Dim->Bright";
      }
    };


    var util = new Object;
    algo.initialized = false;

    //random position function for new star
    function getNewNumberRange(minVal, maxVal) {
      minVal = Math.random() * minVal;
      maxVal = Math.random() * maxVal;
      return Math.floor(Math.random() * (maxVal - minVal + 1)) + minVal;
    }

    //set color of star - if multicolor, choose random color. If not random, return user chosen color
    function getNewColor(isMultiColor, zColor) {
      if (isMultiColor === 1) {
        var tr = Math.floor(Math.random() * 255);   // random red level
        var tg = Math.floor(Math.random() * 255);   // random green level
        var tb = Math.floor(Math.random() * 255);   // random blue level
        return (tr << 16) + (tg << 8) + tb;         // returned combined color
      } else {
        // If not multicolor, return chosen color
        return zColor;
      }
    }

    // initialize the stars and load random positions
    util.initialize = function(width, height) {
      for (var i = 0; i < stars.length; i++) {
        stars[i] = {
          x: getNewNumberRange(-10, 10),
          y: getNewNumberRange(-10, 10),
          z: depth,
          c: getNewColor(algo.multiColor, algo.presetColor)
        };
      }

      algo.initialized = true;
      return;
    };

    // main QLC+ routine where the work is done
    algo.rgbMap = function(width, height, rgb, step) {
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

      // find center of display
      var halfWidth = width / 2;
      var halfHeight = height / 2;

      // start moving the stars by looping through this routine, addressing each star individually (i is the star number)
      for (var i = 0; i < algo.presetStars - 1; i++) {

        // decrease depth on each pass through 
        if (height >= width) { stars[i].z -= height / (height / 4); }
        else { stars[i].z -= width / (width / 4); }

        // if star is off screen (depth is zero or less) then create a new star near the center of the display
        if (stars[i].z <= 0) {
          stars[i].x = getNewNumberRange(-10, 10);
          stars[i].y = getNewNumberRange(-10, 10);
          stars[i].z = depth;
          stars[i].c = getNewColor(algo.multiColor, rgb);
        }

        // calculate the stars next position
        var k = 200 / stars[i].z;                           // how far away the star is
        var px = Math.floor(stars[i].x * k + halfWidth);    // x position of star
        var py = Math.floor(stars[i].y * k + halfHeight);   // y position of star

        if (px >= 0 && px < width && py >= 0 && py < height) {
          // if star is in the center, then it should be darker (farther away)
          // and lighten as it moves "closer" (out to the edges)
          if (algo.invertColor == 1) {
            var colorLevel = (stars[i].z * 2);
          } else {
            var colorLevel = (255 - (stars[i].z * 2));
          }

          // parse out individual colors of current star color
          var r = (stars[i].c >> 16) & 0x00FF;
          var g = (stars[i].c >> 8) & 0x00FF;
          var b = stars[i].c & 0x00FF;

          // Adjust star brightness level based on how far away it is and by chosen star color
          var rr = r - (255 - colorLevel);
          var gg = g - (255 - colorLevel);
          var bb = b - (255 - colorLevel);

          // Limit each color element to the maximum for chosen color or make 0 if below 0
          if (rr > r) { rr = r; }
          if (rr < 0) { rr = 0; }
          if (gg > g) { gg = g; }
          if (gg < 0) { gg = 0; }
          if (bb > b) { bb = b; }
          if (bb < 0) { bb = 0; }

          // put all the individual rgb colors back together
          var pRGB = (rr << 16) + (gg << 8) + bb;
          px = Math.floor(px); // get rid of x and y position fractions
          py = Math.floor(py);
          map[py][px] = pRGB; // store the star's combined color in the map
          // console.log("i: " + i + " px: " + px + " py: " + py + " pz: " + stars[i].z + " color: " + r+g+b);
        } else {
          // console.log("i: " + i + " k: " + k + " px: " + px + " width: " + width + " py: " + py + " height: " + height);
          stars[i].z = 0;     // if here, the star was outside screen for some reason so force a new star
        }
      }
      return map; // return the map back to QLC+

    };

    algo.rgbMapStepCount = function(width, height) {
      return 2;
    };

    // Development tool access
    testAlgo = algo;

    return algo;
  }
)();
