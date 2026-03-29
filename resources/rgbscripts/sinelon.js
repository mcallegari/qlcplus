/*
  Q Light Controller Plus
  sinelon.js

  FastLED-inspired "Sinelon" (single moving dot with fading trail)
  Ported to QLC+ RGBScript by Branson/Agent

  Licensed under the Apache License, Version 2.0
*/


(function(){
  var algo = {};
  algo.apiVersion = 2;
  algo.name = "Sinelon";
  algo.author = "Branson Matheson with help from Augment";
  algo.acceptColors = 1;
  algo.properties = new Array();

  // Properties
  algo.speed = 20; // 1..50
  algo.properties.push("name:speed|type:range|display:Speed|values:1,50|write:setSpeed|read:getSpeed");
  algo.fade = 60; // 0..100 (higher = faster fade)
  algo.properties.push("name:fade|type:range|display:Fade (0-100)|values:0,100|write:setFade|read:getFade");
  algo.tail = 8; // 1..50
  algo.properties.push("name:tail|type:range|display:Tail Length|values:1,50|write:setTail|read:getTail");

  var util = {};
  util.initialized = false;
  util.map = null;
  util.phase = 0.0;

  algo.setSpeed = function(v){ algo.speed = parseInt(v, 10); };
  algo.getSpeed = function(){ return algo.speed; };
  algo.setFade = function(v){ algo.fade = parseInt(v, 10); };
  algo.getFade = function(){ return algo.fade; };
  algo.setTail = function(v){ algo.tail = parseInt(v, 10); };
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
    var r = (rgb >> 16) & 0xFF;
    var g = (rgb >> 8) & 0xFF;
    var b = rgb & 0xFF;
    r = Math.floor(r * scale255 / 255);
    g = Math.floor(g * scale255 / 255);
    b = Math.floor(b * scale255 / 255);
    return (r << 16) + (g << 8) + b;
  }

  function addColor(dst, add){
    var dr = (dst >> 16) & 0xFF; var dg = (dst >> 8) & 0xFF; var db = dst & 0xFF;
    var ar = (add >> 16) & 0xFF; var ag = (add >> 8) & 0xFF; var ab = add & 0xFF;
    var nr = dr + ar; if (nr>255) nr=255;
    var ng = dg + ag; if (ng>255) ng=255;
    var nb = db + ab; if (nb>255) nb=255;
    return (nr<<16) + (ng<<8) + nb;
  }

  function fadeMap(map, width, height, fadePct){
    // fadePct: 0..100, higher means more fade
    var scale = 255 - Math.floor(fadePct * 255 / 100);
    for (var y=0;y<height;y++){
      for (var x=0;x<width;x++){
        map[y][x] = scaleColor(map[y][x], scale);
      }
    }
  }

  algo.rgbMap = function(width, height, _rgb, _step){
    void _rgb; void _step; // QLC+ API requirement
    if (!util.initialized || util.map === null){
      util.map = makeMap(width, height, 0);
      util.phase = 0.0;
      util.initialized = true;
    }

    // Fade existing trail
    fadeMap(util.map, width, height, algo.fade);

    // Advance phase by speed (scale to reasonable motion)
    var inc = (algo.speed / 50.0) * 0.35; // tune factor
    util.phase += inc;
    // Wrap phase
    if (util.phase > 1000000.0) util.phase = 0.0;

    // Compute horizontal position 0..width-1
    var s = Math.sin(util.phase);
    var xPos = Math.floor(((s + 1.0) * 0.5) * (width - 1));
    var yPos = Math.floor(height / 2);

    // Draw head and tail
    for (var d=0; d<=algo.tail; d++){
      var atten = 255 - Math.floor(255 * (d / (algo.tail+1)));
      var col = scaleColor(_rgb, atten);
      var x1 = xPos - d; var x2 = xPos + d;
      if (x1>=0 && x1<width) util.map[yPos][x1] = addColor(util.map[yPos][x1], col);
      if (d!==0 && x2>=0 && x2<width) util.map[yPos][x2] = addColor(util.map[yPos][x2], col);
    }

    // Return a copy to avoid external mutation
    var out = makeMap(width, height, 0);
    for (var y=0;y<height;y++) for (var x=0;x<width;x++) out[y][x]=util.map[y][x];
    return out;
  };

  algo.rgbMapStepCount = function(_width, _height){ void _width; void _height; return 2; };

  return algo;
})();

