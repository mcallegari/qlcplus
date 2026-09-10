/*
  Q Light Controller Plus
  stagepulse.js

  Concentric ring pulses radiating outward from a fixed impact point.
  Intended for floor LED matrices — rings expand from the staff stamp point.

  Impact X/Y set the origin as a percentage of the fixture group dimensions.
  Default (50/50) = center; shift toward an edge to match your stage layout.

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
  algo.name = "Stage Pulse";
  algo.author = "Branson Matheson";
  algo.acceptColors = 1;
  algo.properties = new Array();

  algo.speed = 8;
  algo.properties.push("name:speed|type:range|display:Speed|values:1,20|write:setSpeed|read:getSpeed");

  algo.ringSpacing = 12;
  algo.properties.push("name:ringSpacing|type:range|display:Ring Spacing|values:3,50|write:setRingSpacing|read:getRingSpacing");

  algo.ringWidth = 3;
  algo.properties.push("name:ringWidth|type:range|display:Ring Width|values:1,12|write:setRingWidth|read:getRingWidth");

  algo.impactX = 50;
  algo.properties.push("name:impactX|type:range|display:Impact X %|values:0,100|write:setImpactX|read:getImpactX");

  algo.impactY = 50;
  algo.properties.push("name:impactY|type:range|display:Impact Y %|values:0,100|write:setImpactY|read:getImpactY");

  algo.setSpeed       = function(v){ var n=parseInt(v,10); if(!isNaN(n)&&n>=1&&n<=20)  algo.speed=n; };
  algo.getSpeed       = function(){ return algo.speed; };
  algo.setRingSpacing = function(v){ var n=parseInt(v,10); if(!isNaN(n)&&n>=3&&n<=50)  algo.ringSpacing=n; };
  algo.getRingSpacing = function(){ return algo.ringSpacing; };
  algo.setRingWidth   = function(v){ var n=parseInt(v,10); if(!isNaN(n)&&n>=1&&n<=12)  algo.ringWidth=n; };
  algo.getRingWidth   = function(){ return algo.ringWidth; };
  algo.setImpactX     = function(v){ var n=parseInt(v,10); if(!isNaN(n)&&n>=0&&n<=100) algo.impactX=n; };
  algo.getImpactX     = function(){ return algo.impactX; };
  algo.setImpactY     = function(v){ var n=parseInt(v,10); if(!isNaN(n)&&n>=0&&n<=100) algo.impactY=n; };
  algo.getImpactY     = function(){ return algo.impactY; };

  var util = {};
  util.rings       = [];
  util.initialized = false;
  util.speedAcc    = 0.0;

  function scaleColor(rgb, k255){
    if(k255<=0) return 0; if(k255>=255) return rgb;
    var r=(rgb>>16)&255, g=(rgb>>8)&255, b=rgb&255;
    return ((Math.floor(r*k255/255))<<16)|((Math.floor(g*k255/255))<<8)|Math.floor(b*k255/255);
  }

  function addColor(a, b){
    var r=((a>>16)&255)+((b>>16)&255);
    var g=((a>>8)&255)+((b>>8)&255);
    var bl=(a&255)+(b&255);
    if(r>255)r=255; if(g>255)g=255; if(bl>255)bl=255;
    return (r<<16)+(g<<8)+bl;
  }

  algo.rgbMap = function(width, height, rgb, step){
    if(!util.initialized){
      util.rings    = [{r: 0}];
      util.speedAcc = 0.0;
      util.initialized = true;
    }

    // Advance ring radii
    util.speedAcc += algo.speed / 8.0;
    var steps = Math.floor(util.speedAcc);
    util.speedAcc -= steps;
    for(var i=0; i<util.rings.length; i++) util.rings[i].r += steps;

    // Spawn a new ring whenever the innermost ring reaches ringSpacing distance
    // (or there are no rings left at all, e.g. all retired past maxD in one jump)
    if(util.rings.length === 0 || util.rings[util.rings.length-1].r >= algo.ringSpacing){
      util.rings.push({r: 0});
    }

    // Retire rings that have left the stage
    var maxD = Math.sqrt(width*width + height*height) + algo.ringWidth + 1;
    var kept = [];
    for(var i=0; i<util.rings.length; i++){
      if(util.rings[i].r < maxD) kept.push(util.rings[i]);
    }
    util.rings = kept;

    var cx = (width  > 1) ? (width-1)  * (algo.impactX / 100.0) : 0;
    var cy = (height > 1) ? (height-1) * (algo.impactY / 100.0) : 0;
    var rw = algo.ringWidth;

    var map = new Array(height);
    for(var y=0; y<height; y++){
      map[y] = new Array(width);
      for(var x=0; x<width; x++){
        var dx = x - cx, dy = y - cy;
        var d = Math.sqrt(dx*dx + dy*dy);
        var col = 0;

        for(var ri=0; ri<util.rings.length; ri++){
          var rel = d - util.rings[ri].r; // -ve = inside ring, +ve = outside (ahead)
          if(rel > rw * 0.25 || rel < -rw) continue; // skip far-outside or deep-inside

          var t;
          if(rel <= 0){
            // Behind leading edge: bright at front, decaying inward
            t = Math.exp(rel * 3.0 / rw);
          } else {
            // Slight leading glow
            t = Math.max(0, 1.0 - rel / (rw * 0.25));
          }
          col = addColor(col, scaleColor(rgb, Math.floor(255 * t)));
        }

        map[y][x] = col;
      }
    }

    return map;
  };

  algo.rgbMapStepCount = function(width, height){ return 256; };

  return algo;
})();
