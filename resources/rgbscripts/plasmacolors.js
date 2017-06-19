/*
  Q Light Controller Plus
  plasmacolors.js

  Copyright (c) Nathan Durnan

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

  var algo = new Object;
  algo.apiVersion = 2;
  algo.name = "Plasma (Colors)";
  algo.author = "Nathan Durnan";
  algo.acceptColors = 0;
  algo.properties = new Array();
  algo.rstepcount = 0;
  algo.gstepcount = 50;
  algo.bstepcount = 100;
  algo.color1Index = 0;
  algo.properties.push(
    "name:color1Index|type:list|display:Color 1|" +
    "values:" + colorPalette.names.toString() + "|" +
    "write:setColor1|read:getColor1");
  algo.color2Index = 6;
  algo.properties.push(
    "name:color2Index|type:list|display:Color 2|" +
    "values:" + colorPalette.names.toString() + "|" +
    "write:setColor2|read:getColor2");
  algo.color3Index = 16;
  algo.properties.push(
    "name:color3Index|type:list|display:Color 3|" +
    "values:" + colorPalette.names.toString() + "|" +
    "write:setColor3|read:getColor3");
  algo.color4Index = 22;
  algo.properties.push(
    "name:color4Index|type:list|display:Color 4|" +
    "values:" + colorPalette.names.toString() + "|" +
    "write:setColor4|read:getColor4");
  algo.color5Index = 31;
  algo.properties.push(
    "name:color5Index|type:list|display:Color 5|" +
    "values:" + colorPalette.names.toString() + "|" +
    "write:setColor5|read:getColor5");
  algo.presetSize = 5;
  algo.properties.push(
    "name:presetSize|type:range|display:Size|" +
    "values:1,20|write:setSize|read:getSize");
  algo.ramp = 15;
  algo.properties.push(
    "name:ramp|type:range|display:Ramp|" +
    "values:10,30|write:setRamp|read:getRamp");
  algo.stepsize = 25;
  algo.properties.push(
    "name:stepsize|type:range|display:Speed|" +
    "values:1,50|write:setStep|read:getStep");
  algo.colorIndex = new Array(
    algo.color1Index,
    algo.color2Index,
    algo.color3Index,
    algo.color4Index,
    algo.color5Index);

  var util = new Object;
  util.initialized = false;
  util.gradientData = new Array();
  util.colorArray = new Array();

  algo.setColor = function(_index, _preset)
  {
    var i = colorPalette.names.indexOf(_preset);
    if (i == -1) 
      i = (colorPalette.collection.length - 1);
    algo.colorIndex[_index] = i;
    return algo.colorIndex[_index];
  };

  algo.getColor = function(_index)
  {
    var i = algo.colorIndex[_index];
    if (i < 0) i = 0;
    if (i >= colorPalette.collection.length)
      i = (colorPalette.collection.length - 1);
    return colorPalette.collection[i][0];
  };

  algo.setColor1 = function(_preset)
  {
    algo.color1Index = algo.setColor(0, _preset);
    util.initialize();
  };
  algo.getColor1 = function()
  {
    return algo.getColor(0);
  };

  algo.setColor2 = function(_preset)
  {
    algo.color2Index = algo.setColor(1, _preset);
    util.initialize();
  };
  algo.getColor2 = function()
  {
    return algo.getColor(1);
  };

  algo.setColor3 = function(_preset)
  {
    algo.color3Index = algo.setColor(2, _preset);
    util.initialize();
  };
  algo.getColor3 = function()
  {
    return algo.getColor(2);
  };

  algo.setColor4 = function(_preset)
  {
    algo.color4Index = algo.setColor(3, _preset);
    util.initialize();
  };
  algo.getColor4 = function()
  {
    return algo.getColor(3);
  };

  algo.setColor5 = function(_preset)
  {
    algo.color5Index = algo.setColor(4, _preset);
    util.initialize();
  };
  algo.getColor5 = function()
  {
    return algo.getColor(4);
  };

  algo.setSize = function(_size)
  {
    algo.presetSize = _size;
    util.initialize();
  };
  algo.getSize = function()
  {
    return algo.presetSize;
  };

  algo.setRamp = function(_ramp)
  {
    algo.ramp = _ramp;
    util.initialize();
  };
  algo.getRamp = function()
  {
    return algo.ramp;
  };

  algo.setStep = function(_step)
  {
    algo.stepsize = _step;
    util.initialize();
  };
  algo.getStep = function()
  {
    return algo.stepsize;
  };

  util.initialize = function()
  {
    // calculate the gradient for the selected preset
    // with the given width
    var gradIdx = 0;
    util.gradientData = new Array();
    util.colorArray = new Array();
    for (var j = 0; j < algo.colorIndex.length; j++)
    {
      util.colorArray[j] = colorPalette.collection[algo.colorIndex[j]][1];
    }
    for (var i = 0; i < util.colorArray.length; i++)
    {
      var sColor = util.colorArray[i];
      var eColor = util.colorArray[i + 1];
      if (eColor == undefined)
        eColor = util.colorArray[0];

      util.gradientData[gradIdx++] = sColor;
      var sr = (sColor >> 16) & 0x00FF;
      var sg = (sColor >> 8) & 0x00FF;
      var sb = sColor & 0x00FF;
      var er = (eColor >> 16) & 0x00FF;
      var eg = (eColor >> 8) & 0x00FF;
      var eb = eColor & 0x00FF;

      var stepR = ((er - sr) / 300);
      var stepG = ((eg - sg) / 300);
      var stepB = ((eb - sb) / 300);

      for (var s = 1; s < 300; s++)
      {
        var gradR = Math.floor(sr + (stepR * s)) & 0x00FF;
        var gradG = Math.floor(sg + (stepG * s)) & 0x00FF;
        var gradB = Math.floor(sb + (stepB * s)) & 0x00FF;
        var gradRGB = (gradR << 16) + (gradG << 8) + gradB;
        util.gradientData[gradIdx++] = gradRGB;
      }
    }
    util.initialized = true;
  }

  function fade(t) { return t * t * t * (t * (t * 6 - 15) + 10); }
  function lerp( t, a, b) { return a + t * (b - a); }
  function grad(hash, x, y, z) {
    var h = hash & 15;                   // CONVERT LO 4 BITS OF HASH CODE
    var u = h<8 ? x : y,                 // INTO 12 GRADIENT DIRECTIONS.
      v = h<4 ? y : h==12||h==14 ? x : z;
    return ((h&1) == 0 ? u : -u) + ((h&2) == 0 ? v : -v);
  } 
  function scale(n) { return (1 + n)/2; }

  // This is a port of Ken Perlin's Java code. The
  // original Java code is at http://cs.nyu.edu/%7Eperlin/noise/.
  // Note that in this version, a number from 0 to 1 is returned.
  util.noise = function(x, y, z)
  {
    var p = new Array(512);
    var permutation = [ 151,160,137,91,90,15,131,13,201,95,96,53,194,233, 7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,190, 6,148,247,120,234,75, 0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,88,237,149,56,87,174,20, 125,136,171,168,68,175,74,165,71,134,139,48,27,166,77,146,158,231,83,111,229, 122,60,211,133,230,220,105,92,41,55,46,245,40,244,102,143,54, 65,25,63,161,1, 216,80,73,209,76,132,187,208,89,18,169,200,196,135,130,116,188,159,86,164,100, 109,198,173,186,3,64,52,217,226,250,124,123,5,202,38,147,118,126,255,82,85,212, 207,206,59,227,47,16,58,17,182,189,28,42,223,183,170,213,119,248,152, 2,44,154, 163,70,221,153,101,155,167,43,172,9,129,22,39,253,19,98,108,110,79,113,224,232, 178,185,112,104,218,246,97,228,251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,49,192,214, 31,181,199,106,157,184, 84,204,176,115, 121,50,45,127,4,150,254,138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180];

    for (var i=0; i < 256 ; i++) 
      p[256+i] = p[i] = permutation[i]; 

    var X = Math.floor(x) & 255,                  // FIND UNIT CUBE THAT
      Y = Math.floor(y) & 255,                  // CONTAINS POINT.
      Z = Math.floor(z) & 255;
      x -= Math.floor(x);                       // FIND RELATIVE X,Y,Z
      y -= Math.floor(y);                       // OF POINT IN CUBE.
      z -= Math.floor(z);
    var u = fade(x),                              // COMPUTE FADE CURVES
      v = fade(y),                              // FOR EACH OF X,Y,Z.
      w = fade(z);
    var A = p[X  ]+Y, AA = p[A]+Z, AB = p[A+1]+Z, // HASH COORDINATES OF
      B = p[X+1]+Y, BA = p[B]+Z, BB = p[B+1]+Z; // THE 8 CUBE CORNERS,

    return scale(lerp(w, lerp(v, lerp(u, grad(p[AA  ], x  , y  , z   ),  // AND ADD
                      grad(p[BA  ], x-1, y  , z   )),    // BLENDED
              lerp(u, grad(p[AB  ], x  , y-1, z   ),     // RESULTS
                      grad(p[BB  ], x-1, y-1, z   ))),   // FROM  8
      lerp(v, lerp(u, grad(p[AA+1], x  , y  , z-1 ),     // CORNERS
                      grad(p[BA+1], x-1, y  , z-1 )),    // OF CUBE
              lerp(u, grad(p[AB+1], x  , y-1, z-1 ),
                      grad(p[BB+1], x-1, y-1, z-1 )))));
  }

  algo.rgbMap = function(width, height, rgb, step)
  {
    if (util.initialized == false)
      util.initialize();

    var size = algo.presetSize/2;  // set a scaling value
    var speed = Math.pow(100 , (algo.stepsize / 50)); // create a more uniform speed control
    algo.bstepcount += (speed / 500);
    algo.bstepcount = (algo.bstepcount % 256);  // A rolling step count for the noise function
    var square = width>height ? width : height; // keep the patten square
    
    var map = new Array(height);
    for (var y = 0; y < height; y++)
    {
      map[y] = new Array();

      for (var x = 0; x < width; x++)
      {
        var nx = x / square;  // Normalize nx & ny to 0 - 1
        var ny = y / square;
        var n = util.noise( size*nx, size*ny, algo.bstepcount);
        var gradStep = Math.round( Math.pow(n , (algo.ramp/10)) * util.gradientData.length);
        map[y][x] = util.gradientData[gradStep];
      }
    }

    return map;
  };

  algo.rgbMapStepCount = function(width, height)
  {
    if (util.initialized == false)
    util.initialize();

    return width * height;  // This make no difference to the script ;-)
  };

  // Development tool access
  testAlgo = algo;

  return algo;
}
)();
