/*
  Q Light Controller Plus
  fractal.js

  Evolving fractal (Mandelbrot/Julia) with:
  - Color options (acceptColors = 5, used as a gradient)
  - Movement: In, Out, Left, Right (zoom/pan)
  - Moving center around via drift (amplitude + speed)

  Colors (acceptColors = 5) — recommended roles:
    1) Gradient start
    2) Mid 1
    3) Mid 2
    4) Mid 3
    5) Gradient end

  Author: Branson Matheson with help from Augment
  Licensed under the Apache License, Version 2.0
*/

(function(){
  var algo = {};
  algo.apiVersion = 3;
  algo.name = "Fractal";
  algo.author = "Branson Matheson with help from Augment";
  algo.acceptColors = 5;

  // --- Properties ---
  algo.properties = [];

  // Fractal type
  var FT_LABELS = ["Mandelbrot","Julia"];
  algo.ftIndex = 0;
  algo.properties.push("name:type|type:list|display:Fractal|values:" + FT_LABELS.join(',') + "|write:setType|read:getType");
  algo.setType = function(label){ for (var i=0;i<FT_LABELS.length;i++){ if (label === FT_LABELS[i]){ algo.ftIndex = i; return; } } };
  algo.getType = function(){ return FT_LABELS[algo.ftIndex]; };

  // Movement
  var MV_LABELS = ["None","In","Out","Left","Right"];
  algo.mvIndex = 0;
  algo.properties.push("name:move|type:list|display:Movement|values:" + MV_LABELS.join(',') + "|write:setMove|read:getMove");
  algo.setMove = function(label){ for (var i=0;i<MV_LABELS.length;i++){ if (label === MV_LABELS[i]){ algo.mvIndex = i; return; } } };
  algo.getMove = function(){ return MV_LABELS[algo.mvIndex]; };

  // Speed (affects zoom/pan and evolution)
  algo.speed = 5;
  algo.properties.push("name:speed|type:range|display:Speed|values:0,10|write:setSpeed|read:getSpeed");
  algo.setSpeed = function(v){ var n=parseInt(v,10); if(isNaN(n))n=5; if(n<0)n=0; if(n>10)n=10; algo.speed=n; };
  algo.getSpeed = function(){ return algo.speed; };

  // Iterations (detail vs performance)
  algo.iter = 80;
  algo.properties.push("name:iter|type:range|display:Iterations|values:20,200|write:setIter|read:getIter");
  algo.setIter = function(v){ var n=parseInt(v,10); if(isNaN(n))n=80; if(n<20)n=20; if(n>200)n=200; algo.iter=n; };
  algo.getIter = function(){ return algo.iter; };

  // Center drift amount (% of current scale)
  algo.driftAmt = 30;
  algo.properties.push("name:driftAmt|type:range|display:Center Drift (%)|values:0,100|write:setDriftAmt|read:getDriftAmt");
  algo.setDriftAmt = function(v){ var n=parseInt(v,10); if(isNaN(n))n=30; if(n<0)n=0; if(n>100)n=100; algo.driftAmt=n; };
  algo.getDriftAmt = function(){ return algo.driftAmt; };

  // Center drift speed
  algo.driftSpeed = 3;
  algo.properties.push("name:driftSpeed|type:range|display:Drift Speed|values:0,10|write:setDriftSpeed|read:getDriftSpeed");
  algo.setDriftSpeed = function(v){ var n=parseInt(v,10); if(isNaN(n))n=3; if(n<0)n=0; if(n>10)n=10; algo.driftSpeed=n; };
  algo.getDriftSpeed = function(){ return algo.driftSpeed; };

  // --- Colors ---
  var util = {};
  util.colors = [0x000764, 0x2068CB, 0xEDFFFF, 0xFFAA00, 0x000000]; // default palette
  algo.rgbMapSetColors = function(c){ if (c && c.length){ util.colors = c; } };
  algo.rgbMapGetColors = function(){ return util.colors; };

  // --- State ---
  util.centerX = -0.5; // default Mandelbrot center
  util.centerY = 0.0;
  util.scale = 1.8;     // horizontal half-span; smaller => zoom in
  util.phase = 0.0;     // evolution phase

  util.lastW = 0; util.lastH = 0;

  function ensureState(width, height){
    if (util.lastW !== width || util.lastH !== height){
      util.lastW = width; util.lastH = height;
      // keep current center/scale when size changes
    }
  }

  // --- Helpers ---
  function clamp01(x){ return (x<0?0:(x>1?1:x)); }
  void clamp01; // Reserved for future use
  function lerp(a,b,t){ return a + (b-a)*t; }
  function lerpColor(c1,c2,t){
    var r1=(c1>>16)&255, g1=(c1>>8)&255, b1=c1&255;
    var r2=(c2>>16)&255, g2=(c2>>8)&255, b2=c2&255;
    var r=Math.floor(lerp(r1,r2,t)); var g=Math.floor(lerp(g1,g2,t)); var b=Math.floor(lerp(b1,b2,t));
    return (r<<16)|(g<<8)|b;
  }
  function sampleGradient(cols, t){
    if (!cols || cols.length===0) return 0;
    if (cols.length===1) return cols[0];
    if (t<=0) return cols[0]; if (t>=1) return cols[cols.length-1];
    var seg = (cols.length-1)*t; var i = Math.floor(seg); var f = seg - i;
    return lerpColor(cols[i], cols[i+1], f);
  }

  // --- Core ---
  algo.rgbMap = function(width, height, _rgb, _step){
    void _rgb; void _step; // QLC+ API requirement
    ensureState(width, height);

    var sp = algo.speed / 10.0;

    // Evolve phase
    util.phase += 0.06 * (0.5 + sp);
    if (util.phase > 1000000) util.phase = 0;

    // Movement (zoom/pan)
    if (algo.mvIndex === 1){ // In
      util.scale *= (1.0 - 0.06 * sp);
      if (util.scale < 1e-6) util.scale = 1e-6;
    } else if (algo.mvIndex === 2){ // Out
      util.scale *= (1.0 + 0.06 * sp);
      if (util.scale > 1000) util.scale = 1000;
    } else if (algo.mvIndex === 3){ // Left
      util.centerX -= util.scale * 0.10 * sp;
    } else if (algo.mvIndex === 4){ // Right
      util.centerX += util.scale * 0.10 * sp;
    }

    // Center drift (around baseline center)
    var driftA = (algo.driftAmt / 100.0) * util.scale;
    var driftS = algo.driftSpeed / 10.0;
    var cx = util.centerX + driftA * 0.7 * Math.sin(util.phase * (0.9 + 0.4*driftS));
    var cy = util.centerY + driftA * 0.5 * Math.cos(util.phase * (1.1 + 0.6*driftS));

    // Aspect-correct scales
    var sx = util.scale;
    var sy = sx * (height / (width || 1));

    // Julia parameter evolves in time
    var jcx = -0.8 + 0.6 * Math.cos(util.phase * 0.7);
    var jcy =  0.156 + 0.6 * Math.sin(util.phase * 1.1);

    var maxIter = algo.iter|0;

    var map = new Array(height);
    for (var y=0;y<height;y++){
      var row = new Array(width);
      // Precompute imaginary value for this scanline
      var iy = cy + ((y + 0.5) - height*0.5) / (height*0.5) * sy;
      for (var x=0;x<width;x++){
        var ix = cx + ((x + 0.5) - width*0.5) / (width*0.5) * sx;

        var zx, zy, cx0, cy0;
        if (algo.ftIndex === 0){
          // Mandelbrot: z0 = 0, c = (ix, iy)
          zx = 0.0; zy = 0.0; cx0 = ix; cy0 = iy;
        } else {
          // Julia: z0 = (ix, iy), c = (jcx, jcy)
          zx = ix; zy = iy; cx0 = jcx; cy0 = jcy;
        }

        var iter = 0;
        var zx2 = 0.0, zy2 = 0.0;
        while (iter < maxIter && (zx2 + zy2) <= 4.0){
          // z = z^2 + c
          // (zx+izy)^2 = (zx^2 - zy^2) + i(2*zx*zy)
          zy = 2.0*zx*zy + cy0;
          zx = zx2 - zy2 + cx0;
          zx2 = zx*zx; zy2 = zy*zy;
          iter++;
        }

        var col;
        if (iter >= maxIter){
          // Interior
          col = util.colors.length ? util.colors[0] : 0x000000;
        } else {
          // Smooth coloring
          var log_zn = 0.5 * Math.log(zx2 + zy2);
          var nu = Math.log(log_zn / Math.log(2.0)) / Math.log(2.0);
          var t = (iter + 1 - nu) / maxIter; if (t < 0) t = 0; if (t > 1) t = 1;
          col = sampleGradient(util.colors, t);
        }
        row[x] = col;
      }
      map[y] = row;
    }

    return map;
  };

  algo.rgbMapStepCount = function(_w,_h){ void _w; void _h; return 2048; };

  return algo;
})();

