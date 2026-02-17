/*
  Q Light Controller Plus
  lightning.js

  Lightning-like strobe: initial bright flash followed by random decreasing flashes.

  Author: Branson Matheson with help from Augment
  License: Apache License, Version 2.0
*/

(function(){
  var algo = {};
  algo.apiVersion = 2;
  algo.name = "Lightning";
  algo.author = "Branson Matheson with help from Augment";
  algo.acceptColors = 1;
  algo.properties = new Array();

  // Configuration
  // Initial bright flash duration (steps)
  algo.initialDuration = 3; // 1..50
  algo.properties.push("name:initialDuration|type:range|display:Initial Duration (steps)|values:1,50|write:setInitialDuration|read:getInitialDuration");

  // Total length of one lightning event (steps). Tail length = totalLength - initialDuration
  algo.totalLength = 40; // 5..300
  algo.properties.push("name:totalLength|type:range|display:Total Length (steps)|values:5,300|write:setTotalLength|read:getTotalLength");

  // Frequency of random flashes during the tail (percent chance per step)
  algo.tailFrequency = 40; // 0..100
  algo.properties.push("name:tailFrequency|type:range|display:Tail Flash Frequency (%)|values:0,100|write:setTailFrequency|read:getTailFrequency");

  // Internal state
  var util = {};
  util.initialized = false;
  util.lastStep = -1;
  util.cycleIndex = 0; // increments each time the algorithm restarts its cycle

  // Property setters/getters
  algo.setInitialDuration = function(v){
    var n = parseInt(v, 10);
    if (!isNaN(n) && n >= 1 && n <= 50) {
      algo.initialDuration = n;
      if (algo.totalLength <= algo.initialDuration) {
        algo.totalLength = algo.initialDuration + 1; // ensure some tail exists
      }
    }
  };
  algo.getInitialDuration = function(){ return algo.initialDuration; };

  algo.setTotalLength = function(v){
    var n = parseInt(v, 10);
    if (!isNaN(n) && n >= 5 && n <= 300) {
      algo.totalLength = n;
      if (algo.totalLength <= algo.initialDuration) {
        algo.totalLength = algo.initialDuration + 1;
      }
    }
  };
  algo.getTotalLength = function(){ return algo.totalLength; };

  algo.setTailFrequency = function(v){
    var n = parseInt(v, 10);
    if (!isNaN(n) && n >= 0 && n <= 100) { algo.tailFrequency = n; }
  };
  algo.getTailFrequency = function(){ return algo.tailFrequency; };

  // Helpers
  function scaleColor(rgb, scale255)
  {
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

  function makeMap(width, height, fill)
  {
    var m = new Array(height);
    for (var y = 0; y < height; y++) {
      m[y] = new Array(width);
      for (var x = 0; x < width; x++) m[y][x] = fill;
    }
    return m;
  }

  // Deterministic pseudo-random helpers based on step/cycle
  function prng(seed)
  {
    // Simple LCG-like mixing; stays deterministic per step/cycle
    var s = seed & 0x7fffffff;
    s = (s * 1103515245 + 12345) & 0x7fffffff;
    return (s % 1000) / 1000.0; // 0..1
  }

  algo.rgbMap = function(width, height, rgb, step)
  {
    if (util.initialized === false){
      util.lastStep = -1;
      util.cycleIndex = 0;
      util.initialized = true;
    }

    // Detect loop wrap to increment cycle index
    if (typeof step === "number"){
      if (util.lastStep !== -1 && step < util.lastStep) util.cycleIndex++;
      util.lastStep = step;
    }

    var total = algo.totalLength;
    if (total < 1) total = 1;
    var s = step % total;

    var idur = algo.initialDuration;
    if (idur < 1) idur = 1;
    if (idur >= total) idur = total - 1; // keep at least 1 tail step

    var tailLen = total - idur;
    if (tailLen < 0) tailLen = 0;

    var map = makeMap(width, height, 0);

    var brightness = 0;

    if (s < idur)
    {
      // Initial bright flash window
      brightness = 255;
    }
    else
    {
      // Tail: random flashes with decreasing brightness envelope
      var tailIndex = s - idur;
      var tailFactor = 1.0;
      if (tailLen > 0) {
        tailFactor = 1.0 - (tailIndex / tailLen);
        if (tailFactor < 0.0) tailFactor = 0.0;
      }

      var baseSeed = (util.cycleIndex * 1315423911) ^ (s * 2654435761);
      var chance = prng(baseSeed);
      var flashProb = algo.tailFrequency / 100.0;

      if (chance < flashProb && tailFactor > 0.0)
      {
        var jitter = 0.8 + 0.4 * prng(baseSeed ^ 0x5bd1e995);
        var scaled = Math.floor(255 * tailFactor * jitter);
        if (scaled < 0) scaled = 0;
        if (scaled > 255) scaled = 255;
        brightness = scaled;
      }
      else
      {
        brightness = 0;
      }
    }

    var color = scaleColor(rgb, brightness);

    // Fill entire matrix uniformly for a strobe-like flash
    for (var y = 0; y < height; y++){
      for (var x = 0; x < width; x++){
        map[y][x] = (brightness > 0) ? color : 0;
      }
    }

    return map;
  };

  algo.rgbMapStepCount = function(_width, _height)
  {
    // One full lightning event length
    return algo.totalLength;
  };

  return algo;
})();

