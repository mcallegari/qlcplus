/*
  Q Light Controller Plus
  flicker.js

  Simulates the ambient light cast FROM a fire onto surfaces.
  Creates warm, organic flickering like firelight on walls/objects.

  - Color 1: Coals base color (dim, slow pulsing)
  - Color 2: Flame flicker color (bright, random bursts)
  - Flame Sources: positioned across width, affecting nearby fixtures
  - Flicker Amount: how often flames flare up
  - Color Variation: intensity range from dim coals to bright flames

  Licensed under the Apache License, Version 2.0
*/

(function(){
  var algo = {};
  algo.apiVersion = 3;
  algo.name = "Fire Flicker";
  algo.author = "Augment";
  algo.acceptColors = 2; // Color 1 = coals, Color 2 = flames

  algo.properties = new Array();

  // Flicker speed 1..10
  algo.speed = 5;
  algo.properties.push("name:speed|type:range|display:Flicker Speed|values:1,10|write:setSpeed|read:getSpeed");

  // Flicker amount 0..100 how often flames flare up
  algo.flickerAmount = 40;
  algo.properties.push("name:flickerAmount|type:range|display:Flicker Amount (%)|values:0,100|write:setFlickerAmount|read:getFlickerAmount");

  // Smoothness 1..10 softens flame edges
  algo.smoothness = 6;
  algo.properties.push("name:smoothness|type:range|display:Smoothness|values:1,10|write:setSmoothness|read:getSmoothness");

  // Number of flame sources 1..5
  algo.flameSources = 3;
  algo.properties.push("name:flameSources|type:range|display:Flame Sources|values:1,5|write:setFlameSources|read:getFlameSources");

  // Color variation 0..100 range from dim coals to bright flames
  algo.colorVariation = 20;
  algo.properties.push("name:colorVariation|type:range|display:Color Variation (%)|values:0,100|write:setColorVariation|read:getColorVariation");

  // Spatial variation 0..100 how much flames drift from base position
  algo.spatialVariation = 30;
  algo.properties.push("name:spatialVariation|type:range|display:Spatial Variation (%)|values:0,100|write:setSpatialVariation|read:getSpatialVariation");

  // Coal pulse speed 1..10
  algo.coalSpeed = 3;
  algo.properties.push("name:coalSpeed|type:range|display:Coal Pulse Speed|values:1,10|write:setCoalSpeed|read:getCoalSpeed");

  var util = {};
  util.phase = 0.0;
  util.flameStates = null; // per-source: {pos, intensity, phase}
  util.coalPhases = null; // per-pixel coal pulse phases
  util.primaryColor = 0xFF2200; // coals - deep red/orange default
  util.secondaryColor = 0xFFAA00; // flames - bright orange/yellow default
  util.w = 0;
  util.h = 0;

  // Setters/getters
  algo.setSpeed = function(v){ var n = parseInt(v, 10); if (!isNaN(n) && n >= 1 && n <= 10) algo.speed = n; };
  algo.getSpeed = function(){ return algo.speed; };
  algo.setFlickerAmount = function(v){ var n = parseInt(v, 10); if (!isNaN(n) && n >= 0 && n <= 100) algo.flickerAmount = n; };
  algo.getFlickerAmount = function(){ return algo.flickerAmount; };
  algo.setSmoothness = function(v){ var n = parseInt(v, 10); if (!isNaN(n) && n >= 1 && n <= 10) algo.smoothness = n; };
  algo.getSmoothness = function(){ return algo.smoothness; };
  algo.setFlameSources = function(v){ var n = parseInt(v, 10); if (!isNaN(n) && n >= 1 && n <= 5) algo.flameSources = n; };
  algo.getFlameSources = function(){ return algo.flameSources; };
  algo.setColorVariation = function(v){ var n = parseInt(v, 10); if (!isNaN(n) && n >= 0 && n <= 100) algo.colorVariation = n; };
  algo.getColorVariation = function(){ return algo.colorVariation; };
  algo.setSpatialVariation = function(v){ var n = parseInt(v, 10); if (!isNaN(n) && n >= 0 && n <= 100) algo.spatialVariation = n; };
  algo.getSpatialVariation = function(){ return algo.spatialVariation; };
  algo.setCoalSpeed = function(v){ var n = parseInt(v, 10); if (!isNaN(n) && n >= 1 && n <= 10) algo.coalSpeed = n; };
  algo.getCoalSpeed = function(){ return algo.coalSpeed; };

  // Colors handling (API v3)
  algo.rgbMapSetColors = function(rawColors){
    if (Array.isArray(rawColors) && rawColors.length >= 1){
      util.primaryColor = rawColors[0];
      if (rawColors.length >= 2) util.secondaryColor = rawColors[1];
      else util.secondaryColor = util.primaryColor;
    }
  };

  // Blend two colors
  function lerpColor(a, b, t){
    if (t <= 0) return a; if (t >= 1) return b;
    var ar = (a >> 16) & 255, ag = (a >> 8) & 255, ab = a & 255;
    var br = (b >> 16) & 255, bg = (b >> 8) & 255, bb = b & 255;
    var r = Math.floor(ar + (br - ar) * t);
    var g = Math.floor(ag + (bg - ag) * t);
    var bl = Math.floor(ab + (bb - ab) * t);
    return (r << 16) | (g << 8) | bl;
  }

  // Scale brightness of a color
  function scaleBrightness(c, k){
    if (k >= 1) return c;
    if (k <= 0) return 0x000000;
    var r = (c >> 16) & 255, g = (c >> 8) & 255, b = c & 255;
    r = Math.floor(r * k); g = Math.floor(g * k); b = Math.floor(b * k);
    return (r << 16) | (g << 8) | b;
  }

  algo.rgbMap = function(width, height, _rgb, _step){
    void _rgb; void _step; // QLC+ API requirement
    var numSources = algo.flameSources;
    var spatialK = algo.spatialVariation / 100.0;

    // Initialize flame source states (positioned across width)
    if (!util.flameStates || util.flameStates.length !== numSources){
      util.flameStates = [];
      for (var i = 0; i < numSources; i++){
        // Evenly distribute sources across width with some randomness
        var basePos = (i + 0.5) / numSources; // 0.1, 0.3, 0.5, 0.7, 0.9 for 5 sources
        util.flameStates.push({
          basePos: basePos,
          pos: basePos,
          intensity: Math.random(),
          phase: Math.random() * 6.2831853
        });
      }
    }

    // Initialize coal pulse data per pixel (multiple phases + speed variation)
    if (!util.coalPhases || util.w !== width || util.h !== height){
      util.coalPhases = [];
      for (var cy = 0; cy < height; cy++){
        util.coalPhases[cy] = [];
        for (var cx = 0; cx < width; cx++){
          util.coalPhases[cy][cx] = {
            ph1: Math.random() * 6.2831853,
            ph2: Math.random() * 6.2831853,
            ph3: Math.random() * 6.2831853,
            speedMult: 0.5 + Math.random() * 1.0, // 0.5 to 1.5x speed variation
            ampBias: 0.6 + Math.random() * 0.4    // some coals glow brighter than others
          };
        }
      }
      util.w = width;
      util.h = height;
    }

    // Advance global phase
    var speedFactor = algo.speed * 0.1;
    util.phase += speedFactor;

    // Update each flame source
    for (var fi = 0; fi < numSources; fi++){
      var flame = util.flameStates[fi];
      flame.phase += speedFactor * (0.8 + Math.random() * 0.4);

      // Initialize random state if needed
      if (flame.targetIntensity === undefined) {
        flame.targetIntensity = Math.random();
        flame.currentIntensity = flame.targetIntensity;
        flame.holdTime = 0;
        flame.transitionSpeed = 0.1 + Math.random() * 0.2;
      }

      // Random flicker: occasionally pick a new random target intensity
      flame.holdTime -= speedFactor;
      if (flame.holdTime <= 0) {
        // Pick new random target and hold time
        flame.targetIntensity = Math.random();
        flame.holdTime = 0.5 + Math.random() * 2.0; // hold for random duration
        flame.transitionSpeed = 0.05 + Math.random() * 0.15;
      }

      // Smoothly transition toward target (adds organic movement)
      var diff = flame.targetIntensity - flame.currentIntensity;
      flame.currentIntensity += diff * flame.transitionSpeed;

      // Add small random noise on top for extra jitter
      var noise = (Math.random() - 0.5) * 0.1;
      flame.intensity = Math.max(0, Math.min(1, flame.currentIntensity + noise));

      // Position drifts randomly around base position
      if (flame.targetPos === undefined) {
        flame.targetPos = flame.basePos;
        flame.currentPos = flame.basePos;
      }
      // Occasionally pick new drift target
      if (Math.random() < 0.02 * speedFactor) {
        flame.targetPos = flame.basePos + (Math.random() - 0.5) * spatialK * 0.3;
      }
      flame.currentPos += (flame.targetPos - flame.currentPos) * 0.05;
      flame.pos = flame.currentPos;
    }

    var map = [];
    var flickerK = algo.flickerAmount / 100.0; // controls how many flames are "active"
    var colorVarK = algo.colorVariation / 100.0; // range from coals to flames
    var smoothK = algo.smoothness / 10.0;

    // Coal base intensity (dim, pulsing slowly)
    var coalBaseBright = 0.15 + 0.15 * colorVarK; // dimmer with more color variation
    // Flame peak intensity
    var flamePeakBright = 0.5 + 0.5 * colorVarK; // brighter with more color variation

    // Coal speed factor (1-10 maps to very slow to moderate)
    var coalSpeedFactor = algo.coalSpeed * 0.03; // 0.03 to 0.3

    for (var y = 0; y < height; y++){
      map[y] = [];
      for (var x = 0; x < width; x++){
        var xNorm = width > 1 ? x / (width - 1) : 0.5; // 0 to 1 across width

        // Coal layer: variable slow pulsing on color 1 at low intensity
        var coal = util.coalPhases[y][x];
        var coalTime = util.phase * coalSpeedFactor * coal.speedMult;

        // Multiple waves at different frequencies for organic, unpredictable pulsing
        var c1 = Math.sin(coal.ph1 + coalTime * 0.7);           // very slow base
        var c2 = Math.sin(coal.ph2 + coalTime * 1.3) * 0.5;     // slow secondary
        var c3 = Math.sin(coal.ph3 + coalTime * 2.9) * 0.25;    // occasional faster flicker

        // Combine with bias toward staying dim (coals mostly glow steady, occasionally brighten)
        var coalRaw = (c1 + c2 + c3) / 1.75; // normalize to roughly -1 to 1
        var coalPulse = 0.5 + 0.5 * coalRaw; // 0 to 1
        coalPulse = coalPulse * coal.ampBias; // some coals naturally brighter

        // Bias toward lower values (coals mostly dim, occasional glow-ups)
        coalPulse = Math.pow(coalPulse, 1.3);
        coalPulse = 0.4 + 0.6 * coalPulse; // range 0.4 to 1.0

        var coalBright = coalBaseBright * coalPulse;

        // Find influence from nearby flame sources
        var flameInfluence = 0;
        var spreadWidth = 1.0 / (numSources * (1.5 - spatialK * 0.5)); // how wide each flame affects

        for (var fj = 0; fj < numSources; fj++){
          var flame = util.flameStates[fj];
          var dist = Math.abs(xNorm - flame.pos);

          // Gaussian-ish falloff from flame center
          var falloff = Math.exp(-dist * dist / (spreadWidth * spreadWidth * 0.5));

          // Only contribute if this flame is "active" based on flicker amount
          var activeThreshold = 1.0 - flickerK;
          var contribution = flame.intensity > activeThreshold ?
                            (flame.intensity - activeThreshold) / flickerK : 0;
          if (flickerK <= 0) contribution = 0;

          flameInfluence += falloff * contribution;
        }
        if (flameInfluence > 1) flameInfluence = 1;

        // Apply smoothness - softens the flame edges
        flameInfluence = Math.pow(flameInfluence, 1.0 + smoothK * 0.5);

        // Blend between coal (color1, dim) and flame (color2, bright)
        var brightness = coalBright + (flamePeakBright - coalBright) * flameInfluence;
        var colorShift = flameInfluence; // 0 = coal color, 1 = flame color

        var finalColor = lerpColor(util.primaryColor, util.secondaryColor, colorShift);
        finalColor = scaleBrightness(finalColor, brightness);

        map[y][x] = finalColor;
      }
    }

    return map;
  };

  algo.rgbMapStepCount = function(_width, _height){ void _width; void _height; return 2; };

  return algo;
})();
