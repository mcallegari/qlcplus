/*
  Q Light Controller Plus
  clockHands.js

  Analogue clock hands RGB script.
  Draws hour and minute hands on a square (or near-square) LED matrix.

  Derived from circular.js by Hans-Jürgen Tappe
  Author: Stuart Hanlon

  Properties:
    Show Hour Hand    : Yes / No
    Show Minute Hand  : Yes / No
    Hand Style        : Arrow, Paddle, Lollipop, Skeleton, Breguet
    Hand Width %      : 1-20  (blade width as % of radius)
    Hour Length %     : 20-90 (hour hand length as % of radius)
    Minute Length %   : 20-100(minute hand length as % of radius)
    Centre Gap %      : 0-30  (blank hub at centre as % of radius)

  Colours:
    acceptColors = 2
    rgb    = minute hand colour
    rgb2   = hour hand colour

  Clock behaviour:
    - Hour hand moves smoothly between hours (tracks minutes)
    - Minute hand points to exact current minute
    - Both hands use real time via new Date() on every frame

  Masking:
    Set Show Hour Hand or Show Minute Hand to No to use as a mask
    in a QLC+ Collection layered over another RGB effect.

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
    algo.name         = "Clock Hands";
    algo.author       = "Stuart Hanlon";
    algo.acceptColors = 2;  // rgb = minute hand, rgb2 = hour hand

    // -------------------------------------------------------------------------
    // Internal state defaults
    // -------------------------------------------------------------------------
    algo.showHour     = 1;    // 1 = Yes, 0 = No
    algo.showMinute   = 1;
    algo.handStyle    = 0;    // 0=Arrow, 1=Paddle, 2=Lollipop, 3=Skeleton, 4=Breguet
    algo.handWidth    = 5;    // % of radius
    algo.hourLength   = 55;   // % of radius
    algo.minuteLength = 85;   // % of radius
    algo.centreGap    = 10;   // % of radius

    // -------------------------------------------------------------------------
    // Properties
    // -------------------------------------------------------------------------
    algo.properties = new Array();
    algo.properties.push(
        "name:showHour|type:list|display:Show Hour Hand|values:Yes,No|write:setShowHour|read:getShowHour"
    );
    algo.properties.push(
        "name:showMinute|type:list|display:Show Minute Hand|values:Yes,No|write:setShowMinute|read:getShowMinute"
    );
    algo.properties.push(
        "name:handStyle|type:list|display:Hand Style|values:Arrow,Paddle,Lollipop,Skeleton,Breguet|write:setHandStyle|read:getHandStyle"
    );
    algo.properties.push(
        "name:handWidth|type:range|display:Hand Width %|values:1,20|write:setHandWidth|read:getHandWidth"
    );
    algo.properties.push(
        "name:hourLength|type:range|display:Hour Length %|values:20,90|write:setHourLength|read:getHourLength"
    );
    algo.properties.push(
        "name:minuteLength|type:range|display:Minute Length %|values:20,100|write:setMinuteLength|read:getMinuteLength"
    );
    algo.properties.push(
        "name:centreGap|type:range|display:Centre Gap %|values:0,30|write:setCentreGap|read:getCentreGap"
    );

    // -------------------------------------------------------------------------
    // Accessors
    // -------------------------------------------------------------------------
    algo.setShowHour    = function (v) { algo.showHour    = (v === "Yes") ? 1 : 0; };
    algo.getShowHour    = function ()  { return (algo.showHour    === 1) ? "Yes" : "No"; };

    algo.setShowMinute  = function (v) { algo.showMinute  = (v === "Yes") ? 1 : 0; };
    algo.getShowMinute  = function ()  { return (algo.showMinute  === 1) ? "Yes" : "No"; };

    algo.setHandStyle   = function (v) {
        if      (v === "Paddle")   { algo.handStyle = 1; }
        else if (v === "Lollipop") { algo.handStyle = 2; }
        else if (v === "Skeleton") { algo.handStyle = 3; }
        else if (v === "Breguet")  { algo.handStyle = 4; }
        else                       { algo.handStyle = 0; }
    };
    algo.getHandStyle   = function () {
        if (algo.handStyle === 1) { return "Paddle";   }
        if (algo.handStyle === 2) { return "Lollipop"; }
        if (algo.handStyle === 3) { return "Skeleton"; }
        if (algo.handStyle === 4) { return "Breguet";  }
        return "Arrow";
    };

    algo.setHandWidth   = function (v) { algo.handWidth   = Math.max(1,  Math.min(20,  parseInt(v, 10))); };
    algo.getHandWidth   = function ()  { return algo.handWidth;   };

    algo.setHourLength  = function (v) { algo.hourLength  = Math.max(20, Math.min(90,  parseInt(v, 10))); };
    algo.getHourLength  = function ()  { return algo.hourLength;  };

    algo.setMinuteLength= function (v) { algo.minuteLength= Math.max(20, Math.min(100, parseInt(v, 10))); };
    algo.getMinuteLength= function ()  { return algo.minuteLength; };

    algo.setCentreGap   = function (v) { algo.centreGap   = Math.max(0,  Math.min(30,  parseInt(v, 10))); };
    algo.getCentreGap   = function ()  { return algo.centreGap;   };

    // -------------------------------------------------------------------------
    // Geometry helpers
    // -------------------------------------------------------------------------

    // Angle from centre to pixel, measured clockwise from 12 o'clock (north)
    // Returns value in range 0 to 2*PI
    function pixelAngle(offx, offy) {
        // atan2 measures from east (positive x), anticlockwise
        // We want clockwise from north, so swap axes and adjust
        var angle = Math.atan2(offx, -offy);
        if (angle < 0) { angle += 2 * Math.PI; }
        return angle;
    }

    // Angular difference, normalised to -PI..PI
    function angleDiff(a, b) {
        var d = ((a - b) + 3 * Math.PI) % (2 * Math.PI) - Math.PI;
        return d;
    }

    // Decode rgb integer to {r,g,b}
    function splitRgb(rgb) {
        return {
            r: (rgb >> 16) & 0xFF,
            g: (rgb >>  8) & 0xFF,
            b:  rgb        & 0xFF
        };
    }

    // Encode {r,g,b} to rgb integer, clamping to 0-255
    function mergeRgb(r, g, b) {
        return (Math.min(255, Math.max(0, Math.round(r))) << 16) |
               (Math.min(255, Math.max(0, Math.round(g))) <<  8) |
                Math.min(255, Math.max(0, Math.round(b)));
    }

    // -------------------------------------------------------------------------
    // Hand pixel test
    // Returns a factor 0.0-1.0 indicating whether a pixel at (offx, offy)
    // from centre is part of a hand pointing at handAngle,
    // with the given maxRadius, widthFrac (half-width as fraction of radius),
    // and gapRadius (centre hub).
    // style: 0=Arrow, 1=Paddle, 2=Lollipop, 3=Skeleton, 4=Breguet
    // -------------------------------------------------------------------------
    function handPixel(offx, offy, handAngle, maxRadius, widthFrac, gapRadius, style) {

        var pointRadius = Math.sqrt(offx * offx + offy * offy);

        // Outside the hand's reach or inside the centre gap
        if (pointRadius > maxRadius || pointRadius < gapRadius) { return 0; }

        var pAngle = pixelAngle(offx, offy);
        var diff   = angleDiff(pAngle, handAngle);

        // Radial progress along the hand (0 at gap, 1 at tip)
        var radialProgress = (pointRadius - gapRadius) / (maxRadius - gapRadius);
        radialProgress = Math.max(0, Math.min(1, radialProgress));

        // Half-width in radians at this point, varies by style
        var halfWidthRad;

        if (style === 0) {
            // Arrow: tapers from full width at base to zero at tip
            halfWidthRad = widthFrac * (1 - radialProgress);

        } else if (style === 1) {
            // Paddle: constant width along full length
            halfWidthRad = widthFrac;

        } else if (style === 2) {
            // Lollipop: thin shaft (20% of width) with circular bulb at tip
            // Bulb occupies top 15% of hand length
            var bulbStart = 0.85;
            if (radialProgress >= bulbStart) {
                // Bulb — treat as circle centred at tip
                var tipX = Math.sin(handAngle) * maxRadius;
                var tipY = -Math.cos(handAngle) * maxRadius;
                var bulbR = widthFrac * maxRadius * 1.5;
                var dx = offx - tipX;
                var dy = offy - tipY;
                return (Math.sqrt(dx*dx + dy*dy) <= bulbR) ? 1 : 0;
            } else {
                halfWidthRad = widthFrac * 0.2;
            }

        } else if (style === 3) {
            // Skeleton: outlined hand — lit only near the edges of the blade
            // Uses Arrow taper for outer edge, with hollow centre
            var outerHalf = widthFrac * (1 - radialProgress);
            var innerHalf = outerHalf * 0.4;
            var absDiff   = Math.abs(diff);
            return (absDiff <= outerHalf && absDiff >= innerHalf) ? 1 : 0;

        } else if (style === 4) {
            // Breguet: tapered shaft with a teardrop cutout near the tip
            // Shaft tapers like Arrow, cutout is an elliptical hole 60-80% along
            var cutoutStart = 0.58;
            var cutoutEnd   = 0.82;
            halfWidthRad = widthFrac * (1 - radialProgress);
            if (radialProgress >= cutoutStart && radialProgress <= cutoutEnd) {
                // Inside the cutout zone — only light the outer rim
                var cutoutDepth = widthFrac * 0.5;
                if (Math.abs(diff) < halfWidthRad - cutoutDepth) { return 0; }
            }

        } else {
            halfWidthRad = widthFrac * (1 - radialProgress);
        }

        // General case: pixel is lit if angular distance is within halfWidthRad
        return (Math.abs(diff) <= halfWidthRad) ? 1 : 0;
    }

    // -------------------------------------------------------------------------
    // rgbMapStepCount — 60 steps for smooth devtool animation
    // -------------------------------------------------------------------------
    algo.rgbMapStepCount = function (width, height) {
        return 60;
    };

    // -------------------------------------------------------------------------
    // rgbMap
    // -------------------------------------------------------------------------
    algo.rgbMap = function (width, height, rgb, rgb2) {

        // Live time
        var now    = new Date();
        var hour   = now.getHours() % 12;   // 0-11
        var minute = now.getMinutes();       // 0-59
        var second = now.getSeconds();       // 0-59

        // Smooth hand angles (clockwise from 12)
        // Minute hand moves with seconds for sub-minute smoothness
        var minuteAngle = ((minute + second / 60) / 60) * 2 * Math.PI;
        // Hour hand moves smoothly through the hour
        var hourAngle   = ((hour + minute / 60) / 12) * 2 * Math.PI;

        // Matrix geometry
        var cx = width  / 2 - 0.5;   // float centre x
        var cy = height / 2 - 0.5;   // float centre y
        var radius = Math.min(width, height) / 2;

        // Convert % properties to pixel/radian values
        var gapRadius    = radius * (algo.centreGap    / 100);
        var hourRadius   = radius * (algo.hourLength   / 100);
        var minuteRadius = radius * (algo.minuteLength / 100);
        // widthFrac: half-width of hand as fraction of its own radius
        // expressed as radians — PI/180 * degrees, roughly
        // We map handWidth% to an angular fraction scaled to the matrix
        var widthFrac    = (algo.handWidth / 100) * Math.PI * 0.5;

        // Decode colours
        // rgb  = minute hand (first colour)
        // rgb2 = hour hand   (second colour)
        var minCol = splitRgb(rgb);
        var hourCol = splitRgb(rgb2 !== undefined ? rgb2 : rgb);

        // Build map
        var map = new Array(height);
        for (var y = 0; y < height; y++) {
            map[y] = new Array(width);
            for (var x = 0; x < width; x++) {

                var offx = x - cx;
                var offy = y - cy;

                var r = 0, g = 0, b = 0;

                // Draw minute hand first, then hour hand on top
                if (algo.showMinute === 1) {
                    var mf = handPixel(offx, offy, minuteAngle, minuteRadius,
                                       widthFrac, gapRadius, algo.handStyle);
                    if (mf > 0) {
                        r += minCol.r * mf;
                        g += minCol.g * mf;
                        b += minCol.b * mf;
                    }
                }

                if (algo.showHour === 1) {
                    var hf = handPixel(offx, offy, hourAngle, hourRadius,
                                       widthFrac, gapRadius, algo.handStyle);
                    if (hf > 0) {
                        // Hour hand drawn on top — replace, don't add
                        r = hourCol.r * hf;
                        g = hourCol.g * hf;
                        b = hourCol.b * hf;
                    }
                }

                map[y][x] = mergeRgb(r, g, b);
            }
        }

        return map;
    };

    // Development tool access
    testAlgo = algo;

    return algo;

}());
