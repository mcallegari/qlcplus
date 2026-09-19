/*
  Q Light Controller Plus
  lantern.js

  Organic candle/lantern flame for short LED strips (~30 pixels).
  The user-selected color is the mid-flame reference; dimmer pixels
  shift toward dark embers and brighter pixels shift toward warm white.

  Properties:
    Flare on Start  — bright burst on first activation then fades to normal
    Flicker Amount  — 0 = slow gentle breathing, 100 = chaotic guttering

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
  algo.apiVersion = 2;
  algo.name = "Lantern";
  algo.author = "Branson Matheson";
  algo.acceptColors = 1;
  algo.properties = new Array();

  algo.flareOnStart = 1;
  algo.properties.push("name:flareOnStart|type:list|display:Flare on Start|values:Yes,No|write:setFlareOnStart|read:getFlareOnStart");

  algo.flickerAmt = 50;
  algo.properties.push("name:flickerAmt|type:range|display:Flicker Amount|values:0,100|write:setFlickerAmt|read:getFlickerAmt");

  algo.setFlareOnStart = function(v){ algo.flareOnStart = (v === "Yes") ? 1 : 0; };
  algo.getFlareOnStart = function(){ return (algo.flareOnStart === 1) ? "Yes" : "No"; };
  algo.setFlickerAmt  = function(v){ var n=parseInt(v,10); if(!isNaN(n)&&n>=0&&n<=100) algo.flickerAmt=n; };
  algo.getFlickerAmt  = function(){ return algo.flickerAmt; };

  // Flare phases (frame counts):
  //   0..FLARE_DARK   : all pixels black (lantern unlit)
  //   FLARE_DARK..FLARE_PEAK : surge to full white (ignition flash)
  //   FLARE_PEAK..FLARE_END  : cool from white into normal flame
  var FLARE_DARK = 8;
  var FLARE_PEAK = 20;
  var FLARE_END  = 100;
  var TWO_PI = 6.2831853;

  var util = {};
  util.initialized = false;
  util.phase  = 0.0;   // global animation clock
  util.ph1    = null;  // per-pixel slow-phase offsets
  util.ph2    = null;  // per-pixel fast-phase offsets
  util.pixCount = 0;
  util.flareFrame = 0;
  util.flaring    = false;

  function clamp(v, lo, hi){ return v<lo?lo:v>hi?hi:v; }

  function lerpColor(a, b, t){
    if (t<=0) return a; if (t>=1) return b;
    var ar=(a>>16)&255, ag=(a>>8)&255, ab=a&255;
    var br=(b>>16)&255, bg=(b>>8)&255, bb=b&255;
    return ((Math.floor(ar+(br-ar)*t))<<16)|
           ((Math.floor(ag+(bg-ag)*t))<<8)|
            Math.floor(ab+(bb-ab)*t);
  }

  function scaleBrightness(c, k){
    if (k<=0) return 0; if (k>=1) return c;
    var r=(c>>16)&255, g=(c>>8)&255, b=c&255;
    return ((Math.floor(r*k))<<16)|((Math.floor(g*k))<<8)|Math.floor(b*k);
  }

  // Map normalised heat [0..1] to a colour derived from the base flame colour.
  // heat < 0.05  → black (unlit)
  // heat 0..0.5  → dark ember: very dim base colour
  // heat 0.5..1  → base colour lerped toward warm white tip
  function heatToColor(base, heat){
    if (heat < 0.05) return 0x000000;

    if (heat < 0.50){
      var k = heat / 0.50;          // 0..1
      k = k * k;                    // quadratic: slow ramp from dark
      return scaleBrightness(base, 0.05 + k * 0.95);
    }

    // heat 0.5..1 — base to warm white
    var t = (heat - 0.50) / 0.50;  // 0..1
    var hotWhite = lerpColor(0xFFE080, 0xFFFFFF, t * 0.6);
    return lerpColor(base, hotWhite, t * 0.75);
  }

  algo.rgbMap = function(width, height, rgb, step){
    var total = width * height;

    // (Re-)initialise per-pixel state on first call or fixture size change
    if (!util.initialized || util.pixCount !== total){
      util.ph1 = new Array(total);
      util.ph2 = new Array(total);
      for (var i=0; i<total; i++){
        util.ph1[i] = Math.random() * TWO_PI;
        util.ph2[i] = Math.random() * TWO_PI;
      }
      util.pixCount   = total;
      util.phase      = 0.0;
      util.flareFrame = 0;
      util.flaring    = (algo.flareOnStart === 1);
      util.initialized = true;
    }

    util.phase += 0.09;
    if (util.phase > 1e6) util.phase = 0.0;

    // Flare: dark silence → white burst → settle into flame
    var flarePhase = -1; // -1 = no flare
    if (util.flaring){
      util.flareFrame++;
      if (util.flareFrame >= FLARE_END){ util.flaring = false; }
      else { flarePhase = util.flareFrame; }
    }

    var fk    = algo.flickerAmt / 100.0;
    var phase = util.phase;

    // Shared global throb — gives all pixels a common pulse
    var globalSlow = 0.5 + 0.5 * Math.sin(phase * 0.28);
    var globalFast = 0.5 + 0.5 * Math.sin(phase * 2.1)
                              * (0.6 + 0.4 * Math.sin(phase * 0.9));

    var map = new Array(height);
    var idx = 0;

    for (var y=0; y<height; y++){
      map[y] = new Array(width);

      for (var x=0; x<width; x++){
        var p1 = util.ph1[idx];
        var p2 = util.ph2[idx];

        // Per-pixel slow breathing (independent phase per pixel)
        var slow = 0.5 + 0.5 * Math.sin(phase * 0.30 + p1);

        // Per-pixel fast flicker: product of two sinusoids = intermittent dips
        var fast = 0.5 + 0.5 * Math.sin(phase * 3.2 + p2)
                             * (0.65 + 0.35 * Math.sin(phase * 1.6 + p1 * 0.7));

        // Blend per-pixel and global components
        var baseHeat    = slow * 0.55 + globalSlow * 0.45;  // smooth
        var flickerHeat = fast * 0.55 + globalFast * 0.45;  // chaotic

        // Mix by flicker amount; shift range into 0.25..1.0
        var raw  = baseHeat * (1.0 - fk) + flickerHeat * fk;
        var heat = clamp(0.25 + raw * 0.75, 0.0, 1.0);

        var col;

        if (flarePhase >= 0){
          if (flarePhase < FLARE_DARK){
            // Unlit — black
            col = 0x000000;
          } else if (flarePhase < FLARE_PEAK){
            // Surge: black → full white
            var t = (flarePhase - FLARE_DARK) / (FLARE_PEAK - FLARE_DARK);
            col = lerpColor(0x000000, 0xFFFFFF, t * t);
          } else {
            // Cool: white → normal flame
            var t2 = (flarePhase - FLARE_PEAK) / (FLARE_END - FLARE_PEAK);
            col = lerpColor(0xFFFFFF, heatToColor(rgb, heat), t2 * t2);
          }
        } else {
          col = heatToColor(rgb, heat);
        }

        map[y][x] = col;
        idx++;
      }
    }

    return map;
  };

  algo.rgbMapStepCount = function(width, height){ return 256; };

  return algo;
})();
