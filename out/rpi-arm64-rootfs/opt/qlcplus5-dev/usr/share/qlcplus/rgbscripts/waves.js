/*
  Q Light Controller Plus
  waves.js

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
    var util = new Object;
    util.initialized = false;

    var algo = new Object;
    algo.apiVersion = 2;
    algo.name = "Waves";
    algo.author = "Nathan Durnan";

    algo.properties = new Array();
    algo.taillength = 50;
    algo.properties.push("name:taillength|type:range|display:Tail Length %|values:0,100|write:setTail|read:getTail");
    algo.tailfade = 1;
    algo.properties.push("name:tailfade|type:list|display:Fade Tail|values:No,Yes|write:setFade|read:getFade");
    algo.direction = 0;
    algo.properties.push("name:direction|type:list|display:Direction|values:Right,Left,In,Out|write:setDirection|read:getDirection");
    algo.orientation = 0;
    algo.properties.push("name:orientation|type:list|display:Orientation|values:Horizontal,Vertical|write:setOrientation|read:getOrientation");

    algo.setTail = function(_tail)
    {
      algo.taillength = _tail;
      util.initialize();
    };

    algo.getTail = function()
    {
      return algo.taillength;
    };

    algo.setFade = function(_fade)
    {
      if (_fade === "Yes") { algo.tailfade = 1; }
      else if (_fade === "No") { algo.tailfade = 0; }
    };

    algo.getFade = function()
    {
      if (algo.tailfade === 0) { return "No"; }
      else if (algo.tailfade === 1) { return "Yes"; }
    };

    algo.setDirection = function(_direction)
    {
      if (_direction === "Right") { algo.direction = 0; }
      else if (_direction === "Left") { algo.direction = 1; }
      else if (_direction === "In") { algo.direction = 2; }
      else if (_direction === "Out") { algo.direction = 3; }
    };

    algo.getDirection = function()
    {
      if (algo.direction === 0) { return "Right"; }
      else if (algo.direction === 1) { return "Left"; }
      else if (algo.direction === 2) { return "In"; }
      else if (algo.direction === 3) { return "Out"; }
    };

    algo.setOrientation = function(_orientation)
    {
      if (_orientation === "Vertical") { algo.orientation = 1; }
      else if (_orientation === "Horizontal") { algo.orientation = 0; }
    };

    algo.getOrientation = function()
    {
      if (algo.orientation === 1) { return "Vertical"; }
      else if (algo.orientation === 0) { return "Horizontal"; }
    };

    util.initialize = function()
    {
      // fixed size fade array
      util.fadeSteps = 100;
      var _step = (1 / util.fadeSteps);

      util.fadeObject = new Array(util.fadeSteps);
      util.fadeObject[0] = 1;
      for (var f = 1; f < util.fadeSteps; f++)
      {
        util.fadeObject[f] = (_step * (util.fadeSteps - f));
      }
      util.initialized = true;
    };

    algo.rgbMap = function(width, height, rgb, step)
    {
      if (util.initialized === false) { util.initialize(); }
      var span = ((algo.orientation === 0) ? width : height);
      var center = Math.floor((span + 1) / 2) - 1;
      var isEven = (span % 2 === 0);
      var tailSteps = Math.round(span * algo.taillength/100);
      if (tailSteps === 0) { tailSteps = 1; }

      var map = new Array(height);
      for (var y = 0; y < height; y++)
      {
        map[y] = new Array();
        for (var x = 0; x < width; x++)
        {
          var pos = ((algo.orientation === 0) ? x : y);
          var fill = false;
          var stepPos = pos;
          // Create an index of the pixel relative the the direction
          switch (algo.direction)
          {
            case 0: /* Right */
              stepPos = pos;
              break;
            case 1: /* Left  */
              stepPos = (span - 1 - pos);
              break;
            case 2: /* In    */
              if (pos <= center) {
                stepPos =  pos;
              } else {
                stepPos = (span - 1 - pos);
              }
              break;
            case 3: /* Out   */
              if (pos <= center) {
                stepPos = (center - pos);
              } else {
                stepPos = (pos - center - (isEven ? 1 : 0));
              }
              break;
          }
          // Decide whether or not to fill the pixel
          switch (algo.direction)
          {
            case 0: // Right - no break
            case 1: // Left
              fill = ((stepPos <= step) && (stepPos > (step - tailSteps)));
            break;
            case 2: // In - no break
            case 3: // Out
              if (stepPos <= center)
              {
                fill = ((stepPos <= step) && (stepPos > (step - tailSteps)));
              }
              else
              {
                fill = (((span - 1- stepPos) <= step) &&
                       ((span - 1 - stepPos) > (step - tailSteps)));
              }
              break;
          }
          // Determine the fade for this pixel
          var thisRgb = rgb;
          if (fill && (algo.tailfade === 1))
          {
            var thisTailStep = Math.round(util.fadeSteps * (step - stepPos) / tailSteps);
            var r = Math.round(((rgb >> 16) & 0x00FF) * util.fadeObject[thisTailStep]);
            var g = Math.round(((rgb >> 8) & 0x00FF) * util.fadeObject[thisTailStep]);
            var b = Math.round((rgb & 0x00FF) * util.fadeObject[thisTailStep]);
            thisRgb = (r << 16) + (g << 8) + b;
          }
          // Populate the matrix
          map[y][x] = (fill ? thisRgb : 0);
          map[y][x] = (fill ? thisRgb : 0);
        }
      }
      return map;
    };

    algo.rgbMapStepCount = function(width, height)
    {
      var span = parseInt((algo.orientation === 0) ? width : height);
      var isEven = (span % 2 === 0);
      var tailSteps = Math.round(span * algo.taillength/100);
      if (tailSteps === 0) { tailSteps = 1; }
      if ((algo.direction === 0) || (algo.direction === 1)) {
        return (span + tailSteps - (isEven ? 0 : 1));
      } else if ((algo.direction === 2) || (algo.direction === 3)) {
        return Math.round(Math.floor((span + 1) / 2) + tailSteps - 1);
      }
    };

    // Development tool access
    testAlgo = algo;
    return algo;
    }
)();
