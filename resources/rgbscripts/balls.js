/*
  Q Light Controller Plus
  balls.js

  Copyright (c) Rob Nieuwenhuizen, Tim Cullingworth

  Licensed under the Apache License, Version 2.0 (the 'License');
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0.txt

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an 'AS IS' BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
*/

// Development tool access
var testAlgo;

(
  function () {
    var algo = new Object;
    algo.apiVersion = 3;
    algo.name = "Balls";
    algo.author = "Rob Nieuwenhuizen, Tim Cullingworth";
    algo.acceptColors = 5;
    algo.properties = new Array();
    algo.presetSize = 1;
    algo.properties.push(
      "name:presetSize|type:range|display:Size|" +
      "values:1,20|write:setSize|read:getSize");
    algo.presetNumber = 5;
    algo.properties.push(
      "name:presetNumber|type:range|display:Number|" +
      "values:1,15|write:setNumber|read:getNumber");
    algo.presetCollision = 0;
    algo.properties.push(
      "name:presetCollision|type:list|display:Self Collision|" +
      "values:No,Yes|write:setCollision|read:getCollision");
    algo.presetSize = 5;
    algo.properties.push(
      "name:presetIndex|type:list|display:Preset|" +
      "values:User Defined,Random|" +
      "write:setPreset|read:getPreset");
    algo.presetIndex = 0;

    algo.initialized = false;
    
    var util = new Object;
    util.colorArray = new Array(algo.acceptColors);

    algo.setSize = function (_size) {
      algo.presetSize = _size;
    };
    algo.getSize = function () {
      return algo.presetSize;
    };

    algo.setNumber = function (_step) {
      algo.presetNumber = _step;
      algo.initialized = false;
    };
    algo.getNumber = function () {
      return algo.presetNumber;
    };
    algo.setCollision = function (_colision) {
      if (_colision === "Yes") { algo.presetCollision = 0; }
      else if (_colision === "No") { algo.presetCollision = 1; }
    };
    algo.getCollision = function () {
      if (algo.presetCollision === 0) { return "Yes"; }
      else if (algo.presetCollision === 1) { return "No"; }
    };

    util.getRawColor = function (idx) {
      idx = idx % util.colorArray.length;
      var color = util.colorArray[idx];
      return color;
    }
    
    algo.setPreset = function(_preset)
    {
      algo.acceptColors = 0;
      if (_preset === "User Defined")
      {
        algo.presetIndex = 0;
        algo.acceptColors = 5;
        util.colorArray = [ 0x00FF00, 0xFFAA00, 0x0000FF, 0xFFFF00, 0x00AAFF ];
      }
      else if (_preset === "Random")
      {
        algo.presetIndex = 1;
        util.colorArray = new Array();
        for (var i = 0; i < algo.presetNumber; i++)
        {
          do
          {
            var ballR = Math.round(Math.random() * 255);  // Chose random
            var ballG = Math.round(Math.random() * 255);  // colour for
            var ballB = Math.round(Math.random() * 255);  // each Ball
          } while ((ballR + ballG + ballB) < 356);  // if it it to dim try again
          util.colorArray[i] = (ballR << 16) + (ballG << 8) + ballB;
        }
      }
      else { algo.presetIndex = 0; }
      util.initialized = false;
    };

    algo.getPreset = function()
    {
      if (algo.presetIndex === 0) { return "User Defined"; }
      else if (algo.presetIndex === 1) { return "Random"; }
      else { return "Rainbow"; }
    };
  
    algo.rgbMapSetColors = function(rawColors)
    {
      if (! Array.isArray(rawColors))
        return;
      if (algo.acceptColors > 0)
        util.colorArray = Array();
      for (var i = 0; i < algo.acceptColors; i++) {
        var isNumber = (rawColors[i] === rawColors[i]);
        var color = rawColors[i];
        if (i < rawColors.length && isNumber)
        {
          util.colorArray.push(color);
        }
      }
    }

    util.initialize = function (width, height) {
      algo.ball = new Array(algo.presetNumber);
      algo.direction = new Array(algo.presetNumber);

      for (var i = 0; i < algo.presetNumber; i++) {
        var x = Math.random() * (width - 1); // set random start
        var y = Math.random() * (height - 1); // locations for balls
        algo.ball[i] = [y, x];
        var yDirection = (Math.random() * 2) - 1; // and random directions
        var xDirection = (Math.random() * 2) - 1;
        algo.direction[i] = [yDirection, xDirection];
      }
      algo.initialized = true;
      return;
    };

    algo.rgbMap = function (width, height, rgb, progstep) {
      if (algo.initialized === false) {
        util.initialize(width, height);
      }

      var map = new Array(height); // Clear map data
      for (var y = 0; y < height; y++) {
        map[y] = new Array();

        for (var x = 0; x < width; x++) {
          map[y][x] = 0;
        }
      }

      for (var i = 0; i < algo.presetNumber; i++) {  // for each ball displayed
        rgb = util.getRawColor(i);  // use RGB for ball random colour
        var r = (rgb >> 16) & 0x00FF;  // split colour in to
        var g = (rgb >> 8) & 0x00FF;   // separate parts
        var b = rgb & 0x00FF;
        var yx = algo.ball[i];  // ball's location, as float
        var step = algo.direction[i];  // ball's direction / speed, as float
        var my = Math.floor(yx[0]);  // workout closest map location for ball
        var mx = Math.floor(yx[1]);
        var boxSize = Math.round(algo.presetSize / 2);  // area size to draw ball

        for (var ry = my - boxSize; ry < my + boxSize + 2; ry++) {  // area for faded edges

          for (var rx = mx - boxSize; rx < mx + boxSize + 2; rx++) {  // to display ball

            if (rx < width && rx > -1 && ry < height && ry > -1) {  // if edges are off the map dont draw
              var pointRGB = map[ry][rx];    // get curent colour on the map
              var pointr = (pointRGB >> 16) & 0x00FF;// so that colours mix and don't over
              var pointg = (pointRGB >> 8) & 0x00FF; // write.
              var pointb = pointRGB & 0x00FF;  // splt rgb in to components
              var ballr = r;
              var ballg = g;
              var ballb = b;
              var offx = rx - yx[1];  // calculate the off set differance of map location
              var offy = ry - yx[0];  // to the float location of the ball, using the hypotenuse
              var hyp = 1 - (Math.sqrt((offx * offx) + (offy * offy)) / ((algo.presetSize / 2) + 1));

              if (hyp < 0) { hyp = 0; } // if the distance multiplyed by ball size is negative = 0
              pointr += Math.round(ballr * hyp); // dim mapped ball colours by the distance from
              pointg += Math.round(ballg * hyp); // the ball center ( hyp = 1, full colour / 0, off)
              pointb += Math.round(ballb * hyp); // add the ball colour to the mapped location
              if (pointr > 255) { pointr = 255; } // if addind the colours over saturates
              if (pointg > 255) { pointg = 255; } // reduce it to the maximum
              if (pointb > 255) { pointb = 255; }

              pointRGB = (pointr << 16) + (pointg << 8) + pointb; // combine colours

              map[ry][rx] = pointRGB; // set mapped point
            }
          }
        }

        if (algo.presetCollision === 0) {  // if colision detection is on
          // Ball collision detection
          for (var ti = 0; ti < algo.presetNumber; ti++) {  // check all balls

            if (ti !== i) {  // but not the current one
              var disy = (yx[0] + step[0]) - algo.ball[ti][0];  // calculate distance
              var disx = (yx[1] + step[1]) - algo.ball[ti][1];  // to current ball
              var dish = Math.sqrt((disx * disx) + (disy * disy));
              if (dish < (1.414) * (algo.presetSize / 2)) {  // if to close
                var stepy = step[0];  // swap speed / direction of current ball
                var stepx = step[1];  // with ball that is to close
                algo.direction[i][0] = algo.direction[ti][0];
                algo.direction[i][1] = algo.direction[ti][1];
                algo.direction[ti][0] = stepy;
                algo.direction[ti][1] = stepx;
              }
            }
          }
        }

        // edge collision detection
        if (yx[0] <= 0 && step[0] < 0) { step[0] *= -1; } // top edge and moving up
        else if (yx[0] >= height - 1 && step[0] > 0) { step[0] *= -1; } // bottom edge and moving down

        if (yx[1] <= 0 && step[1] < 0) { step[1] *= -1; } // left edge and moving left
        else if (yx[1] >= width - 1 && step[1] > 0) { step[1] *= -1; } // right edge and moving right

        yx[0] += step[0]; // set ball's next location
        yx[1] += step[1];

        algo.ball[i] = yx; // update location
        algo.direction[i] = step; // and direction / speed
      }
      return map;
    };

    algo.rgbMapStepCount = function (width, height) {
      // This make no difference to the script ;-)
      return 2;
    };

    // Development tool access
    testAlgo = algo;

    return algo;
  }
)();
