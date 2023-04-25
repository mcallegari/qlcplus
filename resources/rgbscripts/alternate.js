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
  var colorPalette = new Object;
  colorPalette.collection = new Array(
    ["White", 0xFFFFFF], // 0
    ["Cream", 0xFFFF7F], // 1
    ["Pink", 0xFF7F7F], // 2
    ["Rose", 0x7F3F3F], // 3
    ["Coral", 0x7F3F1F], // 4
    ["Dim Red", 0x7F0000], // 5
    ["Red", 0xFF0000], // 6
    ["Orange", 0xFF3F00], // 7
    ["Dim Orange", 0x7F1F00], // 8
    ["Goldenrod", 0x7F3F00], // 9
    ["Gold", 0xFF7F00], // 10
    ["Yellow", 0xFFFF00], // 11
    ["Dim Yellow", 0x7F7F00], // 12
    ["Lime", 0x7FFF00], // 13
    ["Pale Green", 0x3F7F00], // 14
    ["Dim Green", 0x007F00], // 15
    ["Green", 0x00FF00], // 16
    ["Seafoam", 0x00FF3F], // 17
    ["Turquoise", 0x007F3F], // 18
    ["Teal", 0x007F7F], // 19
    ["Cyan", 0x00FFFF], // 20
    ["Electric Blue", 0x007FFF], // 21
    ["Blue", 0x0000FF], // 22
    ["Dim Blue", 0x00007F], // 23
    ["Pale Blue", 0x1F1F7F], // 24
    ["Indigo", 0x1F00BF], // 25
    ["Purple", 0x3F00BF], // 26
    ["Violet", 0x7F007F], // 27
    ["Magenta", 0xFF00FF], // 28
    ["Hot Pink", 0xFF003F], // 29
    ["Deep Pink", 0x7F001F], // 30
    ["OFF", 0x000000]); // 31

  colorPalette.makeSubArray = function(_index) {
    var _array = new Array();
    for (var i = 0; i < colorPalette.collection.length; i++) {
      _array.push(colorPalette.collection[parseInt(i, 10)][parseInt(_index, 10)]);
    }
    return _array;
  };
  colorPalette.names = colorPalette.makeSubArray(0);

  var algo = new Object;
  algo.apiVersion = 2;
  algo.name = "Alternate";
  algo.author = "Hans-Jürgen Tappe";

  var x = 0;
  var y = 0;

  algo.acceptColors = 0;
  algo.properties = new Array();

  algo.getColorIndex = function(_name) {
    var idx = colorPalette.names.indexOf(_name);
    if (idx === -1) {
      idx = (colorPalette.collection.length - 1);
    }
    return idx;
  };

  algo.color1Index = algo.getColorIndex("Red");
  algo.properties.push("name:color1Index|type:list|display:Color 1|" +
    "values:" + colorPalette.names.toString() + "|" +
    "write:setColor1Index|read:getColor1Name");
  algo.color2Index = algo.getColorIndex("Green");
  algo.properties.push("name:color2Index|type:list|display:Color 2|" +
    "values:" + colorPalette.names.toString() + "|" +
    "write:setColor2Index|read:getColor2Name");

  algo.getColorName = function(_index) {
    if (_index < 0) {
      _index = 0;
    }
    if (_index >= colorPalette.collection.length) {
      _index = (colorPalette.collection.length - 1);
    }
    return colorPalette.collection[parseInt(_index, 10)][0];
  };
  algo.getColorValue = function(_index) {
    if (_index < 0) {
      _index = 0;
    }
    else if (_index >= colorPalette.collection.length) {
      _index = (colorPalette.collection.length - 1);
    }
    return colorPalette.collection[parseInt(_index, 10)][1];
  };

  algo.setColor1Index = function(_name) {
    algo.color1Index = algo.getColorIndex(_name);
  };
  algo.getColor1Name = function() {
    return algo.getColorName(algo.color1Index);
  };
  algo.getColor1Value = function() {
    return algo.getColorValue(algo.color1Index);
  };

  algo.setColor2Index = function(_name) {
    algo.color2Index = algo.getColorIndex(_name);
  };
  algo.getColor2Name = function() {
    return algo.getColorName(algo.color2Index);
  };
  algo.getColor2Value = function() {
    return algo.getColorValue(algo.color2Index);
  };

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

  algo.rgbMap = function(width, height, rgb, step) {
    var map = new Array(height);
    var colorSelectOne = (step === 1) ? false : true;
    var rowColorOne = colorSelectOne;
    var realBlockSize = algo.blockSize;

    for (y = 0; y < height; y++) {
      map[parseInt(y, 10)] = new Array(width);
      for (x = 0; x < width; x++) {
        map[parseInt(y, 10)][parseInt(x, 10)] = 0;
      }
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
          if (rest < 0.5 || lowRest == 0.5) {
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
          map[parseInt(y, 10)][parseInt(x, 10)] = algo.getColor1Value();
        } else {
          map[parseInt(y, 10)][parseInt(x, 10)] = algo.getColor2Value();
        }
      }
    }
    // Align centered
    if (algo.align === 1) {
      if (algo.orientation === 0) {
        for (y = 0; y < yMax; y++) {
          for (x = 0; x < xMax; x++) {
            map[parseInt(y, 10)][parseInt(width - x - 1, 10)] = map[parseInt(y, 10)][parseInt(x, 10)];
          }
        }
      } else if (algo.orientation === 1) {
        for (y = 0; y < yMax; y++) {
          for (x = 0; x < xMax; x++) {
            map[parseInt(height - y - 1, 10)][parseInt(x, 10)] = map[parseInt(y, 10)][parseInt(x, 10)];
          }
        }
      } else if (algo.orientation === 2) {
        for (y = 0; y < yMax; y++) {
          for (x = 0; x < xMax; x++) {
            map[parseInt(height - y - 1, 10)][parseInt(x, 10)] = map[parseInt(y, 10)][parseInt(x, 10)];
            map[parseInt(y, 10)][parseInt(width - x - 1, 10)] = map[parseInt(y, 10)][parseInt(x, 10)];
            map[parseInt(height - y - 1, 10)][parseInt(width - x - 1, 10)] = map[parseInt(y, 10)][parseInt(x, 10)];
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
