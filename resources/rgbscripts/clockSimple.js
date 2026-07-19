/*
  Q Light Controller Plus
  clockSimple.js

  Real-time clock RGB script.
  Scales to any matrix size - no fixed pixel count required.

  Time Unit:   Hour, Minute or Second
  Hour Format: 12h (0-11) or 24h (0-23)
  Orientation: Horizontal or Vertical
  Display Mode:
    Dot      - a dot travels across the full matrix width/height
               dot width is set as a percentage of the matrix (1-50%, default 5%)
               e.g. 5% on a 186-pixel strip = ~9 pixels wide, centred on the time position
    Progress - pixels fill from 0 up to the current time position (progress bar)

  Derived from stripes.js by Massimo Callegari
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

// Development tool access
var testAlgo;

(function () {

    var algo = new Object;
    algo.apiVersion   = 2;
    algo.name         = "Clock Simple";
    algo.author       = "Stuart Hanlon";
    algo.acceptColors = 1;

    // Internal state
    algo.orientation  = 0;  // 0 = Horizontal, 1 = Vertical
    algo.cron         = 1;  // 0 = Hour, 1 = Minute, 2 = Second
    algo.hourFormat   = 0;  // 0 = 12h, 1 = 24h
    algo.displayMode  = 0;  // 0 = Dot, 1 = Progress
    algo.dotWidth     = 5;  // dot width as a percentage of matrix dimension (1-50)

    // -------------------------------------------------------------------------
    // Properties
    // -------------------------------------------------------------------------
    algo.properties = new Array();
    algo.properties.push(
        "name:orientation|type:list|display:Orientation|values:Horizontal,Vertical|write:setOrientation|read:getOrientation"
    );
    algo.properties.push(
        "name:cron|type:list|display:Time Unit|values:Hour,Minute,Second|write:setCron|read:getCron"
    );
    algo.properties.push(
        "name:hourFormat|type:list|display:Hour Format|values:12h,24h|write:setHourFormat|read:getHourFormat"
    );
    algo.properties.push(
        "name:displayMode|type:list|display:Display Mode|values:Dot,Progress|write:setDisplayMode|read:getDisplayMode"
    );
    algo.properties.push(
        "name:dotWidth|type:range|display:Dot Width %|values:1,50|write:setDotWidth|read:getDotWidth"
    );

    // -------------------------------------------------------------------------
    // Orientation accessors
    // -------------------------------------------------------------------------
    algo.setOrientation = function (val) {
        algo.orientation = (val === "Vertical") ? 1 : 0;
    };
    algo.getOrientation = function () {
        return (algo.orientation === 1) ? "Vertical" : "Horizontal";
    };

    // -------------------------------------------------------------------------
    // Cron (time unit) accessors
    // -------------------------------------------------------------------------
    algo.setCron = function (val) {
        if      (val === "Hour")   { algo.cron = 0; }
        else if (val === "Minute") { algo.cron = 1; }
        else if (val === "Second") { algo.cron = 2; }
        else                       { algo.cron = 1; }
    };
    algo.getCron = function () {
        if (algo.cron === 0) { return "Hour";   }
        if (algo.cron === 2) { return "Second"; }
        return "Minute";
    };

    // -------------------------------------------------------------------------
    // Hour format accessors (only relevant when cron = Hour)
    // -------------------------------------------------------------------------
    algo.setHourFormat = function (val) {
        algo.hourFormat = (val === "24h") ? 1 : 0;
    };
    algo.getHourFormat = function () {
        return (algo.hourFormat === 1) ? "24h" : "12h";
    };

    // -------------------------------------------------------------------------
    // Display mode accessors
    // -------------------------------------------------------------------------
    algo.setDisplayMode = function (val) {
        algo.displayMode = (val === "Progress") ? 1 : 0;
    };
    algo.getDisplayMode = function () {
        return (algo.displayMode === 1) ? "Progress" : "Dot";
    };

    // -------------------------------------------------------------------------
    // Dot width accessors (only relevant in Dot mode)
    // -------------------------------------------------------------------------
    algo.setDotWidth = function (val) {
        var v = parseInt(val, 10);
        if (v < 1)  { v = 1;  }
        if (v > 50) { v = 50; }
        algo.dotWidth = v;
    };
    algo.getDotWidth = function () {
        return algo.dotWidth;
    };

    // -------------------------------------------------------------------------
    // rgbMapStepCount
    // 60 steps keeps the devtool animating smoothly at 1000ms intervals.
    // -------------------------------------------------------------------------
    algo.rgbMapStepCount = function (width, height) {
        return 60;
    };

    // -------------------------------------------------------------------------
    // rgbMap  -  called every tick; fetches live time here
    // -------------------------------------------------------------------------
    algo.rgbMap = function (width, height, rgb, step) {

        // Get the current real time on every call
        var now    = new Date();
        var hour   = now.getHours();    // 0-23
        var minute = now.getMinutes();  // 0-59
        var second = now.getSeconds();  // 0-59

        // Work out the time value and its maximum possible value
        var timeValue, timeMax;
        if (algo.cron === 0) {
            if (algo.hourFormat === 1) {
                timeValue = hour;       // 0-23
                timeMax   = 24;
            } else {
                timeValue = hour % 12;  // 0-11
                timeMax   = 12;
            }
        } else if (algo.cron === 2) {
            timeValue = second;         // 0-59
            timeMax   = 60;
        } else {
            timeValue = minute;         // 0-59
            timeMax   = 60;
        }

        // Scale the time value proportionally across the matrix dimension
        // e.g. minute 30 on a 186-pixel strip  -->  pixel 93
        var dimension   = (algo.orientation === 0) ? width : height;
        var scaledPos   = Math.round((timeValue / timeMax) * (dimension - 1));

        // Build the pixel map
        var map = new Array(height);
        for (var y = 0; y < height; y++) {
            map[y] = new Array(width);
            for (var x = 0; x < width; x++) {

                // Which axis position are we evaluating?
                var pos = (algo.orientation === 0) ? x : y;

                var lit;
                if (algo.displayMode === 1) {
                    // Progress mode: light everything from 0 up to scaledPos
                    lit = (pos <= scaledPos);
                } else {
                    // Dot mode: light a band of pixels centred on scaledPos
                    // halfSpan is how many pixels either side of centre to light
                    var spanPixels = Math.max(1, Math.round((algo.dotWidth / 100) * dimension));
                    var halfSpan   = Math.floor(spanPixels / 2);
                    lit = (pos >= scaledPos - halfSpan && pos <= scaledPos + halfSpan);
                }

                map[y][x] = lit ? rgb : 0;
            }
        }

        return map;
    };

    // Development tool access
    testAlgo = algo;

    return algo;

}());
