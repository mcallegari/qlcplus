/*
  Q Light Controller Plus
  fire.js

  FastLED-inspired "Fire/Heat" effect with options:
  - Orientation (Bottom->Top or Top->Bottom)
  - Flame types: Standard (smooth), Cartoon (banded)
  - Speed (simulation iterations per frame)
  - Fire Width (%), Fire Height (%)
  - Width Concentration (%): distribution across Fire Width (lower = more concentrated, less fire at edges)
  - Coals vs Flames percentage (blend)
  - Coals: plasma-like layer with user colors and config

  Licensed under the Apache License, Version 2.0
*/

(function(){
  var algo = {};
  algo.apiVersion = 3;
  algo.name = "Fire";
  algo.author = "Branson Matheson with help from Augment";
  algo.acceptColors = 5; // coals palette (like plasma)
  // Colors (acceptColors = 5) — recommended roles:
  //   1) Coals Bright (hot/bright coal glow)
  //   2) Coals Hot (orange/red)
  //   3) Coals Dark (deep red/blue)
  //   4) Coals Accent/Glow (secondary hue)
  //   5) Optional Tip/Extra tone (used by gradient as needed)

  algo.properties = new Array();

  // Core fire params
  algo.cooling = 30; // 0..100 higher = more cooling
  algo.properties.push("name:cooling|type:range|display:Cooling|values:0,100|write:setCooling|read:getCooling");
  algo.sparking = 60; // 0..100 higher = more sparks
  algo.properties.push("name:sparking|type:range|display:Sparking|values:0,100|write:setSparking|read:getSparking");
  algo.speed = 5; // 1..10 (simulation iterations per frame)
  algo.properties.push("name:speed|type:range|display:Speed|values:1,10|write:setSpeed|read:getSpeed");

  // Flame type
  algo.flameType = 0; // 0 standard, 1 cartoon
  algo.properties.push("name:flameType|type:list|display:Flame Type|values:Standard,Cartoon|write:setFlameType|read:getFlameType");
  // Fire source mode: 0=Bottom, 1=Top, 2=Tunnel (all sides)
  algo.fireSource = 0;
  algo.properties.push("name:fireSource|type:list|display:Fire Source|values:Bottom,Top,Tunnel|write:setFireSource|read:getFireSource");

  // Fire width/height
  algo.fireWidthPct = 100;
  algo.properties.push("name:fireWidthPct|type:range|display:Fire Width (%)|values:10,100|write:setFireWidthPct|read:getFireWidthPct");
  algo.fireHeightPct = 100;
  algo.properties.push("name:fireHeightPct|type:range|display:Fire Height (%)|values:10,100|write:setFireHeightPct|read:getFireHeightPct");
  // Width concentration across the Fire Width
  algo.widthConcentration = 100;
  algo.properties.push("name:widthConcentration|type:range|display:Width Concentration (%)|values:0,100|write:setWidthConcentration|read:getWidthConcentration");

	  // Undulation and warp controls
	  algo.undulation = 60; // 0..100 amount of body undulation
	  algo.properties.push("name:undulation|type:range|display:Undulation Amount|values:0,100|write:setUndulation|read:getUndulation");
	  algo.warpCount = 2; // number of rising warp plumes
	  algo.properties.push("name:warpCount|type:range|display:Warp Plumes|values:0,6|write:setWarpCount|read:getWarpCount");
	  algo.warpStrength = 50; // strength of warp plumes 0..100
	  algo.properties.push("name:warpStrength|type:range|display:Warp Strength|values:0,100|write:setWarpStrength|read:getWarpStrength");


  // Coals layer (plasma-like)
  algo.coalsPct = 30; // 0..100 blend amount of coals vs flames
  algo.properties.push("name:coalsPct|type:range|display:Coals vs Flames (%)|values:0,100|write:setCoalsPct|read:getCoalsPct");
  algo.coalsSize = 5; // 1..20 spatial scale
  algo.properties.push("name:coalsSize|type:range|display:Coals Size|values:1,20|write:setCoalsSize|read:getCoalsSize");
  algo.coalsRamp = 20; // 10..30 contrast gamma
  algo.properties.push("name:coalsRamp|type:range|display:Coals Ramp|values:10,30|write:setCoalsRamp|read:getCoalsRamp");
  algo.coalsSpeed = 25; // 1..50 temporal speed
  algo.properties.push("name:coalsSpeed|type:range|display:Coals Speed|values:1,50|write:setCoalsSpeed|read:getCoalsSpeed");

  var util = {};
  util.initialized = false;
  util.heat = null; // [h][w] 0..255
  util.w = 0; util.h = 0;
  util.coalsPhase = 0.0;
  util.coalsColors = [];
  util.gradient = null; // 256 colors
  util.xPhase1 = null; util.xPhase2 = null; util.xAmp = null; util.xW = 0;


	  util.warps = null; // [{xN:0..1, yN:0..1, ph:phase}]
	  util.warpLastCount = -1;

  // Setters/getters
  algo.setCooling = function(v){ var n = parseInt(v, 10); if (!isNaN(n) && n >= 0 && n <= 100) algo.cooling = n; };
  algo.getCooling = function(){ return algo.cooling; };
  algo.setSparking = function(v){ var n = parseInt(v, 10); if (!isNaN(n) && n >= 0 && n <= 100) algo.sparking = n; };
  algo.getSparking = function(){ return algo.sparking; };
  algo.setSpeed = function(v){ var n = parseInt(v, 10); if (!isNaN(n) && n >= 1 && n <= 10) algo.speed = n; };
  algo.getSpeed = function(){ return algo.speed; };
  // Flame type accepts label strings from list
  algo.setFlameType = function(v){
    if (v === "Standard") { algo.flameType = 0; }
    else if (v === "Cartoon") { algo.flameType = 1; }
    else { var n = parseInt(v, 10); if (!isNaN(n) && (n === 0 || n === 1)) algo.flameType = n; }
  };
  algo.getFlameType = function(){ return (algo.flameType === 0) ? "Standard" : "Cartoon"; };
  // Fire source mode setter/getter
  algo.setFireSource = function(v){
    if (v === "Bottom") { algo.fireSource = 0; }
    else if (v === "Top") { algo.fireSource = 1; }
    else if (v === "Tunnel") { algo.fireSource = 2; }
    else { var n = parseInt(v, 10); if (!isNaN(n) && n >= 0 && n <= 2) algo.fireSource = n; }
  };
  algo.getFireSource = function(){
    if (algo.fireSource === 0) return "Bottom";
    if (algo.fireSource === 1) return "Top";
    return "Tunnel";
  };
  // Fire width/height
  algo.setFireWidthPct = function(v){ var n = parseInt(v, 10); if (!isNaN(n) && n >= 10 && n <= 100) algo.fireWidthPct = n; };
  algo.getFireWidthPct = function(){ return algo.fireWidthPct; };
  algo.setFireHeightPct = function(v){ var n = parseInt(v, 10); if (!isNaN(n) && n >= 10 && n <= 100) algo.fireHeightPct = n; };
  algo.getFireHeightPct = function(){ return algo.fireHeightPct; };
  // Width concentration
  algo.setWidthConcentration = function(v){ var n = parseInt(v, 10); if (!isNaN(n) && n >= 0 && n <= 100) algo.widthConcentration = n; };
  algo.getWidthConcentration = function(){ return algo.widthConcentration; };
  // Coals controls
  algo.setCoalsPct = function(v){ var n = parseInt(v, 10); if (!isNaN(n) && n >= 0 && n <= 100) algo.coalsPct = n; };
  algo.getCoalsPct = function(){ return algo.coalsPct; };
  algo.setCoalsSize = function(v){ var n = parseInt(v, 10); if (!isNaN(n) && n >= 1 && n <= 20) algo.coalsSize = n; };
  algo.getCoalsSize = function(){ return algo.coalsSize; };
  algo.setCoalsRamp = function(v){ var n = parseInt(v, 10); if (!isNaN(n) && n >= 10 && n <= 30) algo.coalsRamp = n; };
  algo.getCoalsRamp = function(){ return algo.coalsRamp; };
  algo.setCoalsSpeed = function(v){ var n = parseInt(v, 10); if (!isNaN(n) && n >= 1 && n <= 50) algo.coalsSpeed = n; };
  algo.getCoalsSpeed = function(){ return algo.coalsSpeed; };

	  // Undulation + warp setters/getters
	  algo.setUndulation = function(v){ var n = parseInt(v, 10); if (!isNaN(n) && n >= 0 && n <= 100) algo.undulation = n; };
	  algo.getUndulation = function(){ return algo.undulation; };
	  algo.setWarpCount = function(v){ var n = parseInt(v, 10); if (!isNaN(n) && n >= 0 && n <= 6) algo.warpCount = n; };
	  algo.getWarpCount = function(){ return algo.warpCount; };
	  algo.setWarpStrength = function(v){ var n = parseInt(v, 10); if (!isNaN(n) && n >= 0 && n <= 100) algo.warpStrength = n; };
	  algo.getWarpStrength = function(){ return algo.warpStrength; };


  // Colors handling (API v3)
  algo.rgbMapSetColors = function(rawColors){
    util.coalsColors = new Array();
    if (Array.isArray(rawColors)){
      for (var i=0;i<rawColors.length;i++) util.coalsColors.push(rawColors[i]);
    }
    util.gradient = null; // rebuild
  };

  // Helpers
  function make2D(width,height,fill){
    var a = new Array(height);
    for (var y=0;y<height;y++){
      a[y] = new Array(width);
      for (var x=0;x<width;x++) a[y][x]=fill;
    }
    return a;
  }

  function lerpColor(a, b, t){
    if (t <= 0) return a; if (t >= 1) return b;
    var ar=(a>>16)&255, ag=(a>>8)&255, ab=a&255;
    var br=(b>>16)&255, bg=(b>>8)&255, bb=b&255;
    var r = Math.floor(ar + (br-ar)*t);
    var g = Math.floor(ag + (bg-ag)*t);
    var b2 = Math.floor(ab + (bb-ab)*t);
    return (r<<16) + (g<<8) + b2;
  }

  function buildGradient(){
    var colors = util.coalsColors;
    if (!Array.isArray(colors) || colors.length === 0){
      // Plasma "Fire" default
      colors = [ 0xFFFF00, 0xFF0000, 0x000040, 0xFF0000 ];
    }
    var n = colors.length;
    var grad = new Array(256);
    for (var i=0;i<256;i++){
      var t = i / 255.0;
      var pos = t * (n - 1);
      var idx = Math.floor(pos);
      var frac = pos - idx;
      var c0 = colors[idx];
      var c1 = colors[(idx+1 < n) ? idx+1 : idx];
      grad[i] = lerpColor(c0, c1, frac);
    }
    util.gradient = grad;
  }

  function heatToColorStandard(heat){
    // More yellow/orange bias with white tips, limited red halo
    var t = heat & 255;
    if (t < 24) return 0x000000; // suppress faint red glow
    // compress low values to reduce halo
    var h = t / 255.0;
    h = Math.pow(h, 1.6);
    var tt = Math.floor(h * 255);

    var r = 0, g = 0, b = 0;
    if (tt < 96){
      // dark red -> orange, start from low red
      r = Math.min(255, Math.floor(40 + tt * 1.9));
      g = Math.floor(tt * 1.0);
    } else if (tt < 192){
      // orange -> yellow
      r = 255;
      g = Math.min(255, Math.floor(110 + (tt - 96) * 1.5));
    } else {
      // yellow -> white tip
      r = 255; g = 255;
      b = Math.min(255, Math.floor((tt - 192) * 2));
    }
    return (r<<16) + (g<<8) + b;
  }

  function heatToColorCartoon(heat){
    var idx = Math.floor(heat / 43);
    if (idx < 0) idx = 0; if (idx > 5) idx = 5;
    var pal = [0x000000, 0x800000, 0xFF0000, 0xFF8000, 0xFFFF00, 0xFFFFFF];
    return pal[idx];
  }

  function plasmaNoise(xn, yn, t, size){
    // Simple sin/cos plasma-like noise 0..1 (time only affects x-term to avoid vertical drift)
    var k = size * 1.0;
    var v = Math.sin(6.2831853 * (xn * k) + t) * Math.cos(6.2831853 * (yn * k * 0.9));
    var n = 0.5 + 0.5 * v;
    return n;
  }

  function blend(ca, cb, alpha){ // alpha 0..1
    if (alpha <= 0) return ca; if (alpha >= 1) return cb;
    var ar=(ca>>16)&255, ag=(ca>>8)&255, ab=ca&255;
    var br=(cb>>16)&255, bg=(cb>>8)&255, bb=cb&255;
    var r = Math.floor(ar*(1-alpha) + br*alpha);
    var g = Math.floor(ag*(1-alpha) + bg*alpha);
    var b = Math.floor(ab*(1-alpha) + bb*alpha);
    return (r<<16) + (g<<8) + b;
  }

  function scaleBrightness(c, k){
    if (k >= 1) return c;
    if (k <= 0) return 0x000000;
    var r=(c>>16)&255, g=(c>>8)&255, b=c&255;
    r = Math.floor(r*k); g = Math.floor(g*k); b = Math.floor(b*k);
    if (r>255) r=255; if (g>255) g=255; if (b>255) b=255;
    return (r<<16) + (g<<8) + b;
  }

  algo.rgbMap = function(width, height, _rgb, _step){
    void _rgb; void _step; // QLC+ API requirement
    if (!util.initialized || util.heat===null || util.w!==width || util.h!==height){
      util.heat = make2D(width,height,0); util.w=width; util.h=height; util.initialized=true;
    }
    if (!util.xPhase1 || util.xW !== width){
      util.xPhase1 = new Array(width); util.xPhase2 = new Array(width); util.xAmp = new Array(width);
      for (var xi=0; xi<width; xi++){
        util.xPhase1[xi] = Math.random()*6.2831853;
        util.xPhase2[xi] = Math.random()*6.2831853;
        util.xAmp[xi]   = 0.85 + Math.random()*0.3; // 0.85..1.15 per-column amp
      }
      util.xW = width;
    }

    // Advance animation phase (used by flame undulation, shimmer and coals)
    var speedScale = Math.pow(100, (algo.coalsSpeed / 50.0)) / 500.0; // similar to plasma
    var simSpeed = 0.02 * algo.speed;
    util.coalsPhase += speedScale + simSpeed;
    if (util.coalsPhase > 1000000.0) util.coalsPhase = 0.0;

    if (util.gradient === null){ buildGradient(); }
    // Fire source mode: 0=Bottom, 1=Top, 2=Tunnel
    var isTunnel = (algo.fireSource === 2);
    // Direction sign for vertical travel: Bottom=up (+1), Top=down (-1)
    var dir = (algo.fireSource === 1) ? -1 : 1;

	    // Undulation amount scale (0..1)
	    var uScale = algo.undulation / 100.0;



    var map = new Array(height);
    var baseAlpha = algo.coalsPct / 100.0;
    var square = (width > height) ? width : height;

    // Compute active fire region (centered)
    var effW = Math.max(1, Math.floor(width * (algo.fireWidthPct / 100.0)));
    var left = Math.floor((width - effW) / 2);
    var right = left + effW - 1;
    // Only apply horizontal feathering if width is less than 100%
    var feather = (algo.fireWidthPct >= 100) ? 0 : Math.max(1, Math.floor(effW * 0.1));

    // Vertical height mask
    var capH = Math.max(1, Math.floor(height * (algo.fireHeightPct / 100.0)));
    var vfeather = Math.max(1, Math.floor(capH * 0.1));

	    // Initialize and advance warp plumes
	    var wc = (algo.warpCount|0); if (wc < 0) wc = 0; if (wc > 6) wc = 6;
	    if (!util.warps || util.warps.length !== wc){
	      util.warps = new Array(wc);
	      for (var wi=0; wi<wc; wi++){
	        // Distribute plumes more evenly across width to avoid left/right bias
	        // Add small random jitter to prevent perfectly uniform appearance
	        var baseXN = (wc > 1) ? (wi / (wc - 1)) : 0.5;
	        var jitter = 0.1 * (Math.random() - 0.5); // +/- 5% jitter
	        var xN = baseXN + jitter;
	        if (xN < 0.05) xN = 0.05; if (xN > 0.95) xN = 0.95;
	        util.warps[wi] = {
          xN: xN,
          yN: (dir === 1 ? (0.0 + Math.random()*0.06) : (1.0 - Math.random()*0.06)),
          ph: Math.random()*6.2831853,
          vyS: 0.90 + Math.random()*0.60,
          pull: 0.60 + Math.random()*0.80
        };
	      }
	      util.warpLastCount = wc;
	    }
	    for (var wi2=0; wi2<wc; wi2++){
	      var w = util.warps[wi2];
	      w.ph += 0.03 * algo.speed; if (w.ph > 1000000.0) w.ph = 0.0;
	      var vy = (0.10 + 0.08 * Math.sin(0.5 * w.ph + wi2*0.7));
		      vy = vy * (w.vyS ? w.vyS : 1.0);

	      w.yN += dir * vy * (algo.speed / 10.0) * 0.30;
		      // slow plume rise so flame visibly flows around plumes
		      w.yN -= dir * vy * (algo.speed / 10.0) * 0.24;

	      var vx = 0.015 * Math.sin(0.7 * w.ph + wi2*1.1);
	      w.xN += vx; if (w.xN < 0.05) w.xN = 0.05; if (w.xN > 0.95) w.xN = 0.95;

	      // Respawn plumes when they exit the visible area
	      // Use distributed respawn to maintain even coverage
	      if (dir === 1){
	        if (w.yN > 1.2){
	          w.yN = -0.05 + Math.random()*0.08;
	          // Respawn near original distributed position with jitter
	          var baseXN = (wc > 1) ? (wi2 / (wc - 1)) : 0.5;
	          w.xN = baseXN + 0.2 * (Math.random() - 0.5);
	          if (w.xN < 0.05) w.xN = 0.05; if (w.xN > 0.95) w.xN = 0.95;
	          w.ph = Math.random()*6.2831853;
	          w.vyS = 0.90 + Math.random()*0.60;
	          w.pull = 0.60 + Math.random()*0.80;
	        }
	      } else {
	        if (w.yN < -0.2){
	          w.yN = 1.05 - Math.random()*0.08;
	          var baseXN = (wc > 1) ? (wi2 / (wc - 1)) : 0.5;
	          w.xN = baseXN + 0.2 * (Math.random() - 0.5);
	          if (w.xN < 0.05) w.xN = 0.05; if (w.xN > 0.95) w.xN = 0.95;
	          w.ph = Math.random()*6.2831853;
	          w.vyS = 0.90 + Math.random()*0.60;
	          w.pull = 0.60 + Math.random()*0.80;
	        }
	      }
	    }


    for (var y=0;y<height;y++){
      map[y] = new Array(width);
      // Base is always at the display bottom. yAct is distance from bottom (0 at bottom)
      var yAct = (height - 1 - y);


	      // Row-level warp offset (bends centerline toward nearby rising plumes)
	      var yNrow = (capH > 1) ? (yAct / (capH - 1)) : 0.0;
	      var taperRow = 1.0 - yNrow;
	      var midRow = (left + right) * 0.5;
	      var rowWarpOffset = 0.0;
	      if (wc > 0){
	        var wStrength = algo.warpStrength / 100.0;
	        var spread = 0.18; // vertical spread in yN
	        for (var wi3=0; wi3<wc; wi3++){
	          var ww = util.warps[wi3];
	          var wx = left + (effW * ww.xN);
	          var dyw = yNrow - ww.yN; var adyw = (dyw < 0) ? -dyw : dyw;
	          var fall = 1.0 / (1.0 + (adyw / spread) * (adyw / spread));
	          rowWarpOffset += wStrength * (wx - midRow) * fall;
	        }
	        rowWarpOffset = rowWarpOffset * 0.45 * taperRow;
		        // amplify row-level bend for visibility
		        rowWarpOffset = rowWarpOffset * (0.80/0.45);

	      }

      for (var x=0;x<width;x++){
        // TUNNEL MODE: fire from all 4 edges toward center
        if (isTunnel) {
          var flickerPhase = util.coalsPhase;

          // Fire depth - how far flames reach from each edge toward center
          var maxDepthV = Math.floor(height / 2);
          var maxDepthH = Math.floor(width / 2);
          var fireDepthV = Math.max(1, Math.floor(maxDepthV * (algo.fireHeightPct / 100.0)));
          var fireDepthH = Math.max(1, Math.floor(maxDepthH * (algo.fireHeightPct / 100.0)));

          // Distance from each edge (0 = at edge)
          var distBottom = (height - 1) - y;
          var distTop = y;
          var distLeft = x;
          var distRight = (width - 1) - x;

          // Minimum distance to any edge (for coals)
          var minDist = Math.min(distBottom, distTop, distLeft, distRight);

          // ===== BOTTOM EDGE FIRE (going up) =====
          var heatBottom = 0;
          if (distBottom < fireDepthV) {
            var t = distBottom / fireDepthV; // 0 at edge, 1 at max depth
            var taper = 1.0 - t;
            // Flicker varies along x, animates with phase
            var flicker = 0.7 + 0.3 * Math.sin(flickerPhase * 4.5 + x * 0.2 - distBottom * 0.3);
            heatBottom = Math.floor(taper * flicker * 255);
          }

          // ===== TOP EDGE FIRE (going down) =====
          var heatTop = 0;
          if (distTop < fireDepthV) {
            var t = distTop / fireDepthV;
            var taper = 1.0 - t;
            var flicker = 0.7 + 0.3 * Math.sin(flickerPhase * 4.7 + x * 0.22 + distTop * 0.3);
            heatTop = Math.floor(taper * flicker * 255);
          }

          // ===== LEFT EDGE FIRE (going right) =====
          var heatLeft = 0;
          if (distLeft < fireDepthH) {
            var t = distLeft / fireDepthH;
            var taper = 1.0 - t;
            var flicker = 0.7 + 0.3 * Math.sin(flickerPhase * 4.3 + y * 0.25 - distLeft * 0.25);
            heatLeft = Math.floor(taper * flicker * 255);
          }

          // ===== RIGHT EDGE FIRE (going left) =====
          var heatRight = 0;
          if (distRight < fireDepthH) {
            var t = distRight / fireDepthH;
            var taper = 1.0 - t;
            var flicker = 0.7 + 0.3 * Math.sin(flickerPhase * 4.6 + y * 0.23 + distRight * 0.25);
            heatRight = Math.floor(taper * flicker * 255);
          }

          // Clamp heat values
          if (heatBottom < 0) heatBottom = 0; if (heatBottom > 255) heatBottom = 255;
          if (heatTop < 0) heatTop = 0; if (heatTop > 255) heatTop = 255;
          if (heatLeft < 0) heatLeft = 0; if (heatLeft > 255) heatLeft = 255;
          if (heatRight < 0) heatRight = 0; if (heatRight > 255) heatRight = 255;

          // Use maximum heat from all edges
          var maxHeat = Math.max(heatBottom, heatTop, heatLeft, heatRight);
          var tunnelFlame = heatToColorStandard(maxHeat);

          // ===== COALS at edges =====
          var coalsDepth = Math.max(2, Math.min(fireDepthV, fireDepthH) * 0.25);
          var coalsN = 0;
          if (minDist < coalsDepth) {
            coalsN = 1.0 - (minDist / coalsDepth);
          }
          if (coalsN < 0) coalsN = 0;
          if (coalsN > 1) coalsN = 1;

          // Coals plasma effect
          var xn = x / square; var yn = y / square;
          var cn1 = plasmaNoise(xn, yn, flickerPhase * 0.7, algo.coalsSize / 10.0);
          var cn2 = plasmaNoise(xn * 1.5, yn * 1.5, flickerPhase * 0.9, algo.coalsSize / 7.0);
          var coalsPlasma = 0.6 * cn1 + 0.4 * cn2;
          coalsPlasma = Math.pow(coalsPlasma, algo.coalsRamp / 15.0);

          var coalsIdx = Math.floor(coalsPlasma * 255);
          if (coalsIdx < 0) coalsIdx = 0; if (coalsIdx > 255) coalsIdx = 255;
          var tunnelCoals = util.gradient[coalsIdx];

          // ===== Final output =====
          var tunnelOut = 0x000000;
          var coalsAlpha = (algo.coalsPct / 100.0) * coalsN;

          if (maxHeat > 0 || coalsN > 0) {
            if (coalsN > 0 && coalsAlpha > 0) {
              // Blend coals and flames - coals at edge, flames inside
              var coalsBright = scaleBrightness(tunnelCoals, coalsAlpha);
              if (maxHeat > 10) {
                // Mix flames with coals
                var flameWeight = 1.0 - coalsN;
                tunnelOut = blend(coalsBright, tunnelFlame, flameWeight);
              } else {
                tunnelOut = coalsBright;
              }
            } else {
              tunnelOut = tunnelFlame;
            }
          }

          map[y][x] = tunnelOut;
          continue;
        }

        var inside = (x >= left && x <= right);
        // Width concentration shaping across Fire Width (center stronger when concentration is low)
        var widthWeight = 1.0;
        if (inside){
          var halfW = (effW > 1) ? ((effW - 1) * 0.5) : 0.5;
          var radX = (halfW > 0) ? Math.abs((x - midRow) / halfW) : 0.0; if (radX < 0) radX = 0; if (radX > 1) radX = 1;
          var alphaC = 1.0 - (algo.widthConcentration / 100.0);
          var baseX = 1.0 - radX;
          var powX = 1.0 + 3.0 * alphaC;
          var wExp = Math.pow(baseX, powX);
          widthWeight = (1.0 - alphaC) + alphaC * wExp;
        }
        // Per-column dynamic tip height and soft top fade
        var px1 = util.xPhase1 ? util.xPhase1[x] : 0.0;
        var px2 = util.xPhase2 ? util.xPhase2[x] : 0.0;
        var swing = Math.max(1, Math.floor(capH * 0.10)); // +/-10% of cap height
        var dyn = Math.sin(0.6 * util.coalsPhase + px1 * 0.6) + 0.5 * Math.sin(0.4 * util.coalsPhase - px2 * 0.7);
        if (dyn > 1) dyn = 1; if (dyn < -1) dyn = -1;
        var localCap = capH + Math.floor(dyn * swing);
        if (localCap < 1) localCap = 1; if (localCap > height) localCap = height;
        var dTop = (localCap - 1) - yAct;
        var overFeather = Math.max(1, Math.floor(vfeather * 0.8));
        var vFade;
        if (dTop >= vfeather) { vFade = 1.0; }
        else if (dTop >= 0) { vFade = dTop / vfeather; }
        else if (dTop > -overFeather) { vFade = 1.0 + (dTop / overFeather); }
        else { vFade = 0.0; }

        var heat = 0;
        if (inside){
          if (algo.flameType === 0){
            // Stylized flame via SDF around a wavering centerline (bright base, tapered tip)
            var yN = (capH > 1) ? (yAct / (capH - 1)) : 0.0; // 0 at base -> 1 at tip
            var taper = 1.0 - yN;
            var p1 = px1;
            var p2 = px2;
            var mid = (left + right) * 0.5;
            // Per-pixel warp influence near rising plumes
            var colWarpOffset = 0.0, rBoost = 0.0;
            if (wc > 0){
              var wStrength2 = algo.warpStrength / 100.0;
              var hspread = 0.35 * effW;
              var spread2 = 0.22;
              for (var wi4=0; wi4<wc; wi4++){
                var ww2 = util.warps[wi4];
                var wx2 = left + (effW * ww2.xN);
                var dy2 = yN - ww2.yN; var ady2 = (dy2 < 0) ? -dy2 : dy2;
                var vfall2 = 1.0 / (1.0 + (ady2 / spread2) * (ady2 / spread2));
                var dx2 = x - wx2; var adx2 = (dx2 < 0) ? -dx2 : dx2;
                var hfall2 = 1.0 / (1.0 + (adx2 / hspread) * (adx2 / hspread));
                var pull2 = ww2.pull ? ww2.pull : 1.0;
                var wfac = wStrength2 * pull2 * vfall2 * hfall2;
                colWarpOffset += wfac * dx2 * 0.85;
                rBoost += wfac * 0.55;
              }
              colWarpOffset = colWarpOffset * taper;
              rBoost = rBoost * taper;
            }
	            // Extra signed push so bright tongues split around plumes
	            if (wc > 0){
	              var wStrength3 = algo.warpStrength / 100.0;
	              var hspreadS = 0.35 * effW;
	              var spreadS = 0.22;
	              for (var wi6=0; wi6<wc; wi6++){
	                var wpS = util.warps[wi6];
	                var wxS = left + (effW * wpS.xN);
	                var dyS = yN - wpS.yN; var adyS = (dyS < 0) ? -dyS : dyS;
	                var dxS = x - wxS; var adxS = (dxS < 0) ? -dxS : dxS;
	                var sdxS = (dxS < 0) ? -1 : (dxS > 0 ? 1 : 0);
	                var vfallS = 1.0 / (1.0 + (adyS / spreadS) * (adyS / spreadS));
	                var hfallS = 1.0 / (1.0 + (adxS / hspreadS) * (adxS / hspreadS));
	                var pullS = wpS.pull ? wpS.pull : 1.0;
	                colWarpOffset += (wStrength3 * pullS * vfallS * hfallS) * sdxS * 0.95 * taper;
	              }
	            }


            var concNamp = algo.widthConcentration / 100.0; if (concNamp < 0) concNamp = 0; if (concNamp > 1) concNamp = 1;
            var wanderMult = 1.0 - 0.85 * concNamp * (0.7 + 0.3 * taper);
            var amp = (0.60 * effW * taper * uScale) * (util.xAmp ? util.xAmp[x] : 1.0) * wanderMult;
            /* dir calculated above */
            var und = 0.6 * Math.sin(3.4 * yN - dir * 0.9 * util.coalsPhase + p1)
                    + 0.4 * Math.sin(2.1 * yN - dir * 0.7 * util.coalsPhase - p2);
            // Add a gentle global sway so the whole body breathes
            var sway = (0.15 * effW * taper * uScale) * wanderMult * Math.sin(3.0 * yN - dir * 0.8 * util.coalsPhase + p2 * 0.5);
            var cx = mid + und * amp + sway + rowWarpOffset + colWarpOffset;
            var rMin = effW * 0.15;
            var rMax = effW * 0.35;
            var baseR = rMin + (rMax - rMin) * Math.pow(taper, 0.8);
            baseR += rBoost * baseR;
	            // Create a notch near plumes so bright core deflects around them
	            baseR = baseR * (1.0 - ((rBoost > 0.5) ? 0.5 : rBoost));

            var g1 = Math.exp(-0.5 * Math.pow((yN - 0.18) / 0.12, 2));
            var g2 = Math.exp(-0.5 * Math.pow((yN - 0.60) / 0.10, 2));
            baseR += 0.25 * baseR * g1 + 0.18 * baseR * g2; // bulges along the body
            // Add tongue undulation along the body for more visible waviness
            baseR += (0.12 * uScale) * baseR * Math.sin(4.2 * yN - dir * 1.6 * util.coalsPhase + p1 * 0.6);
            // Width Concentration: widen base radius more at base when high
            var concN = algo.widthConcentration / 100.0; if (concN < 0) concN = 0; if (concN > 1) concN = 1;
            var widen = 0.60 + 1.00 * concN;
            var scaleR = 1.0 + (widen - 1.0) * (0.6 + 0.4 * taper);
            baseR = baseR * scaleR;
            var maxRad = effW * (0.50 + 0.10 * concN); if (maxRad < 1) maxRad = 1;
            if (baseR > maxRad) baseR = maxRad;
            var edgeNoise = 0.10 * effW * taper * Math.sin(-dir * util.coalsPhase * 1.9 + yAct * 0.35 + x * 0.11 + p2 * 0.3);
            var rad = baseR + edgeNoise; if (rad < 1) rad = 1;
            var d = Math.abs(x - cx);
            if (d <= rad){
              var core = 1.0 - (d / rad);
              core = Math.pow(core, 1.2);
              var flicker = 0.80 + 0.20 * Math.sin(-dir * util.coalsPhase * 5.5 + yAct * 0.7 + x * 0.13 + p1 * 0.7);
              var rise = 0.90 + 0.10 * Math.sin(6.0 * yN - dir * 3.0 * util.coalsPhase);
              var intensity = core * (0.70 + 0.30 * taper) * flicker * rise;
              heat = Math.floor(intensity * 255 * vFade);
            } else { heat = 0; }
          } else {
            // Cartoon: same SDF shape but banded palette
            var yN2 = (capH > 1) ? (yAct / (capH - 1)) : 0.0;
            var taper2 = 1.0 - yN2;
            var p1c = util.xPhase1 ? util.xPhase1[x] : 0.0;
            var p2c = util.xPhase2 ? util.xPhase2[x] : 0.0;
            var midc = (left + right) * 0.5;
            // Per-pixel warp influence near rising plumes (cartoon)
            var colWarpOffset = 0.0, rBoost = 0.0;
            if (wc > 0){
              var wStrength2 = algo.warpStrength / 100.0;
              var hspread = 0.35 * effW;
              var spread2 = 0.22;
              for (var wi5=0; wi5<wc; wi5++){
                var ww3 = util.warps[wi5];
                var wx3 = left + (effW * ww3.xN);
                var dy3 = yN2 - ww3.yN; var ady3 = (dy3 < 0) ? -dy3 : dy3;
                var vfall3 = 1.0 / (1.0 + (ady3 / spread2) * (ady3 / spread2));
                var dx3 = x - wx3; var adx3 = (dx3 < 0) ? -dx3 : dx3;
                var hfall3 = 1.0 / (1.0 + (adx3 / hspread) * (adx3 / hspread));
                var pull3 = ww3.pull ? ww3.pull : 1.0;
                var wfac2 = wStrength2 * pull3 * vfall3 * hfall3;
                colWarpOffset += wfac2 * dx3 * 0.85;
                rBoost += wfac2 * 0.55;
              }
              colWarpOffset = colWarpOffset * taper2;
              rBoost = rBoost * taper2;
            }
	            // Extra signed push (cartoon) so bright tongues split around plumes
	            if (wc > 0){
	              var wStrength4 = algo.warpStrength / 100.0;
	              var hspreadC = 0.35 * effW;
	              var spreadC = 0.22;
	              for (var wi7=0; wi7<wc; wi7++){
	                var wpC = util.warps[wi7];
	                var wxC = left + (effW * wpC.xN);
	                var dyC = yN2 - wpC.yN; var adyC = (dyC < 0) ? -dyC : dyC;
	                var dxC = x - wxC; var adxC = (dxC < 0) ? -dxC : dxC;
	                var sdxC = (dxC < 0) ? -1 : (dxC > 0 ? 1 : 0);
	                var vfallC = 1.0 / (1.0 + (adyC / spreadC) * (adyC / spreadC));
	                var hfallC = 1.0 / (1.0 + (adxC / hspreadC) * (adxC / hspreadC));
	                var pullC = wpC.pull ? wpC.pull : 1.0;
	                colWarpOffset += (wStrength4 * pullC * vfallC * hfallC) * sdxC * 0.95 * taper2;
	              }
	            }


            var concNamp2 = algo.widthConcentration / 100.0; if (concNamp2 < 0) concNamp2 = 0; if (concNamp2 > 1) concNamp2 = 1;
            var wanderMult2 = 1.0 - 0.85 * concNamp2 * (0.7 + 0.3 * taper2);
            var camp = (0.60 * effW * taper2 * uScale) * (util.xAmp ? util.xAmp[x] : 1.0) * wanderMult2;
            /* dir calculated above */
            var undc = 0.6 * Math.sin(3.2 * yN2 - dir * 1.0 * util.coalsPhase + p1c)
                      + 0.4 * Math.sin(2.0 * yN2 - dir * 0.8 * util.coalsPhase - p2c);
            // Gentle global sway for cartoon as well
            var swayc = (0.12 * effW * taper2 * uScale) * wanderMult2 * Math.sin(3.0 * yN2 - dir * 0.8 * util.coalsPhase + p1c * 0.4);
            var cxc = midc + undc * camp + swayc + rowWarpOffset + colWarpOffset;
            var rMin2 = effW * 0.15;
            var rMax2 = effW * 0.35;
            var baseR2 = rMin2 + (rMax2 - rMin2) * Math.pow(taper2, 0.8);
            baseR2 += rBoost * baseR2;
	            // Create a notch near plumes (cartoon)
	            baseR2 = baseR2 * (1.0 - ((rBoost > 0.5) ? 0.5 : rBoost));

            var g1c = Math.exp(-0.5 * Math.pow((yN2 - 0.18) / 0.12, 2));
            var g2c = Math.exp(-0.5 * Math.pow((yN2 - 0.60) / 0.10, 2));
            baseR2 += 0.25 * baseR2 * g1c + 0.18 * baseR2 * g2c;
            // Tongue-like undulation for cartoon
            baseR2 += (0.10 * uScale) * baseR2 * Math.sin(4.0 * yN2 - dir * 1.4 * util.coalsPhase + p2c * 0.5);
            // Width Concentration: widen base radius (cartoon) more at base when high
            var concN2 = algo.widthConcentration / 100.0; if (concN2 < 0) concN2 = 0; if (concN2 > 1) concN2 = 1;
            var widen2 = 0.60 + 1.00 * concN2;
            var scaleR2 = 1.0 + (widen2 - 1.0) * (0.6 + 0.4 * taper2);
            baseR2 = baseR2 * scaleR2;
            var maxRad2 = effW * (0.50 + 0.10 * concN2); if (maxRad2 < 1) maxRad2 = 1;
            if (baseR2 > maxRad2) baseR2 = maxRad2;
            var edgeNoise2 = 0.10 * effW * taper2 * Math.sin(-dir * util.coalsPhase * 2.0 + yAct * 0.33 + x * 0.09 + p1c * 0.25);
            var rad2 = baseR2 + edgeNoise2; if (rad2 < 1) rad2 = 1;
            var d2 = Math.abs(x - cxc);
            if (d2 <= rad2){
              var core2 = 1.0 - (d2 / rad2);
              core2 = Math.pow(core2, 1.05);
              var flicker2 = 0.82 + 0.18 * Math.sin(-dir * util.coalsPhase * 5.0 + yAct * 0.6 + x * 0.12 + p2c * 0.6);
              var rise2 = 0.90 + 0.10 * Math.sin(6.0 * yN2 - dir * 3.0 * util.coalsPhase);
              var intensity2 = core2 * (0.72 + 0.28 * taper2) * flicker2 * rise2;
              heat = Math.floor(intensity2 * 255 * vFade);
            } else { heat = 0; }
          }
          if (heat < 0) heat = 0; if (heat > 255) heat = 255;
        }
        // Apply width concentration shaping before color mapping
        heat = Math.floor(heat * (typeof widthWeight === 'number' ? widthWeight : 1.0));
        if (heat < 0) heat = 0; if (heat > 255) heat = 255;
        // For very high Width Concentration, lightly fill base gaps near the bottom
        if (inside){
          var concFill = algo.widthConcentration / 100.0; if (concFill < 0) concFill = 0; if (concFill > 1) concFill = 1;
          var baseFac = (capH > 1) ? (1.0 - (yAct / (capH - 1))) : 1.0; if (baseFac < 0) baseFac = 0; if (baseFac > 1) baseFac = 1;
          var kfill = (concFill > 0.90) ? ((concFill - 0.90) / 0.10) : 0.0;
          var minHeat = Math.floor(255 * 0.12 * kfill * baseFac);
          if (heat < minHeat) heat = minHeat;
        }

        var flame;
        var isEdge = false;
        if (algo.flameType === 0){
          flame = heatToColorStandard(heat);
          // Height-based shaping: dim and warm the tip so the base reads as strongest
          var cwVis = (capH > 1) ? (1.0 - (yAct / (capH - 1))) : 1.0;
          flame = scaleBrightness(flame, 0.6 + 0.4 * cwVis); // 1.0 at base -> 0.6 at tip
          var r2=(flame>>16)&255, g2=(flame>>8)&255, b2=flame&255;
          var gscale = 0.6 + 0.4 * cwVis;  // reduce green at tip to avoid yellow at top
          var bscale = 0.5 + 0.5 * cwVis;  // reduce blue even more at tip
          g2 = Math.floor(g2 * gscale);
          b2 = Math.floor(b2 * bscale);
          flame = (r2<<16) + (g2<<8) + b2;
        } else {
          // Cartoon: banded colors (outline disabled for performance and to avoid dependency on heat buffer)
          flame = heatToColorCartoon(heat);
        }

        // Coals color (more defined near base)
        var xn = x / square; var yn = y / square;
        var wbase = (capH > 1) ? (1.0 - (yAct / (capH - 1))) : 1.0;
        if (wbase < 0) wbase = 0; if (wbase > 1) wbase = 1;
        var n1 = plasmaNoise(xn, yn, -dir * util.coalsPhase, algo.coalsSize / 10.0);
        var n2 = plasmaNoise(xn*1.7, yn*1.7, -dir * util.coalsPhase * 1.3, algo.coalsSize / 6.5);
        var n = (0.78 * n1 + 0.22 * n2 * wbase); if (n < 0) n = 0; if (n > 1) n = 1;
        var gamma = (algo.coalsRamp / 10.0) + (0.5 * wbase);
        var expo = Math.pow(n, gamma);
        var gi = Math.floor(expo * 255); if (gi<0) gi=0; if (gi>255) gi=255;
        var coals = util.gradient[gi];

        var out;
        if (!inside){
          out = 0x000000; // outside horizontal fire area = black
        } else {
          // Coals stronger near the base; flip automatically with orientation
          var coalsWeight = (capH > 1) ? (1.0 - (yAct / (capH - 1))) : 1.0;
          if (coalsWeight < 0) coalsWeight = 0; if (coalsWeight > 1) coalsWeight = 1;
          var alpha = baseAlpha * coalsWeight;
          // In Standard mode, suppress coals under bright flames to limit red halo
          if (algo.flameType === 0){
            var heatNorm = heat / 255.0;
            var suppress = 1.0 - (heatNorm * heatNorm);
            if (suppress < 0) suppress = 0; if (suppress > 1) suppress = 1;
            alpha = alpha * suppress;
          }

          out = (alpha > 0) ? blend(flame, coals, alpha) : flame;

          // Horizontal and vertical feathering
          var hDist = Math.min(x - left, right - x);
          var hFade = (feather <= 0) ? 1.0 : ((hDist < feather) ? (hDist / feather) : 1.0);
          var fade = hFade * vFade;
          if (fade < 0) fade = 0; if (fade > 1) fade = 1;
          out = scaleBrightness(out, fade);

          if (algo.flameType === 1 && isEdge) { out = 0x000000; }
        }
        map[y][x] = out;
      }
    }

    return map;
  };

  algo.rgbMapStepCount = function(_width,_height){ return 2; };

  return algo;
})();
