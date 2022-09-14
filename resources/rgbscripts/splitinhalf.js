/*
  Q Light Controller
  splitinhalf.js

  Copyright (c) Jan Fries

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

(
    /**
     * This script splits an array into two halfs and alternates between them.
     */
    function()
    {
        var algo = new Object;
        algo.apiVersion = 2;
        algo.name = "Split in Half";
        algo.author = "Jan Fries";
		
		algo.properties = new Array();
		
		algo.orientation = 0;
		algo.properties.push("name:orientation|type:list|" + "display:Orientation|values:Horizontal,Vertical|" + "write:setOrientation|read:getOrientation");
		algo.setOrientation = function(_orientation) {
			if (_orientation === "Vertical") {
				algo.orientation = 1;
			} else {
				algo.orientation = 0;
			}
		};
		algo.getOrientation = function() {
		if (parseInt(algo.orientation) === 1) {
			return "Vertical";
		} else {
			return "Horizontal";
		}
  };

        algo.rgbMap = function(width, height, rgb, step)
        {
            var map = new Array(height);
            var i = step;
            for (var y = 0; y < height; y++)
            {
                map[y] = new Array();
                for (var x = 0; x < width; x++)
                {
					if (algo.orientation === 0) {
						if ((i % 2) === 0 ) {
							if (x <= (width / 2 - 1)) {
								map[y][x] = rgb;
							} else {
								map[y][x] = 0;
							}
						} else {
							if (x <= (width / 2 - 1)) {
								map[y][x] = 0;
							} else {
								map[y][x] = rgb;
							}
						}
					} else {
						if ((i % 2) === 0 ) {
							if (y <= (height / 2 - 1)) {
								map[y][x] = rgb;
							} else {
								map[y][x] = 0;
							}
						} else {
							if (y <= (height / 2 - 1)) {
								map[y][x] = 0;
							} else {
								map[y][x] = rgb;
							}
						}
					}
                }
            }

            return map;
        };

        algo.rgbMapStepCount = function(width, height)
        {
            return 2;
        };

        return algo;
    }
)();
