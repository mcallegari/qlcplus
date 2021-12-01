/*
  Q Light Controller Plus
  empty.js

  Copyright (c) Branson Matheson
  Based on work https://slicker.me/javascript/fire/fire.htm

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

(
  function()
  {
    var util = new Object;
    util.initialized = false;

    var algo = new Object;
    algo.apiVersion = 2;
    algo.name = "Flames";
    algo.author = "Branson Matheson";
    algo.acceptColors = 1;

    algo.properties = new Array();
    algo.fireAmount = 40;
    algo.properties.push("name:fireAmount|type:range|display:Amount|values:1,200|write:setAmount|read:getAmount");
    algo.getAmount = function() {
      return algo.fireAmount;
    };  
    algo.setAmount = function(_size) {
	    // Only set if the input is valid.
      if (!(parseInt(_size) === NaN) && parseInt(_size) > 0) {
        algo.fireAmount = parseInt(_size);
        util.initialized = false;
      }
    };

    algo.fireDepth = 40;
    algo.properties.push("name:fireDepth|type:range|display:Depth|values:1,100|write:setDepth|read:getDepth");
    algo.getDepth = function() {
      return algo.fireDepth;
    };  
    algo.setDepth = function(_size) {
	    // Only set if the input is valid.
      if (!(parseInt(_size) === NaN) && parseInt(_size) > 0) {
        algo.fireDepth = parseInt(_size);
        util.initialized = false;
      }
    };

    function limit(n, m) {
      if ( n <= 0 ) { return 0 }
      if ( n > m ) { return m }
      return n
    }

    util.initialize = function(width, height) {
      // generate intensity map. 
      fire = new Array(height+1);
      for (y=0; y<height+1; y++) {
        fire[y] = new Array()
        for ( x=0; x<width; x++) {
          if ( y === height ) {
            fire[y][x]=Math.floor(Math.random() * 0xFF); // randomize the bottom row 
          } else {
            fire[y][x] = 0;
          }
        }
      }
      util.initialized = true;
    }

    util.getColor = function(a, rgb) {
      // get multiplier for intensity 
      var i = a/255 || 0
      var r = (rgb >> 16) & 0x00FF;
      var g = (rgb >> 8) & 0x00FF;
      var b = rgb & 0x00FF;
      var nr = limit((r * i), 0xFF);
      var ng = limit((g * i), 0xFF);
      var nb = limit((b * i), 0xFF);
      // add hilights
      if ( nr > 0x80 ) {
        ng=ng+Math.floor((0xA0*Math.random())); // yellow
      } else if ( ng > 0x80 ) {
        nb=nb+Math.floor((0x80*Math.random())); // cyan
        // nr=nr+Math.floor((0xA0*Math.random())); // yellow
      } else if ( nb > 0x80 ) {
        nr=nr+Math.floor((0x80*Math.random())); // purple
      }
      //var new_color = 
      // console.log('i:' + i + ' rgb:' + rgb + ' r:' + r + ' g:' + g + ' b:' + b + ' newcolor: ' + new_color);
      return ((nr << 16) + (ng << 8) + nb);
    }

    util.getNextStep = function(width, height, rgb) {
      // gen rbgMap
      var rgbMap = new Array(width);
      for (y = 0; y <= height; y++) {
        rgbMap[y] = new Array();
        for (x = 0; x <= width; x++) {
          rgbMap[y][x] = 0;
        }
      }      

      //shift up and randomize bottom, starting from bottom
      for(y=height; y>-1; y--) {     
        if ( y === height ) {
          for(x=0; x<width; x++) {    
            var count = algo.fireAmount;
            if ( fire[y][x] > 0x80 ) {count--;}
          }
          for(i=0; i<count; i++) {
            var x = Math.floor(Math.random()*width);
            fire[y][x] = (fire[y][x] + Math.floor(0x80+(Math.random() * 0x5F))) % 0xFF;
          }
        } else {
          for(x=0; x<width; x++) {    
            //console.log('pos:'+y+','+x)
            //fire[y][x] = Math.floor((Math.random() * 0xFF));
            if ( fire[y][x] > 0x80 ) { 
              fire[y][(x+1)%width]=fire[y][x]/2;
              fire[y][(x-1)%width]=fire[y][x]/2;
            }
            // console.log('y: ' + y + ' fire['+y+']['+x+']=' +fire[y][x] )
            fire[y][x]=Math.floor((                   // add the cell values:
            fire[(y+1)%(height+1)][(x-1)%width]*.9+   // below, left
            fire[(y+1)%(height+1)][x]+                // below
            fire[(y+1)%(height+1)][(x+1)%width]*.9+   // below, right
            fire[(y+2)%(height+1)][(x-1)%width]*.8+   // two rows below, left
            fire[(y+2)%(height+1)][x]*.8+             // two rows below
            fire[(y+2)%(height+1)][(x+1)%width]*.8+   // two rows below, right
            fire[(y+2)%(height+1)][(x+2)%width]*.7+   // two rows below, two to the left
            fire[(y+2)%(height+1)][(x-2)%width]*.7    // two rows below, two to the right
            )/8+(algo.fireDepth/10)) || fire[y][x];   // division to lower the value as the fire goes up
            rgbMap[y][x] = util.getColor(fire[y][x], rgb); 
          }
        }
      }
      return rgbMap;
    }

    algo.rgbMap = function(width, height, rgb, step) {
      if (util.initialized === false) { 
        util.initialize(width, height); 
      }
      var rgbMap = util.getNextStep(width, height, rgb);
      return rgbMap;
    };

    algo.rgbMapStepCount = function(width, height) {
      return width;
    };

    // Development tool access
    testAlgo = algo;

    return algo;
  }
)();
