/*
  Q Light Controller Plus
  empty.js

  Copyright (c) Your Name

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
    algo.apiVersion = 3;
    algo.name = "Script name";
    algo.author = "Your Name";
    algo.acceptColors = 2;
    algo.properties = new Array();

    var util = new Object;
    util.colorArray = new Array(algo.acceptColors);

    /**
     * Evaluates the rawColors parameter and returns the idx value
     *
     * @param idx The index of the color in the array
     * @return The requested array color or zero in case of invalid input
     */
    util.getRawColor = function (idx) {
      var color = 0;
      if (Array.isArray(util.colorArray) && util.colorArray.length > idx && util.colorArray[idx]) {
        color = util.colorArray[idx];
      }
      return color;
    }
  
    /**
     * Ingests the colors as received through the parameter
     *
     * @param rawColors The array of colors 
     */
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

    /**
      * The actual "algorithm" for this RGB script. Produces a map of
      * size($width, $height) each time it is called.
      *
      * @param width The width of the matrix in pixels
      * @param height The height of the matrix in pixels
      * @param rgb Tells the color requested by user in the UI, interpolated between stepCount.
      * @param step The step number that is requested (0 to (algo.rgbMapStepCount - 1))
      * @return A two-dimensional matrix array[height][width].
      */
    algo.rgbMap = function(width, height, rgb, step)
    {
      var map = new Array(height);
      for (var y = 0; y < height; y++)
      {
        map[y] = new Array();
        for (var x = 0; x < width; x++) {
          map[y][x] = rgb; // <-- step color goes here
        }
      }

      return map;
    };

    /**
      * Tells RGB Matrix how many steps this algorithm produces with size($width, $height)
      *
      * @param width The width of the map
      * @param height The height of the map
      * @return Number of steps required for a map of size($width, $height)
      */
    algo.rgbMapStepCount = function(width, height)
    {
      return width;
    };

    // Development tool access
    testAlgo = algo;

    return algo;
  }
)();
