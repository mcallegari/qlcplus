/*
  Q Light Controller Plus
  empty.js

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
    algo.name = "Script name";
    algo.author = "Your Name";
    algo.properties = new Array();

    /**
      * The actual "algorithm" for this RGB script. Produces a map of
      * size($width, $height) each time it is called.
      *
      * @param step The step number that is requested (0 to (algo.rgbMapStepCount - 1))
      * @param rgb Tells the color requested by user in the UI.
      * @return A two-dimensional array[height][width].
      */
    algo.rgbMap = function(width, height, rgb, step)
    {
	var map = new Array(height);
	for (var y = 0; y < height; y++)
	{
	    map[y] = new Array();
	    for (var x = 0; x < width; x++)
	      map[y][x] = 0; // <-- elapsed color goes here
	}

	return map;
    }

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
    }

    // Development tool access
    testAlgo = algo;

    return algo;
    }
)()
