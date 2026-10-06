/* This approach doesn't work, needs to refer to a Property that is set via API ect
*/


/*
  Q Light Controller Plus
  levelGauge.js

  Level gauge RGB script.
  Displays a fill level driven by the QLC+ step value (0-255).

  Intended use: ArtNet/sACN input (0-255) mapped directly to the RGB Matrix
  step via QLC+ input mapping. 255 = full / 0 = empty.

  Display Modes:
    Bar Horizontal     - fills left to right
    Bar Vertical       - fills bottom to top
    Bar Horizontal Rev - fills right to left
    Bar Vertical Rev   - fills top to bottom
    Bar Split          - fills from centre outward (both directions)
    Radial             - pie/sweep fill clockwise from 12 o'clock
    Radial Expand      - expanding filled circle outward from centre

  Colours:
    acceptColors = 2
    rgb  = fill colour
    rgb2 = background colour (unfilled portion)
           set rgb2 to black (0,0,0) to leave unfilled pixels dark

  Author: Stuart Hanlon

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

var testAlgo;

(function () {

    var algo = new Object;
    algo.apiVersion   = 2;
    algo.name         = "Level Gauge";
    algo.author       = "Stuart Hanlon";
    algo.acceptColors = 2;  // rgb = fill colour, rgb2 = background colour

    // -------------------------------------------------------------------------
    // Internal state
    // -------------------------------------------------------------------------
    algo.displayMode = 0;
    // 0 = Bar Horizontal
    // 1 = Bar Vertical
    // 2 = Bar Horizontal Rev
    // 3 = Bar Vertical Rev
    // 4 = Bar Split
    // 5 = Radial
    // 6 = Radial Expand

    // -------------------------------------------------------------------------
    // Properties
    // -------------------------------------------------------------------------
    algo.properties = new Array();
    algo.properties.push(
        "name:displayMode|type:list|display:Display Mode|values:Bar Horizontal,Bar Vertical,Bar Horizontal Rev,Bar Vertical Rev,Bar Split,Radial,Radial Expand|write:setDisplayMode|read:getDisplayMode"
    );

    // -------------------------------------------------------------------------
    // Accessors
    // -------------------------------------------------------------------------
    algo.setDisplayMode = function (v) {
        if      (v === "Bar Vertical")     { algo.displayMode = 1; }
        else if (v === "Bar Horizontal Rev"){ algo.displayMode = 2; }
        else if (v === "Bar Vertical Rev") { algo.displayMode = 3; }
        else if (v === "Bar Split")        { algo.displayMode = 4; }
        else if (v === "Radial")           { algo.displayMode = 5; }
        else if (v === "Radial Expand")    { algo.displayMode = 6; }
        else                               { algo.displayMode = 0; }
    };

    algo.getDisplayMode = function () {
        if (algo.displayMode === 1) { return "Bar Vertical";      }
        if (algo.displayMode === 2) { return "Bar Horizontal Rev";}
        if (algo.displayMode === 3) { return "Bar Vertical Rev";  }
        if (algo.displayMode === 4) { return "Bar Split";         }
        if (algo.displayMode === 5) { return "Radial";            }
        if (algo.displayMode === 6) { return "Radial Expand";     }
        return "Bar Horizontal";
    };

    // -------------------------------------------------------------------------
    // Colour helpers
    // -------------------------------------------------------------------------
    function splitRgb(rgb) {
        return {
            r: (rgb >> 16) & 0xFF,
            g: (rgb >>  8) & 0xFF,
            b:  rgb        & 0xFF
        };
    }

    function mergeRgb(r, g, b) {
        return (Math.min(255, Math.max(0, Math.round(r))) << 16) |
               (Math.min(255, Math.max(0, Math.round(g))) <<  8) |
                Math.min(255, Math.max(0, Math.round(b)));
    }

    // -------------------------------------------------------------------------
    // rgbMapStepCount
    // 255 steps = direct 1:1 mapping with DMX/ArtNet value
    // 255 = full, 0 = empty
    // -------------------------------------------------------------------------
    algo.rgbMapStepCount = function (width, height) {
        return 256;  // steps 0-255
    };

    // -------------------------------------------------------------------------
    // rgbMap
    // step: 0-255 representing the fill level (255 = full, 0 = empty)
    // rgb:  fill colour
    // rgb2: background colour
    // -------------------------------------------------------------------------
    algo.rgbMap = function (width, height, rgb, rgb2) {

        // In the devtool rgb2 arrives as the step parameter
        // In QLC+ with acceptColors=2, rgb2 is the second colour
        // We detect which we have by checking if rgb2 > 255 (a colour) or <= 255 (a step)
        var step, bgRgb;
        if (rgb2 > 255) {
            // Running inside QLC+ with two colours — extract step from somewhere else
            // QLC+ passes step as the 4th argument when acceptColors=2
            step  = arguments[4] !== undefined ? arguments[4] : 127;
            bgRgb = rgb2;
        } else {
            // Running in devtool — rgb2 is actually the step
            step  = rgb2;
            bgRgb = 0;  // black background in devtool
        }

        // Normalise level to 0.0-1.0
        // 255 = full (1.0), 0 = empty (0.0)
        var level = Math.max(0, Math.min(255, step)) / 255;

        var fillCol = splitRgb(rgb);
        var backCol = splitRgb(bgRgb);

        // Centre of matrix (for radial modes)
        var cx = width  / 2 - 0.5;
        var cy = height / 2 - 0.5;
        var maxRadius = Math.min(width, height) / 2;

        var map = new Array(height);
        for (var y = 0; y < height; y++) {
            map[y] = new Array(width);
            for (var x = 0; x < width; x++) {

                var filled = false;

                if (algo.displayMode === 0) {
                    // Bar Horizontal: fill left to right
                    var threshold = level * width;
                    filled = (x < threshold);

                } else if (algo.displayMode === 1) {
                    // Bar Vertical: fill bottom to top
                    var threshold = (1 - level) * height;
                    filled = (y >= threshold);

                } else if (algo.displayMode === 2) {
                    // Bar Horizontal Reversed: fill right to left
                    var threshold = (1 - level) * width;
                    filled = (x >= threshold);

                } else if (algo.displayMode === 3) {
                    // Bar Vertical Reversed: fill top to bottom
                    var threshold = level * height;
                    filled = (y < threshold);

                } else if (algo.displayMode === 4) {
                    // Bar Split: fill from centre outward horizontally
                    var centre    = width / 2;
                    var halfFill  = level * centre;
                    filled = (x >= centre - halfFill && x <= centre + halfFill);

                } else if (algo.displayMode === 5) {
                    // Radial: pie sweep clockwise from 12 o'clock
                    var offx  = x - cx;
                    var offy  = y - cy;
                    var dist  = Math.sqrt(offx * offx + offy * offy);
                    if (dist <= maxRadius) {
                        // Angle clockwise from 12 o'clock, 0 to 2*PI
                        var angle = Math.atan2(offx, -offy);
                        if (angle < 0) { angle += 2 * Math.PI; }
                        filled = (angle <= level * 2 * Math.PI);
                    }

                } else if (algo.displayMode === 6) {
                    // Radial Expand: filled circle growing from centre
                    var offx = x - cx;
                    var offy = y - cy;
                    var dist = Math.sqrt(offx * offx + offy * offy);
                    filled = (dist <= level * maxRadius);
                }

                // Assign fill or background colour
                if (filled) {
                    map[y][x] = mergeRgb(fillCol.r, fillCol.g, fillCol.b);
                } else {
                    map[y][x] = mergeRgb(backCol.r, backCol.g, backCol.b);
                }
            }
        }

        return map;
    };

    // Development tool access
    testAlgo = algo;

    return algo;

}());
