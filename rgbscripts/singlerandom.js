/*
  Q Light Controller
  singlerandom.js

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
     * This algorithm produces RGB maps where exactly one pixel is lit at a time
     * on a single map (=step). The pixel is chosen at random for each step, ensuring
     * that each pixel appears exactly once per round ($width * $height steps).
     */
    function()
    {
        var algo = new Object;
        algo.apiVersion = 1;
        algo.name = "Single Random";
        algo.author = "Heikki Junnila";
        algo.initialized = false;
        algo.rgb = 0;
        algo.width = 0;
        algo.height = 0;

        var util = new Object;
        /**
         * Create an array of length($width * height), with each item
         * containing a sub-array representing one point (y, x) in a map.
         */
        util.createStepList = function(width, height)
        {
            var list = new Array(height * width);
            var i = 0;
            for (var y = 0; y < height; y++)
            {
                for (var x = 0; x < width; x++)
                {
                    list[i] = [y, x];
                    i++;
                }
            }

            return list;
        }

        /**
         * Create one full map of size($width, $height), where exactly
         * one cell($sx, $sy) is colored with $rgb. Leave other cells
         * black (0).
         */
        util.createStep = function(width, height, sy, sx, rgb)
        {
            var map = new Array(height);
            for (var y = 0; y < height; y++)
            {
                map[y] = new Array(width);
                for (var x = 0; x < width; x++)
                {
                    if (sy == y && sx == x)
                        map[y][x] = rgb;
                    else
                        map[y][x] = 0;
                }
            }

            return map;
        }

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
            // Create a new step list only when attributes change to keep the
            // script running with as little extra overhead as possible.
            if (algo.rgb != rgb || algo.width != width || algo.height != height)
            {
                // To ensure that ALL points are included in the steps exactly
                // once, a list of possible steps is created (from (0,0) to (w-1,h-1)).
                var stepList = util.createStepList(width, height);

                algo.steps = new Array(width * height);
                for (var i = 0; i < width * height; i++)
                {
                    // Pick a random index from the list of possible steps. The
                    // item in the list tells the actual map point that is lit
                    // in this step.
                    var index = Math.floor(Math.random() * (stepList.length));
                    var yx = stepList[index];
                    var map = util.createStep(width, height, yx[0], yx[1], rgb);
                    algo.steps[i] = map;

                    // Remove the used item from the list of possible steps so that
                    // it won't be picked again.
                    stepList.splice(index, 1);
                }

                algo.rgb = rgb;
                algo.width = width;
                algo.height = height;
            }

            return algo.steps[step];
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
            // All pixels in the map must be used exactly once, each one separately
            // at a time. Therefore, the maximum number of steps produced by this
            // script on a 5 * 5 grid is 25.
            return width * height;
        }

        // Development tool access
        testAlgo = algo;

        return algo;
    }
)()
