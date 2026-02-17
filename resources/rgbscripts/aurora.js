/*
  Q Light Controller Plus
  aurora.js

  Slow flowing noise bands with cool palettes (Aurora)
  Ported to QLC+ RGBScript

  Licensed under the Apache License, Version 2.0
*/


(function(){
  var algo = {};
  algo.apiVersion = 3;
  algo.name = "Aurora";
  algo.author = "Branson Matheson with help from Augment";
  algo.acceptColors = 5; // palette
  algo.properties = new Array();

  algo.speed = 15; // 1..50
  algo.properties.push("name:speed|type:range|display:Speed|values:1,50|write:setSpeed|read:getSpeed");
  algo.scale = 20; // 4..60 (bigger = larger bands)
  algo.properties.push("name:scale|type:range|display:Scale|values:4,60|write:setScale|read:getScale");
  algo.contrast = 80; // 0..100
  algo.properties.push("name:contrast|type:range|display:Contrast|values:0,100|write:setContrast|read:getContrast");

  var util = {};
  util.initialized = false;
  util.colors = [];
  util.t = 0;
  util.perm = null;

  algo.setSpeed=function(v){algo.speed=parseInt(v);} ; algo.getSpeed=function(){return algo.speed;};
  algo.setScale=function(v){algo.scale=parseInt(v);} ; algo.getScale=function(){return algo.scale;};
  algo.setContrast=function(v){algo.contrast=parseInt(v);} ; algo.getContrast=function(){return algo.contrast;};

  function defaultPalette(){
    return [0x001018,0x003048,0x106070,0x40B0A0,0xA0FFE0];
  }

  algo.rgbMapSetColors = function(rawColors){
    util.colors = [];
    if (Array.isArray(rawColors)){
      for (var i=0;i<algo.acceptColors;i++) if (i<rawColors.length) util.colors.push(rawColors[i]);
    }
    if (util.colors.length===0) util.colors = defaultPalette();
  };

  function getPalette(){ return (util.colors && util.colors.length)? util.colors : defaultPalette(); }

  function lerpColor(a,b,t){ var ar=(a>>16)&255,ag=(a>>8)&255,ab=a&255; var br=(b>>16)&255,bg=(b>>8)&255,bb=b&255; var r=Math.floor(ar+(br-ar)*t), g=Math.floor(ag+(bg-ag)*t), c=Math.floor(ab+(bb-ab)*t); return (r<<16)+(g<<8)+c; }
  function samplePalette(pal,idx){ if(pal.length===1) return pal[0]; var x=idx*(pal.length-1); var i=Math.floor(x), j=Math.min(pal.length-1,i+1); return lerpColor(pal[i], pal[j], x-i); }
  function scaleColor(rgb,s){ if(s<=0) return 0; if(s>=255) return rgb; var r=(rgb>>16)&255,g=(rgb>>8)&255,b=rgb&255; r=Math.floor(r*s/255); g=Math.floor(g*s/255); b=Math.floor(b*s/255); return (r<<16)+(g<<8)+b; }

  // Simple Perlin noise (borrowed from plasma.js, trimmed)
  function initPerm(){
    util.perm = new Array(512);
    var p = new Array(256);
    for (var i=0;i<256;i++) p[i]=i;
    for (var i2=255;i2>0;i2--){ var j=Math.floor(Math.random()*(i2+1)); var tmp=p[i2]; p[i2]=p[j]; p[j]=tmp; }
    for (var k=0;k<512;k++) util.perm[k]=p[k&255];
  }
  function fade(t){ return t*t*t*(t*(t*6-15)+10); }
  function lerp(a,b,t){ return a + t*(b-a); }
  function grad(hash,x,y,z){ var h=hash&15; var u=h<8?x:y; var v=h<4?y:h===12||h===14?x:z; return ((h&1)?-u:u)+((h&2)?-v:v); }
  function noise(x,y,z){
    if (!util.perm) initPerm();
    var X = Math.floor(x)&255, Y=Math.floor(y)&255, Z=Math.floor(z)&255;
    x -= Math.floor(x); y-=Math.floor(y); z-=Math.floor(z);
    var u=fade(x), v=fade(y), w=fade(z);
    var A=util.perm[X]+Y, AA=util.perm[A]+Z, AB=util.perm[A+1]+Z;
    var B=util.perm[X+1]+Y, BA=util.perm[B]+Z, BB=util.perm[B+1]+Z;
    return lerp(
      lerp( lerp(grad(util.perm[AA],x,y,z), grad(util.perm[BA],x-1,y,z), u),
            lerp(grad(util.perm[AB],x,y-1,z), grad(util.perm[BB],x-1,y-1,z), u), v),
      lerp( lerp(grad(util.perm[AA+1],x,y,z-1), grad(util.perm[BA+1],x-1,y,z-1), u),
            lerp(grad(util.perm[AB+1],x,y-1,z-1), grad(util.perm[BB+1],x-1,y-1,z-1), u), v), w);
  }

  function makeMap(w,h,fill){ var m=new Array(h); for (var y=0;y<h;y++){ m[y]=new Array(w); for (var x=0;x<w;x++) m[y][x]=fill; } return m; }

  algo.rgbMap = function(width,height,_rgb,_step){
    void _rgb; void _step; // QLC+ API requirement
    if (!util.initialized){ util.t=0; util.initialized=true; }
    var pal = getPalette();
    util.t += algo.speed/300.0; // slow drift

    var map = makeMap(width,height,0);
    var sc = Math.max(1, algo.scale)/50.0; // low frequency

    // diagonal bands using 2D noise
    for (var y=0;y<height;y++){
      for (var x=0;x<width;x++){
        var nx = (x*sc), ny=(y*sc);
        var v = noise(nx*0.8, ny*0.8 + util.t*0.7, util.t*0.2); // -1..1
        v = (v+1)/2; // 0..1
        // increase contrast
        var c = algo.contrast/100.0; var mid=0.5; v = mid + (v-mid)*(1+2*c);
        if (v<0) v=0; if (v>1) v=1;
        var col = samplePalette(pal, v);
        // slight vertical fade top/bottom
        var ed = 1 - 0.25*Math.abs((y - height/2)/(height/2));
        map[y][x] = scaleColor(col, Math.floor(255*ed));
      }
    }
    return map;
  };

  algo.rgbMapStepCount = function(_width,_height){ void _width; void _height; return 64; };

  return algo;
})();

