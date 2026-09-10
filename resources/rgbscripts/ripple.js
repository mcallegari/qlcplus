/*
  Q Light Controller Plus
  ripple.js

  Expanding ripple circles from random centers
  Ported from FastLED-style idea to QLC+ RGBScript

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
  algo.name = "Ripple";
  algo.author = "Branson Matheson";
  algo.acceptColors = 1;
  algo.properties = new Array();

  // Controls
  algo.density = 20; // spawn chance per frame (0..100)
  algo.properties.push("name:density|type:range|display:Spawn Rate|values:0,100|write:setDensity|read:getDensity");
  algo.fade = 75; // 0..100
  algo.properties.push("name:fade|type:range|display:Fade|values:0,100|write:setFade|read:getFade");
  algo.thickness = 2; // 1..10
  algo.properties.push("name:thickness|type:range|display:Ring Thickness|values:1,10|write:setThickness|read:getThickness");
  algo.speed = 15; // 1..40
  algo.properties.push("name:speed|type:range|display:Speed|values:1,40|write:setSpeed|read:getSpeed");
  algo.maxRings = 6; // 1..20
  algo.properties.push("name:maxRings|type:range|display:Max Rings|values:1,20|write:setMaxRings|read:getMaxRings");

  var util = {};
  util.initialized = false;
  util.map = null;
  util.ripples = [];

  algo.setDensity = function(v){ algo.density = parseInt(v); };
  algo.getDensity = function(){ return algo.density; };
  algo.setFade = function(v){ algo.fade = parseInt(v); };
  algo.getFade = function(){ return algo.fade; };
  algo.setThickness = function(v){ algo.thickness = parseInt(v); };
  algo.getThickness = function(){ return algo.thickness; };
  algo.setSpeed = function(v){ algo.speed = parseInt(v); };
  algo.getSpeed = function(){ return algo.speed; };
  algo.setMaxRings = function(v){ algo.maxRings = parseInt(v); };
  algo.getMaxRings = function(){ return algo.maxRings; };

  function makeMap(w,h){ var m=new Array(h); for (var y=0;y<h;y++){ m[y]=new Array(w); for (var x=0;x<w;x++) m[y][x]=0; } return m; }

  function scaleColor(rgb, scale255){ if (scale255<=0) return 0; var r=(rgb>>16)&255,g=(rgb>>8)&255,b=rgb&255; r=Math.floor(r*scale255/255); g=Math.floor(g*scale255/255); b=Math.floor(b*scale255/255); return (r<<16)+(g<<8)+b; }

  function addColor(dst, add){ var r=((dst>>16)&255)+((add>>16)&255); var g=((dst>>8)&255)+((add>>8)&255); var b=(dst&255)+(add&255); if(r>255)r=255; if(g>255)g=255; if(b>255)b=255; return (r<<16)+(g<<8)+b; }

  function fadeMap(map,w,h,fadePct){ var keep = 255 - Math.floor(fadePct*255/100); for (var y=0;y<h;y++) for (var x=0;x<w;x++) map[y][x]=scaleColor(map[y][x], keep); }

  algo.rgbMap = function(width, height, rgb, _step){
    if (!util.initialized || util.map.length!==height || util.map[0].length!==width){
      util.map = makeMap(width,height);
      util.ripples=[];
      util.initialized=true;
    }

    // Fade previous frame
    fadeMap(util.map, width, height, algo.fade);

    // Spawn
    if (util.ripples.length < algo.maxRings){
      var chance = Math.random()*100;
      if (chance < algo.density){
        util.ripples.push({ x: Math.floor(Math.random()*width), y: Math.floor(Math.random()*height), r: 0 });
      }
    }

    // Grow and draw
    var speed = Math.max(1, algo.speed)/8.0;
    var newRipples = [];
    for (var i=0;i<util.ripples.length;i++){
      var rp = util.ripples[i];
      rp.r += speed;
      var maxR = Math.sqrt(width*width + height*height);
      if (rp.r < maxR){ newRipples.push(rp); }

      var thick = algo.thickness;
      var rmin = rp.r - thick; if (rmin < 0) rmin = 0;
      var rmax = rp.r + thick;

      // Draw ring by checking distance window
      for (var y=0;y<height;y++){
        for (var x=0;x<width;x++){
          var dx = x - rp.x; var dy = y - rp.y;
          var d = Math.sqrt(dx*dx + dy*dy);
          if (d >= rmin && d <= rmax){
            // intensity peaks at ring center
            var t = 1 - Math.abs(d - rp.r)/thick; if (t<0) t=0; if (t>1) t=1;
            var col = scaleColor(rgb, Math.floor(255*t));
            util.map[y][x] = addColor(util.map[y][x], col);
          }
        }
      }
    }
    util.ripples = newRipples;

    return util.map;
  };

  algo.rgbMapStepCount = function(_width,_height){ return 256; };

  return algo;
})();

