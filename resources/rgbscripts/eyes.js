/*
  Q Light Controller Plus
  eyes.js

  Creepy eyes appearing randomly across the matrix with independent blinking.
  Features: 6 eye types, size variance, collision detection, natural blink timing.
  Colors interpolate between two user-specified colors.

  Author: Branson Matheson with help from Augment
  License: Apache License, Version 2.0
*/


(function(){
  var algo = {};
  algo.apiVersion = 3;
  algo.name = "Eyes";
  algo.author = "Branson Matheson with help from Augment";
  algo.acceptColors = 2;
  algo.properties = new Array();

  // Eye pair spawning
  algo.maxPairs = 20;
  algo.properties.push("name:maxPairs|type:range|display:Max Eye Pairs|values:1,40|write:setMaxPairs|read:getMaxPairs");
  algo.spawnDelay = 30;
  algo.properties.push("name:spawnDelay|type:range|display:Spawn Delay (steps)|values:5,200|write:setSpawnDelay|read:getSpawnDelay");

  // Eye appearance
  algo.eyeSizeMin = 4;
  algo.properties.push("name:eyeSizeMin|type:range|display:Eye Size Min|values:2,20|write:setEyeSizeMin|read:getEyeSizeMin");
  algo.eyeSizeMax = 12;
  algo.properties.push("name:eyeSizeMax|type:range|display:Eye Size Max|values:2,20|write:setEyeSizeMax|read:getEyeSizeMax");
  algo.eyeSpacingRatio = 1.5;
  algo.properties.push("name:eyeSpacingRatio|type:range|display:Eye Spacing Ratio|values:0.5,4.0|write:setEyeSpacingRatio|read:getEyeSpacingRatio");
  algo.minSpacing = 8;
  algo.properties.push("name:minSpacing|type:range|display:Min Pair Spacing|values:2,30|write:setMinSpacing|read:getMinSpacing");

  // Blinking behavior
  algo.blinkInterval = 100;
  algo.properties.push("name:blinkInterval|type:range|display:Blink Interval (frames)|values:10,500|write:setBlinkInterval|read:getBlinkInterval");
  algo.blinkChance = 50;
  algo.properties.push("name:blinkChance|type:range|display:Blink Chance (%)|values:0,100|write:setBlinkChance|read:getBlinkChance");
  algo.blinkRandomness = 50;
  algo.properties.push("name:blinkRandomness|type:range|display:Blink Randomness (%)|values:0,100|write:setBlinkRandomness|read:getBlinkRandomness");
  algo.blinkDuration = 12;
  algo.properties.push("name:blinkDuration|type:range|display:Blink Duration (steps)|values:4,40|write:setBlinkDuration|read:getBlinkDuration");

  // Eye lifetime
  algo.eyeLife = 500;
  algo.properties.push("name:eyeLife|type:range|display:Eye Life (steps)|values:100,2000|write:setEyeLife|read:getEyeLife");
  algo.eyeLifeRandomness = 50;
  algo.properties.push("name:eyeLifeRandomness|type:range|display:Eye Life Randomness (%)|values:0,100|write:setEyeLifeRandomness|read:getEyeLifeRandomness");

  var util = {};
  util.initialized = false;
  util.eyes = [];
  util.stepCounter = 0;
  util.colors = [0xFF0000, 0x00FF00];
  util.eyeMaxLifetime = {}; // max lifetime per pair
  util.eyeBlinkTimer = {}; // shared blink timer per pair

  algo.setMaxPairs = function(v){ algo.maxPairs = parseInt(v); };
  algo.getMaxPairs = function(){ return algo.maxPairs; };
  algo.setSpawnDelay = function(v){ algo.spawnDelay = parseInt(v); };
  algo.getSpawnDelay = function(){ return algo.spawnDelay; };
  algo.setEyeSizeMin = function(v){ algo.eyeSizeMin = parseInt(v); };
  algo.getEyeSizeMin = function(){ return algo.eyeSizeMin; };
  algo.setEyeSizeMax = function(v){ algo.eyeSizeMax = parseInt(v); };
  algo.getEyeSizeMax = function(){ return algo.eyeSizeMax; };
  algo.setEyeSpacingRatio = function(v){ algo.eyeSpacingRatio = parseFloat(v); };
  algo.getEyeSpacingRatio = function(){ return algo.eyeSpacingRatio; };
  algo.setMinSpacing = function(v){ algo.minSpacing = parseInt(v); };
  algo.getMinSpacing = function(){ return algo.minSpacing; };
  algo.setBlinkInterval = function(v){ algo.blinkInterval = parseInt(v); };
  algo.getBlinkInterval = function(){ return algo.blinkInterval; };
  algo.setBlinkChance = function(v){ algo.blinkChance = parseInt(v); };
  algo.getBlinkChance = function(){ return algo.blinkChance; };
  algo.setBlinkRandomness = function(v){ algo.blinkRandomness = parseInt(v); };
  algo.getBlinkRandomness = function(){ return algo.blinkRandomness; };
  algo.setBlinkDuration = function(v){ algo.blinkDuration = parseInt(v); };
  algo.getBlinkDuration = function(){ return algo.blinkDuration; };
  algo.setEyeLife = function(v){ algo.eyeLife = parseInt(v); };
  algo.getEyeLife = function(){ return algo.eyeLife; };
  algo.setEyeLifeRandomness = function(v){ algo.eyeLifeRandomness = parseInt(v); };
  algo.getEyeLifeRandomness = function(){ return algo.eyeLifeRandomness; };

  algo.rgbMapSetColors = function(rawColors){
    util.colors = [0xFF0000, 0x00FF00];
    if (Array.isArray(rawColors)){
      if (rawColors[0] === rawColors[0]) util.colors[0] = rawColors[0];
      if (rawColors[1] === rawColors[1]) util.colors[1] = rawColors[1];
    }
  };

  function lerpColor(a, b, t){
    var ar=(a>>16)&255, ag=(a>>8)&255, ab=a&255;
    var br=(b>>16)&255, bg=(b>>8)&255, bb=b&255;
    var r = Math.floor(ar + (br-ar)*t);
    var g = Math.floor(ag + (bg-ag)*t);
    var b2 = Math.floor(ab + (bb-ab)*t);
    return (r<<16) + (g<<8) + b2;
  }

  function randomColor(){
    var t = Math.random();
    return lerpColor(util.colors[0], util.colors[1], t);
  }

  // Create zero-filled 2D array
  function makeMap(w, h){
    var m = new Array(h);
    for (var y = 0; y < h; y++){
      m[y] = new Array(w);
      for (var x = 0; x < w; x++) m[y][x] = 0;
    }
    return m;
  }

  // Scale RGB color by brightness (0-255)
  function scaleColor(rgb, scale255){
    if (scale255 <= 0) return 0;
    if (scale255 >= 255) return rgb;
    var r = (rgb >> 16) & 255, g = (rgb >> 8) & 255, b = rgb & 255;
    r = Math.floor(r * scale255 / 255);
    g = Math.floor(g * scale255 / 255);
    b = Math.floor(b * scale255 / 255);
    return (r << 16) + (g << 8) + b;
  }

  // Add two RGB colors with clamping
  function addColor(dst, add){
    var r = ((dst >> 16) & 255) + ((add >> 16) & 255);
    var g = ((dst >> 8) & 255) + ((add >> 8) & 255);
    var b = (dst & 255) + (add & 255);
    if (r > 255) r = 255;
    if (g > 255) g = 255;
    if (b > 255) b = 255;
    return (r << 16) + (g << 8) + b;
  }

  // Compute eye shape factor (0-1) for pixel at (x,y) relative to eye center
  function eyeFactor(x, y, cx, cy, eyeType, blinkPhase, eyeSize){
    var dx = x - cx, dy = y - cy;
    var dist = Math.sqrt(dx * dx + dy * dy);
    var sz = eyeSize;
    var factor = 0;

    // Compute outer eye shape based on type
    if (eyeType === "Round"){
      var d = dist / sz;
      factor = Math.max(0, 1 - d * d);
    } else if (eyeType === "Slanted"){
      var dx2 = dx + 0.2 * dy, dy2 = dy * 0.85;
      var d = Math.sqrt(dx2 * dx2 + dy2 * dy2) / sz;
      factor = Math.max(0, 1 - d * d);
    } else if (eyeType === "Cat"){
      var dx2 = dx * 0.6, dy2 = dy;
      var d = Math.sqrt(dx2 * dx2 + dy2 * dy2) / sz;
      factor = Math.max(0, 1 - d * d);
    } else if (eyeType === "Square"){
      var adx = Math.abs(dx), ady = Math.abs(dy);
      factor = Math.max(0, 1 - Math.max(adx, ady) / sz);
    } else if (eyeType === "Diamond"){
      var adx = Math.abs(dx), ady = Math.abs(dy);
      factor = Math.max(0, 1 - (adx + ady) / sz);
    } else if (eyeType === "Oval"){
      var dx2 = dx, dy2 = dy * 0.6;
      var d = Math.sqrt(dx2 * dx2 + dy2 * dy2) / sz;
      factor = Math.max(0, 1 - d * d);
    }

    // Blend pupil (dark center) with eye white
    var pupilSz = sz * 0.3;
    var pupil = Math.max(0, 1 - (dist / pupilSz) * (dist / pupilSz));
    factor = Math.max(factor * 0.3, pupil);

    // Apply windowshade blink effect (top-to-bottom close, bottom-to-top open)
    if (blinkPhase > 0){
      var t = blinkPhase / Math.max(1, algo.blinkDuration);
      var maxLidHeight = sz * 2;
      var lidHeight = (t <= 0.5) ? (t * 2 * maxLidHeight) : ((1 - (t - 0.5) * 2) * maxLidHeight);
      if ((dy + sz) < lidHeight) factor = 0;
    }

    return factor;
  }

  algo.rgbMap = function(width, height, _rgb, _step){
    void _rgb; void _step; // QLC+ API requirement
    if (!util.initialized){
      util.eyes = [];
      util.stepCounter = 0;
      util.initialized = true;
    }

    util.stepCounter++;

    // Spawn new eye pairs with collision detection
    if (util.eyes.length < algo.maxPairs * 2 && (util.stepCounter % Math.max(1, algo.spawnDelay)) === 0){
      var types = ["Round", "Slanted", "Cat", "Square", "Diamond", "Oval"];
      var typeIdx = Math.floor(Math.random() * types.length);
      var eyeType = types[typeIdx];
      var color = randomColor();

      // Random eye size for this pair (determine size first for collision detection)
      var randomSize = algo.eyeSizeMin + Math.random() * (algo.eyeSizeMax - algo.eyeSizeMin);
      randomSize = Math.floor(randomSize);

      // Find valid spawn position (no collision)
      var maxAttempts = 20;
      var validPos = false;
      var cx = 0, cy = 0;

      for (var attempt = 0; attempt < maxAttempts; attempt++){
        var eyeSpacing = Math.max(1, Math.floor(randomSize * algo.eyeSpacingRatio));
        cx = Math.floor(Math.random() * (width - eyeSpacing - 2)) + eyeSpacing / 2 + 1;
        cy = Math.floor(Math.random() * height);

        // Collision detection: check distance to all existing eyes
        validPos = true;
        var requiredDist = randomSize + algo.minSpacing;
        for (var i = 0; i < util.eyes.length; i++){
          var dx = cx - util.eyes[i].x;
          var dy2 = cy - util.eyes[i].y;
          var dist = Math.sqrt(dx * dx + dy2 * dy2);
          if (dist < requiredDist + util.eyes[i].size){
            validPos = false;
            break;
          }
        }
        if (validPos) break;
      }

      if (validPos){
        var eyeSpacing = Math.max(1, Math.floor(randomSize * algo.eyeSpacingRatio));
        var spacing = eyeSpacing / 2;
        var maxLife = algo.eyeLife;
        if (algo.eyeLifeRandomness > 0){
          maxLife = Math.max(100, maxLife + (Math.random() - 0.5) * algo.eyeLifeRandomness);
        }

        var pairId = util.eyes.length / 2;
        util.eyeMaxLifetime[pairId] = maxLife;
        util.eyeBlinkTimer[pairId] = Math.floor(Math.random() * algo.blinkInterval);

        // Create eye pair (explicit props to avoid for..in)
        var leftEye = { x: cx - spacing, y: cy, type: eyeType, color: color, size: randomSize, blinkStart: -1, lifetime: 0, pairId: pairId };
        var rightEye = { x: cx + spacing, y: cy, type: eyeType, color: color, size: randomSize, blinkStart: -1, lifetime: 0, pairId: pairId };

        util.eyes.push(leftEye);
        util.eyes.push(rightEye);
      }
    }

    // Update eye pairs: lifetime, blinking, and removal
    var eyesToRemove = [];
    for (var i = 0; i < util.eyes.length; i += 2){
      var eye1 = util.eyes[i];
      var eye2 = util.eyes[i + 1];
      var pairId = eye1.pairId;

      eye1.lifetime++;
      eye2.lifetime++;

      // Check lifetime expiration
      if (eye1.lifetime >= util.eyeMaxLifetime[pairId]){
        eyesToRemove.push(i, i + 1);
        delete util.eyeBlinkTimer[pairId];
        continue;
      }

      // Update shared blink timer for pair
      util.eyeBlinkTimer[pairId]++;

      // Trigger blink when timer reaches interval
      if (eye1.blinkStart < 0 && util.eyeBlinkTimer[pairId] >= algo.blinkInterval){
        util.eyeBlinkTimer[pairId] = 0;
        var blinkChance = algo.blinkChance;
        if (algo.blinkRandomness > 0){
          blinkChance *= (1 + (Math.random() - 0.5) * (algo.blinkRandomness / 100.0));
        }
        if (Math.random() * 100 < Math.max(0, Math.min(100, blinkChance))){
          eye1.blinkStart = 0;
          eye2.blinkStart = 0;
        }
      }

      // Update blink animation for both eyes
      if (eye1.blinkStart >= 0){
        eye1.blinkStart++;
        if (eye1.blinkStart > algo.blinkDuration) eye1.blinkStart = -1;
      }
      if (eye2.blinkStart >= 0){
        eye2.blinkStart++;
        if (eye2.blinkStart > algo.blinkDuration) eye2.blinkStart = -1;
      }
    }

    // Remove expired pairs (reverse order to preserve indices)
    for (var r = eyesToRemove.length - 1; r >= 0; r--){
      util.eyes.splice(eyesToRemove[r], 1);
    }

    var map = makeMap(width, height);

    // Render all eyes to map
    for (var i = 0; i < util.eyes.length; i++){
      var eye = util.eyes[i];
      var blinkPhase = (eye.blinkStart >= 0) ? eye.blinkStart : 0;

      for (var y = 0; y < height; y++){
        for (var x = 0; x < width; x++){
          var f = eyeFactor(x, y, eye.x, eye.y, eye.type, blinkPhase, eye.size);
          if (f > 0){
            map[y][x] = addColor(map[y][x], scaleColor(eye.color, Math.floor(255 * f)));
          }
        }
      }
    }

    return map;
  };

  algo.rgbMapStepCount = function(_width, _height){ void _width; void _height; return 256; };

  return algo;
})();

