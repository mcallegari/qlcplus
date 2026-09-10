/*
  Q Light Controller Plus
  pacifica.js

  FastLED-inspired "Pacifica" layered ocean waves
  Ported to QLC+ RGBScript

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
  algo.name = "Pacifica";
  algo.author = "FastLED idea, port by Branson Matheson";
  algo.acceptColors = 5; // palette-driven
  algo.properties = new Array();

  // Controls
  algo.speed = 20; // 1..50
  algo.properties.push("name:speed|type:range|display:Speed|values:1,50|write:setSpeed|read:getSpeed");
  algo.scale = 12; // 4..40 larger -> wider waves
  algo.properties.push("name:scale|type:range|display:Scale|values:4,40|write:setScale|read:getScale");
  algo.depth = 60; // 0..100 contribution of deeper layers
  algo.properties.push("name:depth|type:range|display:Depth|values:0,100|write:setDepth|read:getDepth");

  var util = {};
  util.initialized = false;
  util.colors = [];
  util.t1 = 0; util.t2 = 0; util.t3 = 0;

  algo.setSpeed = function(v){ algo.speed = parseInt(v); };
  algo.getSpeed = function(){ return algo.speed; };
  algo.setScale = function(v){ algo.scale = parseInt(v); };
  algo.getScale = function(){ return algo.scale; };
  algo.setDepth = function(v){ algo.depth = parseInt(v); };
  algo.getDepth = function(){ return algo.depth; };

  function defaultPalette(){
    // Deep ocean blues/teal
    return [0x002244, 0x004477, 0x0088AA, 0x00AACC, 0x99EEFF];
  }

  algo.rgbMapSetColors = function(rawColors){
    if (!Array.isArray(rawColors)) return;
    util.colors = [];
    for (var i=0; i<algo.acceptColors; i++){
      if (i < rawColors.length && rawColors[i]===rawColors[i]) util.colors.push(rawColors[i]);
    }
    if (util.colors.length === 0) util.colors = defaultPalette();
  };

  function getPalette(){
    if (!util.colors || util.colors.length===0) return defaultPalette();
    return util.colors;
  }

  function lerpColor(a, b, t){
    var ar=(a>>16)&255, ag=(a>>8)&255, ab=a&255;
    var br=(b>>16)&255, bg=(b>>8)&255, bb=b&255;
    var r = Math.floor(ar + (br-ar)*t);
    var g = Math.floor(ag + (bg-ag)*t);
    var b2 = Math.floor(ab + (bb-ab)*t);
    return (r<<16) + (g<<8) + b2;
  }

  function samplePalette(pal, idx){
    // idx 0..1 -> color in palette
    if (pal.length===1) return pal[0];
    var x = idx * (pal.length-1);
    var i = Math.floor(x), j = Math.min(pal.length-1, i+1);
    var t = x - i;
    return lerpColor(pal[i], pal[j], t);
  }

  function makeMap(w,h,fill){
    var m=new Array(h); for (var y=0;y<h;y++){ m[y]=new Array(w); for (var x=0;x<w;x++) m[y][x]=fill; } return m;
  }

  function scaleColor(rgb, scale255){
    if (scale255<=0) return 0; if (scale255>=255) return rgb;
    var r=(rgb>>16)&255,g=(rgb>>8)&255,b=rgb&255;
    r=Math.floor(r*scale255/255); g=Math.floor(g*scale255/255); b=Math.floor(b*scale255/255);
    return (r<<16)+(g<<8)+b;
  }

  algo.rgbMap = function(width, height, _rgb, _step){
    if (!util.initialized){ util.t1=0; util.t2=0; util.t3=0; util.initialized=true; }

    // Advance timers with different speeds
    var sp = algo.speed/50.0;
    util.t1 += 0.03*sp; util.t2 += 0.019*sp; util.t3 += 0.013*sp;

    var pal = getPalette();
    var map = makeMap(width, height, 0);

    // Layer weights
    var w1 = 1.0;
    var w2 = 0.6 * (algo.depth/100.0);
    var w3 = 0.4 * (algo.depth/100.0);

    var sc = Math.max(1, algo.scale);
    var sq = (width>height?width:height);

    for (var y=0;y<height;y++){
      for (var x=0;x<width;x++){
        var nx = (x - width/2)/sq; var ny = (y - height/2)/sq;
        var a1 = Math.sin((nx*sc + util.t1) + Math.sin(ny*sc*0.7 + util.t1*1.3));
        var a2 = Math.sin((ny*sc*0.6 + util.t2) + Math.sin(nx*sc*0.5 + util.t2*1.2));
        var a3 = Math.sin(((nx+ny)*sc*0.33 + util.t3));
        var v = (w1*(a1*0.5+0.5) + w2*(a2*0.5+0.5) + w3*(a3*0.5+0.5)) / (w1+w2+w3);
        // slight shoreline foam
        var foam = Math.max(0, Math.sin( (nx*0.8 - ny*0.6) * sc + util.t1*0.7 ));
        v = Math.min(1, v*0.85 + 0.15*foam);
        var color = samplePalette(pal, v);
        // subtle dimming toward edges
        var ed = 1 - 0.3*(Math.abs(nx)*0.8 + Math.abs(ny)*0.8);
        map[y][x] = scaleColor(color, Math.max(0, Math.floor(255*ed)));
      }
    }

    return map;
  };

  algo.rgbMapStepCount = function(_width,_height){ return 2; };

  return algo;
})();

