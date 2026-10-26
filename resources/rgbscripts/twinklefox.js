/*
  Q Light Controller Plus
  twinklefox.js

  FastLED "TwinkleFox" style twinkles with palettes, attack/decay and optional
  incandescent warming on the fade. Ported to QLC+ RGBScript.

  Controls
  - Speed (1..10): overall animation rate (attack/decay progression)
  - Density (0..10): how many twinkles spawn over time
  - Background (% 0..40): dim background level under twinkles
  - Palette: set of curated palettes; or provide custom colors (API v3)
  - Incandescent Warmth (% 0..100): warms the color as it fades (tail becomes amber)

  API: v3 (acceptColors) — if custom colors are provided, they override the selected palette

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

(function(){
  var algo = {};
  algo.apiVersion = 3;
  algo.name = "TwinkleFox";
  algo.author = "FastLED idea, port by Branson Matheson";
  algo.acceptColors = 5; // optional custom palette (engine/UI support at most 5 colors)
  algo.properties = [];

  // --- Properties ---
  algo.speed = 6; // 1..10
  algo.properties.push("name:speed|type:range|display:Speed|values:1,10|write:setSpeed|read:getSpeed");
  algo.setSpeed = function(v){ var n=parseInt(v,10); if(isNaN(n))n=6; if(n<1)n=1; if(n>10)n=10; algo.speed=n; };
  algo.getSpeed = function(){ return algo.speed; };

  algo.density = 5; // 0..10
  algo.properties.push("name:density|type:range|display:Density|values:0,10|write:setDensity|read:getDensity");
  algo.setDensity = function(v){ var n=parseInt(v,10); if(isNaN(n))n=5; if(n<0)n=0; if(n>10)n=10; algo.density=n; };
  algo.getDensity = function(){ return algo.density; };

  algo.bg = 0; // 0..40 percent
  algo.properties.push("name:bg|type:range|display:Background (%)|values:0,40|write:setBg|read:getBg");
  algo.setBg = function(v){ var n=parseInt(v,10); if(isNaN(n))n=0; if(n<0)n=0; if(n>40)n=40; algo.bg=n; };
  algo.getBg = function(){ return algo.bg; };

  var PAL_NAMES = ["TwinkleFox","Holly","Ice","RetroC9","Cloud","Party","RedWhite"];
  algo.palIndex = 0;
  algo.properties.push("name:palette|type:list|display:Palette|values:" + PAL_NAMES.join(',') + "|write:setPalette|read:getPalette");
  algo.setPalette = function(label){ for (var i=0;i<PAL_NAMES.length;i++){ if (label===PAL_NAMES[i]){ algo.palIndex=i; return; } } };
  algo.getPalette = function(){ return PAL_NAMES[algo.palIndex]; };

  algo.warmth = 40; // 0..100
  algo.properties.push("name:warmth|type:range|display:Incandescent Warmth (%)|values:0,100|write:setWarmth|read:getWarmth");
  algo.setWarmth = function(v){ var n=parseInt(v,10); if(isNaN(n))n=40; if(n<0)n=0; if(n>100)n=100; algo.warmth=n; };
  algo.getWarmth = function(){ return algo.warmth; };

  // --- State & helpers ---
  var util = {};
  util.colors = [];

  function makeMap(w,h,fill){ var m=new Array(h); for(var y=0;y<h;y++){ m[y]=new Array(w); for(var x=0;x<w;x++) m[y][x]=fill; } return m; }

  // Deterministic per-pixel pseudo-random hash (0..1), so that rgbMap() output
  // only depends on (width, height, step) - never on prior calls or
  // Math.random() - which is required for reproducible color fades.
  function hash01(x,y,salt){
    var h = (x*374761393 + y*668265263 + salt*2246822519) >>> 0;
    h = (h ^ (h >>> 13)) >>> 0;
    h = (h * 1274126177) >>> 0;
    h = (h ^ (h >>> 16)) >>> 0;
    return h / 4294967296;
  }

  function lerp(a,b,t){ return a + (b-a)*t; }
  function clamp01(x){ return (x<0?0:(x>1?1:x)); }
  function lerpColor(c1,c2,t){
    var r1=(c1>>16)&255,g1=(c1>>8)&255,b1=c1&255;
    var r2=(c2>>16)&255,g2=(c2>>8)&255,b2=c2&255;
    var r=Math.floor(lerp(r1,r2,t)), g=Math.floor(lerp(g1,g2,t)), b=Math.floor(lerp(b1,b2,t));
    return (r<<16)|(g<<8)|b;
  }
  function scaleColor(rgb, scale255){ if(scale255<=0)return 0; if(scale255>=255)return rgb; var r=(rgb>>16)&255,g=(rgb>>8)&255,b=rgb&255; r=Math.floor(r*scale255/255); g=Math.floor(g*scale255/255); b=Math.floor(b*scale255/255); return (r<<16)|(g<<8)|b; }
  function addColor(dst, add){ var dr=(dst>>16)&255,dg=(dst>>8)&255,db=dst&255; var ar=(add>>16)&255,ag=(add>>8)&255,ab=add&255; var nr=dr+ar; if(nr>255)nr=255; var ng=dg+ag; if(ng>255)ng=255; var nb=db+ab; if(nb>255)nb=255; return (nr<<16)|(ng<<8)|nb; }

  // Default palettes
  function palTwinkleFox(){ return [0x3F1F00,0x7F3300,0xFF7F00,0xFFD080,0xFFFFFF]; }
  function palHolly(){ return [0x002000,0x106010,0x30A030,0xFF0000,0xFFFFFF]; }
  function palIce(){ return [0x001020,0x003070,0x3FA0FF,0xA0D8FF,0xFFFFFF]; }
  function palRetroC9(){ return [0xFF0000,0x00FF00,0x0000FF,0xFF6600,0x00FFFF,0xFF00FF,0xFFFFFF]; }
  function palCloud(){ return [0x000000,0x202020,0x404040,0x808080,0xFFFFFF]; }
  function palParty(){ return [0xFF0040,0x0040FF,0x40FF00,0xFFD400,0xFF00FF,0x00FFFF]; }
  function palRedWhite(){ return [0x400000,0x800000,0xFF0000,0xFFFFFF]; }

  function getSelectedPalette(){
    switch(PAL_NAMES[algo.palIndex]){
      case 'Holly': return palHolly();
      case 'Ice': return palIce();
      case 'RetroC9': return palRetroC9();
      case 'Cloud': return palCloud();
      case 'Party': return palParty();
      case 'RedWhite': return palRedWhite();
      default: return palTwinkleFox();
    }
  }

  algo.rgbMapSetColors = function(raw){
    util.colors = [];
    if (raw && raw.length){ for (var i=0;i<algo.acceptColors && i<raw.length;i++){ if (raw[i]===raw[i]) util.colors.push(raw[i]); }
    }
  };
  algo.rgbMapGetColors = function(){ return util.colors; };

  function getPalette(){ return (util.colors && util.colors.length)? util.colors : getSelectedPalette(); }

  function samplePalette(pal, t){ if(pal.length===1) return pal[0]; var x=t*(pal.length-1); var i=Math.floor(x); if(i<0)i=0; var j=(i+1>=pal.length)?pal.length-1:(i+1); var f=x-i; return lerpColor(pal[i], pal[j], f); }

  // Brightness shaping: fast attack, slow decay
  function attackDecay(phase){ // phase in [0..1]
    var a=0.18; // attack fraction
    if (phase < a){ return clamp01(phase / a); }
    var d = (phase - a) / (1 - a); // 0..1
    var k = 1.9; // decay exponent
    return clamp01(1 - Math.pow(d, k));
  }

  function whitenAtTop(rgb, bright){ // add white near the top brightness
    var t = (bright>0.8)? ((bright-0.8)/0.2) : 0.0; if (t<0) t=0; if (t>1) t=1;
    if (t<=0) return rgb;
    return lerpColor(rgb, 0xFFFFFF, t*0.7);
  }

  function warmOnFade(rgb, bright, warmthPct){
    if (warmthPct<=0) return rgb;
    // Apply more warmth when brightness is low (decay tail)
    var w = (1 - bright) * (warmthPct/100.0);
    if (w<=0) return rgb;
    var r=(rgb>>16)&255,g=(rgb>>8)&255,b=rgb&255;
    // Pull toward amber: boost red slightly, reduce blue more than green
    var r2 = Math.min(255, Math.floor(r + 60*w));
    var g2 = Math.max(0, Math.floor(g - 40*w));
    var b2 = Math.max(0, Math.floor(b - 90*w));
    return (r2<<16)|(g2<<8)|b2;
  }

  // Cycle length (in steps) for a pixel's twinkle: spawn, attack/decay, then a
  // random-length dark gap before the next spawn. Longer gap = lower density.
  function cyclePixel(x,y,width,height,step,phaseStep){
    // Average number of idle frames between spawns for this pixel, driven by
    // density: higher density -> shorter gaps -> more twinkles on screen.
    var densityK = algo.density/10.0;
    var avgGap = 4 + Math.floor(400 * (1.0 - densityK)); // ~4..404 frames
    var activeLen = Math.max(2, Math.ceil(1.0/phaseStep)); // frames to finish attack/decay
    var period = activeLen + Math.max(1, Math.round(avgGap * (0.5 + hash01(x,y,7))));

    // Random per-pixel offset so twinkles don't spawn in lockstep.
    var offset = Math.floor(hash01(x,y,11) * period);
    var pos = (step + offset) % period;
    if (pos >= activeLen) return null; // idle gap, no twinkle active

    var ph = clamp01(pos / (activeLen - 1));
    var b = attackDecay(ph);

    // choose palette position per pixel seed, drift slowly with y for variety
    var tpal = hash01(x,y,13);
    tpal = clamp01(tpal * 0.85 + (y/(height>1?height-1:1))*0.15);

    return { b: b, tpal: tpal };
  }

  algo.rgbMap = function(width, height, _rgb, step){
    var pal = getPalette();
    var out = makeMap(width,height,0);

    // background
    var bgScale = Math.floor(255 * (algo.bg/100.0));
    var bgColor = scaleColor(0xFFFFFF, bgScale); // neutral gray background
    if (bgScale>0){ for (var y=0;y<height;y++){ for (var x=0;x<width;x++){ out[y][x]=bgColor; } } }

    // higher speed -> faster attack/decay progression
    var phaseStep = 0.06 * (algo.speed/10.0);
    if (phaseStep < 0.01) phaseStep = 0.01;

    for (var y=0;y<height;y++){
      for (var x=0;x<width;x++){
        var tw = cyclePixel(x,y,width,height,step,phaseStep);
        if (tw === null) continue;

        var baseColor = samplePalette(pal, tw.tpal);
        // whiten near top and warm on fade
        var c1 = whitenAtTop(baseColor, tw.b);
        var c2 = warmOnFade(c1, tw.b, algo.warmth);
        var col = scaleColor(c2, Math.floor(tw.b*255));
        out[y][x] = addColor(out[y][x], col);
      }
    }

    return out;
  };

  algo.rgbMapStepCount = function(_w,_h){ return 100; };

  // Development tool access
  testAlgo = algo;

  return algo;
})();

