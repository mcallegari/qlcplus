/*
  Q Light Controller Plus
  flyingobjects.js

  Copyright (c) Hans-Jürgen Tappe
  derived from on balls.js, Copyright (c) Tim Cullingworth

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

(function()
  {
    var algo = new Object;
    algo.apiVersion = 2;
    algo.name = "Flying Objects";
    algo.author = "Hans-Jürgen Tappe";
    algo.acceptColors = 2;
    algo.properties = new Array();

    algo.presetSize = 5;
    algo.properties.push("name:presetSize|type:range|display:Size|values:5,20|write:setSize|read:getSize");
    algo.presetNumber = 3;
    algo.properties.push("name:presetNumber|type:range|display:Number|values:1,50|write:setNumber|read:getNumber");
    algo.presetRandom = 1;
    algo.properties.push("name:presetRandom|type:list|display:Random Colour|values:No,Yes|write:setRandom|read:getRandom");
    algo.presetCollision = 1;
    algo.properties.push("name:presetCollision|type:list|display:Self Collision|values:No,Yes|write:setCollision|read:getCollision");
    algo.selectedAlgo = "trees";
    algo.properties.push("name:selectedAlgo|type:list|display:Objects|"
      + "values:Balls,Bells,Snowflake,Snowman,Tornado,Ufo,Ventilator,Xmas Stars,Xmas Trees|"
      + "write:setSelectedAlgo|read:getSelectedAlgo");

    algo.initialized = false;
    algo.boxRadius = 1;
    algo.realsize = 1;
    algo.progstep = 0;
    algo.totalSteps = 1024;
    // Optimize multiple calculations
    algo.twoPi = 2 * Math.PI;
    algo.halfPi = Math.PI / 2;

    var util = new Object;

    // Algorithms ----------------------------

    let ballsAlgo = new Object;
    ballsAlgo.getMapPixelColor = function(i, rx, ry, r, g, b)
    {
      // calculate the offset difference of algo.map location to the float
      // location of the object
      let offx = rx - algo.obj[i].x;
      let offy = ry - algo.obj[i].y;
      let factor = 1 - (Math.sqrt( (offx * offx) + (offy * offy))/((algo.presetSize/2)+1));
      if (factor < 0) {
        factor = 0;
      }
      return getColor(r * factor, g * factor, b * factor, algo.map[ry][rx]);
    }
      
    let bellsAlgo = new Object;
    bellsAlgo.getMapPixelColor = function(i, rx, ry, r, g, b)
    {
      // calculate the offset difference of algo.map location to the float
      // location of the object
      let offx = rx - algo.obj[i].x;
      let offy = ry - algo.obj[i].y;

      // Calculate color intensity
      if (ry === Math.floor(algo.obj[i].y) + algo.boxRadius + 1) {
        // The bell bottom:
        // offx remains the same.
        // offy is 0
        offy = 0;
      } else {
        // Offset to bottom
        offy += (algo.presetSize / 2) + 0.7;
      }
      let factor = ((1 - (Math.sqrt((offx * offx * 0.7) + (offy * offy * 1.0)))) * 1.0) + offy * 1.0;
      //let factor = ((1 - (Math.cos((offx * offx * 0.7) + (offy * offy * 1.0)))) * 1.0) + offy * 1.0;
    
      // add the object color to the algo.mapped location
      return getColor(r * factor, g * factor, b * factor, algo.map[ry][rx]);
    };

    let snowmanAlgo = new Object;
    snowmanAlgo.getMapPixelColor = function(i, rx, ry, r, g, b)
    {
      // calculate the offset difference of algo.map location to the float
      // location of the object
      let offx = rx - algo.obj[i].x;
 
      let size = algo.presetSize * 2 / 5;
      let offy = ry - algo.obj[i].y + (algo.presetSize - size) / 2;

      let factor1 = 1 - (Math.sqrt((offx * offx) + (offy * offy)) / ((size / 2) + 1));
      // Set a bit towards background by dimming.
      if (factor1 < 0) {
        factor1 = 0;
      }

      size = algo.presetSize * 4 / 5;
      offy = ry - algo.obj[i].y - (algo.presetSize - size) / 2;

      let factor2 = 1 - (Math.sqrt((offx * offx) + (offy * offy)) / ((size / 2) + 1));
      if (factor2 < 0) {
        factor2 = 0;
      }

      // Merge the two balls
      let factor = Math.max(factor1, factor2);
      if (factor > 1) {
        factor = 1;
      }

      return getColor(r * factor, g * factor, b * factor, algo.map[ry][rx]);
    }

    let snowflakeAlgo = new Object;
    snowflakeAlgo.getMapPixelColor = function(i, rx, ry, r, g, b)
    {
      let factor = 1.0;
      // calculate the offset difference of algo.map location to the float
      // location of the object
      let offx = rx - algo.obj[i].x;
      let offy = ry - algo.obj[i].y;
      let distance = Math.sqrt(offx * offx + offy * offy);
      let angle = getAngle(i, rx, ry);
      let baseIntensity = 0;
      //let percent = 0;
      
      angle += (distance / (algo.presetSize / 2)) * algo.halfPi;
      // Repeat the pattern
      angle = 5 * angle;
//      angle -= (algo.twoPi) * (algo.progstep / 8 % 1);
      angle = (angle + algo.twoPi) % (algo.twoPi);

      // Blind in the inner edges
      //percent = Math.max(0, 1 - (angle / (0.3 * algo.twoPi)));
      // apply a scale factor for the percent / input in the asec function
      //factor = (1.0 - baseIntensity) * (Math.acos(1 / (2.5 * percent + 1)) / (algo.halfPi)) + baseIntensity;
      factor = Math.cos(angle);

      // Blind out the outside edges
      percent = Math.max(0, 1 - (distance / (algo.presetSize / 2)));
      // apply a scale factor for the percent / input in the asec function
      factor *= Math.acos(1 / (2.5 * percent + 1)) / (algo.halfPi);
      
      return getColor(r * factor, g * factor, b * factor, algo.map[ry][rx]);
    }

    let tornadoAlgo = new Object;
    tornadoAlgo.getMapPixelColor = function(i, rx, ry, r, g, b)
    {
      let factor = 1.0;
      // calculate the offset difference of algo.map location to the float
      // location of the object
      let offx = rx - algo.obj[i].x;
      let offy = ry - algo.obj[i].y;
      let distance = Math.sqrt(offx * offx + offy * offy);
      let angle = getAngle(i, rx, ry);
      let baseIntensity = 0;
      let percent = 0;
      
      // Repeat the pattern
      let fanblades = 3;
      if (algo.presetSize >= 9) {
    	  fanblades = 5;
      }
      
      angle -= (distance / (algo.presetSize / 2)) * algo.halfPi;
      // Repeat the pattern
      angle = fanblades * angle;
      // Turn along the distance
      angle += (algo.twoPi) * (algo.progstep / 8 % 1);
      // Normalize the angle
      angle = (angle + algo.twoPi) % (algo.twoPi);

      // Blind along the edges
      factor = Math.cos(angle);

      // Blind out the outside edges
      percent = Math.max(0, 1 - (distance / (algo.presetSize / 2)));
      // apply a scale factor for the percent / input in the asec function
      factor *= Math.acos(1 / (2.5 * percent + 1)) / (algo.halfPi);
      
      // Draw a center
      // asec consumes values > 1. asec(x) = acos(1/x)
      percent = Math.max(0, 1 - (distance / (0.2 * algo.presetSize / 2)));
      // apply a scale factor for the percent / input in the asec function
      let factorC = Math.acos(1 / (2.5 * percent + 1)) / (algo.halfPi);

      factor = Math.max(factorC, factor * 1.5);

      return getColor(r * factor, g * factor, b * factor, algo.map[ry][rx]);
    }

    let ufoAlgo = new Object;
    ufoAlgo.getMapPixelColor = function(i, rx, ry, r, g, b)
    {
      let offx = rx - algo.obj[i].x;
 
      let size = algo.presetSize * 2 / 5;
      let offy1 = ry - algo.obj[i].y;

      let factor1 = 1 - (Math.sqrt((offx * offx) + (offy1 * offy1)) / ((size / 2) + 1));
      // Set a bit towards background by dimming.
      if (factor1 < 0) {
        factor1 = 0;
      }
       // factor1 = 0;

      let factor2 = 0;
      let offy2 = ry - algo.obj[i].y - size / 4;
      if (offy2 <= 0) {
        size = algo.presetSize;
        factor2 = 1 - (Math.sqrt((offx * offx) + (offy2 * offy2) * (size / 1.5)) / ((size / 2) + 1));
        if (factor2 < 0) {
          factor2 = 0;
        }
      }

      let factor = factor1 + factor2;
      if (factor > 1) {
        factor = 1;
      }

      return getColor(r * factor, g * factor, b * factor, algo.map[ry][rx]);

    }

    let ventilatorAlgo = new Object;
    ventilatorAlgo.getMapPixelColor = function(i, rx, ry, r, g, b)
    {
      let factor = 1.0;
      // calculate the offset difference of algo.map location to the float
      // location of the object
      let offx = rx - algo.obj[i].x;
      let offy = ry - algo.obj[i].y;
      let distance = Math.sqrt(offx * offx + offy * offy);
      let angle = getAngle(i, rx, ry);
      // Optimize multiple calculations
      let baseIntensity = 0.1;
      let percent = 0;

      // Repeat the pattern
      let fanblades = 3;
      if (algo.presetSize >= 10) {
    	  fanblades = 5;
      }
      angle = fanblades * angle;
      angle -= (algo.twoPi) * (algo.progstep / 8 % 1);
      angle = (angle + algo.twoPi) % (algo.twoPi);

      // Draw a center cover
      // asec consumes values > 1. asec(x) = acos(1/x)
      percent = Math.max(0, 1 - (distance / (0.2 * algo.presetSize / 2)));
      // apply a scale factor for the percent / input in the asec function
      let factorC = Math.acos(1 / (2.5 * percent + 1)) / (algo.halfPi);

      // Calculate intensity by angle
      //factor = (1.0 - baseIntensity) * angle / (algo.twoPi) + baseIntensity;
      factor = Math.max(0, (1.0 + baseIntensity) * angle / (algo.twoPi) - baseIntensity);

      // Blind out the inner edges
      percent = Math.max(0, 1 - (angle / algo.twoPi));
      // apply a scale factor for the percent / input in the asec function
      factor *= (1.0 - baseIntensity) * (Math.acos(1 / (2.5 * percent + 1)) / (algo.algo.halfPi)) + baseIntensity;

      // Blind out the outside edges
      percent = Math.max(0, 1 - (distance / (algo.presetSize / 2)));
      // apply a scale factor for the percent / input in the asec function
      factor *= Math.acos(1 / (2.5 * percent + 1)) / (algo.halfPi);
      
      factor = Math.max(factorC, factor * 1.5);

      return getColor(r * factor, g * factor, b * factor, algo.map[ry][rx]);
    }

    let treesAlgo = new Object;
    treesAlgo.getMapPixelColor = function(i, rx, ry, r, g, b)
    {
      // calculate the offset difference of algo.map location to the float
      // location of the object
      let offx = rx - algo.obj[i].x;
      let offy = ry - algo.obj[i].y;

      // Calculate color intensity
      if (ry === Math.floor(algo.obj[i].y) + algo.boxRadius + 1) {
        // The tree foot:
        // offx remains the same.
        // offy is 0
        offy = 0;
      } else {
        // Offset to bottom
        offy += (algo.presetSize / 2) + 0.7;
      }
      let factor = ((1 - (Math.sqrt((offx * offx * 1.8) + (offy * offy * 1.4)))) * 2.5) + offy * 3;
      factor = factor / 3.27;
 
      // add the object color to the algo.mapped location
      return getColor(r * factor, g * factor, b * factor, algo.map[ry][rx]);
    };
      
    let starsAlgo = new Object;
    starsAlgo.getMapPixelColor = function(i, rx, ry, r, g, b)
    {
      // calculate the offset difference of algo.map location to the float
      // location of the object
      let offx = Math.abs(Math.round(rx - algo.obj[i].x));
      let offy = Math.abs(Math.round(ry - algo.obj[i].y));

      // Calculate color pixel positions
      if ( // The tips
        // Top and bottom tips
        (offy > Math.floor(algo.realsize / 4) && offy <= algo.realsize / 2 &&
          offx <= Math.floor(algo.realsize / 2) - offy) ||
        // Outer tips
        (offy >= 0 && offy <= Math.floor(algo.realsize / 4) &&
          offx <= offy + Math.round(algo.realsize / 4))
      ) {
        // add the color to the algo.mapped location
        return getColor(r, g, b, algo.map[ry][rx]);
      }
        return algo.map[ry][rx];
    }

    // Setters and Getters ------------------------

    algo.setSize = function(_size)
    {
      algo.presetSize = _size;
      algo.initialized = false;
    };

    algo.getSize = function()
    {
      return algo.presetSize;
    };

    algo.setNumber = function(_step)
    {
      algo.presetNumber = _step;
      algo.initialized = false;
    };

    algo.getNumber = function()
    {
      return algo.presetNumber;
    };

    algo.setRandom = function(_random)
    {
      if (_random === "Yes") {
        algo.presetRandom = 0;
      } else if (_random === "No") {
        algo.presetRandom = 1;
      }
      algo.initialized = false;
    };

    algo.getRandom = function()
    {
      if (algo.presetRandom === 0) {
        return "Yes";
      } else if (algo.presetRandom === 1) {
        return "No";
      }
    };

    algo.setCollision = function(_collision)
    {
      if (_collision === "Yes") {
        algo.presetCollision = 0;
      } else if (_collision === "No") {
        algo.presetCollision = 1;
      }
    };

    algo.getCollision = function()
    {
      if (algo.presetCollision === 0) {
        return "Yes";
      } else if (algo.presetCollision === 1) {
        return "No";
      }
    };

    algo.setSelectedAlgo= function(_selected)
    {
      if (_selected === "Xmas Stars") {
        algo.selectedAlgo = "stars";
      } else if (_selected === "Balls") {
        algo.selectedAlgo = "balls";
      } else if (_selected === "Bells") {
        algo.selectedAlgo = "bells";
      } else if (_selected === "Snowflake") {
        algo.selectedAlgo = "snowflake";
      } else if (_selected === "Snowman") {
        algo.selectedAlgo = "snowman";
      } else if (_selected === "Tornado") {
        algo.selectedAlgo = "tornado";
      } else if (_selected === "Ufo") {
        algo.selectedAlgo = "ufo";
      } else if (_selected === "Ventilator") {
        algo.selectedAlgo = "ventilator";
      } else {
        algo.selectedAlgo = "trees";
      }
    };

    algo.getSelectedAlgo = function()
    {
      if (algo.selectedAlgo === "stars") {
        return "Xmas Stars";
      } else if (algo.selectedAlgo === "bells") {
        return "Bells";
      } else if (algo.selectedAlgo === "balls") {
        return "Balls";
      } else if (algo.selectedAlgo === "snowflake") {
        return "Snowflake";
      } else if (algo.selectedAlgo === "snowman") {
        return "Snowman";
      } else if (algo.selectedAlgo === "tornado") {
        return "Tornado";
      } else if (algo.selectedAlgo === "ufo") {
        return "Ufo";
      } else if (algo.selectedAlgo === "ventilator") {
          return "Ventilator";
      } else {
        return "Xmas Trees";
      }
    };

    // General Purpose Functions ------------------

    // calculate the angle from 0 to 2 pi startin north and counting clockwise
    function getAngle(i, rx, ry)
    {
        let offx = rx - algo.obj[i].x;
        let offy = ry - algo.obj[i].y;
        let angle = 0;
        // catch offx == 0
        if (offx === 0) {
          // This where the asymptote goes
      	  if (offy < 0) {
      		angle = -1 * Math.PI / 2;
      	  } else {
      		angle = Math.PI / 2;
      	  }
        } else {
          let gradient = offy / offx;
          angle = Math.atan(gradient);
        }
        angle += Math.PI / 2;
        if (offx < 0) {
          angle += Math.PI;  
        }
        return angle;
    }

    // Combine RGB color from color channels
    function mergeRgb(r, g, b)
    {
      // Stay within boundaries for the final color
      r = Math.max(0, Math.min(255, Math.round(r)));
      g = Math.max(0, Math.min(255, Math.round(g)));
      b = Math.max(0, Math.min(255, Math.round(b)));

      return ((r << 16) + (g << 8) + b);
    }

    function getColor(r, g, b, mRgb)
    {
      // Stay within boundaries for the input values (do not overshoot in calculation)
      r = Math.max(0, Math.min(255, Math.round(r)));
      g = Math.max(0, Math.min(255, Math.round(g)));
      b = Math.max(0, Math.min(255, Math.round(b)));

      // split rgb in to components
      let pointr = (mRgb >> 16) & 0x00FF;
      let pointg = (mRgb >> 8) & 0x00FF;
      let pointb = mRgb & 0x00FF;

      // add the color to the algo.mapped location
      pointr += r;
      pointg += g;
      pointb += b;

      // set algo.mapped point
      return mergeRgb(pointr, pointg, pointb);
    }

    // Key Functions ------------------------------

    util.initialize = function(width, height)
    {
      algo.obj = new Array(algo.presetNumber);

      for (let i = 0; i < algo.presetNumber; i++) {
        algo.obj[i] = {
          // set random start locations for objects
          x: Math.random() * (width - 1),
          y: Math.random() * (height - 1),
        };
          // set random directions
        do {
          do {
            algo.obj[i].xDirection = (Math.random() * 2) - 1;
          } while (Math.abs(algo.obj[i].xDirection) < 0.1);
          do {
            algo.obj[i].yDirection = (Math.random() * 2) - 1;
          } while (Math.abs(algo.obj[i].yDirection) < 0.1);
        } while (Math.abs(algo.obj[i].xDirection + algo.obj[i].yDirection) < 0.3);
        do {
          // Chose random colour for each object
          algo.obj[i].r = Math.round(Math.random() * 255);
          algo.obj[i].g = Math.round(Math.random() * 255);
          algo.obj[i].b = Math.round(Math.random() * 255);
          // try again if it is too dim
        } while ((algo.obj[i].r + algo.obj[i].g + algo.obj[i].b) < 125);
      }

      // area size to draw object
      algo.boxRadius = Math.round(algo.presetSize / 2);
      algo.realsize = Math.floor(algo.presetSize / 2) * 2 + 1;

      algo.initialized = true;
      return;
    };

    algo.rgbMap = function(width, height, rgb, progstep)
    {
      if (algo.initialized === false) {
        util.initialize(width, height);
      }
      algo.progstep = progstep;

      // Clear algo.map data
      algo.map = new Array(height);
      for (let y = 0; y < height; y++) {
        algo.map[y] = new Array();
        for (let x = 0; x < width; x++) {
          algo.map[y][x] = 0;
        }
      }

      // for each object displayed
      for (let i = 0; i < algo.presetNumber; i++) {
        // workout closest map location for object
        let mx = Math.floor(algo.obj[i].x);
        let my = Math.floor(algo.obj[i].y);

        // split colour
        let r = algo.obj[i].r;
        let g = algo.obj[i].g;
        let b = algo.obj[i].b;
        if (algo.presetRandom != 0) {
          r = (rgb >> 16) & 0x00FF;
          g = (rgb >> 8) & 0x00FF;
          b = rgb & 0x00FF;
        }

        for (let ry = my - algo.boxRadius; ry < my + algo.boxRadius + 2; ry++) {
          for (let rx = mx - algo.boxRadius; rx < mx + algo.boxRadius + 2; rx++) {
            // Draw only if edges are on the map
            if (rx < width && rx > -1 && ry < height && ry > -1) {
              // DEVELOPMENT: Draw a box for debugging.
              //algo.map[ry][rx] = getColor(0, 0, 40, algo.map[ry][rx]);

              // add the object color to the mapped location
              if (algo.selectedAlgo === "stars") {
                algo.map[ry][rx] = starsAlgo.getMapPixelColor(i, rx, ry, r, g, b);
              } else if (algo.selectedAlgo === "balls") {
                algo.map[ry][rx] = ballsAlgo.getMapPixelColor(i, rx, ry, r, g, b);
              } else if (algo.selectedAlgo === "bells") {
                algo.map[ry][rx] = bellsAlgo.getMapPixelColor(i, rx, ry, r, g, b);
              } else if (algo.selectedAlgo === "snowflake") {
                algo.map[ry][rx] = snowflakeAlgo.getMapPixelColor(i, rx, ry, r, g, b);
              } else if (algo.selectedAlgo === "snowman") {
                algo.map[ry][rx] = snowmanAlgo.getMapPixelColor(i, rx, ry, r, g, b);
              } else if (algo.selectedAlgo === "tornado") {
                algo.map[ry][rx] = tornadoAlgo.getMapPixelColor(i, rx, ry, r, g, b);
              } else if (algo.selectedAlgo === "ufo") {
                algo.map[ry][rx] = ufoAlgo.getMapPixelColor(i, rx, ry, r, g, b);
              } else if (algo.selectedAlgo === "ventilator") {
                algo.map[ry][rx] = ventilatorAlgo.getMapPixelColor(i, rx, ry, r, g, b);
              } else {
                algo.map[ry][rx] = treesAlgo.getMapPixelColor(i, rx, ry, r, g, b);
              }
            }
          }
        }

        // if collision detection is on
        if (algo.presetCollision === 0) {
          // object collision detection
          // check all objects
          for (let ti = 0; ti < algo.presetNumber; ti++) {
            // but not the current one
            if (ti !== i) {
              // calculate distance to current object
              let disx = (algo.obj[i].x + algo.obj[i].xDirection) - algo.obj[ti].x;
              let disy = (algo.obj[i].y + algo.obj[i].yDirection) - algo.obj[ti].y;
              let dish = Math.sqrt((disx * disx) + (disy * disy));
              // if to close
              if (dish < (1.414) * (algo.presetSize / 2)) {
                // swap speed / direction of current object
                let stepx = algo.obj[i].xDirection;
                let stepy = algo.obj[i].yDirection;
                algo.obj[i].xDirection = algo.obj[ti].xDirection;
                algo.obj[i].yDirection = algo.obj[ti].yDirection;
                algo.obj[ti].xDirection = stepx;
                algo.obj[ti].yDirection = stepy;
              }
            }
          }
        }

        // edge collision detection
        if (algo.obj[i].y <= 0 && algo.obj[i].yDirection < 0) {
          // top edge and moving up
          algo.obj[i].yDirection *= -1;
        } else if (algo.obj[i].y >= height - 1 && algo.obj[i].yDirection > 0) {
          // bottom edge and moving down
          algo.obj[i].yDirection *= -1;
        }

        if (algo.obj[i].x <= 0 && algo.obj[i].xDirection < 0) {
          // left edge and moving left
          algo.obj[i].xDirection *= -1;
        } else if (algo.obj[i].x >= width - 1 && algo.obj[i].xDirection > 0) {
          // right edge and moving right
          algo.obj[i].xDirection *= -1;
        }

        // set object's next location
        algo.obj[i].x += algo.obj[i].xDirection;
        algo.obj[i].y += algo.obj[i].yDirection;
      }

      return algo.map;
    };

    algo.rgbMapStepCount = function(width, height)
    {
      // This make no difference to the script, except for fading the colors
      return algo.totalSteps;
    };

    // Development tool access
    testAlgo = algo;

    return algo;
  }
)();
