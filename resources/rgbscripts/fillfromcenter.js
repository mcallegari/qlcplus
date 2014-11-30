/*
  Q Light Controller Plus
  fillfromcenter.js

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
    algo.name = "Fill From Center";
    algo.author = "Massimo Callegari";

    algo.orientation = 0;
    algo.properties = new Array();
    algo.properties.push("name:orientation|type:list|display:Orientation|values:Horizontal,Vertical|write:setOrientation|read:getOrientation");

    algo.setOrientation = function(_orientation)
    {
      if (_orientation == "Vertical")
        algo.orientation = 1;
      else
        algo.orientation = 0;
    }

    algo.getOrientation = function()
    {
      if (algo.orientation == 1)
        return "Vertical";
      else
        return "Horizontal";
    }

    algo.rgbMap = function(width, height, rgb, step)
    {
        var center = algo.rgbMapStepCount(width, height) - 1;
        var isEven = 0;
        if (algo.orientation == 0)
            isEven = (width % 2 == 0);
        else
            isEven = (height % 2 == 0);

        var map = new Array(height);
        for (var y = 0; y < height; y++)
        {
            map[y] = new Array();
            for (var x = 0; x < width; x++)
            {
                var fill = 0;
                if (algo.orientation == 0)
                {
                    if (x <= center + step + (isEven ? 1 : 0 ) && x >= center - step)
                        fill = 1;
                }
                else
                {
                    if (y <= center + step + (isEven ? 1 : 0 ) && y >= center - step)
                        fill = 1;
                }

                if(fill == 1)
                    map[y][x] = rgb;
                else
                    map[y][x] = 0;
            }
        }

        return map;
    }

    algo.rgbMapStepCount = function(width, height)
    {
      if (algo.orientation == 0)
        return Math.floor((parseInt(width) + 1) / 2);
      else
        return Math.floor((parseInt(height) + 1) / 2);
    }

    // Development tool access
    testAlgo = algo;

    return algo;
    }
)()
