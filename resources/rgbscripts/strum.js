/*
  Q Light Controller Plus
  strum.js

  Copyright (c) Branson Matheson

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
void testAlgo; // Reserved for development/debugging

(function() {
    var algo = {};
    algo.apiVersion = 2;
    algo.name = "Strum";
    algo.author = "Branson Matheson with help from Augment";
    algo.acceptColors = 1;

    algo.properties = new Array();

    // String count (4-6)
    algo.stringCount = 6;
    algo.properties.push("name:stringCount|type:range|display:String Count|values:4,6|write:setStringCount|read:getStringCount");

    // String width (1-3 pixels)
    algo.stringWidth = 1;
    algo.properties.push("name:stringWidth|type:range|display:String Width|values:1,3|write:setStringWidth|read:getStringWidth");

    // String spacing (1-3 pixels between strings)
    algo.stringSpacing = 1;
    algo.properties.push("name:stringSpacing|type:range|display:String Spacing|values:1,3|write:setStringSpacing|read:getStringSpacing");

    // Strum type
    var STRUM_LABELS = ["Down", "Up", "Up-Down", "Constant", "Pick Bottom"];
    algo.strumType = 0;
    algo.properties.push("name:strumType|type:list|display:Strum Type|values:" + STRUM_LABELS.join(',') + "|write:setStrumType|read:getStrumType");

    // Strum speed (1-10)
    algo.speed = 5;
    algo.properties.push("name:speed|type:range|display:Speed|values:1,10|write:setSpeed|read:getSpeed");

    // Vibration intensity (how much strings vibrate after being strummed)
    algo.vibration = 5;
    algo.properties.push("name:vibration|type:range|display:Vibration|values:1,10|write:setVibration|read:getVibration");

    // Setters and getters
    algo.setStringCount = function(v) { 
        var n = parseInt(v, 10); 
        if (!isNaN(n)) algo.stringCount = Math.max(4, Math.min(6, n));
    };
    algo.getStringCount = function() { return algo.stringCount; };

    algo.setStringWidth = function(v) { var n = parseInt(v, 10); if (!isNaN(n)) algo.stringWidth = Math.max(1, Math.min(3, n)); };
    algo.getStringWidth = function() { return algo.stringWidth; };

    algo.setStringSpacing = function(v) { var n = parseInt(v, 10); if (!isNaN(n)) algo.stringSpacing = Math.max(1, Math.min(3, n)); };
    algo.getStringSpacing = function() { return algo.stringSpacing; };

    algo.setStrumType = function(label) {
        for (var i = 0; i < STRUM_LABELS.length; i++) {
            if (label === STRUM_LABELS[i]) { algo.strumType = i; return; }
        }
    };
    algo.getStrumType = function() { return STRUM_LABELS[algo.strumType]; };

    algo.setSpeed = function(v) { var n = parseInt(v, 10); if (!isNaN(n)) algo.speed = Math.max(1, Math.min(10, n)); };
    algo.getSpeed = function() { return algo.speed; };

    algo.setVibration = function(v) { var n = parseInt(v, 10); if (!isNaN(n)) algo.vibration = Math.max(1, Math.min(10, n)); };
    algo.getVibration = function() { return algo.vibration; };

    var util = {};
    util.frameCount = 0;
    util.stringStates = [];  // Track each string's vibration state for constant mode

    // Utility functions
    function scaleColor(rgb, brightness) {
        brightness = Math.max(0, Math.min(255, brightness));
        var r = ((rgb >> 16) & 0xFF) * brightness / 255;
        var g = ((rgb >> 8) & 0xFF) * brightness / 255;
        var b = (rgb & 0xFF) * brightness / 255;
        return (Math.floor(r) << 16) | (Math.floor(g) << 8) | Math.floor(b);
    }

    function makeMap(width, height, value) {
        var m = new Array(height);
        for (var y = 0; y < height; y++) {
            m[y] = new Array(width);
            for (var x = 0; x < width; x++) m[y][x] = value;
        }
        return m;
    }

    // Calculate total width needed for strings
    function getStringsWidth() {
        return algo.stringCount * algo.stringWidth + (algo.stringCount - 1) * algo.stringSpacing;
    }

    // Get the Y position of a string (center of string)
    function getStringY(stringIndex, height) {
        var totalWidth = getStringsWidth();
        var startY = Math.floor((height - totalWidth) / 2);
        return startY + stringIndex * (algo.stringWidth + algo.stringSpacing) + Math.floor(algo.stringWidth / 2);
    }

    algo.rgbMapStepCount = function(width, height) {
        void width; void height; // QLC+ API requirement
        // Constant and Pick Bottom modes loop forever
        if (algo.strumType === 3 || algo.strumType === 4) return 2;

        // Steps: strum across + vibration decay
        // For up-down, we need double the strum time
        var strumSteps = algo.stringCount * 3;
        if (algo.strumType === 2) strumSteps *= 2;  // Up-Down needs more steps
        var vibrateSteps = 40;  // Vibration decay time
        return strumSteps + vibrateSteps;
    };

    algo.rgbMap = function(width, height, rgb, step) {
        // Default color: gold/bronze string color if no color provided
        if (rgb === 0) rgb = 0xDAA520;

        var map = makeMap(width, height, 0);
        var speedFactor = algo.speed / 5;
        var vibrationAmp = algo.vibration / 10;
        var strumDelay = 3 / speedFactor;  // Frames between each string hit

        // Initialize string states for constant mode
        if (util.stringStates.length !== algo.stringCount) {
            util.stringStates = [];
            for (var i = 0; i < algo.stringCount; i++) {
                util.stringStates.push({ lastHitTime: -1000 });
            }
        }

        // Track when each string was hit (step number)
        var stringHitTimes = [];

        // Constant mode: continuous strumming with looping
        if (algo.strumType === 3) {
            util.frameCount++;
            var cycleLength = algo.stringCount * strumDelay + 20;  // Full strum + pause
            var cyclePos = util.frameCount % cycleLength;

            // Alternate direction each cycle
            var cycleNum = Math.floor(util.frameCount / cycleLength);
            var goingDown = (cycleNum % 2 === 0);

            for (var s = 0; s < algo.stringCount; s++) {
                var hitTime;
                if (goingDown) {
                    hitTime = s * strumDelay;
                } else {
                    hitTime = (algo.stringCount - 1 - s) * strumDelay;
                }
                // Check if this string should be hit this cycle
                if (cyclePos >= hitTime && cyclePos < hitTime + 1) {
                    util.stringStates[s].lastHitTime = util.frameCount;
                }
                stringHitTimes.push(util.stringStates[s].lastHitTime);
            }
        } else if (algo.strumType === 4) {
            // Pick Bottom: only pick the bottom string (last string index) repeatedly
            util.frameCount++;
            var pickCycle = 15 / speedFactor;  // Time between picks
            var cyclePos = util.frameCount % pickCycle;

            for (var s = 0; s < algo.stringCount; s++) {
                if (s === algo.stringCount - 1) {
                    // Bottom string - pick it
                    if (cyclePos < 1) {
                        util.stringStates[s].lastHitTime = util.frameCount;
                    }
                }
                stringHitTimes.push(util.stringStates[s].lastHitTime);
            }
        } else {
            // Non-constant modes use step-based timing
            for (var s = 0; s < algo.stringCount; s++) {
                var hitTime;
                if (algo.strumType === 0) {
                    // Down strum: high strings (index 0) first, low strings last
                    hitTime = s * strumDelay;
                } else if (algo.strumType === 1) {
                    // Up strum: low strings first, high strings last
                    hitTime = (algo.stringCount - 1 - s) * strumDelay;
                } else {
                    // Up-Down: up first, then down
                    var halfStrum = algo.stringCount * strumDelay;
                    stringHitTimes.push({
                        up: (algo.stringCount - 1 - s) * strumDelay,
                        down: halfStrum + s * strumDelay
                    });
                    continue;
                }
                stringHitTimes.push(hitTime);
            }
        }

        // Draw each string
        for (var s = 0; s < algo.stringCount; s++) {
            var baseY = getStringY(s, height);

            // Calculate vibration for this string
            var vibration = 0;
            var brightness = 100;  // Base brightness for unplayed string

            var timeSinceHit;
            if (algo.strumType === 3 || algo.strumType === 4) {
                // Constant and Pick Bottom modes: use frame count based timing
                timeSinceHit = util.frameCount - stringHitTimes[s];
            } else if (algo.strumType === 2) {
                // Up-Down: check both hit times
                var upTime = step - stringHitTimes[s].up;
                var downTime = step - stringHitTimes[s].down;
                // Use the most recent hit
                if (downTime >= 0 && stringHitTimes[s].down <= step) {
                    timeSinceHit = downTime;
                } else if (upTime >= 0) {
                    timeSinceHit = upTime;
                } else {
                    timeSinceHit = -1;
                }
            } else {
                timeSinceHit = step - stringHitTimes[s];
            }

            if (timeSinceHit >= 0) {
                // String has been hit - calculate vibration
                var decay = Math.exp(-timeSinceHit * 0.05 / speedFactor);
                var frequency = 8 + s * 2;  // Higher strings vibrate faster
                vibration = Math.sin(timeSinceHit * frequency * 0.3 * speedFactor) * decay * vibrationAmp * 2;

                // Brightness peaks when hit, then decays
                if (timeSinceHit < 3) {
                    brightness = 255;  // Flash on hit
                } else {
                    brightness = 100 + 155 * decay;
                }
            }

            // Draw the string (horizontal line across the matrix)
            for (var x = 0; x < width; x++) {
                // Add some wave motion along the string
                var waveOffset = 0;
                if (timeSinceHit >= 0) {
                    var wavePhase = x * 0.3 - timeSinceHit * speedFactor * 0.5;
                    waveOffset = vibration * Math.sin(wavePhase);
                }

                // Draw string pixels (accounting for width)
                for (var w = 0; w < algo.stringWidth; w++) {
                    var drawY = Math.round(baseY + waveOffset - Math.floor(algo.stringWidth / 2) + w);
                    if (drawY >= 0 && drawY < height) {
                        var pixelBrightness = brightness;
                        // Slightly dimmer at edges of thick strings
                        if (algo.stringWidth > 1 && (w === 0 || w === algo.stringWidth - 1)) {
                            pixelBrightness *= 0.7;
                        }
                        map[drawY][x] = scaleColor(rgb, Math.floor(pixelBrightness));
                    }
                }
            }
        }

        return map;
    };

    testAlgo = algo;
    return algo;
})();

