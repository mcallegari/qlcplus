/*
  Q Light Controller Plus
  waterflow.js

  Directional water flow with turbulence and caustic glints.
  Ported to QLC+ RGBScript.

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
  algo.name = "Water Flow";
  algo.author = "Branson Matheson";
  algo.acceptColors = 3; // deep / surface / foam
  algo.properties = new Array();

  algo.speed = 20;
  algo.properties.push("name:speed|type:range|display:Speed|values:1,50|write:setSpeed|read:getSpeed");

  algo.direction = 0; // 0=Right 1=Left 2=Down 3=Up
  algo.properties.push("name:direction|type:list|display:Direction|values:Right,Left,Down,Up|write:setDirection|read:getDirection");

  algo.turbulence = 30;
  algo.properties.push("name:turbulence|type:range|display:Turbulence|values:0,100|write:setTurbulence|read:getTurbulence");

  algo.caustics = 25;
  algo.properties.push("name:caustics|type:range|display:Caustics|values:0,100|write:setCaustics|read:getCaustics");

  algo.scale = 10;
  algo.properties.push("name:scale|type:range|display:Scale|values:1,30|write:setScale|read:getScale");

  var util = {};
  util.t = 0;
  util.initialized = false;
  util.colors = [];

  algo.setSpeed      = function(v){ algo.speed      = parseInt(v); };
  algo.getSpeed      = function(){ return algo.speed; };
  algo.setTurbulence = function(v){ algo.turbulence = parseInt(v); };
  algo.getTurbulence = function(){ return algo.turbulence; };
  algo.setCaustics   = function(v){ algo.caustics   = parseInt(v); };
  algo.getCaustics   = function(){ return algo.caustics; };
  algo.setScale      = function(v){ algo.scale      = parseInt(v); };
  algo.getScale      = function(){ return algo.scale; };

  algo.setDirection = function(v){
    if      (v==="Right") algo.direction=0;
    else if (v==="Left")  algo.direction=1;
    else if (v==="Down")  algo.direction=2;
    else                  algo.direction=3;
  };
  algo.getDirection = function(){
    return ["Right","Left","Down","Up"][algo.direction];
  };

  function defaultPalette(){ return [0x001133, 0x0077BB, 0xCCEEFF]; }

  algo.rgbMapSetColors = function(rawColors){
    if (!Array.isArray(rawColors)) return;
    util.colors = [];
    for (var i=0; i<algo.acceptColors; i++){
      if (i < rawColors.length && rawColors[i]===rawColors[i]) util.colors.push(rawColors[i]);
    }
    if (util.colors.length===0) util.colors = defaultPalette();
  };

  function getPalette(){ return (util.colors && util.colors.length) ? util.colors : defaultPalette(); }

  function lerpColor(a, b, t){
    var ar=(a>>16)&255, ag=(a>>8)&255, ab=a&255;
    var br=(b>>16)&255, bg=(b>>8)&255, bb=b&255;
    return (Math.floor(ar+(br-ar)*t)<<16) + (Math.floor(ag+(bg-ag)*t)<<8) + Math.floor(ab+(bb-ab)*t);
  }

  function samplePalette(pal, idx){
    if (pal.length===1) return pal[0];
    var x = idx*(pal.length-1), i=Math.floor(x), j=Math.min(pal.length-1,i+1);
    return lerpColor(pal[i], pal[j], x-i);
  }

  function scaleColor(rgb, s255){
    if (s255<=0) return 0; if (s255>=255) return rgb;
    var r=(rgb>>16)&255, g=(rgb>>8)&255, b=rgb&255;
    return (Math.floor(r*s255/255)<<16) + (Math.floor(g*s255/255)<<8) + Math.floor(b*s255/255);
  }

  function sin01(v){ return Math.sin(v)*0.5+0.5; }

  algo.rgbMap = function(width, height, _rgb, _step){
    if (!util.initialized){ util.t=0; util.initialized=true; }

    var sp    = algo.speed/50.0;
    var turb  = algo.turbulence/100.0;
    var camt  = algo.caustics/100.0;
    var sc    = algo.scale/10.0;
    var t     = util.t;
    util.t   += 0.035 * sp;

    var pal = getPalette();
    var map = new Array(height);

    for (var y=0; y<height; y++){
      map[y] = new Array(width);
      for (var x=0; x<width; x++){
        // Normalize to [0,1]
        var nx = width>1  ? x/(width-1)  : 0.5;
        var ny = height>1 ? y/(height-1) : 0.5;

        // Remap so flowPos advances in flow direction, crossPos is perpendicular
        var fp, cp;
        switch(algo.direction){
          case 0: fp=nx;   cp=ny;   break; // right
          case 1: fp=1-nx; cp=ny;   break; // left
          case 2: fp=ny;   cp=nx;   break; // down
          default: fp=1-ny; cp=nx;  break; // up
        }

        // Primary laminar-flow wave: stripes moving along fp, gently warped by cp
        var warp = turb * 0.35 * (Math.sin(cp*sc*5.3 + t*0.8) + 0.5*Math.sin(cp*sc*8.7 - t*1.1));
        var v1 = sin01((fp + warp) * sc * 7.0 - t * 3.5);

        // Secondary cross-current: turbulent eddies
        var v2 = sin01(cp*sc*4.7 + fp*sc*2.1 + t*1.9) * sin01(cp*sc*6.3 - fp*sc*1.6 - t*2.7);
        // v2 peaks at both sin = high simultaneously → blob-like eddies

        // Blend layers: low turbulence = laminar flow, high = churning eddies
        var v = v1*(1-turb*0.5) + v2*turb*0.5;

        // Caustic highlights: product of two high-freq waves → narrow bright streaks
        var caustic = 0.0;
        if (camt > 0){
          var c1 = sin01(fp*sc*19.3 + cp*sc*11.7 - t*5.9);
          var c2 = sin01(cp*sc*13.9 + fp*sc* 7.3 + t*4.3);
          caustic = c1 * c2;
          caustic = caustic * caustic * caustic; // sharpen to bright spots
          caustic *= camt;
        }

        // Color: map wave value to palette; caustics push toward palette peak (foam)
        var palIdx = Math.min(1.0, v * 0.85 + caustic * 0.8);
        var color  = samplePalette(pal, palIdx);

        // Strong caustics: bloom toward white
        if (caustic > 0.4){
          color = lerpColor(color, 0xFFFFFF, Math.min(1, (caustic-0.4)*camt*2.5));
        }

        map[y][x] = color;
      }
    }
    return map;
  };

  algo.rgbMapStepCount = function(_w,_h){ return 2; };

  return algo;
})();
