/*
  Q Light Controller Plus
  randompixelperrowmulticolor.js

  Copyright (c) Doug Puckett

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
        algo.name = "Random Pixel Per Row Multicolor";
        algo.author = "Doug Puckett";
        algo.properties = [];
        algo.acceptColors = 0;

        /**
          * The actual "algorithm" for this RGB script. Produces a map of
          * size($width, $height) each time it is called.
          *
          * @param step The step number that is requested (0 to (algo.rgbMapStepCount - 1))
          * @param rgb Tells the color requested by user in the UI.
          * @return A two-dimensional array[height][width].
          */
        algo.rgbMap = function (width, height, rgb, step) {
            var map = new Array(height);
            for (var y = 0; y < height; y++) {
                map[y] = [];
                //get a random pixel in the curent row (y)
                var index = Math.floor(Math.random() * (width));

                var gradR = Math.floor(Math.random() * (255));
                var gradG = Math.floor(Math.random() * (255));
                var gradB = Math.floor(Math.random() * (255));
                var gradRGB = (gradR << 16) + (gradG << 8) + gradB;

                for (var x = 0; x < width; x++) {
                    if (x === index) {
                        map[y][x] = gradRGB;  //if this is the random pixel from above, set it to the user's chosen color
                    } else {
                        map[y][x] = 0;    //otherwise, turn it off
                    }
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
        algo.rgbMapStepCount = function (width, height) {
            return width * height;
        };

        // Development tool access
        testAlgo = algo;

        return algo;
    }
)();
