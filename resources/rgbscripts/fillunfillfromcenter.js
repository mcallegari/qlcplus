/*
  Q Light Controller Plus
  fillunfillfromcenter.js

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
    algo.name = "Fill Unfill From Center";
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
        var center = 0;
        var isEven = 0;
        if (algo.orientation == 1)
        {
            center = Math.floor((parseInt(height) + 1) / 2) - 1;
            isEven = (height % 2 == 0);
        }
        else
        {
            center = Math.floor((parseInt(width) + 1) / 2) - 1;
            isEven = (width % 2 == 0);
        }
        var centerStep = center + 1;

        var map = new Array(height);
        for (var y = 0; y < height; y++)
        {
            map[y] = new Array();
            for (var x = 0; x < width; x++)
            {
                var cmpAxis = x;
                if (algo.orientation == 1)
                    cmpAxis = y;
                if (step < centerStep)
                {
                    if (cmpAxis <= center + step + (isEven ? 1 : 0 ) && cmpAxis >= center - step)
                        map[y][x] = rgb;
                    else
                        map[y][x] = 0;
                }
                else
                {
                    var step2 = step - centerStep
                    if (cmpAxis <= center + step2 + (isEven ? 1 : 0 ) && cmpAxis >= center - step2)
                        map[y][x] = 0;
                    else
                        map[y][x] = rgb;
                }
            }
        }

        return map;
    }

    algo.rgbMapStepCount = function(width, height)
    {
        if (algo.orientation == 0)
            return Math.floor((parseInt(width) + 1) / 2) * 2 - 1;
        else
            return Math.floor((parseInt(height) + 1) / 2) * 2 - 1;
    }

    // Development tool access
    testAlgo = algo;

    return algo;
    }
)()
