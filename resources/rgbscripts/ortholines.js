/*
  Q Light Controller Plus
  ortholines.js

  Orthogonal right-angle line features with plasma colorization.
  - Lines consist of two perpendicular segments (an "L" corner)
  - Entire field can be rotated
  - Growth modes:
      * None: static lengths
      * Maintain Total (H<->V): keeps H+V constant while exchanging length
      * Uniform Scale: scales both H and V in sync
  - Sync modes:
      * All Same: all features animate in phase
      * Randomize: each feature has a random phase offset

  The line pixels are colorized by a lightweight plasma function based on
  sin/cos mixes, animated over time.

  Controls
  - Features: number of L-corner features
  - Total Length (px): base total length H+V for each feature
  - Rotate (deg): rotates the orthogonal axes
  - Growth Mode: None | Maintain Total (H<->V) | Uniform Scale
  - Grow Amount (%): how much to extend/shrink
  - Sync Mode: All Same | Randomize
  - Speed (1..10): animation speed

  Licensed under the Apache License, Version 2.0
*/

(function(){
  var algo = {};
  algo.apiVersion = 3;
  algo.name = "OrthoLines";
  algo.author = "Branson Matheson with help from Augment";
  algo.acceptColors = 0; // plasma-driven
  algo.properties = [];

  // --- Properties ---
  algo.featureCount = 24; // 1..80
  algo.properties.push("name:features|type:range|display:Features|values:1,80|write:setFeatures|read:getFeatures");
  algo.setFeatures = function(v){ var n=parseInt(v,10); if(isNaN(n)) n=24; if(n<1)n=1; if(n>80)n=80; algo.featureCount=n; };
  algo.getFeatures = function(){ return algo.featureCount; };

  algo.totalLen = 40; // pixels 4..200
  algo.properties.push("name:totlen|type:range|display:Total Length (px)|values:4,200|write:setTotalLen|read:getTotalLen");
  algo.setTotalLen = function(v){ var n=parseInt(v,10); if(isNaN(n)) n=40; if(n<4)n=4; if(n>200)n=200; algo.totalLen=n; };
  algo.getTotalLen = function(){ return algo.totalLen; };

  algo.rotateDeg = 0; // 0..359
  algo.properties.push("name:rotate|type:range|display:Rotate (deg)|values:0,359|write:setRotate|read:getRotate");
  algo.setRotate = function(v){ var n=parseInt(v,10); if(isNaN(n)) n=0; if(n<0)n=0; if(n>359)n=359; algo.rotateDeg=n; };
  algo.getRotate = function(){ return algo.rotateDeg; };

  var GROW_LABELS = ["None","Maintain Total (H<->V)","Uniform Scale"];
  algo.growModeIndex = 1;
  algo.properties.push("name:growmode|type:list|display:Growth Mode|values:" + GROW_LABELS.join(',') + "|write:setGrowMode|read:getGrowMode");
  algo.setGrowMode = function(label){ for (var i=0;i<GROW_LABELS.length;i++){ if(label===GROW_LABELS[i]){ algo.growModeIndex=i; return; } } };
  algo.getGrowMode = function(){ return GROW_LABELS[algo.growModeIndex]; };

  algo.growAmtPct = 60; // 0..100
  algo.properties.push("name:growamt|type:range|display:Grow Amount (%)|values:0,100|write:setGrowAmt|read:getGrowAmt");
  algo.setGrowAmt = function(v){ var n=parseInt(v,10); if(isNaN(n)) n=60; if(n<0)n=0; if(n>100)n=100; algo.growAmtPct=n; };
  algo.getGrowAmt = function(){ return algo.growAmtPct; };

  var SYNC_LABELS = ["All Same","Randomize"];
  algo.syncIndex = 1;
  algo.properties.push("name:sync|type:list|display:Sync Mode|values:" + SYNC_LABELS.join(',') + "|write:setSync|read:getSync");
  algo.setSync = function(label){ for (var i=0;i<SYNC_LABELS.length;i++){ if(label===SYNC_LABELS[i]){ algo.syncIndex=i; return; } } };
  algo.getSync = function(){ return SYNC_LABELS[algo.syncIndex]; };

  algo.speed = 6; // 1..10
  algo.properties.push("name:speed|type:range|display:Speed|values:1,10|write:setSpeed|read:getSpeed");
  algo.setSpeed = function(v){ var n=parseInt(v,10); if(isNaN(n)) n=6; if(n<1)n=1; if(n>10)n=10; algo.speed=n; };
  algo.getSpeed = function(){ return algo.speed; };
  var COLOR_LABELS = ["Plasma per pixel","Solid per feature"];
  algo.colorModeIndex = 0;
  algo.properties.push("name:colormode|type:list|display:Line Color Style|values:" + COLOR_LABELS.join(',') + "|write:setColorMode|read:getColorMode");
  algo.setColorMode = function(label){ for (var i=0;i<COLOR_LABELS.length;i++){ if(label===COLOR_LABELS[i]){ algo.colorModeIndex=i; return; } } };
  algo.getColorMode = function(){ return COLOR_LABELS[algo.colorModeIndex]; };

  var CHANGE_LABELS = ["Slide","Random Steps"];
  algo.changeModeIndex = 0;
  algo.properties.push("name:changemode|type:list|display:Line Change|values:" + CHANGE_LABELS.join(',') + "|write:setChangeMode|read:getChangeMode");
  algo.setChangeMode = function(label){ for (var i=0;i<CHANGE_LABELS.length;i++){ if(label===CHANGE_LABELS[i]){ algo.changeModeIndex=i; return; } } };
  algo.getChangeMode = function(){ return CHANGE_LABELS[algo.changeModeIndex]; };

  algo.plasmaSpeed = 6;
  algo.properties.push("name:plasmaspeed|type:range|display:Plasma Speed|values:0,10|write:setPlasmaSpeed|read:getPlasmaSpeed");
  algo.setPlasmaSpeed = function(v){ var n=parseInt(v,10); if(isNaN(n)) n=6; if(n<0)n=0; if(n>10)n=10; algo.plasmaSpeed=n; };
  algo.getPlasmaSpeed = function(){ return algo.plasmaSpeed; };

  var PALETTE_LABELS = ["Rainbow","Fire","Ocean","User Defined"];
  algo.paletteIndex = 0;
  algo.properties.push("name:palette|type:list|display:Plasma Colors|values:" + PALETTE_LABELS.join(',') + "|write:setPalette|read:getPalette");
  algo.setPalette = function(label){
    for (var i=0;i<PALETTE_LABELS.length;i++){
      if (label===PALETTE_LABELS[i]){ algo.paletteIndex=i; break; }
    }
    algo.acceptColors = (algo.paletteIndex===3)?5:0;
    util.gradInitialized = false;
  };
  algo.getPalette = function(){ return PALETTE_LABELS[algo.paletteIndex]; };

  algo.life = 300; // frames
  algo.properties.push("name:life|type:range|display:Lifetime (frames)|values:10,5000|write:setLife|read:getLife");
  algo.setLife = function(v){ var n=parseInt(v,10); if(isNaN(n)) n=300; if(n<10)n=10; if(n>5000)n=5000; algo.life=n; };
  algo.getLife = function(){ return algo.life; };

  algo.lifeRandPct = 30;
  algo.properties.push("name:liferand|type:range|display:Lifetime Randomness (%)|values:0,100|write:setLifeRand|read:getLifeRand");
  algo.setLifeRand = function(v){ var n=parseInt(v,10); if(isNaN(n)) n=30; if(n<0)n=0; if(n>100)n=100; algo.lifeRandPct=n; };
  algo.getLifeRand = function(){ return algo.lifeRandPct; };

  algo.fadePct = 10;
  algo.properties.push("name:fadepct|type:range|display:Fade Portion (%)|values:0,50|write:setFadePct|read:getFadePct");
  algo.setFadePct = function(v){ var n=parseInt(v,10); if(isNaN(n)) n=10; if(n<0)n=0; if(n>50)n=50; algo.fadePct=n; };
  algo.getFadePct = function(){ return algo.fadePct; };

  algo.activePct = 100;
  algo.properties.push("name:activepct|type:range|display:Active Lines (%)|values:0,100|write:setActivePct|read:getActivePct");
  algo.setActivePct = function(v){ var n=parseInt(v,10); if(isNaN(n)) n=100; if(n<0)n=0; if(n>100)n=100; algo.activePct=n; };
  algo.getActivePct = function(){ return algo.activePct; };


  // --- State ---
  var util = {};
  util.features = [];
  util.lastW = 0; util.lastH = 0; util.frame = 0;
  util.gradientData = []; util.colorArray = []; util.gradInitialized = false;


  // --- Helpers ---
  function clamp(x,a,b){ return x<a?a:(x>b?b:x); }
  function addColor(dst, add){ var dr=(dst>>16)&255,dg=(dst>>8)&255,db=dst&255; var ar=(add>>16)&255,ag=(add>>8)&255,ab=add&255; var nr=dr+ar; if(nr>255)nr=255; var ng=dg+ag; if(ng>255)ng=255; var nb=db+ab; if(nb>255)nb=255; return (nr<<16)|(ng<<8)|nb; }

  function hsv2rgb(h, s, v){ // h:0..255 s:0..255 v:0..255
    if(s===0){ return (v<<16)|(v<<8)|v; }
    var r, g, b; var region = Math.floor(h/43); var rem = (h - region*43)*6;
    var p = (v*(255 - s))>>8; var q = (v*(255 - ((s*rem)>>8)))>>8; var t = (v*(255 - ((s*(255 - rem))>>8)))>>8;
    if(region===0){ r=v; g=t; b=p; }
    else if(region===1){ r=q; g=v; b=p; }
    else if(region===2){ r=p; g=v; b=t; }
    else if(region===3){ r=p; g=q; b=v; }
    else if(region===4){ r=t; g=p; b=v; }
    else { r=v; g=p; b=q; }
    return (r<<16)|(g<<8)|b;
  }
  void hsv2rgb; // Reserved for future use

  // Plasma field value 0..1 and palette gradient utilities
  function scaleColor(rgb, a){ var r=(rgb>>16)&255, g=(rgb>>8)&255, b=rgb&255; r=Math.floor(r*a); g=Math.floor(g*a); b=Math.floor(b*a); if(r<0)r=0;if(g<0)g=0;if(b<0)b=0; return (r<<16)|(g<<8)|b; }

  function plasmaV(x, y, w, h, t){
    var nx = x / (w>0?w:1), ny = y / (h>0?h:1);
    var v = 0.0;
    v += Math.sin((nx*3.0 + t*0.9) * Math.PI*2);
    v += Math.sin((ny*3.0 + t*1.1) * Math.PI*2);
    v += Math.sin(((nx+ny)*2.0 + t*0.7) * Math.PI*2);
    v = (v/3.0 + 1.0)*0.5; // 0..1
    return v;
  }

  util.initGradient = function(){
    var gradIdx = 0; util.gradientData = [];
    var colorArray = algo.rgbMapGetColors();
    for (var i=0; i<colorArray.length; i++){
      var sColor = colorArray[i]; var eColor = colorArray[(i+1)%colorArray.length];
      if (!sColor) sColor = 0; if (!eColor) eColor = 0;
      util.gradientData[gradIdx++] = sColor;
      var sr=(sColor>>16)&255, sg=(sColor>>8)&255, sb=sColor&255;
      var er=(eColor>>16)&255, eg=(eColor>>8)&255, eb=eColor&255;
      var stepR=(er-sr)/300, stepG=(eg-sg)/300, stepB=(eb-sb)/300;
      for (var s=1;s<300;s++){
        var gradR = (sr + stepR*s)|0; var gradG=(sg + stepG*s)|0; var gradB=(sb + stepB*s)|0;
        var gradRGB = ((gradR & 255)<<16) | ((gradG & 255)<<8) | (gradB & 255);
        util.gradientData[gradIdx++] = gradRGB;
      }
    }
    util.gradInitialized = true;
  };

  util.ensureGradient = function(){ if (!util.gradInitialized) util.initGradient(); };

  algo.rgbMapSetColors = function(rawColors){
    if (!rawColors || !rawColors.length) return;
    util.colorArray = new Array(rawColors.length);
    for (var i=0;i<util.colorArray.length;i++){
      util.colorArray[i] = (i<rawColors.length)?rawColors[i]:0;
    }
    util.gradInitialized = false;
  };

  algo.rgbMapGetColors = function(){
    if (algo.paletteIndex === 1){ return [0xFFFF00 >>> 0, 0xFF0000 >>> 0, 0x000040 >>> 0, 0xFF0000 >>> 0]; }
    else if (algo.paletteIndex === 2){ return [0x003AB9 >>> 0, 0x02EAFF >>> 0]; }
    else if (algo.paletteIndex === 3){ if (!util.colorArray || util.colorArray.length<=0) return [0x00FF00 >>> 0, 0xFFAA00 >>> 0, 0x0000FF >>> 0, 0xFFFF00 >>> 0, 0xFFFFFF >>> 0]; return util.colorArray; }
    else { return [0xFF0000 >>> 0, 0x00FF00 >>> 0, 0x0000FF >>> 0]; }
  };

  function sampleColor(x,y,w,h,t){
    util.ensureGradient();
    var v = plasmaV(x,y,w,h,t); var idx = Math.floor(v * (util.gradientData.length-1));
    if (idx<0) idx=0; if (idx>=util.gradientData.length) idx=util.gradientData.length-1;
    return util.gradientData[idx];
  }

  function drawLinePerPixel(map, w, h, x0, y0, x1, y1, t, alpha){
    var x=Math.floor(x0), y=Math.floor(y0); var x2=Math.floor(x1), y2=Math.floor(y1);
    var dx=Math.abs(x2-x), dy=Math.abs(y2-y); var sx=(x<x2)?1:-1; var sy=(y<y2)?1:-1; var err=dx-dy;
    var done = false;
    while(!done){ if(x>=0&&x<w&&y>=0&&y<h){ var col=sampleColor(x,y,w,h,t); if(alpha<0.999){ col=scaleColor(col, alpha); } map[y][x]=addColor(map[y][x], col); }
      if (x===x2 && y===y2) { done = true; } else { var e2=err*2; if(e2>-dy){ err-=dy; x+=sx; } if(e2<dx){ err+=dx; y+=sy; } }
    }
  }

  function drawLineSolid(map, w, h, x0, y0, x1, y1, color){
    var x=Math.floor(x0), y=Math.floor(y0); var x2=Math.floor(x1), y2=Math.floor(y1);
    var dx=Math.abs(x2-x), dy=Math.abs(y2-y); var sx=(x<x2)?1:-1; var sy=(y<y2)?1:-1; var err=dx-dy;
    var done = false;
    while(!done){ if(x>=0&&x<w&&y>=0&&y<h){ map[y][x]=addColor(map[y][x], color); }
      if (x===x2 && y===y2) { done = true; } else { var e2=err*2; if(e2>-dy){ err-=dy; x+=sx; } if(e2<dx){ err+=dx; y+=sy; } }
    }
  }



  function makeMap(w,h,fill){ var m=new Array(h); for (var y=0;y<h;y++){ m[y]=new Array(w); for(var x=0;x<w;x++) m[y][x]=fill; } return m; }

  function ensureFeatures(w,h){
    if (util.lastW!==w || util.lastH!==h){ util.features=[]; util.lastW=w; util.lastH=h; util.frame=0; }
    while (util.features.length < algo.featureCount){ util.features.push(spawnFeature(w,h)); }
    if (util.features.length > algo.featureCount){ util.features.length = algo.featureCount; }
  }

  function randBetween(a,b){ return a + Math.random()*(b-a); }

  function spawnFeature(w,h){
    var cx = randBetween(0.05*w, 0.95*w);
    var cy = randBetween(0.05*h, 0.95*h);
    var split = randBetween(0.25, 0.75); // fraction of totalLen for H
    var baseSum = algo.totalLen;
    var h0 = Math.max(1, Math.floor(baseSum * split));
    var v0 = Math.max(1, baseSum - h0);
    var dirH = (Math.random()<0.5)?1:-1; var dirV=(Math.random()<0.5)?1:-1;
    var phase = (algo.syncIndex===0)?0:Math.random();
    var born = util.frame;
    var baseLife = algo.life; var ldelta = Math.floor(baseLife * (algo.lifeRandPct/100.0));
    var life = baseLife + Math.floor((Math.random()*2-1) * ldelta); if (life < 5) life = 5;
    var fade = Math.floor(life * (algo.fadePct/100.0)); if (fade < 0) fade = 0;
    // Random change state
    var r = h0 / (baseSum>0?baseSum:1); var rTarget = r;
    var scale = 1.0; var scaleTarget = 1.0; var changeProg = 1.0; var changeDur = 1;
    return {cx:cx, cy:cy, h0:h0, v0:v0, baseSum:baseSum, dirH:dirH, dirV:dirV, phase:phase,
            born:born, life:life, fade:fade, alive:true,
            r:r, rTarget:rTarget, scale:scale, scaleTarget:scaleTarget,
            changeProg:changeProg, changeDur:changeDur};
  }

  function updateFeatures(){
    // Maintain active lines percentage and lifetimes; update random change state
    var target = Math.floor(algo.featureCount * (algo.activePct/100.0));
    var aliveCount = 0; var i;
    for (i=0;i<util.features.length;i++){ if (util.features[i].alive) aliveCount++; }
    // Spawn to reach target
    for (i=0;i<util.features.length && aliveCount<target;i++){
      if (!util.features[i].alive){ util.features[i] = spawnFeature(util.lastW, util.lastH); aliveCount++; }
    }
    // If too many alive, mark extras as not alive
    for (i=util.features.length-1;i>=0 && aliveCount>target;i--){ if (util.features[i].alive){ util.features[i].alive=false; aliveCount--; } }

    // Update alive features
    for (i=0;i<util.features.length;i++){
      var fe = util.features[i]; if (!fe.alive) continue;
      var age = util.frame - fe.born; if (age >= fe.life){ fe.alive=false; continue; }
      if (algo.changeModeIndex===1){ // Random Steps
        // progress towards targets
        if (fe.changeDur <= 0) fe.changeDur = 1;
        fe.changeProg += 1.0 / fe.changeDur;
        if (fe.changeProg >= 1.0){
          // commit and choose new targets
          fe.r = fe.rTarget; fe.scale = fe.scaleTarget;
          // duration depends on speed
          var durScale = (11 - algo.speed)/10.0; var base = 20 + Math.floor(Math.random()*80);
          fe.changeDur = Math.max(5, Math.floor(base * durScale)); fe.changeProg = 0.0;
          var amt = algo.growAmtPct/100.0;
          if (algo.growModeIndex===1){ // Maintain Total: pick new ratio around 0.5
            var A = 0.5 * amt; var rt = 0.5 + (Math.random()*2-1) * A; if (rt<0.05) rt=0.05; if (rt>0.95) rt=0.95; fe.rTarget = rt;
          } else if (algo.growModeIndex===2){ // Uniform Scale: pick new scale around 1.0
            var smin = 1.0 - amt; if (smin<0.1) smin=0.1; var smax = 1.0 + amt; if (smax>2.5) smax=2.5; fe.scaleTarget = smin + Math.random()*(smax-smin);
          } else { fe.rTarget = fe.r; fe.scaleTarget = fe.scale; fe.changeProg = 0.0; fe.changeDur = 30; }
        }
      }
    }
  }

  function triangle01(x){ // x in [0,1) -> triangle wave 0..1
    var f = x - Math.floor(x);
    return f<0.5 ? f*2.0 : 2.0 - f*2.0;
  }

  function render(w,h){
    var map = makeMap(w,h,0);
    var tAnim = util.frame * (algo.speed/10.0) * 0.02;      // growth/length animation
    var tPlasma = util.frame * (algo.plasmaSpeed/10.0) * 0.02; // plasma motion
    var angle = algo.rotateDeg * Math.PI/180.0;
    var ca = Math.cos(angle), sa = Math.sin(angle);
    var growAmt = algo.growAmtPct/100.0;

    for (var i=0;i<util.features.length;i++){
      var fe = util.features[i]; if (!fe.alive) continue;

      // fade in/out alpha from lifetime
      var age = util.frame - fe.born; var alpha = 1.0;
      var f = fe.fade; if (f>0){ if (age < f) alpha = age / f; else if (age > fe.life - f) alpha = (fe.life - age) / f; else alpha = 1.0; }
      if (alpha <= 0) continue;

      var H = fe.h0, V = fe.v0;
      if (algo.changeModeIndex===0){ // Slide
        var ph = triangle01(tAnim + fe.phase);
        if (algo.growModeIndex===1){ // Maintain Total: exchange H<->V
          var half = fe.baseSum*0.5;
          var span = fe.baseSum*0.5*growAmt; // up to 50% swing
          H = clamp(half + (ph-0.5)*2.0*span, 1, fe.baseSum-1);
          V = fe.baseSum - H;
        } else if (algo.growModeIndex===2){ // Uniform Scale
          var s = 1.0 + (ph-0.5)*2.0*growAmt; if (s<0.1) s=0.1; if (s>2.5) s=2.5;
          H = Math.max(1, Math.floor(fe.h0 * s));
          V = Math.max(1, Math.floor(fe.v0 * s));
        }
      } else { // Random Steps
        if (algo.growModeIndex===1){
          var r = fe.r*(1.0-fe.changeProg) + fe.rTarget*fe.changeProg;
          H = clamp(Math.floor(fe.baseSum * r), 1, fe.baseSum-1);
          V = fe.baseSum - H;
        } else if (algo.growModeIndex===2){
          var s2 = fe.scale*(1.0-fe.changeProg) + fe.scaleTarget*fe.changeProg; if (s2<0.1) s2=0.1; if (s2>2.5) s2=2.5;
          H = Math.max(1, Math.floor(fe.h0 * s2));
          V = Math.max(1, Math.floor(fe.v0 * s2));
        }
      }

      // elbow at (cx,cy). Compute endpoints with rotation
      var ex = fe.cx, ey = fe.cy;
      var hx = ex + fe.dirH * (H*ca);
      var hy = ey + fe.dirH * (H*sa);
      var vx = ex + fe.dirV * (-V*sa);
      var vy = ey + fe.dirV * ( V*ca);

      // draw
      if (algo.colorModeIndex===0){ // Plasma per pixel
        drawLinePerPixel(map,w,h, ex,ey, hx,hy, tPlasma, alpha);
        drawLinePerPixel(map,w,h, ex,ey, vx,vy, tPlasma, alpha);
      } else { // Solid per feature: sample plasma at elbow
        var featColor = sampleColor(Math.floor(ex), Math.floor(ey), w, h, tPlasma);
        if (alpha < 0.999) featColor = scaleColor(featColor, alpha);
        drawLineSolid(map,w,h, ex,ey, hx,hy, featColor);
        drawLineSolid(map,w,h, ex,ey, vx,vy, featColor);
      }
    }

    return map;
  }

  algo.rgbMap = function(width,height,_rgb,_step){
    void _rgb; void _step; // QLC+ API requirement
    ensureFeatures(width,height);
    updateFeatures();
    var out = render(width,height);
    util.frame += 1;
    return out;
  };

  algo.rgbMapStepCount = function(_w,_h){ void _w; void _h; return 4096; };

  return algo;
})();

