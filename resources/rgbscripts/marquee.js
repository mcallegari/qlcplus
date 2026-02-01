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
  var algo = new Object();
  algo.apiVersion = 3;
  algo.name = "Marquee";
  algo.author = "Branson Matheson";
  algo.acceptColors = 2;
  algo.properties = new Array();
  algo.edgeDepth = 2;
  algo.properties.push(
    "name:depth|type:range|display:Depth|values:1,10000|write:setDepth|read:getDepth"
  );
  algo.marquee = 0;
  algo.properties.push(
    "name:marquee|type:list|display:Marquee|values:None,Forward,Backward|write:setMarquee|read:getMarquee"
  );
  algo.marqueeCount = 3;
  algo.properties.push(
    "name:marqueeCount|type:range|display:Marquee Spaces|values:1,100|write:setMarqueeCount|read:getMarqueeCount"
  );

  var util = new Object();
  util.initialized = false;
  util.width = 0;
  util.height = 0;
  util.featureColor = 0;
  util.step = algo.marqueeCount;
  util.colorArray = new Array(algo.acceptColors);

  util.lights = new Array();
  util.feature = new Array();

  algo.setDepth = function (_amount) {
    algo.edgeDepth = parseInt(_amount, 10);
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
    algo.marqueeCount = parseInt(_amount, 10);
    util.initialized = false;
  };

  algo.getMarqueeCount = function () {
    return algo.marqueeCount;
  };

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


  util.initialize = function (width, height) {
    // initialize feature
    util.featureColor = util.getRawColor(0);
    util.feature = new Array();
    var maxDistance = Math.min(width, height) / 2;
    for (var y = 0; y < height; y++) {
      util.feature[y] = new Array();
      var y_distance = y;
      if (y >= height / 2) {
        y_distance = height - y - 1;
      }

      for (var x = 0; x < width; x++) {
        // write color
        var distance = algo.edgeDepth + 1;
        util.feature[y][x] = 0;

        var x_distance = x;
        if (x >= width / 2) {
          x_distance = width - x - 1;
        }

        distance = Math.min(x_distance, y_distance);
        if (distance <= algo.edgeDepth && distance <= maxDistance) {
          var percent = ((algo.edgeDepth - distance) / algo.edgeDepth) * 100;
          util.feature[y][x] = util.fadeColor(util.featureColor, percent);
        } else {
          util.feature[y][x] = 0;
        }
      }
    }

    // initialize lights array: 2 heights, 2 widths, 4 duplicate corner pixels
    // only if the dimensions have changes (not on color change)
    if (util.width != width || util.height != height || util.initialized !== true) {
      var length = height * 2 + width * 2 - 4;
      util.lights = new Array(length);
      var pointAmount = Math.floor(util.lights.length / (algo.marqueeCount + 1));
      var mediumDistance = length / pointAmount;
      for (var i = 0; i < util.lights.length; i++) {
        if (i % mediumDistance < 1) {
          util.lights[i] = 1;
        } else {
          util.lights[i] = 0;
        }
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

  util.getNextStep = function (width, height) {
    var x = 0;
    var y = 0;
    var map = new Array(height);
    for (y = 0; y <= height - 1; y++) {
      map[y] = new Array(width);
      for (x = 0; x <= width - 1; x++) {
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
    var marqueeColor = util.getRawColor(1);
    var p = 0;
    // left
    for (y = 0; y < height; y++) {
      x = 0;
      if (util.lights[p] === 1) {
        map[y][x] = marqueeColor;
      }
      p += 1;
    }
    // bottom
    for (x = 1; x < width; x++) {
      y = height - 1;
      if (util.lights[p] === 1) {
        map[y][x] = marqueeColor;
      }
      p += 1;
    }
    // right
    for (y = height - 2; y >= 0; y--) {
      x = width - 1;
      if (util.lights[p] === 1) {
        map[y][x] = marqueeColor;
      }
      p += 1;
    }
    // top
    for (x = width - 2; x >= 0; x--) {
      y = 0;
      if (util.lights[p] === 1) {
        map[y][x] = marqueeColor;
      }
      p += 1;
    }
    return map;
  };

  algo.rgbMap = function(width, height, rgb, step) {
    if (
      util.initialized === false ||
      util.featureColor != util.getRawColor(0) ||
      util.width !== width ||
      util.height !== height
    ) {
      util.initialize(width, height);
    }

    var map = util.getNextStep(width, height);
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
