/*
  Q Light Controller Plus
  fireflies.js

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

(function() {
    var algo = {};
    algo.apiVersion = 2;
    algo.name = "Fireflies";
    algo.author = "Branson Matheson with help from Augment";
    algo.acceptColors = 1;  // Use color picker for firefly color

    algo.properties = new Array();

    // Count of fireflies (1-200)
    algo.count = 10;
    algo.properties.push("name:count|type:range|display:Count|values:1,200|write:setCount|read:getCount");

    // Trail length (0-20)
    algo.trailLength = 5;
    algo.properties.push("name:trailLength|type:range|display:Trail Length|values:0,20|write:setTrailLength|read:getTrailLength");

    // Appear delay (how long before all fireflies appear)
    algo.appearDelay = 50;
    algo.properties.push("name:appearDelay|type:range|display:Appear Delay|values:0,200|write:setAppearDelay|read:getAppearDelay");

    // Speed (1-10)
    algo.speed = 5;
    algo.properties.push("name:speed|type:range|display:Speed|values:1,10|write:setSpeed|read:getSpeed");

    // Pulse (depth effect - flying toward/away from viewer)
    algo.pulseEnabled = 1;
    algo.properties.push("name:pulseEnabled|type:list|display:Pulse (Depth)|values:No,Yes|write:setPulseEnabled|read:getPulseEnabled");

    // Blink mode (fireflies blink on/off like real fireflies)
    algo.blinkEnabled = 1;
    algo.properties.push("name:blinkEnabled|type:list|display:Blink|values:No,Yes|write:setBlinkEnabled|read:getBlinkEnabled");

    // Visibility percentage (how often fireflies are visible when blinking, 10-90%)
    algo.visibility = 30;
    algo.properties.push("name:visibility|type:range|display:Visibility %|values:10,90|write:setVisibility|read:getVisibility");

    // Setters and getters
    algo.setCount = function(v) {
        var n = parseInt(v, 10);
        if (!isNaN(n)) {
            algo.count = Math.max(1, Math.min(200, n));
            util.initialized = false;
        }
    };
    algo.getCount = function() { return algo.count; };

    algo.setTrailLength = function(v) { var n = parseInt(v, 10); if (!isNaN(n)) algo.trailLength = Math.max(0, Math.min(20, n)); };
    algo.getTrailLength = function() { return algo.trailLength; };

    algo.setAppearDelay = function(v) { var n = parseInt(v, 10); if (!isNaN(n)) algo.appearDelay = Math.max(0, Math.min(200, n)); };
    algo.getAppearDelay = function() { return algo.appearDelay; };

    algo.setSpeed = function(v) { var n = parseInt(v, 10); if (!isNaN(n)) algo.speed = Math.max(1, Math.min(10, n)); };
    algo.getSpeed = function() { return algo.speed; };

    algo.setPulseEnabled = function(v) { algo.pulseEnabled = (v === "Yes") ? 1 : 0; };
    algo.getPulseEnabled = function() { return algo.pulseEnabled === 1 ? "Yes" : "No"; };

    algo.setBlinkEnabled = function(v) { algo.blinkEnabled = (v === "Yes") ? 1 : 0; };
    algo.getBlinkEnabled = function() { return algo.blinkEnabled === 1 ? "Yes" : "No"; };

    algo.setVisibility = function(v) { var n = parseInt(v, 10); if (!isNaN(n)) algo.visibility = Math.max(10, Math.min(90, n)); };
    algo.getVisibility = function() { return algo.visibility; };

    var util = {};
    util.initialized = false;
    util.fireflies = [];
    util.frameCount = 0;
    util.width = 0;
    util.height = 0;

    // Utility functions
    function scaleColor(rgb, scale255) {
        if (scale255 <= 0) return 0;
        if (scale255 >= 255) return rgb;
        var r = (rgb >> 16) & 255, g = (rgb >> 8) & 255, b = rgb & 255;
        r = Math.floor(r * scale255 / 255);
        g = Math.floor(g * scale255 / 255);
        b = Math.floor(b * scale255 / 255);
        return (r << 16) + (g << 8) + b;
    }

    function addColor(c1, c2) {
        var r1 = (c1 >> 16) & 255, g1 = (c1 >> 8) & 255, b1 = c1 & 255;
        var r2 = (c2 >> 16) & 255, g2 = (c2 >> 8) & 255, b2 = c2 & 255;
        var r = Math.min(255, r1 + r2);
        var g = Math.min(255, g1 + g2);
        var b = Math.min(255, b1 + b2);
        return (r << 16) + (g << 8) + b;
    }

    function makeMap(width, height, fill) {
        var m = new Array(height);
        for (var y = 0; y < height; y++) {
            m[y] = new Array(width);
            for (var x = 0; x < width; x++) m[y][x] = fill;
        }
        return m;
    }

    function createFirefly(width, height, index) {
        // Random starting position
        var x = Math.random() * width;
        var y = Math.random() * height;
        // Random velocity (slow, drifting movement)
        var angle = Math.random() * Math.PI * 2;
        var speed = 0.2 + Math.random() * 0.3;
        var vx = Math.cos(angle) * speed;
        var vy = Math.sin(angle) * speed;

        // Depth (z-axis) for pulse effect - 0 = far, 1 = close
        var depth = Math.random();
        var depthDirection = (Math.random() > 0.5) ? 1 : -1;
        var depthSpeed = 0.005 + Math.random() * 0.01;

        // Blink state - durations based on visibility percentage
        var blinkOn = true;
        var blinkTimer = Math.random() * 100;  // Random starting phase
        // Base cycle is ~100 frames, visibility controls on/off ratio
        var visRatio = algo.visibility / 100;
        var blinkOnDuration = (50 + Math.random() * 30) * visRatio * 2;      // Longer when visibility higher
        var blinkOffDuration = (50 + Math.random() * 30) * (1 - visRatio) * 2; // Longer when visibility lower

        // Appear time - staggered based on index with randomness
        var appearTime = (index / Math.max(1, algo.count)) * algo.appearDelay + Math.random() * (algo.appearDelay * 0.5);

        // Trail history - array of {x, y} positions
        var trail = [];

        return {
            x: x, y: y,
            vx: vx, vy: vy,
            depth: depth,
            depthDirection: depthDirection,
            depthSpeed: depthSpeed,
            blinkOn: blinkOn,
            blinkTimer: blinkTimer,
            blinkOnDuration: blinkOnDuration,
            blinkOffDuration: blinkOffDuration,
            appearTime: appearTime,
            trail: trail
        };
    }

    util.initialize = function(width, height) {
        util.fireflies = [];
        util.frameCount = 0;
        for (var i = 0; i < algo.count; i++) {
            util.fireflies.push(createFirefly(width, height, i));
        }
        util.width = width;
        util.height = height;
        util.initialized = true;
    };

    algo.rgbMap = function(width, height, rgb, step) {
        // Default color: dark yellow/gold if no color provided
        if (rgb === 0) rgb = 0xCCAA00;

        if (!util.initialized || util.width !== width || util.height !== height) {
            util.initialize(width, height);
        }

        // Increment frame counter
        util.frameCount++;

        // Adjust firefly count if changed
        while (util.fireflies.length < algo.count) {
            util.fireflies.push(createFirefly(width, height, util.fireflies.length));
        }
        while (util.fireflies.length > algo.count) {
            util.fireflies.pop();
        }

        var map = makeMap(width, height, 0);
        var speedFactor = algo.speed / 5;

        for (var i = 0; i < util.fireflies.length; i++) {
            var ff = util.fireflies[i];

            // Check if this firefly has appeared yet
            if (util.frameCount < ff.appearTime) {
                continue;  // Skip this firefly, hasn't appeared yet
            }

            // Store current position in trail BEFORE updating position
            if (algo.trailLength > 0) {
                ff.trail.unshift({ x: ff.x, y: ff.y });
                while (ff.trail.length > algo.trailLength) {
                    ff.trail.pop();
                }
            } else {
                ff.trail = [];
            }

            // Update position (always, even when not visible)
            ff.x += ff.vx * speedFactor;
            ff.y += ff.vy * speedFactor;

            // Bounce off edges with some randomness
            if (ff.x < 0) { ff.x = 0; ff.vx = Math.abs(ff.vx) * (0.8 + Math.random() * 0.4); }
            if (ff.x >= width) { ff.x = width - 0.1; ff.vx = -Math.abs(ff.vx) * (0.8 + Math.random() * 0.4); }
            if (ff.y < 0) { ff.y = 0; ff.vy = Math.abs(ff.vy) * (0.8 + Math.random() * 0.4); }
            if (ff.y >= height) { ff.y = height - 0.1; ff.vy = -Math.abs(ff.vy) * (0.8 + Math.random() * 0.4); }

            // Random direction changes (drifting behavior)
            if (Math.random() < 0.02) {
                var angleChange = (Math.random() - 0.5) * Math.PI * 0.5;
                var currentAngle = Math.atan2(ff.vy, ff.vx);
                var currentSpeed = Math.sqrt(ff.vx * ff.vx + ff.vy * ff.vy);
                var newAngle = currentAngle + angleChange;
                ff.vx = Math.cos(newAngle) * currentSpeed;
                ff.vy = Math.sin(newAngle) * currentSpeed;
            }

            // Update depth (z-axis movement for pulse effect)
            ff.depth += ff.depthDirection * ff.depthSpeed * speedFactor;
            if (ff.depth >= 1) { ff.depth = 1; ff.depthDirection = -1; }
            if (ff.depth <= 0) { ff.depth = 0; ff.depthDirection = 1; }

            // Update blink timer
            ff.blinkTimer += speedFactor;
            var visRatio = algo.visibility / 100;
            if (ff.blinkOn) {
                if (ff.blinkTimer >= ff.blinkOnDuration) {
                    ff.blinkOn = false;
                    ff.blinkTimer = 0;
                    ff.blinkOffDuration = (50 + Math.random() * 30) * (1 - visRatio) * 2;  // Randomize next off duration
                }
            } else {
                if (ff.blinkTimer >= ff.blinkOffDuration) {
                    ff.blinkOn = true;
                    ff.blinkTimer = 0;
                    ff.blinkOnDuration = (50 + Math.random() * 30) * visRatio * 2;  // Randomize next on duration
                }
            }

            // Calculate brightness based on pulse (depth) and blink
            var brightness = 1.0;

            // Apply pulse (depth) effect - closer = brighter
            if (algo.pulseEnabled === 1) {
                brightness = 0.3 + 0.7 * ff.depth;  // 0.3 when far, 1.0 when close
            }

            // Apply blink - if off, brightness is 0
            if (algo.blinkEnabled === 1 && !ff.blinkOn) {
                brightness = 0;
            }

            // Only draw if visible
            if (brightness > 0) {
                // Draw trail (fading)
                for (var t = 0; t < ff.trail.length; t++) {
                    var trailPos = ff.trail[t];
                    var tx = Math.floor(trailPos.x);
                    var ty = Math.floor(trailPos.y);
                    if (tx >= 0 && tx < width && ty >= 0 && ty < height) {
                        var trailFade = 1 - ((t + 1) / (algo.trailLength + 1));
                        var trailBrightness = Math.floor(brightness * trailFade * 180);
                        var trailColor = scaleColor(rgb, trailBrightness);
                        map[ty][tx] = addColor(map[ty][tx], trailColor);
                    }
                }

                // Draw firefly head (brightest point)
                var fx = Math.floor(ff.x);
                var fy = Math.floor(ff.y);
                if (fx >= 0 && fx < width && fy >= 0 && fy < height) {
                    var headBrightness = Math.floor(brightness * 255);
                    var headColor = scaleColor(rgb, headBrightness);
                    map[fy][fx] = addColor(map[fy][fx], headColor);
                }
            }
        }

        return map;
    };

    algo.rgbMapStepCount = function(width, height) {
        return 2;  // Continuous animation
    };

    // Development tool access
    testAlgo = algo;

    return algo;
})();

