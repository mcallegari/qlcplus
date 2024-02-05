/*
  Q Light Controller Plus
  strobe.js

  Copyright (c) Rob Nieuwenhuizen

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
    function () {
      var algo = {};
      algo.apiVersion = 2;
      algo.name = "Strobe";
      algo.author = "Rob Nieuwenhuizen";
      algo.properties = [];
      algo.acceptColors = 1;
      algo.properties.push("name:frequency|type:range|display:Frequency|values:2,10|write:setFreq|read:getFreq");
      algo.frequency = 2;
      algo.setFreq = function(_freq){
        algo.frequency = parseInt(_freq);
      };
      algo.getFreq = function(){
        return algo.frequency;
      };

      /**
        * The actual "algorithm" for this RGB script. Produces a map of
        * size($width, $height) each time it is called.
        *
        * @param step The step number that is requested (0 to (algo.rgbMapStepCount - 1))
        * @param rgb Tells the color requested by user in the UI.
        * @return A two-dimensional array[height][width].
        */
      algo.rgbMap = function (width, height, rgb, step)
      {
        var map = new Array();
        for (var y = 0; y < height; y++)
        {
          var _map = new Array();
          for (var x = 0; x < width; x++)
          {
            _map.push((step % algo.frequency) !== 0 ? 0 : rgb);
          }
          map.push(_map);
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
      algo.rgbMapStepCount = function (width, height)
      {
        return algo.frequency;
      };

      // Development tool access
      testAlgo = algo;

      return algo;
    }
)();
