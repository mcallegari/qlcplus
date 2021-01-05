/*
  Q Light Controller Plus
  Noise.js

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
    function()
    {
        var algo = new Object;
        algo.apiVersion = 2;
        algo.name = "Noise";
        algo.author = "Doug Puckett";
        algo.properties = [];
        algo.acceptColors = 1;
        algo.noisePercentage = "High";
        var dCounter = 0;

        algo.properties.push("name:noisePercentage|type:list|display:Noise Coverage|values:Low,Medium,High|write:setAmount|read:getAmount");

        algo.setAmount = function (_amount) {
            algo.noisePercentage = _amount;
        };

        algo.getAmount = function () {
            return algo.noisePercentage;
        };

        // QLC+ rgbMap function where the work is done
        algo.rgbMap = function (width, height, rgb, step)
        {
            var map = new Array(height);

            for (var y = 0; y < height; y++)
            {
                map[y] = [];

                for (var x = 0; x < width; x++)
                {
                    var r = (rgb >> 16) & 0x00FF;  // split color of user selected color
                    var g = (rgb >> 8) & 0x00FF;
                    var b = rgb & 0x00FF;

                    // create random color level from 1 to 255
                    var colorLevel = Math.floor(Math.random() * 255);

                    // Assign random color value to temp variables
                    var rr = colorLevel;
                    var gg = colorLevel;
                    var bb = colorLevel;

                    // Limit each color element to the maximum for chosen color or make 0 if below 0
                    if (rr > r) { rr = r; }
                    if (rr < 0) { rr = 0; }
                    if (gg > g) { gg = g; }
                    if (gg < 0) { gg = 0; }
                    if (bb > b) { bb = b; }
                    if (bb < 0) { bb = 0; }

                    var cColor = (rr << 16) + (gg << 8) + bb;   // put rgb parts back together
                    var vDiv = 0;                               // for noise amount use

                    // setup for noise reduction :)
                    switch (algo.noisePercentage)
                    {
                        case "Low":
                            vDiv = Math.random() * 4 + 7;
                        break;
                        case "Medium":
                            vDiv = Math.random() * 5;
                        break;
                        case "High":
                            vDiv = 0;
                        break;
                    }

                    dCounter += 1;              // counter for noise trigger
                    if (dCounter >= vDiv) {     // compare counter to user noise amount selection value
                        dCounter = 0;           // clear the counter
                        map[y][x] = cColor;     // set pixel to color created above
                    }
                    else {
                        map[y][x] = 0;          // otherwise, clear it
                    }
                }
            }

            return map;
        };

        algo.rgbMapStepCount = function (width, height) {
            return width * height;
        };

        // Development tool access
        testAlgo = algo;

        return algo;
    }
)();
