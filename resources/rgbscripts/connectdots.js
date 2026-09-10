/*
  Q Light Controller Plus
  connectdots.js

  Moving dot clusters connected by lines ("constellations"). Each feature is a
  cluster of dots that move in straight segments for random durations. All dots
  in a feature are mutually connected by lines. Features have a lifetime and are
  regenerated with a new dot count around a base with randomness.

  Colors (API v3):
  - Color[0] = Line color
  - Color[1] = Dot color

  Controls
  - Dots / Feature: base number of dots in each feature
  - Dot Randomness (%): +/- percent variation around the base
  - Features: how many features (clusters)
  - Lifetime (frames): how long a feature lives before respawn
  - Speed (1..10): movement speed and segment duration scaling

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

(function(){
  var algo = {};
  algo.apiVersion = 3;
  algo.name = "ConnectDots";
  algo.author = "Branson Matheson";
  algo.acceptColors = 2; // [line, dot]
  algo.properties = [];

  // --- Properties ---
  algo.baseDots = 18; // 2..100
  algo.properties.push("name:basedots|type:range|display:Dots / Feature|values:2,100|write:setBaseDots|read:getBaseDots");
  algo.setBaseDots = function(v){ var n=parseInt(v,10); if(isNaN(n)) n=18; if(n<2) n=2; if(n>100) n=100; algo.baseDots=n; };
  algo.getBaseDots = function(){ return algo.baseDots; };

  algo.dotRandPct = 40; // 0..100  +/- % of base
  algo.properties.push("name:dotrand|type:range|display:Dot Randomness (%)|values:0,100|write:setDotRand|read:getDotRand");
  algo.setDotRand = function(v){ var n=parseInt(v,10); if(isNaN(n)) n=40; if(n<0) n=0; if(n>100) n=100; algo.dotRandPct=n; };
  algo.getDotRand = function(){ return algo.dotRandPct; };

  algo.featureCount = 2; // 1..8
  algo.properties.push("name:features|type:range|display:Features|values:1,8|write:setFeatureCount|read:getFeatureCount");
  algo.setFeatureCount = function(v){ var n=parseInt(v,10); if(isNaN(n)) n=2; if(n<1)n=1; if(n>8)n=8; algo.featureCount=n; };
  algo.getFeatureCount = function(){ return algo.featureCount; };

  algo.life = 220; // frames 10..1000
  algo.properties.push("name:lifetime|type:range|display:Lifetime (frames)|values:10,1000|write:setLifetime|read:getLifetime");
  algo.setLifetime = function(v){ var n=parseInt(v,10); if(isNaN(n)) n=220; if(n<10)n=10; if(n>1000)n=1000; algo.life=n; };
  algo.getLifetime = function(){ return algo.life; };

  algo.speed = 6; // 1..10
  algo.properties.push("name:speed|type:range|display:Speed|values:1,10|write:setSpeed|read:getSpeed");
  algo.setSpeed = function(v){ var n=parseInt(v,10); if(isNaN(n)) n=6; if(n<1)n=1; if(n>10)n=10; algo.speed=n; };
  algo.getSpeed = function(){ return algo.speed; };

  // Connect mode: "Complete" (all pairs) or "Tree (N-1)" minimal spanning tree
  var CONNECT_LABELS = ["Complete","Tree (N-1)"];
  algo.connectModeIndex = 0;
  algo.properties.push("name:connect|type:list|display:Connect Mode|values:" + CONNECT_LABELS.join(',') + "|write:setConnectMode|read:getConnectMode");
  algo.setConnectMode = function(label){ for (var i=0;i<CONNECT_LABELS.length;i++){ if (label === CONNECT_LABELS[i]){ algo.connectModeIndex = i; return; } } };
  algo.getConnectMode = function(){ return CONNECT_LABELS[algo.connectModeIndex]; };

  // Lifetime randomness (per-feature spawn)
  algo.lifeRandPct = 30; // 0..100
  algo.properties.push("name:liferand|type:range|display:Lifetime Randomness (%)|values:0,100|write:setLifeRand|read:getLifeRand");
  algo.setLifeRand = function(v){ var n=parseInt(v,10); if(isNaN(n)) n=30; if(n<0)n=0; if(n>100)n=100; algo.lifeRandPct=n; };
  algo.getLifeRand = function(){ return algo.lifeRandPct; };


  // --- State ---
  var util = {};
  util.colors = [0xB0B0B0, 0xFFFFFF]; // line, dot
  util.features = [];
  util.lastW = 0; util.lastH = 0; util.frame = 0;

  algo.rgbMapSetColors = function(raw){ if (raw && raw.length){ var out=[]; for(var i=0;i<algo.acceptColors && i<raw.length;i++){ if (raw[i]===raw[i]) out.push(raw[i]); } if(out.length>0) util.colors=out; } };
  algo.rgbMapGetColors = function(){ return util.colors; };

  // --- Helpers ---
  function clamp(x,a,b){ return x<a?a:(x>b?b:x); }
  function makeMap(w,h,fill){ var m=new Array(h); for (var y=0;y<h;y++){ m[y]=new Array(w); for(var x=0;x<w;x++) m[y][x]=fill; } return m; }
  function addColor(dst, add){ var dr=(dst>>16)&255,dg=(dst>>8)&255,db=dst&255; var ar=(add>>16)&255,ag=(add>>8)&255,ab=add&255; var nr=dr+ar; if(nr>255)nr=255; var ng=dg+ag; if(ng>255)ng=255; var nb=db+ab; if(nb>255)nb=255; return (nr<<16)|(ng<<8)|nb; }
  function scaleColor(rgb, s255){ if(s255<=0)return 0; if(s255>=255)return rgb; var r=(rgb>>16)&255,g=(rgb>>8)&255,b=rgb&255; r=Math.floor(r*s255/255); g=Math.floor(g*s255/255); b=Math.floor(b*s255/255); return (r<<16)|(g<<8)|b; }

  // Bresenham line
  function drawLine(map,w,h,x0,y0,x1,y1,color){
    var x= Math.floor(x0), y=Math.floor(y0); var x2=Math.floor(x1), y2=Math.floor(y1);
    var dx = Math.abs(x2-x), dy = Math.abs(y2-y); var sx = (x<x2)?1:-1; var sy=(y<y2)?1:-1; var err=dx-dy;
    while(true){ if(x>=0&&x<w&&y>=0&&y<h){ map[y][x] = addColor(map[y][x], color); }
      if (x===x2 && y===y2) break; var e2=err*2; if(e2>-dy){ err-=dy; x+=sx; } if(e2<dx){ err+=dx; y+=sy; }
    }
  }

  function drawDot(map,w,h,xf,yf,color){ var x=Math.floor(xf), y=Math.floor(yf); if(x>=0&&x<w&&y>=0&&y<h){ map[y][x] = addColor(map[y][x], color); }
    // tiny 4-neighbor glow
    var dim = scaleColor(color, 120);
    if (x>0) map[y][x-1] = addColor(map[y][x-1], dim);
    if (x+1<w) map[y][x+1] = addColor(map[y][x+1], dim);
    if (y>0) map[y-1][x] = addColor(map[y-1][x], dim);
    if (y+1<h) map[y+1][x] = addColor(map[y+1][x], dim);
  }

  // Build a minimal spanning tree (Prim-like), producing exactly N-1 edges
  function buildEdgesTree(dots){
    var n = dots.length; var edges = [];
    if (n <= 1) return edges;
    var used = new Array(n); for (var i=0;i<n;i++) used[i]=false; used[0]=true; var inCount=1;
    while (inCount < n){
      var bi=-1, bj=-1; var best=1e20;
      for (var i=0;i<n;i++){
        if (!used[i]) continue; var ai=dots[i];
        for (var j=0;j<n;j++){
          if (used[j]) continue; var aj=dots[j];
          var dx=ai.x-aj.x, dy=ai.y-aj.y; var d=dx*dx+dy*dy;
          if (d < best){ best=d; bi=i; bj=j; }
        }
      }
      if (bj<0){ // fallback in case of numerical issues
        for (var k=0;k<n;k++){ if (!used[k]){ bi=0; bj=k; break; } }
      }
      edges.push([bi,bj]); used[bj]=true; inCount++;
    }
    return edges;
  }


  function randBetween(a,b){ return a + Math.random()*(b-a); }
  function newVel(speed){ var ang = Math.random()*Math.PI*2; var v = 0.2 + 0.8*(algo.speed/10.0); var s=v*speed; return {vx:Math.cos(ang)*s, vy:Math.sin(ang)*s}; }

  function spawnFeature(w,h){
    // decide dot count
    var base = algo.baseDots;
    var delta = Math.floor(base * (algo.dotRandPct/100.0));
    var count = base + Math.floor((Math.random()*2-1)*delta);
    if (count<2) count=2; if (count>200) count=200;

    // place around a random center
    var cx = randBetween(0.25*w, 0.75*w);
    var cy = randBetween(0.25*h, 0.75*h);
    var rad = Math.max(2, Math.min(w,h) * 0.35); // cluster radius

    var dots = new Array(count);
    for (var i=0;i<count;i++){
      var r = Math.random()*rad*0.8; var a = Math.random()*Math.PI*2;
      var x = cx + Math.cos(a)*r; var y = cy + Math.sin(a)*r;
      var vel = newVel(1.0);
      var seg = Math.floor(20 + Math.random()*60 * (11-algo.speed)/10.0);
      dots[i] = {x:x,y:y,vx:vel.vx,vy:vel.vy, seg:seg};
    }
    var baseLife = algo.life;
    var ldelta = Math.floor(baseLife * (algo.lifeRandPct/100.0));
    var life = baseLife + Math.floor((Math.random()*2-1) * ldelta);
    if (life < 5) life = 5;
    return {born:util.frame, life:life, dots:dots};
  }

  function ensureFeatures(w,h){
    if (util.lastW!==w || util.lastH!==h){ util.features=[]; util.lastW=w; util.lastH=h; util.frame=0; }
    while (util.features.length < algo.featureCount){ util.features.push(spawnFeature(w,h)); }
    if (util.features.length > algo.featureCount){ util.features.length = algo.featureCount; }
  }

  function updateFeatures(w,h){
    // respawn expired
    for (var f=0; f<util.features.length; f++){
      var feat = util.features[f];
      if (util.frame - feat.born >= feat.life){ util.features[f] = spawnFeature(w,h); }
    }
    // update dots
    var spd = 1.0; // already scaled inside newVel
    for (var f2=0; f2<util.features.length; f2++){
      var fe = util.features[f2]; var dots = fe.dots;
      for (var i=0;i<dots.length;i++){
        var d = dots[i]; d.x += d.vx*spd; d.y += d.vy*spd; d.seg -= 1;
        // bounce on edges
        if (d.x<0){ d.x=0; d.vx=Math.abs(d.vx); }
        if (d.x>w-1){ d.x=w-1; d.vx=-Math.abs(d.vx); }
        if (d.y<0){ d.y=0; d.vy=Math.abs(d.vy); }
        if (d.y>h-1){ d.y=h-1; d.vy=-Math.abs(d.vy); }
        if (d.seg<=0){ var nv=newVel(1.0); d.vx=nv.vx; d.vy=nv.vy; d.seg=Math.floor(20 + Math.random()*60 * (11-algo.speed)/10.0); }
      }
    }
  }

  function render(w,h){
    var map = makeMap(w,h,0);
    var lineCol = util.colors[0]; var dotCol = (util.colors.length>1?util.colors[1]:util.colors[0]);
    var lineColDim = scaleColor(lineCol, 160); // softer lines

    for (var f=0; f<util.features.length; f++){
      var fe = util.features[f]; var n = fe.dots.length; if (n<2) continue;
      // fade near end of life
      var lifeLeft = fe.life - (util.frame - fe.born); var fade = clamp(lifeLeft/Math.max(1,fe.life), 0, 1);
      var lineFade = Math.floor(160 * fade);
      var dotFade = Math.floor(255 * fade);
      var lineC = scaleColor(lineCol, lineFade);
      var dotC = scaleColor(dotCol, dotFade);

      // lines based on connect mode
      if (algo.connectModeIndex === 0){
        // Complete graph
        for (var i=0;i<n;i++){
          for (var j=i+1;j<n;j++){
            var a = fe.dots[i], b = fe.dots[j];
            drawLine(map,w,h,a.x,a.y,b.x,b.y,lineC);
          }
        }
      } else {
        // Tree (N-1)
        var edges = buildEdgesTree(fe.dots);
        for (var e=0;e<edges.length;e++){
          var i2 = edges[e][0], j2 = edges[e][1];
          var a2 = fe.dots[i2], b2 = fe.dots[j2];
          drawLine(map,w,h,a2.x,a2.y,b2.x,b2.y,lineC);
        }
      }
      // dots on top
      for (var k=0;k<n;k++){
        var d = fe.dots[k]; drawDot(map,w,h,d.x,d.y,dotC);
      }
    }
    return map;
  }

  algo.rgbMap = function(width,height,_rgb,_step){
    ensureFeatures(width,height);
    updateFeatures(width,height);
    var out = render(width,height);
    util.frame += 1;
    return out;
  };

  algo.rgbMapStepCount = function(_w,_h){ return 4096; };

  return algo;
})();

