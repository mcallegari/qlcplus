/*
  Q Light Controller Plus
  meteor.js

  FastLED-inspired "Meteor Rain" (falling meteors with tails)
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
  algo.name = "Meteor";
  algo.author = "FastLED idea, port by Branson Matheson";
  algo.acceptColors = 1;
  algo.properties = new Array();

  algo.count = 3; // active meteors
  algo.properties.push("name:count|type:range|display:Meteors|values:1,10|write:setCount|read:getCount");
  algo.speed = 20; // 1..50
  algo.properties.push("name:speed|type:range|display:Speed|values:1,50|write:setSpeed|read:getSpeed");
  algo.tail = 8; // 1..30
  algo.properties.push("name:tail|type:range|display:Tail Length|values:1,30|write:setTail|read:getTail");
  algo.fade = 60; // 0..100, map fade each frame
  algo.properties.push("name:fade|type:range|display:Fade (0-100)|values:0,100|write:setFade|read:getFade");

  var util = new Object;
  util.initialized = false;
  util.map = null;
  util.meteors = [];
  util.speedAcc = 0.0;

  algo.setCount = function(v){ algo.count = parseInt(v); };
  algo.getCount = function(){ return algo.count; };
  algo.setSpeed = function(v){ algo.speed = parseInt(v); };
  algo.getSpeed = function(){ return algo.speed; };
  algo.setTail = function(v){ algo.tail = parseInt(v); };
  algo.getTail = function(){ return algo.tail; };
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

  function spawnMeteor(width){
    var m = { x: Math.floor(Math.random()*width), y: -1 };
    return m;
  }

  algo.rgbMap = function(width, height, rgb, step){
    if (!util.initialized || util.map===null || util.map.length!==height || util.map[0].length!==width){
      util.map = makeMap(width,height,0);
      util.meteors=[];
      util.speedAcc=0.0;
      util.initialized=true;
    }

    // fade background
    fadeMap(util.map, width, height, algo.fade);

    // speed accumulator controls step movement
    util.speedAcc += (algo.speed/50.0) * 0.5;
    var advance = 0;
    if (util.speedAcc >= 1.0){ advance = Math.floor(util.speedAcc); util.speedAcc -= advance; }

    // move existing meteors
    for (var a=0; a<advance; a++){
      for (var i=0;i<util.meteors.length;i++){
        util.meteors[i].y += 1;
      }
    }

    // cull meteors offscreen
    var next = [];
    for (var i=0;i<util.meteors.length;i++){
      if (util.meteors[i].y - algo.tail < height) next.push(util.meteors[i]);
    }
    util.meteors = next;

    // spawn new meteors
    while (util.meteors.length < algo.count){
      util.meteors.push(spawnMeteor(width));
    }

    // draw meteors with tails
    for (var i=0;i<util.meteors.length;i++){
      var mx = util.meteors[i].x;
      var my = util.meteors[i].y;
      for (var t=0; t<=algo.tail; t++){
        var yy = my - t;
        if (yy>=0 && yy<height){
          var atten = 255 - Math.floor(255 * (t/(algo.tail+1)));
          var col = scaleColor(rgb, atten);
          util.map[yy][mx] = addColor(util.map[yy][mx], col);
        }
      }
    }

    var out = makeMap(width,height,0);
    for (var y=0;y<height;y++) for (var x=0;x<width;x++) out[y][x]=util.map[y][x];
    return out;
  };

  algo.rgbMapStepCount = function(width,height){ return 2; };

  testAlgo = algo; return algo;
})();

