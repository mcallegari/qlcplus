/*
  Q Light Controller Plus
  juggle.js

  FastLED-inspired "Juggle" (multiple sinelons with phase offsets)
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


(function(){
  var algo = {};
  algo.apiVersion = 2;
  algo.name = "Juggle";
  algo.author = "FastLED idea, port by Branson Matheson";
  algo.acceptColors = 1;
  algo.properties = new Array();

  algo.count = 4; // 1..16 dots
  algo.properties.push("name:count|type:range|display:Dots|values:1,16|write:setCount|read:getCount");
  algo.speed = 20; // 1..50
  algo.properties.push("name:speed|type:range|display:Speed|values:1,50|write:setSpeed|read:getSpeed");
  algo.fade = 70; // 0..100
  algo.properties.push("name:fade|type:range|display:Fade (0-100)|values:0,100|write:setFade|read:getFade");
  algo.tail = 6; // 1..40
  algo.properties.push("name:tail|type:range|display:Tail Length|values:1,40|write:setTail|read:getTail");

  var util = {};
  util.initialized = false;
  util.map = null;
  util.phase = 0.0;

  algo.setCount = function(v){ algo.count = parseInt(v); };
  algo.getCount = function(){ return algo.count; };
  algo.setSpeed = function(v){ algo.speed = parseInt(v); };
  algo.getSpeed = function(){ return algo.speed; };
  algo.setFade = function(v){ algo.fade = parseInt(v); };
  algo.getFade = function(){ return algo.fade; };
  algo.setTail = function(v){ algo.tail = parseInt(v); };
  algo.getTail = function(){ return algo.tail; };

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

  algo.rgbMap = function(width, height, rgb, _step){
    if (!util.initialized || util.map===null || util.map.length!==height || util.map[0].length!==width){
      util.map = makeMap(width,height,0);
      util.phase=0.0;
      util.initialized=true;
    }
    fadeMap(util.map, width, height, algo.fade);

    var inc = (algo.speed/50.0) * 0.4;
    util.phase += inc;

    var midY = Math.floor(height/2);
    for (var i=0;i<algo.count;i++){
      var offs = (i * (2 * Math.PI / Math.max(1, algo.count)));
      var s = Math.sin(util.phase + offs);
      var xPos = Math.floor(((s + 1.0) * 0.5) * (width - 1));
      for (var d=0; d<=algo.tail; d++){
        var atten = 255 - Math.floor(255 * (d / (algo.tail+1)));
        var col = scaleColor(rgb, atten);
        var x1=xPos-d, x2=xPos+d;
        if (x1>=0 && x1<width) util.map[midY][x1] = addColor(util.map[midY][x1], col);
        if (d!==0 && x2>=0 && x2<width) util.map[midY][x2] = addColor(util.map[midY][x2], col);
      }
    }

    var out = makeMap(width,height,0);
    for (var y=0;y<height;y++) for (var x=0;x<width;x++) out[y][x]=util.map[y][x];
    return out;
  };

  algo.rgbMapStepCount = function(_width,_height){ return 2; };

  return algo;
})();

