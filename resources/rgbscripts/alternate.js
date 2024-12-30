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
    "display:Align (for even width)|values:Left,Centered,Split|" +
    "write:setAlign|read:getAlign");
  // Left aligned is default.
  algo.setAlign = function(_align) {
    if (_align === "Centered") {
      algo.align = 1;
    } else if (_align === "Split") {
      algo.align = 2;
    } else {
      algo.align = 0;
    }
  };
  algo.getAlign = function() {
    if (algo.align === 1) {
      return "Centered";
    } else if (algo.align === 2) {
      return "Split";
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
    if (parseInt(algo.orientation, 10) === 1) {
      return "Vertical";
    } else if (algo.orientation === 2) {
      return "Interleaved";
    } else {
      return "Horizontal";
    }
  };

  algo.blockSize = 1;
  algo.properties.push("name:blockSize|type:range|"
    + "display:Block Size / Split (>= 1)|"
    + "values:1,32000|"
    + "write:setBlockSize|read:getBlockSize");
  algo.setBlockSize = function(_size) {
    algo.blockSize = parseInt(_size, 10);
  };
  algo.getBlockSize = function() {
    return algo.blockSize;
  };

  algo.offset = 0;
  algo.properties.push("name:offset|type:range|"
    + "display:Offset (>= 0)|"
    + "values:0,32000|"
    + "write:setOffset|read:getOffset");
  algo.setOffset = function(_size) {
    algo.offset = parseInt(_size, 10);
  };
  algo.getOffset = function() {
    return algo.offset;
  };

  var util = new Object;
  util.colorArray = new Array(algo.acceptColors);

  util.getRawColor = function (idx) {
    var color = 0;
    if (Array.isArray(util.colorArray) && util.colorArray.length > idx && util.colorArray[idx]) {
      color = util.colorArray[idx];
    }
    return color;
  }

  algo.rgbMapSetColors = function(rawColors)
  {
    if (! Array.isArray(rawColors))
      return;
    for (var i = 0; i < algo.acceptColors; i++) {
      if (i < rawColors.length)
      {
        util.colorArray[i] = rawColors[i];
      } else {
        util.colorArray[i] = 0;
      }
    }
  }

  algo.rgbMap = function(width, height, rgb, step) {
    var map = new Array(height);
    var colorSelectOne = (step === 1) ? false : true;
    var rowColorOne = colorSelectOne;
    var realBlockSize = algo.blockSize;

    // Setup the rgb map
    for (y = 0; y < height; y++) {
      map[y] = new Array(width);
    }

    var xMax = width;
    var yMax = height;
    if (algo.align === 1) {
      if (algo.orientation === 0 || algo.orientation === 2) {
        // Centered mode
        xMax = Math.ceil(width / 2);
      }
      if (algo.orientation === 1 || algo.orientation === 2) {
        // Centered mode
        yMax = Math.ceil(height / 2);
      }
    }

    if (algo.align === 2) {
      // Split mode
      if (algo.orientation === 0) {
        // Horizontal
        realBlockSize = width / algo.blockSize;
      } else if (algo.orientation === 1) {
        // Vertical
        realBlockSize = height / algo.blockSize;
      } else if (algo.orientation === 2) {
        // Interleaved
        realBlockSize = Math.min(width, height) / algo.blockSize;
      }
    }

    var effectiveStep;
    var realBlockCount;
    var lowRest;
    var highRest;
    var rest;
    for (y = 0; y < yMax; y++) {
      if (algo.orientation === 0) {
        // Horizontal split; vertical lines
        // Initialize vertical bars, each column the same
        colorSelectOne = (step === 1) ? false : true;
      } else if (algo.orientation === 1) {
        // Horizontal Bars, count steps by row
        effectiveStep = y + Math.round(step * realBlockSize) + algo.offset;

        // Initialize start color for each row.
        realBlockCount = Math.floor(effectiveStep / realBlockSize);
        lowRest = effectiveStep - realBlockCount * realBlockSize;
        highRest = (realBlockCount + 1) * realBlockSize - effectiveStep;
        rest = Math.min(lowRest, highRest);
        if (rest < 0.5 || lowRest === 0.5) {
          colorSelectOne = !colorSelectOne;
        }
      } else if (algo.orientation === 2) {
        // Interleaved
        var effectiveY = y + Math.floor(step * realBlockSize) + algo.offset;

        realBlockCount = Math.floor(effectiveY / realBlockSize);
        lowRest = effectiveY - realBlockCount * realBlockSize;
        highRest = (realBlockCount + 1) * realBlockSize - effectiveY;
        rest = Math.min(lowRest, highRest);
        if (rest < 0.5 || lowRest === 0.5) {
          rowColorOne = !rowColorOne;
        }
        colorSelectOne = rowColorOne;
      }

      for (x = 0; x < xMax; x++) {
        if (algo.orientation === 0) {
          // Horizontal split, vertical bars, count steps by column
          effectiveStep = x + algo.offset;
          realBlockCount = Math.floor(effectiveStep / realBlockSize);
          lowRest = effectiveStep - realBlockCount * realBlockSize;
          highRest = (realBlockCount + 1) * realBlockSize - effectiveStep;
          rest = Math.min(lowRest, highRest);
          if (rest < 0.5 || lowRest === 0.5) {
            colorSelectOne = !colorSelectOne;
          }
        } else if (algo.orientation === 2) {
          // vertical split, horizontal Bars, count steps by row and column
          var effectiveX = x + Math.floor(step * realBlockSize) + algo.offset;
          // Change color each step.
          realBlockCount = Math.floor(effectiveX / realBlockSize);
          lowRest = effectiveX - realBlockCount * realBlockSize;
          highRest = (realBlockCount + 1) * realBlockSize - effectiveX;
          rest = Math.min(lowRest, highRest);
          if (rest < 0.5 || lowRest == 0.5) {
            colorSelectOne = !colorSelectOne;
          }
        }
        if (colorSelectOne) {
          map[y][x] = util.getRawColor(0);
        } else {
          map[y][x] = util.getRawColor(1);
        }
      }
    }
    // Align centered
    if (algo.align === 1) {
      if (algo.orientation === 0) {
        for (y = 0; y < yMax; y++) {
          for (x = 0; x < xMax; x++) {
            map[y][width - x - 1] = map[y][x];
          }
        }
      } else if (algo.orientation === 1) {
        for (y = 0; y < yMax; y++) {
          for (x = 0; x < xMax; x++) {
            map[height - y - 1][x] = map[y][x];
          }
        }
      } else if (algo.orientation === 2) {
        for (y = 0; y < yMax; y++) {
          for (x = 0; x < xMax; x++) {
            map[height - y - 1][x] = map[y][x];
            map[y][width - x - 1] = map[y][x];
            map[height - y - 1][width - x - 1] = map[y][x];
          }
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
