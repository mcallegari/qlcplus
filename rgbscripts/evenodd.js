/*
  Q Light Controller
  evenodd.js

  Copyright (c) Heikki Junnila

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  Version 2 as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details. The license is
  in the file "COPYING".

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

// Development tool access
var testAlgo;

(
    /**
     * This algorithm produces a very simple two-step even/odd pattern. Step one lights
     * odd pixels and step two lights even pixels.
     */
    function()
    {
        var algo = new Object;
        algo.apiVersion = 1;
        algo.name = "Even/Odd";
        algo.author = "Heikki Junnila";

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
            var i = step;
            for (var y = 0; y < height; y++)
            {
                map[y] = new Array();
                for (var x = 0; x < width; x++)
                {
                    if ((i % 2) == 0)
                        map[y][x] = rgb;
                    else
                        map[y][x] = 0;
                    i++;
                }
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
            // Only two steps; one for even pixels and another for odd pixels
            return 2;
        }

        // Development tool access
        testAlgo = algo;

        return algo;
    }
)()
