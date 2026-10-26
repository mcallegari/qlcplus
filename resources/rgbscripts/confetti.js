/*
  Q Light Controller Plus
  confetti.js

  FastLED-inspired "Confetti" (random speckles that blink and fade)
  Ported to QLC+ RGBScript by Branson Matheson

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
  var algo = new Object;
  algo.apiVersion = 2;
  algo.name = "Confetti";
  algo.author = "FastLED idea, port by Branson Matheson";
  algo.acceptColors = 1;
  algo.properties = new Array();

  algo.density = 20; // 1..100 percent of pixels per frame that may light
  algo.properties.push("name:density|type:range|display:Density|values:1,100|write:setDensity|read:getDensity");
  algo.fade = 80; // 0..100
  algo.properties.push("name:fade|type:range|display:Fade (0-100)|values:0,100|write:setFade|read:getFade");

  var util = new Object;
  util.initialized = false;
  util.map = null;

  algo.setDensity = function(v){ algo.density = parseInt(v); };
  algo.getDensity = function(){ return algo.density; };
  algo.setFade = function(v){ algo.fade = parseInt(v); };
  algo.getFade = function(){ return algo.fade; };

  function makeMap(width, height, fill){
    var m = new Array(height);
    for (var y=0;y<height;y++){
      m[y] = new Array(width);
      for (var x=0;x<width;x++) m[y][x] = fill;
    }
    return m;
  }
  function scaleColor(rgb, scale255){
    if (scale255 <= 0) return 0;
    if (scale255 >= 255) return rgb;
    var r=(rgb>>16)&255,g=(rgb>>8)&255,b=rgb&255;
    r=Math.floor(r*scale255/255); g=Math.floor(g*scale255/255); b=Math.floor(b*scale255/255);
    return (r<<16)+(g<<8)+b;
  }
  function addColor(dst, add){
    var dr=(dst>>16)&255,dg=(dst>>8)&255,db=dst&255;
    var ar=(add>>16)&255,ag=(add>>8)&255,ab=add&255;
    var nr=dr+ar; if(nr>255)nr=255; var ng=dg+ag; if(ng>255)ng=255; var nb=db+ab; if(nb>255)nb=255;
    return (nr<<16)+(ng<<8)+nb;
  }
  function fadeMap(map, w,h, fadePct){
    var scale = 255 - Math.floor(fadePct * 255 / 100);
    for (var y=0;y<h;y++) for (var x=0;x<w;x++) map[y][x] = scaleColor(map[y][x], scale);
  }

  algo.rgbMap = function(width, height, rgb, step){
    if (!util.initialized || util.map===null || util.map.length!==height || util.map[0].length!==width){
      util.map = makeMap(width,height,0);
      util.initialized=true;
    }

    // fade current pixels
    fadeMap(util.map, width, height, algo.fade);

    // sprinkle confetti
    var total = width * height;
    var sparks = Math.max(1, Math.floor(total * algo.density / 1000)); // scale density
    for (var i=0;i<sparks;i++){
      var x = Math.floor(Math.random()*width);
      var y = Math.floor(Math.random()*height);
      var atten = 128 + Math.floor(Math.random()*127); // 128..255
      var c = scaleColor(rgb, atten);
      util.map[y][x] = addColor(util.map[y][x], c);
    }

    var out = makeMap(width,height,0);
    for (var y=0;y<height;y++) for (var x=0;x<width;x++) out[y][x]=util.map[y][x];
    return out;
  };

  algo.rgbMapStepCount = function(width,height){ return 2; };

  testAlgo = algo; return algo;
})();

