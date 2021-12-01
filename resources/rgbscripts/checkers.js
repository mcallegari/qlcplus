/*
  Q Light Controller Plus
  checkers.js

  Copyright (c) Branson Matheson
  Based on the work of Massimo Callegari and Tim Cullingworth

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
    var colorPalette = new Object;
    colorPalette.collection = new Array(
      ["White"        , 0xFFFFFF],  //  0
      ["Cream"        , 0xFFFF7F],  //  1
      ["Pink"         , 0xFF7F7F],  //  2
      ["Rose"         , 0x7F3F3F],  //  3
      ["Coral"        , 0x7F3F1F],  //  4
      ["Dim Red"      , 0x7F0000],  //  5
      ["Red"          , 0xFF0000],  //  6
      ["Orange"       , 0xFF3F00],  //  7
      ["Dim Orange"   , 0x7F1F00],  //  8
      ["Goldenrod"    , 0x7F3F00],  //  9
      ["Gold"         , 0xFF7F00],  // 10
      ["Yellow"       , 0xFFFF00],  // 11
      ["Dim Yellow"   , 0x7F7F00],  // 12
      ["Lime"         , 0x7FFF00],  // 13
      ["Pale Green"   , 0x3F7F00],  // 14
      ["Dim Green"    , 0x007F00],  // 15
      ["Green"        , 0x00FF00],  // 16
      ["Seafoam"      , 0x00FF3F],  // 17
      ["Turquoise"    , 0x007F3F],  // 18
      ["Teal"         , 0x007F7F],  // 19
      ["Cyan"         , 0x00FFFF],  // 20
      ["Electric Blue", 0x007FFF],  // 21
      ["Blue"         , 0x0000FF],  // 22
      ["Dim Blue"     , 0x00007F],  // 23
      ["Pale Blue"    , 0x1F1F7F],  // 24
      ["Indigo"       , 0x1F00BF],  // 25
      ["Purple"       , 0x3F00BF],  // 26
      ["Violet"       , 0x7F007F],  // 27
      ["Magenta"      , 0xFF00FF],  // 28
      ["Hot Pink"     , 0xFF003F],  // 29
      ["Deep Pink"    , 0x7F001F],  // 30
      ["OFF"          , 0x000000]); // 31
    
    colorPalette.makeSubArray = function(_index)
    {
      var _array = new Array();
      for (var i = 0; i < colorPalette.collection.length; i++)
      {
        _array.push(colorPalette.collection[i][_index]);
      }
      return _array;
    };
    colorPalette.names  = colorPalette.makeSubArray(0);

    var util = new Object;
    util.initialized = false;
    util.altCheck = 0;

    var algo = new Object;
    algo.apiVersion = 2;
    algo.name = "Checkers";
    algo.author = "Branson Matheson";
    algo.acceptColors = 0;
    algo.properties = new Array();
    algo.checkWidth = 8;
    algo.properties.push("name:checkWidth|type:range|display:Check Width|values:1,100|write:setCheckWidth|read:getCheckWidth");
    algo.checkHeight = 8;
    algo.properties.push("name:checkHeight|type:range|display:Check Height|values:1,100|write:setCheckHeight|read:getCheckHeight");
    algo.checkDepth = 0;
    algo.properties.push("name:checkDepth|type:range|display:Check Depth|values:0,64|write:setCheckDepth|read:getCheckDepth");
    algo.checkSlide = 0;
    algo.properties.push("name:checkSlide|type:list|display:Slide|values:None,Up,Down,Left,Right,UpRight,UpLeft,DownRight,DownLeft|write:setSlide|read:getSlide");
    algo.setSlide = function(_slide)
    {
      if (_slide === "Up") { algo.checkSlide = 1; }
      else if (_slide === "Down") { algo.checkSlide = 2; }
      else if (_slide === "Left") { algo.checkSlide = 3; }
      else if (_slide === "Right") { algo.checkSlide = 4; }
      else if (_slide === "UpRight") { algo.checkSlide = 5; }
      else if (_slide === "UpLeft") { algo.checkSlide = 6; }
      else if (_slide === "DownRight") { algo.checkSlide = 7; }
      else if (_slide === "DownLeft") { algo.checkSlide = 8; }
      else { algo.checkSlide = 0; }
    };

    algo.getSlide = function()
    {
      if (algo.checkSlide === 1) { return "Up"; }
      else if (algo.checkSlide === 2) { return "Down"; }
      else if (algo.checkSlide === 3) { return "Left"; }
      else if (algo.checkSlide === 4) { return "Right"; }
      else if (algo.checkSlide === 5) { return "UpRight"; }
      else if (algo.checkSlide === 6) { return "UpLeft"; }
      else if (algo.checkSlide === 7) { return "DownRight"; }
      else if (algo.checkSlide === 8) { return "DownLeft"; }
      else { return "None"; }
    };

    algo.checkAlternate = 0;
    algo.properties.push("name:checkAlternate|type:list|display:Alternate Colors|values:No,Yes|write:setAlternate|read:getAlternate");
    algo.setAlternate = function(_slide)
    {
      if (_slide === "Yes") { algo.checkAlternate = 1; }
      else { algo.checkAlternate = 0; }
    }

    algo.getAlternate = function()
    {
      if (algo.checkAlternate === 1) { return "Yes"; }
      else { return "No"; }
    }

    algo.checkOnColorIndex = 0;
    algo.properties.push(
      "name:checkOnColor|type:list|display:CheckOnColor|" +
      "values:" + colorPalette.names.toString() + "|" +
      "write:setCheckOnColor|read:getCheckOnColor");
    algo.checkOffColorIndex = 31;
    algo.properties.push(
      "name:checkOffColor|type:list|display:CheckOffColor|" +
      "values:" + colorPalette.names.toString() + "|" +
      "write:setCheckOffColor|read:getCheckOffColor");
    
    algo.colorIndex = new Array(
        algo.checkOnColorIndex,
        algo.checkOffColorIndex
        );

    algo.setColor = function(_index, _preset)
    {
      var i = colorPalette.names.indexOf(_preset);
      if (i === -1) {
        i = (colorPalette.collection.length - 1);
      }
      algo.colorIndex[_index] = i;
      return algo.colorIndex[_index];
    };
    
    algo.getColor = function(_index)
    {
      var i = algo.colorIndex[_index];
      if (i < 0) { i = 0; }
      if (i >= colorPalette.collection.length) {
        i = (colorPalette.collection.length - 1);
      }
      return colorPalette.collection[i][0];
    };

    algo.setCheckOnColor = function(_preset)
    {
      algo.checkOnColorIndex = algo.setColor(0, _preset);
      util.initialized = false;
    };
    algo.getCheckOnColor = function()
    {
      return algo.getColor(0);
    };
    
    algo.setCheckOffColor = function(_preset)
    {
      algo.checkOffColorIndex = algo.setColor(1, _preset);
      util.initialized = false;
    };
    algo.getCheckOffColor = function()
    {
      return algo.getColor(1);
    };

    var checkers = new Array();
    function Check(y, x, step, space)
    {
      // upper left corner
      this.x = x;
      this.y = y;
      this.step = step;
      this.space = space;
    }

    function mod(n, m) {
      return ((n % m) + m) % m;
    }

    function limit(n, m) {
      if ( n <= 0 ) { return 0 }
      if ( n > m ) { return m }
      return n
    }

    algo.setCheckWidth = function(_amount)
    {
      algo.checkWidth = _amount;
      util.initialized = false;
    };

    algo.getCheckWidth = function()
    {
      return algo.checkWidth;
    };

    algo.setCheckHeight = function(_amount)
    {
      algo.checkHeight = _amount;
      util.initialized = false;
    };

    algo.getCheckHeight = function()
    {
      return algo.checkHeight;
    };
    algo.setCheckDepth = function(_amount)
    {
      algo.checkDepth = _amount;
      util.initialized = false;
    };

    algo.getCheckDepth = function()
    {
      return algo.checkDepth;
    };

    util.initialize = function(width, height)
    {
      checkers = new Array();
      var i = 0
      var r = 0
      // console.log('creating checkers with boundaries: ' + height + ' ' + width );
      // console.log('creating checkers with size: ' + algo.checkHeight + ' ' + algo.checkWidth );
      for (var y = 0; y < height; y+=Number(algo.checkHeight)) {
        for (var x = 0; x < width; x+=Number(algo.checkWidth)) {
          var space = 0
          if ( Math.floor(r/2) === r/2 ) { 
            if ( Math.floor(i/2) === i/2 ) { space = 1 }
          } else {
            space = 1
            if ( Math.floor(i/2) === i/2 ) { space = 0 }
          }
          // console.log('creating check: ' + i + ' pos: ' +y + ',' + x + ' space:' + space);
          checkers.push(new Check(y, x, 0, space));
          i++;
        }
        r++;
      }
      util.initialized = true;
    };

    util.getColor = function(check, h, w)
    {
        var color = 0
        if ( util.altCheck === 1 ) {
           if (check.space === 1 ) { color = colorPalette.collection[algo.checkOnColorIndex][1]; }
           if (check.space === 0 ) { color = colorPalette.collection[algo.checkOffColorIndex][1]; }
         } else {
            if (check.space === 0 ) { color = colorPalette.collection[algo.checkOnColorIndex][1]; }
            if (check.space === 1 ) { color = colorPalette.collection[algo.checkOffColorIndex][1]; }
         }
        if (algo.checkDepth > 0 ) {
          // calculate distance from center
          var hd = Math.abs(h - (algo.checkHeight/2)); //actual center
          var wd = Math.abs(w - (algo.checkWidth/2)); // actual center
          var ca = Math.floor(Math.sqrt((hd*hd) + (wd*wd))  * algo.checkDepth/2);
          //var ca = Math.abs((hd*algo.checkDepth)+(wd*algo.checkDepth));
          // update color values 
          var r = limit(((color >> 16) & 0x00FF) - ca, 0xFF);  // split colour in to
          var g = limit(((color >> 8) & 0x00FF) - ca, 0xFF);   // separate part
          var b = limit((color & 0x00FF) - ca, 0xFF);

          // brigten colors by amounts based on distance
          var new_color = (r << 16) + (g << 8) + b;
           color = new_color
           // console.log('r:' + r + ' h:' + h + ' hd:' + hd +' ca: ' + ca + ' old: ' + color + ' new: ' + new_color);
        }
        return color
    };

    util.getNextStep = function(width, height)
    {
      // create an empty, black map y by x
      var map = new Array(width);
      for (var y = 0; y <= height; y++)
      {
        map[y] = new Array();
        for (var x = 0; x <= width; x++)
        {
          map[y][x] = 0;
        }
      }

      // checkers onto map
      for ( var i = 0; i < checkers.length; i++) 
      {
        var check = checkers[i]
        // if sliding add one square per step
        if (algo.checkSlide > 0) {
          if ( algo.checkSlide == 1 || algo.checkSlide == 5 || algo.checkSlide == 6 ) { check.y = mod((check.y - 1), height); } 
          if ( algo.checkSlide == 2 || algo.checkSlide == 7 || algo.checkSlide == 8 ) { check.y = mod((check.y + 1), height); }
          if ( algo.checkSlide == 3 || algo.checkSlide == 6 || algo.checkSlide == 8 ) { check.x = mod((check.x - 1), width); } 
          if ( algo.checkSlide == 4 || algo.checkSlide == 5 || algo.checkSlide == 7 ) { check.x = mod((check.x + 1), width); }
        }
        for ( var w = 0; w < algo.checkWidth; w++) 
        {
          for ( var h = 0; h < algo.checkHeight; h++)  
          {
            var color = util.getColor(check, h, w);
            var actualY = mod((check.y + h), height);
            var actualX = mod((check.x + w), width);
            if ( actualY <= height && actualX <= width ) {
              // console.log('mapping check: ' + i + ' pos:' + actualY + ',' + actualX + ' color ' + color)
              map[actualY][actualX] = color;
            }
          }
        }
      }
      // alternate on each pass
      if ( algo.checkAlternate === 1 && util.altCheck === 1 ) { util.altCheck = 0; }
      else { util.altCheck = 1; }

      return map;
    };

    algo.rgbMap = function(width, height, rgb, step)
    {
      // console.log('width: ' + width + ' height: ' + height )
      if (util.initialized === false)
      {
          util.initialize(width, height);
      }
      var rgbmap = util.getNextStep(width, height);
      return rgbmap
    };

    algo.rgbMapStepCount = function(width, height)
    {
      var size = Math.floor((width*height)/(algo.checkWidth));
      return size;
    };

    // Development tool access
    testAlgo = algo;

    return algo;
    }
)();
