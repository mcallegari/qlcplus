/*
  Q Light Controller Plus
  alternate.js

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

(function() {
  var algo = new Object;
  algo.apiVersion = 3;
  algo.name = "Alternate";
  algo.author = "Hans-Jürgen Tappe";

  var x = 0;
  var y = 0;

  algo.acceptColors = 2;
  algo.properties = new Array();

  algo.align = 0;
  algo.properties.push("name:align|type:list|" +
    "display:Align (for even width)|values:Left,Centered|" +
    "write:setAlign|read:getAlign");
  // Left aligned is default.
  algo.setAlign = function(_align) {
    if (_align === "Centered") {
      algo.align = 1;
    } else {
      algo.align = 0;
    }
  };
  algo.getAlign = function() {
    if (algo.align === 1) {
      return "Centered";
    } else {
      return "Left";
    }
  };

  algo.orientation = 0;
  algo.properties.push("name:orientation|type:list|" +
    "display:Orientation|values:Horizontal,Vertical,Interleaved|" +
    "write:setOrientation|read:getOrientation");
  algo.setOrientation = function(_orientation) {
    if (_orientation === "Vertical") {
      algo.orientation = 1;
    } else if (_orientation === "Interleaved") {
      algo.orientation = 2;
    } else {
      algo.orientation = 0;
    }
  };
  algo.getOrientation = function() {
    if (parseInt(algo.orientation) === 1) {
      return "Vertical";
    } else if (algo.orientation === 2) {
      return "Interleaved";
    } else {
      return "Horizontal";
    }
  };

  algo.blockSize = 1;
  algo.properties.push("name:blockSize|type:range|"
    + "display:Block Size (1-16)|"
    + "values:1,16|"
    + "write:setBlockSize|read:getBlockSize");
  algo.setBlockSize = function(_size) {
    algo.blockSize = parseInt(_size);
  };
  algo.getBlockSize = function() {
    return algo.blockSize;
  };

  algo.offset = 0;
  algo.properties.push("name:offset|type:range|"
    + "display:Offset (0-16)|"
    + "values:0,16|"
    + "write:setOffset|read:getOffset");
  algo.setOffset = function(_size) {
    algo.offset = parseInt(_size);
  };
  algo.getOffset = function() {
    return algo.offset;
  };

  algo.getRawColor = function (rawColors, idx) {
    if (Array.isArray(rawColors) && rawColors.length > idx && ! isNaN(rawColors[idx])) {
      return rawColors[idx];
    } else {
      return 0;
    }
  }

  algo.rgbMap = function(width, height, rgb, step, rawColors) {
    var map = new Array(height);
    var effectiveStep = step;
    var colorSelectOne = (step === 1) ? false : true;
    var rowColorOne = colorSelectOne;

    var xMax = width;
    if (algo.align === 1 && width % 2 === 0) {
      xMax = width / 2 + width % 2;
    }
    for (y = 0; y < height; y++) {
      if (algo.orientation === 0) {
        // Initialize vertical bars, each column the same
        colorSelectOne = (step === 1) ? false : true;
      } else if (algo.orientation === 1) {
        // Horizontal Bars, count steps by row
        effectiveStep = y + step * algo.blockSize + algo.offset;
        // Initialize start color for each row.
        if (effectiveStep % algo.blockSize === 0) {
          colorSelectOne = !colorSelectOne;
        }
      } else if (algo.orientation === 2) {
        var effectiveY = y + step * algo.blockSize + algo.offset;
        if (effectiveY % (algo.blockSize) === 0) {
          rowColorOne = !rowColorOne;
        }
        colorSelectOne = rowColorOne;
      }
      map[parseInt(y)] = new Array();
      for (x = 0; x < width; x++) {
        if (algo.orientation === 0) {
          // Vertical bars, count steps by column
          effectiveStep = x + algo.offset;
          if (effectiveStep % algo.blockSize === 0) {
            colorSelectOne = !colorSelectOne;
          }
        } else if (algo.orientation === 2) {
          // Horizontal Bars, count steps by row and column
          var effectiveX = x + step * algo.blockSize + algo.offset;
          // Change color each step.
          if (effectiveX % algo.blockSize === 0) {
            colorSelectOne = !colorSelectOne;
          }
        }
        if (colorSelectOne) {
          map[parseInt(y)][parseInt(x)] = algo.getRawColor(rawColors, 0);
        } else {
          map[parseInt(y)][parseInt(x)] = algo.getRawColor(rawColors, 1);
        }
      }
    }
    // Align centered
    if (algo.align === 1 && width % 2 === 0) {
      for (y = 0; y < height; y++) {
        for (x = 0; x < xMax; x++) {
          map[parseInt(y)][parseInt(x)] = map[parseInt(y)][parseInt(width - x - 1)];
        }
      }
    }

    return map;
  };

  algo.rgbMapStepCount = function(width, height) {
    // Only two steps; one for even pixels and another for odd pixels
    return 2;
  };

  // Development tool access
  testAlgo = algo;

  return algo;
})();
