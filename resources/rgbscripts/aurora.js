/*
  Q Light Controller Plus
  aurora.js

  Slow flowing noise bands with cool palettes (Aurora)
  Ported to QLC+ RGBScript

  Licensed under the Apache License, Version 2.0
*/

// Development tool access
var testAlgo;

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
  util.colors = [];
  util.perm = null;

  algo.setSpeed=function(v){algo.speed=parseInt(v, 10);} ; algo.getSpeed=function(){return algo.speed;};
  algo.setScale=function(v){algo.scale=parseInt(v, 10);} ; algo.getScale=function(){return algo.scale;};
  algo.setContrast=function(v){algo.contrast=parseInt(v, 10);} ; algo.getContrast=function(){return algo.contrast;};

  function defaultPalette(){
    return [4120, 12360, 1073264, 4239264, 10551264]; // 0x001018, 0x003048, 0x106070, 0x40B0A0, 0xA0FFE0
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
  // Fixed permutation table (standard Ken Perlin reference values) so that
  // rgbMap() output only depends on (width, height, step), never on prior
  // calls or Math.random() - required for reproducible color fades.
  function initPerm(){
    var p = [151,160,137,91,90,15,131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,
      8,99,37,240,21,10,23,190,6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,
      57,177,33,88,237,149,56,87,174,20,125,136,171,168,68,175,74,165,71,134,139,48,27,166,
      77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,102,143,54,
      65,25,63,161,1,216,80,73,209,76,132,187,208,89,18,169,200,196,135,130,116,188,159,86,
      164,100,109,198,173,186,3,64,52,217,226,250,124,123,5,202,38,147,118,126,255,82,85,212,
      207,206,59,227,47,16,58,17,182,189,28,42,223,183,170,213,119,248,152,2,44,154,163,70,
      221,153,101,155,167,43,172,9,129,22,39,253,19,98,108,110,79,113,224,232,178,185,112,104,
      218,246,97,228,251,34,242,193,238,210,144,12,191,179,162,241,81,51,145,235,249,14,239,
      107,49,192,214,31,181,199,106,157,184,84,204,176,115,121,50,45,127,4,150,254,138,236,
      205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180];
    util.perm = new Array(512);
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

  algo.rgbMap = function(width,height,_rgb,step){
    void _rgb; // QLC+ API requirement
    var pal = getPalette();
    var t = step*(algo.speed/300.0); // slow drift, driven by step for reproducibility

    var map = makeMap(width,height,0);
    var sc = Math.max(1, algo.scale)/50.0; // low frequency

    // diagonal bands using 2D noise
    for (var y=0;y<height;y++){
      for (var x=0;x<width;x++){
        var nx = (x*sc), ny=(y*sc);
        var v = noise(nx*0.8, ny*0.8 + t*0.7, t*0.2); // -1..1
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

  // Development tool access
  testAlgo = algo;

  return algo;
})();

