/*
  Q Light Controller Plus
  sinewave.js

  Copyright (c) Massimo Callegari

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
    var algo = new Object;
    algo.apiVersion = 2;
    algo.name = "Sine Wave";
    algo.author = "Massimo Callegari";
    algo.acceptColors = 2;

    algo.properties = new Array();
    algo.orientation = 0;
    algo.properties.push("name:orientation|type:list|display:Orientation|values:Horizontal,Vertical|write:setOrientation|read:getOrientation");
    algo.amplitude = 100;
    algo.properties.push("name:amplitude|type:range|display:Amplitude|values:1,100|write:setAmplitude|read:getAmplitude");
    algo.frequency = 100;
    algo.properties.push("name:frequency|type:range|display:Frequency|values:1,400|write:setFrequency|read:getFrequency");

    algo.setOrientation = function(_orientation)
    {
      if (_orientation === "Vertical") { algo.orientation = 1; }
      else { algo.orientation = 0; }
    };

    algo.getOrientation = function()
    {
      if (algo.orientation === 1) { return "Vertical"; }
      else { return "Horizontal"; }
    };

    algo.setAmplitude = function(_amplitude)
    {
      algo.amplitude = _amplitude;
    };

    algo.getAmplitude = function()
    {
      return algo.amplitude;
    };

    algo.setFrequency = function(_frequency)
    {
      algo.frequency = _frequency;
    };

    algo.getFrequency = function()
    {
      return algo.frequency;
    };

    algo.rgbMap = function(width, height, rgb, step)
    {
      var map = new Array(height);
      for (var y = 0; y < height; y++)
      {
          map[y] = new Array();
          for (var x = 0; x < width; x++)
            map[y][x] = 0;
      }

      var stepsHeight = algo.orientation === 0 ? height : width;
      var stepsWidth = algo.orientation === 0 ? width : height;
      var ampFactor = stepsHeight * (algo.amplitude / 200);
      var stepsTotal = Math.floor(stepsWidth * (algo.frequency / 100));
      var lastPos = -1;

      for (var cStep = 0; cStep < stepsWidth; cStep++)
      {
        var sineStep = step + cStep;
        if (sineStep >= stepsWidth)
          sineStep -= stepsWidth;
        var sinPos = Math.sin((sineStep * 2 * Math.PI) / stepsTotal) * ampFactor;
        sinPos = Math.floor((stepsHeight / 2) + sinPos);

        // workaround for rounding issue
        if (sinPos == stepsHeight)
          sinPos--;

        if (algo.orientation === 0)
          map[sinPos][cStep] = rgb;
        else
          map[cStep][sinPos] = rgb;

        // fill the previous column/row
        // to create continuous painting
        if (cStep > 0)
        {
          if (Math.abs(lastPos - sinPos) > 1)
          {
            var startY = (lastPos < sinPos) ? lastPos + 1 : sinPos + 1;
            var stopY = (lastPos < sinPos) ? sinPos + 1: lastPos + 1;
            var col = (lastPos < sinPos) ? cStep : cStep - 1;
            for (var f = startY; f < stopY; f++)
            {
              if (algo.orientation === 0)
                map[f][col] = rgb;
              else
                map[col][f] = rgb;
            }
          }
        }

        lastPos = sinPos;
      }

      return map;
    };

    algo.rgbMapStepCount = function(width, height)
    {
      if (algo.orientation === 0) {
        return width;
      } else {
        return height;
      }
    };

    // Development tool access
    testAlgo = algo;

    return algo;
  }
)();
