/*
  Q Light Controller Plus
  fillunfill.js

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
    algo.name = "Fill Unfill";
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
        var map = new Array(height);
        for (var y = 0; y < height; y++)
        {
            map[y] = new Array();
            for (var x = 0; x < width; x++)
            {
                if (algo.orientation == 1)
                {
                    if (step < height)
                    {
                        if (y <= step)
                            map[y][x] = rgb;
                        else
                            map[y][x] = 0;
                    }
                    else
                    {
                        if (y > step - height)
                            map[y][x] = rgb;
                        else
                            map[y][x] = 0;
                    }
                }
                else
                {
                    if (step < width)
                    {
                        if (x <= step)
                            map[y][x] = rgb;
                        else
                            map[y][x] = 0;
                    }
                    else
                    {
                        if (x > step - width)
                            map[y][x] = rgb;
                        else
                            map[y][x] = 0;
                    }
                }
            }
        }

        return map;
    }

    algo.rgbMapStepCount = function(width, height)
    {
        if (algo.orientation == 0)
            return (width * 2) - 1;
        else
            return (height * 2) - 1;
    }

    // Development tool access
    testAlgo = algo;

    return algo;
    }
)()
