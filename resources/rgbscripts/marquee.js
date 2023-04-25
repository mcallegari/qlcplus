/*
  Q Light Controller Plus
  marquee.js

  Copyright (c) Branson Matheson

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

(function () {
  var colorPalette = new Object();
  colorPalette.collection = new Array(
    ["White"        , 0xFFFFFF],
    ["LightGrey"    , 0xAAAAAA],
    ["MediumGrey"   , 0x999999],
    ["DarkGrey"     , 0x666666],
    ["Cream"        , 0xFFFF7F],
    ["Pink"         , 0xFF7F7F],
    ["Rose"         , 0x7F3F3F],
    ["Coral"        , 0x7F3F1F],
    ["Dim Red"      , 0x7F0000],
    ["Red"          , 0xFF0000],
    ["Orange"       , 0xFF3F00],
    ["Dim Orange"   , 0x7F1F00],
    ["Goldenrod"    , 0x7F3F00],
    ["Gold"         , 0xFF7F00],
    ["Yellow"       , 0xFFFF00],
    ["Dim Yellow"   , 0x7F7F00],
    ["Lime"         , 0x7FFF00],
    ["Pale Green"   , 0x3F7F00],
    ["Dim Green"    , 0x007F00],
    ["Green"        , 0x00FF00],
    ["Seafoam"      , 0x00FF3F],
    ["Turquoise"    , 0x007F3F],
    ["Teal"         , 0x007F7F],
    ["Cyan"         , 0x00FFFF],
    ["Electric Blue", 0x007FFF],
    ["Blue"         , 0x0000FF],
    ["Dim Blue"     , 0x00007F],
    ["Pale Blue"    , 0x1F1F7F],
    ["Indigo"       , 0x1F00BF],
    ["Purple"       , 0x3F00BF],
    ["Violet"       , 0x7F007F],
    ["Magenta"      , 0xFF00FF],
    ["Hot Pink"     , 0xFF003F],
    ["Deep Pink"    , 0x7F001F],
    ["Black"        , 0x000000]
  );

  colorPalette.makeSubArray = function (_index) {
    var _array = new Array();
    for (var i = 0; i < colorPalette.collection.length; i++) {
      _array.push(colorPalette.collection[i][_index]);
    }
    return _array;
  };
  colorPalette.names = colorPalette.makeSubArray(0);

  var algo = new Object();
  algo.apiVersion = 2;
  algo.name = "Marquee";
  algo.author = "Branson Matheson";
  algo.acceptColors = 1;
  algo.properties = new Array();
  algo.edgeDepth = 2;
  algo.properties.push(
    "name:depth|type:range|display:Depth|values:1,10|write:setDepth|read:getDepth"
  );
  algo.marquee = 0;
  algo.properties.push(
    "name:marquee|type:list|display:Marquee|values:None,Forward,Backward|write:setMarquee|read:getMarquee"
  );
  algo.marqueeCount = 3;
  algo.properties.push(
    "name:marqueeCount|type:range|display:Marquee Spaces|values:1,10|write:setMarqueeCount|read:getMarqueeCount"
  );
  algo.marqueeColorIndex = 0;
  algo.properties.push(
    "name:marqueColor|type:list|display:Marquee Light Color|" +
      "values:" +
      colorPalette.names.toString() +
      "|" +
      "write:setMarqueeColorIndex|read:getMarqueeColorIndex"
  );

  var util = new Object();
  util.initialized = false;
  util.width = 0;
  util.height = 0;
  util.step = algo.marqueeCount;

  util.lights = new Array();
  util.feature = new Array();

  algo.setDepth = function (_amount) {
    algo.edgeDepth = _amount;
    util.initialized = false;
  };

  algo.getDepth = function () {
    return algo.edgeDepth;
  };

  algo.setMarquee = function (_marquee) {
    if (_marquee === "None") {
      algo.marquee = 0;
    }
    if (_marquee === "Forward") {
      algo.marquee = 1;
    }
    if (_marquee === "Backward") {
      algo.marquee = 2;
    }
    util.initialized = false;
  };

  algo.getMarquee = function () {
    if (algo.marquee === 0) {
      return "None";
    }
    if (algo.marquee === 1) {
      return "Forward";
    }
    if (algo.marquee === 2) {
      return "Backward";
    }
  };

  algo.setMarqueeCount = function (_amount) {
    algo.marqueeCount = _amount;
    util.initialized = false;
  };

  algo.getMarqueeCount = function () {
    return algo.marqueeCount;
  };

  algo.setMarqueeColorIndex = function (_preset) {
    algo.marqueeColorIndex = colorPalette.names.indexOf(_preset);
    util.initialized = false;
  };

  algo.getMarqueeColorIndex = function () {
    return colorPalette.collection[algo.marqueeColorIndex][0];
  };

  util.initialize = function (width, height, rgb) {
    // initialize feature
    util.feature = new Array();
    for (var y = 0; y <= height - 1; y++) {
      util.feature[y] = new Array();
      for (var x = 0; x <= width - 1; x++) {
        // write color
        var x_distance = algo.edgeDepth + 1;
        var y_distance = algo.edgeDepth + 1;
        var distance = algo.edgeDepth + 1;
        if (x <= algo.edgeDepth) {
          x_distance = x;
        } else if (x >= width - algo.edgeDepth - 1) {
          x_distance = width - x - 1;
        }

        if (y <= algo.edgeDepth) {
          y_distance = y;
        } else if (y >= height - algo.edgeDepth - 1) {
          y_distance = height - y - 1;
        }

        distance = Math.min(x_distance, y_distance);
        if (distance <= algo.edgeDepth) {
          var percent = ((algo.edgeDepth - distance) / algo.edgeDepth) * 100;
          util.feature[y][x] = util.fadeColor(rgb, percent);
        } else {
          util.feature[y][x] = 0;
        }
      }
    }
    // initialize lights array
    var length = height * 2 + width * 2;
    util.lights = new Array(length + algo.marqueeCount + 1);
    var count = algo.marqueeCount;
    count++;
    for (var i = length + count + 1; i >= 0; i--) {
      util.lights[i] = 0;
      if (i % count === 1) {
        util.lights[i] = 1;
      }
    }
    // for testing for change
    util.width = width;
    util.height = height;
    util.initialized = true;
  };

  util.fadeColor = function (rgb, percent) {
    var r = (rgb >> 16) & 0x00ff;
    var g = (rgb >> 8) & 0x00ff;
    var b = rgb & 0x00ff;
    var newR = Math.round(r * (percent / 100));
    var newG = Math.round(g * (percent / 100));
    var newB = Math.round(b * (percent / 100));
    var newRGB = (newR << 16) + (newG << 8) + newB;
    return newRGB;
  };

  util.mergeRgb = function (rgb1, rgb2) {
    if (rgb1 === 0) {
      return rgb2;
    } else if (rgb2 === 0) {
      return rgb1;
    }
    // split rgb into components
    var r1 = (rgb1 >> 16) & 0x00ff;
    var g1 = (rgb1 >> 8) & 0x00ff;
    var b1 = rgb1 & 0x00ff;

    var r2 = (rgb2 >> 16) & 0x00ff;
    var g2 = (rgb2 >> 8) & 0x00ff;
    var b2 = rgb2 & 0x00ff;

    var r = Math.max(r1, r2);
    var g = Math.max(g1, g2);
    var b = Math.max(b1, b2);

    return (r << 16) + (g << 8) + b;
  };

  util.getNextStep = function (width, height, step) {
    var map = new Array(height);
    for (var y = 0; y <= height - 1; y++) {
      map[y] = new Array(width);
      for (var x = 0; x <= width - 1; x++) {
        map[y][x] = util.feature[y][x];
      }
    }

    if (algo.marquee === 0) { return map }

    if (algo.marquee === 1) {
      var first = util.lights.shift();
      util.lights.push(first);
    } else if (algo.marquee === 2) {
      var last = util.lights.pop();
      util.lights.unshift(last);
    }

    // create light map add lights, go around the outside
    var marqueeColor = colorPalette.collection[algo.marqueeColorIndex][1];
    var p = 0;
    // left
    for (var y = 0; y < height; y++) {
      var x = 0;
      if (util.lights[p] === 1) {
        map[y][x] = marqueeColor;
      }
      p += 1;
    }
    // bottom
    for (var x = 0; x < width; x++) {
      var y = height - 1;
      if (util.lights[p] === 1) {
        map[y][x] = marqueeColor;
      }
      p += 1;
    }
    // right
    for (var y = height - 1; y >= 0; y--) {
      var x = width - 1;
      if (util.lights[p] === 1) {
        map[y][x] = marqueeColor;
      }
      p += 1;
    }
    // top
    for (var x = width - 1; x >= 0; x--) {
      var y = 0;
      if (util.lights[p] === 1) {
        map[y][x] = marqueeColor;
      }
      p += 1;
    }
    for (var y = 0; y <= height - 1; y++) {
      for (var x = 0; x <= width - 1; x++) {
        map[y][x] = util.mergeRgb(map[y][x], util.feature[y][x]);
      }
    }
    return map;
  };

  algo.rgbMap = function (width, height, rgb, step) {
    if (
      util.initialized === false ||
      util.width !== width ||
      util.height !== height
    ) {
      util.initialize(width, height, rgb);
    }
    var map = util.getNextStep(width, height, step);
    return map;
  };

  algo.rgbMapStepCount = function (width, height) {
    var size = Number(algo.marqueeCount);
    return size
  };

  // Development tool access
  testAlgo = algo;

  return algo;
})();
