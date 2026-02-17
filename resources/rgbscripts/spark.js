/*
  Q Light Controller Plus
  spark.js

  Electricity on a matrix with three styles:
  - Lightning: burst across the matrix with bright flash and fading tail
  - Spark: short, punchy burst like a spark plug
  - Consistent: continuous arc between two sides, jittering with flicker

  Options
  - Count (1..10): concurrent arcs
  - Frequency (0..10): random delay between bursts (Lightning/Spark); jitter speed (Consistent)
  - Jaggedness (0..10): kink detail (lower = longer intervals between kinks) (Consistent)
  - Jagged % of Span (0..100): max lateral deviation as percent of available width/height (Consistent)
  - Bolt Jumpiness (0..10): how different each new bolt is between the same endpoints (Consistent)

  - Direction: Top->Bottom, Bottom->Top, Left->Right, Right->Left
  - Type: Lightning, Spark, Consistent

  Colors (acceptColors = 3) — recommended roles:
  1) Core (white/blue)
  2) Glow mid (cyan/blue)
  3) Outer glow (violet/blue)

  Author: Branson Matheson with help from Augment
  Licensed under the Apache License, Version 2.0
*/

(function(){
  var algo = {};
  algo.apiVersion = 3;
  algo.name = "Spark";
  algo.author = "Branson Matheson with help from Augment";
  algo.acceptColors = 3;

  // --- Properties ---
  algo.properties = [];

  // Type
  var TYPE_LABELS = ["Lightning","Spark","Consistent"];
  algo.typeIndex = 0;
  algo.properties.push("name:type|type:list|display:Type|values:" + TYPE_LABELS.join(',') + "|write:setType|read:getType");
  algo.setType = function(label){ for (var i=0;i<TYPE_LABELS.length;i++){ if (label === TYPE_LABELS[i]){ algo.typeIndex = i; resetArcs(); return; } } };
  algo.getType = function(){ return TYPE_LABELS[algo.typeIndex]; };

  // Direction
  var DIR_LABELS = ["Top->Bottom","Bottom->Top","Left->Right","Right->Left"];
  algo.dirIndex = 0;
  algo.properties.push("name:direction|type:list|display:Direction|values:" + DIR_LABELS.join(',') + "|write:setDirection|read:getDirection");
  algo.setDirection = function(label){ for (var i=0;i<DIR_LABELS.length;i++){ if (label === DIR_LABELS[i]){ algo.dirIndex = i; resetArcs(); return; } } };
  algo.getDirection = function(){ return DIR_LABELS[algo.dirIndex]; };

  // Count
  algo.count = 3;
  algo.properties.push("name:count|type:range|display:Count|values:1,10|write:setCount|read:getCount");
  algo.setCount = function(v){ var n=parseInt(v,10); if(isNaN(n)) n=3; if(n<1)n=1; if(n>10)n=10; algo.count=n; };
  algo.getCount = function(){ return algo.count; };

  // Frequency (spawn rate / jitter)
  algo.freq = 5;
  algo.properties.push("name:frequency|type:range|display:Frequency|values:0,10|write:setFrequency|read:getFrequency");

  // Jaggedness (Consistent amplitude)
  algo.jag = 5;
  algo.properties.push("name:jaggedness|type:range|display:Jaggedness|values:0,10|write:setJaggedness|read:getJaggedness");
  algo.setJaggedness = function(v){ var n=parseInt(v,10); if(isNaN(n)) n=5; if(n<0)n=0; if(n>10)n=10; algo.jag=n; };
  // Bolt Jumpiness (how different each new bolt is)
  algo.jump = 5;
  algo.properties.push("name:jumpiness|type:range|display:Bolt Jumpiness|values:0,10|write:setJumpiness|read:getJumpiness");
  algo.setJumpiness = function(v){ var n=parseInt(v,10); if(isNaN(n)) n=5; if(n<0)n=0; if(n>10)n=10; algo.jump=n; };
  algo.getJumpiness = function(){ return algo.jump; };

  // Jagged % of span (amplitude as percent of available width/height)
  algo.jperc = 12;
  algo.properties.push("name:jagpercent|type:range|display:Jagged % of Span|values:0,100|write:setJagPercent|read:getJagPercent");
  algo.setJagPercent = function(v){ var n=parseInt(v,10); if(isNaN(n)) n=12; if(n<0)n=0; if(n>100)n=100; algo.jperc=n; };
  algo.getJagPercent = function(){ return algo.jperc; };

  algo.getJaggedness = function(){ return algo.jag; };

  algo.setFrequency = function(v){ var n=parseInt(v,10); if(isNaN(n)) n=5; if(n<0)n=0; if(n>10)n=10; algo.freq=n; };
  algo.getFrequency = function(){ return algo.freq; };

  // --- Colors ---
  var util = {};
  util.colors = [0xFFFFFF, 0x66B2FF, 0x6A5ACD];
  algo.rgbMapSetColors = function(c){ if (c && c.length){ util.colors = c; } };
  algo.rgbMapGetColors = function(){ return util.colors; };

  // --- State ---
  util.arcs = []; // list of active arcs
  util.frame = 0;
  util.lastW = 0; util.lastH = 0;

  function resetArcs(){ util.arcs = []; util.spawnNextAt = undefined; }

  function ensureState(width, height){
    if (util.lastW !== width || util.lastH !== height){
      util.lastW = width; util.lastH = height;
      util.arcs = [];
      util.spawnNextAt = undefined;
    }
  }

  // --- Helpers ---
  function clamp01(x){ return (x<0?0:(x>1?1:x)); }
  function lerp(a,b,t){ return a + (b-a)*t; }
  function lerpColor(c1,c2,t){
    var r1=(c1>>16)&255, g1=(c1>>8)&255, b1=c1&255;
    var r2=(c2>>16)&255, g2=(c2>>8)&255, b2=c2&255;
    var r=Math.floor(lerp(r1,r2,t)); var g=Math.floor(lerp(g1,g2,t)); var b=Math.floor(lerp(b1,b2,t));
    return (r<<16)|(g<<8)|b;
  }
  function sampleGradient(cols, t){
    if (!cols || cols.length===0) return 0x000000;
    if (cols.length===1) return cols[0];
    var n = cols.length-1; var x = t*n; var i = Math.floor(x); if (i<0)i=0; if (i>=n)i=n-1;
    var f = x - i; return lerpColor(cols[i], cols[i+1], f);
  }

  function randRange(a,b){ return a + Math.random()*(b-a); }
  function irand(a,b){ return Math.floor(randRange(a,b+1)); }

  // simple hash-based noise for jagged path
  function hashf(n){ var s = Math.sin(n*12.9898 + 78.233)*43758.5453; return s - Math.floor(s); }
  function initElectrodeMotion(se, width, height){
    void width; void height; // Reserved for future use
    var sp = 0.05 + 0.45*(algo.freq/10.0);
    if (se.axis==='y'){
      se.f1 = se.sx; se.f2 = se.ex;
    } else {
      se.f1 = se.sy; se.f2 = se.ey;
    }
    se.v1 = (Math.random()*2 - 1) * sp;
    se.v2 = (Math.random()*2 - 1) * sp;
  }

  function updateElectrodeMotion(se, width, height){
    var flipP = 0.01 + 0.04*(algo.freq/10.0);
    var sp = 0.05 + 0.45*(algo.freq/10.0);
    if (typeof se.v1 !== 'number' || typeof se.v2 !== 'number') initElectrodeMotion(se,width,height);
    if (se.axis==='y'){
      var maxX = width - 1;
      se.f1 += se.v1; se.f2 += se.v2;
      if (se.f1 < 0){ se.f1 = 0; se.v1 = Math.abs(se.v1); }
      if (se.f1 > maxX){ se.f1 = maxX; se.v1 = -Math.abs(se.v1); }
      if (se.f2 < 0){ se.f2 = 0; se.v2 = Math.abs(se.v2); }
      if (se.f2 > maxX){ se.f2 = maxX; se.v2 = -Math.abs(se.v2); }
      if (Math.random() < flipP){ se.v1 = (Math.random()*2 - 1) * sp; }
      if (Math.random() < flipP){ se.v2 = (Math.random()*2 - 1) * sp; }
      se.sx = Math.round(se.f1); se.ex = Math.round(se.f2);
    } else {
      var maxY = height - 1;
      se.f1 += se.v1; se.f2 += se.v2;
      if (se.f1 < 0){ se.f1 = 0; se.v1 = Math.abs(se.v1); }
      if (se.f1 > maxY){ se.f1 = maxY; se.v1 = -Math.abs(se.v1); }
      if (se.f2 < 0){ se.f2 = 0; se.v2 = Math.abs(se.v2); }
      if (se.f2 > maxY){ se.f2 = maxY; se.v2 = -Math.abs(se.v2); }
      if (Math.random() < flipP){ se.v1 = (Math.random()*2 - 1) * sp; }
      if (Math.random() < flipP){ se.v2 = (Math.random()*2 - 1) * sp; }
      se.sy = Math.round(se.f1); se.ey = Math.round(se.f2);
    }
  }

  function noise1(x, seed){
    var xi = Math.floor(x);
    var xf = x - xi;
    var n0 = hashf(xi + seed*17.0);
    var n1 = hashf(xi + 1 + seed*17.0);
    return n0 + (n1 - n0)*xf;
  }

  function pickStartEnd(width, height, dirIdx){
    if (dirIdx === 0){ // Top->Bottom
      var x0 = irand(0,width-1); return {sx:x0, sy:0, ex:x0, ey:height-1, axis:'y', step:1};
    } else if (dirIdx === 1){ // Bottom->Top
      var x1 = irand(0,width-1); return {sx:x1, sy:height-1, ex:x1, ey:0, axis:'y', step:-1};
    } else if (dirIdx === 2){ // Left->Right
      var y0 = irand(0,height-1); return {sx:0, sy:y0, ex:width-1, ey:y0, axis:'x', step:1};
    } else { // Right->Left
      var y1 = irand(0,height-1); return {sx:width-1, sy:y1, ex:0, ey:y1, axis:'x', step:-1};
    }
  }

  function buildPath(width, height, dirIdx, style, seOpt, seed){
    var se = seOpt || pickStartEnd(width, height, dirIdx);
    var pts = [];
    var x = se.sx, y = se.sy;
    var maxSteps = (se.axis==='y') ? (Math.abs(se.ey - se.sy) + 1) : (Math.abs(se.ex - se.sx) + 1);

    // style-based length: spark short, others go full to far edge
    var len = (style === 'spark') ? Math.max(4, Math.floor(maxSteps*0.2)) : maxSteps;

    var jMag = 1; // random jitter for lightning/spark
    var sd = seed || 0;

	    // include exact start electrode point
	    pts.push([x,y]);


    for (var i=0; i<len; i++){
      // advance along main axis
      if (se.axis==='y') y += se.step; else x += se.step;

      if (style === 'consistent'){
        // Fixed electrodes, jagged lightning-like path using evolving noise, anchored to both endpoints
        var dim = (se.axis==='y') ? width : height;
        var perc = algo.jperc|0; if (perc<0) perc=0; if (perc>100) perc=100;
        var A = (perc/100.0) * dim; // amplitude in px as % of available width/height
        var sp = 0.05 + 0.35*(algo.freq/10.0); // jitter speed
        var t = util.frame * sp + sd*3.1;
        // Position along arc and noise blend (longer intervals between kinks)
        var s = (i+1) / (len || 1); if (s<0) s=0; if (s>1) s=1;
        var fscale = 0.3 + 1.7*(algo.jag/10.0); // more jaggedness => finer details
        var n1 = noise1(s*(1.0*fscale) + t, sd);
        var n2 = noise1(s*(0.3*fscale) - t*0.6 + 5.2, sd+2.7);
        var wav = (n1*2.0 - 1.0) + 0.6*(n2*2.0 - 1.0);
        if (se.axis==='y'){
          var base = (1.0 - s)*se.sx + s*se.ex;
          var tx = Math.round(base + A * wav);
          if (tx<0) tx=0; if (tx>=width) tx=width-1;
          x = (i===len-1) ? se.ex : tx;
        } else {
          var basey = (1.0 - s)*se.sy + s*se.ey;
          var ty = Math.round(basey + A * wav);
          if (ty<0) ty=0; if (ty>=height) ty=height-1;
          y = (i===len-1) ? se.ey : ty;
        }
      } else {
        // lightning/spark: random jitter and occasional larger deviation
        if (se.axis==='y'){
          x += irand(-jMag, jMag); if (x<0) x=0; if (x>=width) x=width-1;
        } else {
          y += irand(-jMag, jMag); if (y<0) y=0; if (y>=height) y=height-1;
        }
        if (style !== 'spark' && (Math.random() < 0.1)){
          if (se.axis==='y'){
            x += irand(-1,1); if (x<0) x=0; if (x>=width) x=width-1;
          } else {
            y += irand(-1,1); if (y<0) y=0; if (y>=height) y=height-1;
          }
        }
      }

      pts.push([x,y]);
      // stop if reached far edge
      if (se.axis==='y'){
        if ((se.step>0 && y>=se.ey) || (se.step<0 && y<=se.ey)) break;
      } else {
        if ((se.step>0 && x>=se.ex) || (se.step<0 && x<=se.ex)) break;
      }
    }
    return pts;
  }

  function spawnArc(width, height){
    var style = (algo.typeIndex===0? 'lightning' : (algo.typeIndex===1? 'spark' : 'consistent'));
    var life;
    if (style==='spark') life = irand(3,6);
    else if (style==='lightning') life = irand(10,18);
    else life = 999999; // consistent: effectively infinite, keep endpoints and jitter path
    var se = pickStartEnd(width, height, algo.dirIndex);
    if (style==='consistent') initElectrodeMotion(se, width, height);
    var seed = Math.random()*6.28318;
    var pts = buildPath(width, height, algo.dirIndex, style, se, seed);
    var arc = { style: style, pts: pts, age: 0, life: life, seed: seed, se: se };
    util.arcs.push(arc);
  }

  function limitArcs(width, height){
    var wanted = algo.count|0; if (wanted<1) wanted=1; if (wanted>10) wanted=10;
    // remove dead arcs
    var next = []; var i;
    for (i=0; i<util.arcs.length; i++){
      var a = util.arcs[i];
      if (a.style==='consistent') next.push(a);
      else if (a.age < a.life) next.push(a);
    }
    util.arcs = next;

    // spawn for consistent to reach count
    for (i=util.arcs.length; i<wanted; i++){
      spawnArc(width,height);
    }
  }

  function scheduleNextSpawn(now){
    var f = algo.freq|0; if (f<0) f=0; if (f>10) f=10;
    var minP = 6, maxP = 180; // frames
    var base = Math.floor(maxP - (maxP - minP) * (f/10.0)); if (base < 1) base = 1;
    var lo = Math.floor(base*0.5); if (lo<1) lo=1; var hi = Math.floor(base*1.5); if (hi<lo) hi=lo;
    util.spawnNextAt = now + irand(lo, hi);
  }

  function scheduleNextRebolt(A, now){
    var f = algo.freq|0; if (f<0) f=0; if (f>10) f=10;
    var minP = 8, maxP = 90; // frames between new bolts
    var base = Math.floor(maxP - (maxP - minP) * (f/10.0)); if (base < 2) base = 2;
    var lo = Math.floor(base*0.5); if (lo<1) lo=1; var hi = Math.floor(base*1.5); if (hi<lo) hi=lo;
    A.reboltAt = now + irand(lo, hi);
  }


  function maybeSpawn(width, height){
    if (algo.typeIndex === 2) return; // consistent: only maintain count
    if (util.arcs.length >= algo.count) return;
    if (typeof util.spawnNextAt !== 'number') scheduleNextSpawn(util.frame);
    if (util.frame >= util.spawnNextAt){
      spawnArc(width, height);
      scheduleNextSpawn(util.frame);
    }
  }

  function kernelWeight(dx,dy){
    var adx = dx<0?-dx:dx; var ady = dy<0?-dy:dy;
    if (adx===0 && ady===0) return 1.0;
    if ((adx===1 && ady===0) || (adx===0 && ady===1)) return 0.6;
    if (adx===1 && ady===1) return 0.4;
    return 0.0;
  }

  // --- Core ---
  algo.rgbMap = function(width, height, _rgb, _step){
    void _rgb; void _step; // QLC+ API requirement
    ensureState(width, height);
    util.frame++;

    // Maintain arcs
    limitArcs(width, height);
    maybeSpawn(width, height);

    // Update arcs and refresh path for 'consistent' keeping fixed endpoints
    var i;
    for (i=0; i<util.arcs.length; i++){
      var A = util.arcs[i];
      if (A.style === 'consistent'){
        updateElectrodeMotion(A.se, width, height);
        if (typeof A.reboltAt !== 'number') scheduleNextRebolt(A, util.frame);
        if (util.frame >= A.reboltAt){
          var jv = algo.jump|0; var delta = (Math.random()*2 - 1) * (0.5 + 6.0*(jv/10.0));
          A.seed += delta;
          scheduleNextRebolt(A, util.frame);
        }
        A.pts = buildPath(width, height, algo.dirIndex, 'consistent', A.se, A.seed);
      }
      A.age++;
    }

    // Intensity buffer
    var intens = new Array(height);
    for (var y=0;y<height;y++){
      var row = new Array(width);
      for (var x=0;x<width;x++) row[x] = 0.0;
      intens[y] = row;
    }

    // Draw arcs
    for (i=0; i<util.arcs.length; i++){
      var a = util.arcs[i];
      var env = 1.0;
      if (a.style === 'lightning'){
        // bright start, decaying with flicker
        var t = a.age / (a.life || 1);
        var base = (a.age<2) ? 1.0 : (1.0 - t);
        var flick = 0.75 + 0.25 * Math.sin(6.0 * t + a.seed + 0.7*util.frame);
        env = base * flick;
        if (env < 0) env = 0; if (env > 1) env = 1;
      } else if (a.style === 'spark'){
        var t2 = a.age / (a.life || 1);
        var base2 = (a.age<1)?1.0:(1.0 - t2);
        env = base2 * (0.8 + 0.2*Math.sin(12.0*t2 + a.seed));
        if (env < 0) env = 0; if (env > 1) env = 1;
      } else { // consistent
        // random brightness flicker rather than pulsing
        var r = hashf(util.frame*0.73 + a.seed*9.1);
        env = 0.6 + 0.4 * r;
        if (env < 0) env = 0; if (env > 1) env = 1;
      }

      var plen = a.pts.length; if (plen === 0) continue;
      for (var pi=0; pi<plen; pi++){
        var p = a.pts[pi]; var px = p[0], py = p[1];
        var along = 1.0 - (pi / (plen || 1)); // brighter near head
        var amp = env * (0.6 + 0.4*along);
        // deposit kernel
        for (var dy=-1; dy<=1; dy++){
          var yy = py + dy; if (yy<0 || yy>=height) continue;
          var rowRef = intens[yy];
          for (var dx=-1; dx<=1; dx++){
            var xx = px + dx; if (xx<0 || xx>=width) continue;
            var w = kernelWeight(dx,dy);
            if (w>0){
              var v = rowRef[xx] + amp * w;
              if (v > 1.0) v = 1.0; rowRef[xx] = v;
            }
          }
        }
      }
    }

    // Map intensity to colors
    var map = new Array(height);
    for (var yy2=0; yy2<height; yy2++){
      var row2 = new Array(width);
      for (var xx2=0; xx2<width; xx2++){
        var it = clamp01(intens[yy2][xx2]);
        row2[xx2] = (it <= 0 ? 0 : sampleGradient(util.colors, it));
      }
      map[yy2] = row2;
    }

    return map;
  };

  algo.rgbMapStepCount = function(_w,_h){ void _w; void _h; return 1024; };

  return algo;
})();

