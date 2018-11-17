/*
  Q Light Controller Plus
  randomfillrow.js

  Copyright (c) David Garyga

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
        algo.apiVersion = 1;
        algo.name = "Random Fill Row";
        algo.author = "David Garyga";
        algo.width = 0;
        algo.height = 0;

        var util = new Object;

        util.createStepList = function(length)
        {
            var list = new Array(length);
            for (var i = 0; i < length; i++) {
                list[i] = i;
            }
            return list;
        };

        util.createStep = function(length, si, lastStep)
        {
            var map = new Array(length);
            for (var i = 0; i < length; i++)
            {
                if (si === i) {
                    map[i] = 1;
                } else if (lastStep !== 0 && lastStep[i] === 1) {
                    map[i] = 1;
                } else {
                    map[i] = 0;
                }
            }

            return map;
        };

        util.createStepRgb = function(width, height, step, rgb)
        {
            var map = new Array(height);
            for (var y = 0; y < height; y++)
            {
                map[y] = new Array(width);
                for (var x = 0; x < width; x++)
                {
                    if (step[y] !== 0) {
                        map[y][x] = rgb;
                    } else {
                        map[y][x] = 0;
                    }
                }
            }

            return map;
        };

        algo.rgbMap = function(width, height, rgb, step)
        {
            if (algo.width !== width || algo.height !== height || parseInt(step) === 0)
            {
                var stepList = util.createStepList(height);

                algo.steps = new Array(height);
                var lastStep = 0;
                for (var i = 0; i < height; i++)
                {
                    var index = Math.floor(Math.random() * (stepList.length));
                    var yx = stepList[index];
                    var map = util.createStep(height, yx, lastStep);
                    algo.steps[i] = map;
                    lastStep = map;

                    stepList.splice(index, 1);
                }

                algo.width = width;
                algo.height = height;
            }

            return util.createStepRgb(width, height, algo.steps[step], rgb);
        };

        algo.rgbMapStepCount = function(width, height)
        {
            return height;
        };

        // Development tool access
        testAlgo = algo;

        return algo;
    }
)();
