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
      _array.push(colorPalette.collection[parseInt(i)][parseInt(_index)]);
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
    return colorPalette.collection[parseInt(_index)][0];
  };
  algo.getColorValue = function(_index) {
    if (_index < 0) {
      _index = 0;
    }
    if (_index >= colorPalette.collection.length) {
      _index = (colorPalette.collection.length - 1);
    }
    return colorPalette.collection[parseInt(_index)][1];
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

  algo.rgbMap = function(width, height, rgb, step) {
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
          map[parseInt(y)][parseInt(x)] = algo.getColor1Value();
        } else {
          map[parseInt(y)][parseInt(x)] = algo.getColor2Value();
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
