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

    // Algorithms ----------------------------

    let ballAlgo = new Object;
    ballAlgo.getMapPixelColor = function(i, rx, ry, r, g, b)
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

    let bellAlgo = new Object;
    bellAlgo.getMapPixelColor = function(i, rx, ry, r, g, b)
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
        offy += (algo.presetRadius) + 0.7;
      }
      let factor = ((1 - (Math.sqrt((offx * offx * 0.7) + (offy * offy * 1.0)))) * 1.0) + offy * 1.0;
      //let factor = ((1 - (Math.cos((offx * offx * 0.7) + (offy * offy * 1.0)))) * 1.0) + offy * 1.0;
    
      // add the object color to the algo.mapped location
      return getColor(r * factor, g * factor, b * factor, algo.map[ry][rx]);
    };

    let diamondAlgo = new Object;
    diamondAlgo.getMapPixelColor = function(i, rx, ry, r, g, b)
    {
      // calculate the offset difference of algo.map location to the float
      // location of the object
      let offx = rx - algo.obj[i].x;
      let offy = ry - algo.obj[i].y;
      let percentX = offx / algo.presetRadius;
      let percentY = offy / algo.presetRadius;
      let saturation = 1;

      let factor = Math.sqrt(saturation * percentX * percentX)
      		+ Math.sqrt(saturation * percentY * percentY);
      factor = 1 - Math.max(0, Math.min(1, factor));

      return getColor(r * factor, g * factor, b * factor, algo.map[ry][rx]);
    }

    let eyeAlgo = new Object;
    eyeAlgo.getMapPixelColor = function(i, rx, ry, r, g, b)
    {
      let tips = 0;
      // calculate the offset difference of algo.map location to the float
      // location of the object
      let offx = Math.abs(rx - algo.obj[i].x);
      let offy = Math.abs(ry - algo.obj[i].y);
      let distance = Math.sqrt(offx * offx + offy * offy);
      let angle = getAngle(i, rx, ry);
      let targetDistance = algo.presetRadius;
      let anglePercent = Math.abs(Math.cos(angle));
      let contraTip = 0.5 * targetDistance * anglePercent;
      let distPercent = distance / (targetDistance - contraTip);
      let factor = blindoutPercent(1 - distPercent, 3);
      
      targetDistance = algo.presetRadius / 3;
      distPercent = distance / (targetDistance);
      factor -= blindoutPercent(1 - distPercent, 1);

      targetDistance = algo.presetRadius / 8;
      distPercent = distance / (targetDistance);
      factor += blindoutPercent(1 - distPercent, 1);

      return getColor(r * factor, g * factor, b * factor, algo.map[ry][rx]);
    }

    let flowerAlgo = new Object;
    flowerAlgo.getMapPixelColor = function(i, rx, ry, r, g, b)
    {
      // calculate the offset difference of algo.map location to the float
      // location of the object
      let offx = Math.abs(Math.round(rx - algo.obj[i].x));
      let offy = Math.abs(Math.round(ry - algo.obj[i].y));
      let distance = Math.sqrt(offx * offx + offy * offy);
      let angle = getAngle(i, rx, ry)
      let baseIntensity = 0.99;
      let leafs = 5;
      
      if (algo.presetSize < 10) {
    	  leafs = 4;
      }

      // Turn each object by a random angle
      angle += algo.obj[i].random * algo.twoPi;
      // Repeat and normalize the pattern
      angle = leafs * angle;
      angle = (angle + algo.twoPi) % (algo.twoPi);

      let scaling = Math.abs(angle - Math.PI) / algo.twoPi * 1.1;
      let percent = Math.min(1, distance / algo.presetRadius + scaling);
      let factor = Math.acos(1 / ((1 - percent) * 5 + 1)) / (algo.halfPi);
      
      // Draw a center
      // asec consumes values > 1. asec(x) = acos(1/x)
      percent = Math.max(0, 1 - (distance / (0.3 * algo.presetRadius)));
      // apply a scale factor for the percent / input in the asec function
      let factorC = algo.halfPi * Math.acos(1 / (2.5 * percent + 1)) / (algo.halfPi);
      factor = Math.max(factorC, factor);

      return getColor(r * factor, g * factor, b * factor, algo.map[ry][rx]);
    }

    let heartAlgo = new Object;
    heartAlgo.getMapPixelColor = function(i, rx, ry, r, g, b)
    {
      // calculate the offset difference of algo.map location to the float
      // location of the object
      let offx = Math.abs(rx + (algo.presetRadius * 2 / 5) - algo.obj[i].x);
      let offy = Math.abs(ry + (algo.presetRadius / 2) - algo.obj[i].y);
      let distance = Math.sqrt(offx * offx + offy * offy);
      let targetDistance = algo.presetRadius * 3 / 7;
      let angle = getAngle(i, rx + (algo.presetRadius / 2), ry + (algo.presetRadius / 2));
      let distPercent = distance / (targetDistance);
      let factor = Math.max(0, blindoutPercent(1 - distPercent, 3));

      offx = Math.abs(rx - (algo.presetRadius * 2 / 5) - algo.obj[i].x);
      offy = Math.abs(ry + (algo.presetRadius / 2) - algo.obj[i].y);
      distance = Math.sqrt(offx * offx + offy * offy);
      targetDistance = algo.presetRadius * 3 / 7;
      angle = getAngle(i, rx - (algo.presetRadius / 2), ry + (algo.presetRadius / 2));
      distPercent = distance / (targetDistance);
      factor = Math.max(factor, blindoutPercent(1 - distPercent, 3));

      offx = Math.abs(rx - algo.obj[i].x);
      offy = Math.abs(ry - algo.obj[i].y);
      let tips = 3; 
      distance = Math.sqrt(offx * offx + offy * offy);
      angle = getAngle(i, rx, ry);
      targetDistance = geometryCalc.getTargetDistance(angle, tips);
      distPercent = distance / targetDistance ;
      factor = Math.max(factor, blindoutPercent(1 - distPercent, 1.5));

      return getColor(r * factor, g * factor, b * factor, algo.map[ry][rx]);
    }

    let hexagonAlgo = new Object;
    hexagonAlgo.getMapPixelColor = function(i, rx, ry, r, g, b)
    {
      let tips = 6;
      // calculate the offset difference of algo.map location to the float
      // location of the object
      let offx = Math.abs(rx - algo.obj[i].x);
      let offy = Math.abs(ry - algo.obj[i].y);
      let distance = Math.sqrt(offx * offx + offy * offy);
      let angle = getAngle(i, rx, ry);
      let targetDistance = geometryCalc.getTargetDistance(angle, tips);
      let distPercent = distance / targetDistance ;
      let factor = blindoutPercent(1 - distPercent, 1.5);

      return getColor(r * factor, g * factor, b * factor, algo.map[ry][rx]);
    }

    let maskAlgo = new Object;
    maskAlgo.getMapPixelColor = function(i, rx, ry, r, g, b)
    {
      // calculate the offset difference of algo.map location to the float
      // location of the object
      let offx = Math.abs(rx - algo.obj[i].x);
      let offy = Math.abs(ry - algo.obj[i].y);
      let distance = Math.sqrt(offx * offx + offy * offy);
      let angle = getAngle(i, rx, ry);
      let targetDistance = algo.presetRadius;
      let anglePercent = Math.abs(Math.cos(angle - Math.PI));
      let contraTip = 0.8 * targetDistance * anglePercent;
      let distPercent = distance / (targetDistance - contraTip);
      let factor = blindoutPercent(1 - distPercent, 3);

      offx = Math.abs(rx - (algo.presetRadius * 2 / 5) - algo.obj[i].x);
      distance = Math.sqrt(offx * offx + offy * offy);
      targetDistance = algo.presetRadius / 5;
      angle = getAngle(i, rx - (algo.presetRadius / 2), ry + (algo.presetRadius / 2));
      distPercent = distance / (targetDistance);
      factor -= blindoutPercent(1 - distPercent, 3);

      offx = Math.abs(rx + (algo.presetRadius * 2 / 5) - algo.obj[i].x);
      distance = Math.sqrt(offx * offx + offy * offy);
      targetDistance = algo.presetRadius / 5;
      angle = getAngle(i, rx + (algo.presetRadius / 2), ry + (algo.presetRadius / 2));
      distPercent = distance / (targetDistance);
      factor -= blindoutPercent(1 - distPercent, 3);

      return getColor(r * factor, g * factor, b * factor, algo.map[ry][rx]);
    }

    let pentagonAlgo = new Object;
    pentagonAlgo.getMapPixelColor = function(i, rx, ry, r, g, b)
    {
      let tips = 5;
      // calculate the offset difference of algo.map location to the float
      // location of the object
      let offx = Math.abs(rx - algo.obj[i].x);
      let offy = Math.abs(ry - algo.obj[i].y);
      let distance = Math.sqrt(offx * offx + offy * offy);
      let angle = (getAngle(i, rx, ry) + Math.PI) % algo.twoPi;
      let targetDistance = geometryCalc.getTargetDistance(angle, tips);
      let distPercent = distance / targetDistance ;
      let factor = blindoutPercent(1 - distPercent, 1.5);

      return getColor(r * factor, g * factor, b * factor, algo.map[ry][rx]);
    }

    let ringAlgo = new Object;
    ringAlgo.getMapPixelColor = function(i, rx, ry, r, g, b)
    {
      // calculate the offset difference of algo.map location to the float
      // location of the object
      let offx = rx - algo.obj[i].x;
      let offy = ry - algo.obj[i].y;
      let distance = Math.sqrt(offx * offx + offy * offy);
	  let percent = Math.max(0, 1 - (distance / (algo.presetRadius)));
	  let factor = algo.halfPi * Math.acos(1 / (2.5 * percent + 1)) / (algo.halfPi);
	  let inner = 0.8;
      if (algo.presetSize < 7) {
    	  inner = 0.2;
      }
	  percent = Math.max(0, 1 - (distance / (0.7 * algo.presetRadius)));
	  factor -= algo.halfPi * Math.acos(1 / (2.5 * percent + 1)) / (algo.halfPi);

	  return getColor(r * factor, g * factor, b * factor, algo.map[ry][rx]);
    }

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
      let lines = 3;
      // calculate the offset difference of algo.map location to the float
      // location of the object
      let offx = Math.abs(rx - algo.obj[i].x);
      let offy = Math.abs(ry - algo.obj[i].y);
      let distance = Math.sqrt(offx * offx + offy * offy);
      let angle = getAngle(i, rx, ry);
      let targetDistance = algo.presetRadius;
      let anglePercent = Math.abs(Math.cos(angle));
      let contraTip = 0.5 * targetDistance * anglePercent;
      let distPercent = distance / (targetDistance - contraTip);
      let factor = 0;
      let c = 0;
      let aWidth = 0;
      let cWidth = 0;

      let intersect = Math.max(4.5, 0.6 * algo.presetRadius);
      lines = 3;
      if (distance > intersect) {
        let xOffset = Math.sin(Math.PI / 3) * intersect;
        let yOffset = Math.cos(Math.PI / 3) * intersect;
        let smallTips = [
          {"x": 0, "y": intersect},
          {"x": 0, "y": -intersect},
          {"x": xOffset, "y": yOffset},
          {"x": xOffset, "y": -yOffset},
          {"x": -xOffset, "y": yOffset},
          {"x": -xOffset, "y": -yOffset},
        ];
        smallTips.forEach(offset => {
          let realx = rx + offset.x;
          let realy = ry + offset.y;
          let realOffx = Math.abs(realx - algo.obj[i].x);
          let realOffy = Math.abs(realy - algo.obj[i].y);
          let realDistance = Math.sqrt(realOffx * realOffx + realOffy * realOffy);
          let realAngle = getAngle(i, realx, realy);
          realAngle *= lines;
          a = Math.sin(realAngle) * realDistance;
          c = Math.abs(Math.cos(realAngle) * realDistance);
          aWidth = 0.9 + 0.02 * algo.presetRadius;
          cWidth = 0.5 * algo.presetRadius;
          a = a / aWidth / lines;
          c = c / cWidth;
          factor = Math.max(factor, 1 - (a * a) + 1 - (c * c) - 1);
        });
      }

      lines = 3;
      offx = Math.abs(rx - algo.obj[i].x);
      offy = Math.abs(ry - algo.obj[i].y);
      distance = Math.sqrt(offx * offx + offy * offy);
      angle = getAngle(i, rx, ry);
      angle *= lines;
      a = Math.abs(Math.sin(angle) * distance);
      c = Math.abs(Math.cos(angle) * distance);
      aWidth = 0.9 + 0.02 * algo.presetRadius;
      cWidth = 1.5 * algo.presetRadius;
      a = a / aWidth / lines;
      c = c / cWidth;
      factor = Math.max(factor, 1 - (a * a) + 1 - (c * c) - 1);

      return getColor(r * factor, g * factor, b * factor, algo.map[ry][rx]);
    }
    snowflakeAlgo.unusedFkt = function(i, rx, ry, r, g, b)
    {
      let factor = 1.0;
      // calculate the offset difference of algo.map location to the float
      // location of the object
      let offx = rx - algo.obj[i].x;
      let offy = ry - algo.obj[i].y;
      let distance = Math.sqrt(offx * offx + offy * offy);
      let angle = getAngle(i, rx, ry);
      let percent = 0;
      let handles = 6;

      // Turn each object by a random angle
      angle += algo.obj[i].random * algo.twoPi;
      // Repeat and normalize the pattern
      angle = handles * angle;
      angle = (angle + algo.twoPi) % (algo.twoPi);

      // Blind in the inner edges
      factor = Math.cos(angle) + 0.2 - (distance / (algo.presetRadius));

      //--------------------------------------
      // Draw the outer parts
//      let outerx = 0;
//      let outery = 0;
      let d = distance;
      var factorR = 0;
      // Repeat and normalize the pattern
      angle = Math.abs(angle - Math.PI);
      angle = (angle + algo.twoPi) % (algo.twoPi);

      d = distance * (1 + 0.2 * angle / Math.PI);
      let percentX = Math.abs(offx / algo.presetRadius);
      let percentY = Math.abs(offy / algo.presetRadius);
      
      factorR = Math.sin(handles / 2 * percentX * Math.PI - algo.halfPi)
      		+ Math.sin(percentY * Math.PI - algo.halfPi);
      factorR += Math.sin(percentX * Math.PI - algo.halfPi)
		+ Math.sin(handles / 2 * percentY * Math.PI - algo.halfPi);

      //--------------------------------------
//      factor = factorR;
//      factor = Math.max(factorR, factor);
      
      // Blind out the outside edges
      percent = Math.max(0, 1 - (distance / (algo.presetRadius)));
      // apply a scale factor for the percent / input in the asec function
      factor *= algo.halfPi * Math.acos(1 / (2.5 * percent + 1)) / (algo.halfPi);

      // Draw a center
      percent = Math.max(0, 1 - (distance / (0.2 * algo.presetRadius)));
      // apply a scale factor for the percent / input in the asec=acos(1/x) function
      let factorC = 0;
      factorC = algo.halfPi * Math.acos(1 / (2.5 * percent + 1)) / (algo.halfPi);
      factor = Math.max(factorC, factor * 1.5);

      return getColor(r * factor, g * factor, b * factor, algo.map[ry][rx]);
    }

    let squareAlgo = new Object;
    squareAlgo.getMapPixelColor = function(i, rx, ry, r, g, b)
    {
      let tips = 4;
      // calculate the offset difference of algo.map location to the float
      // location of the object
      let offx = Math.abs(rx - algo.obj[i].x);
      let offy = Math.abs(ry - algo.obj[i].y);
      let distance = Math.sqrt(offx * offx + offy * offy);
      let angle = getAngle(i, rx, ry);
      if (algo.presetSize >= 8) {
    	  let rotation = algo.twoPi * algo.obj[i].random;
    	  angle = (angle + rotation) % algo.twoPi;
      }
      let targetDistance = geometryCalc.getTargetDistance(angle, tips);
      let distPercent = distance / targetDistance ;
      let factor = blindoutPercent(1 - distPercent, 1.5);

      return getColor(r * factor, g * factor, b * factor, algo.map[ry][rx]);
    }

    let starAlgo = new Object;
    starAlgo.getMapPixelColor = function(i, rx, ry, r, g, b)
    {
      // calculate the offset difference of algo.map location to the float
      // location of the object
      let baseIntensity = 0.3;
      let tips = 5;
      let angle = getAngle(i, rx, ry) + Math.PI;
      let offx = Math.abs(rx - algo.obj[i].x);
      let offy = Math.abs(ry - algo.obj[i].y);
      let distance = Math.sqrt(offx * offx + offy * offy);
      
      if (algo.presetRadius < 10) {
        tips = 6;
        angle = getAngle(i, rx, ry);
        angle *= (tips / 2);
        a = Math.abs(Math.sin(angle) * distance);
        let c = Math.abs(Math.cos(angle) * distance);
        let aWidth = 1.0;
        let cWidth = algo.presetSize;
        a = a / aWidth / (tips / 2);
        c = c / cWidth;
        factor = Math.max(0, 1 - (a * a) + 1 - (c * c) - 1);
      } else {
        // Repeat and normalize the pattern
        let colorAngle = tips * angle;
        colorAngle = (colorAngle + (Math.PI)) % (algo.twoPi);

        // Calculate color pixel positions, base color
        let distPercent = distance / algo.presetRadius;
        let factor = baseIntensity + (1 - baseIntensity)
          * (Math.abs(colorAngle - Math.PI) / algo.twoPi
            + (1 - distPercent));

        let targetDistance = geometryCalc.getTargetDistance(angle, tips);
      
        let tipsAngle = (angle % (algo.twoPi / tips)) - (Math.PI / tips);
        let triangleSide = algo.presetRadius * Math.sqrt(3);
        let angleSide = Math.abs(Math.sin(tipsAngle) * targetDistance);
        let anglePercent = angleSide / triangleSide;
        let expression = 1 + algo.presetSize / 50;
        let contraTip = expression * targetDistance * anglePercent;
        distPercent = distance / (targetDistance - contraTip);
        let sharpness = 0.3 + algo.presetSize / 18;
        factor = factor * blindoutPercent(1 - distPercent, sharpness);
      }

      return getColor(r * factor, g * factor, b * factor, algo.map[ry][rx]);
    }

    let steeringwheelAlgo = new Object;
    steeringwheelAlgo.getMapPixelColor = function(i, rx, ry, r, g, b)
    {
      let factor = 1.0;
      // calculate the offset difference of algo.map location to the float
      // location of the object
      let offx = rx - algo.obj[i].x;
      let offy = ry - algo.obj[i].y;
      let distance = Math.sqrt(offx * offx + offy * offy);
      let angle = getAngle(i, rx, ry);
      let percent = 0;
      let handles = 6;
      let outer = 0.7
	  let inner = 0.5;

	  if (algo.presetSize < 10) {
		  outer = 0.5;
    	  inner = 0.0;
      }

      // Repeat and normalize the pattern
      angle = handles * angle;
      angle = (angle + algo.twoPi) % (algo.twoPi);

      // Blind in the inner edges
      factor = Math.cos(angle) + 0.2 - (distance / (algo.presetRadius));

      //--------------------------------------
      // Draw the ring
      let d = distance;
      var factorR = 0;
	  percent = Math.max(0, 1 - (d / (outer * algo.presetRadius)));
      factorR = algo.halfPi * Math.acos(1 / (2.5 * percent + 1)) / (algo.halfPi);
	  percent = Math.max(0, 1 - (d / (inner * algo.presetRadius)));
      factorR -= algo.halfPi * Math.acos(1 / (2.5 * percent + 1)) / (algo.halfPi);
      //--------------------------------------
      factor = Math.max(factorR, factor);
      
      // Blind out the outside edges
      percent = Math.max(0, 1 - (distance / (algo.presetRadius)));
      // apply a scale factor for the percent / input in the asec function
      factor *= algo.halfPi * Math.acos(1 / (2.5 * percent + 1)) / (algo.halfPi);

      // Draw a center
      percent = Math.max(0, 1 - (distance / (0.2 * algo.presetRadius)));
      // apply a scale factor for the percent / input in the asec=acos(1/x) function
      let factorC = 0;
      factorC = algo.halfPi * Math.acos(1 / (2.5 * percent + 1)) / (algo.halfPi);
      factor = Math.max(factorC, factor * 1.5);

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
      
      angle -= (distance / (algo.presetRadius)) * algo.halfPi;
      // Repeat the pattern
      angle = fanblades * angle;
      // Turn along the distance
      angle += (algo.twoPi) * (algo.progstep / 8 % 1);
      // Normalize the angle
      angle = (angle + algo.twoPi) % (algo.twoPi);

      // Blind along the edges
      factor = Math.cos(angle);

      // Blind out the outside edges
      percent = Math.max(0, 1 - (distance / (algo.presetRadius)));
      // apply a scale factor for the percent / input in the asec function
      factor *= algo.halfPi * Math.acos(1 / (2.5 * percent + 1)) / (algo.halfPi);
      
      // Draw a center
      // asec consumes values > 1. asec(x) = acos(1/x)
      percent = Math.max(0, 1 - (distance / (0.2 * algo.presetRadius)));
      // apply a scale factor for the percent / input in the asec function
      let factorC = algo.halfPi * Math.acos(1 / (2.5 * percent + 1)) / (algo.halfPi);

      factor = Math.max(factorC, factor * 1.5);

      return getColor(r * factor, g * factor, b * factor, algo.map[ry][rx]);
    }

    let treeAlgo = new Object;
    treeAlgo.getMapPixelColor = function(i, rx, ry, r, g, b)
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
        offy += (algo.presetRadius) + 0.7;
      }
      let factor = ((1 - (Math.sqrt((offx * offx * 1.8) + (offy * offy * 1.4)))) * 2.5) + offy * 3;
      factor = factor / 3.27;
 
      // add the object color to the algo.mapped location
      return getColor(r * factor, g * factor, b * factor, algo.map[ry][rx]);
    };

    let triangleAlgo = new Object;
    triangleAlgo.getMapPixelColor = function(i, rx, ry, r, g, b)
    {
      let tips = 3;
      let rotation = algo.twoPi * algo.obj[i].random;
      if (algo.presetSize < 10) {
    	  rotation = Math.PI;
      }
      // calculate the offset difference of algo.map location to the float
      // location of the object
      let offx = Math.abs(rx - algo.obj[i].x);
      let offy = Math.abs(ry - algo.obj[i].y);
      let distance = Math.sqrt(offx * offx + offy * offy);
      let angle = (getAngle(i, rx, ry) + rotation) % algo.twoPi;
      let targetDistance = geometryCalc.getTargetDistance(angle, tips);
      let distPercent = distance / targetDistance ;
      let factor = blindoutPercent(1 - distPercent, 3);

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
      percent = Math.max(0, 1 - (distance / (0.2 * algo.presetRadius)));
      // apply a scale factor for the percent / input in the asec function
      let factorC = algo.halfPi * Math.acos(1 / (2.5 * percent + 1)) / (algo.halfPi);

      // Calculate intensity by angle
      //factor = (1.0 - baseIntensity) * angle / (algo.twoPi) + baseIntensity;
      factor = Math.max(0, (1.0 + baseIntensity) * angle / (algo.twoPi) - baseIntensity);

      // Blind out the inner edges
      percent = Math.max(0, 1 - (angle / algo.twoPi));
      // apply a scale factor for the percent / input in the asec function
      factor *= (1.0 - baseIntensity) * (Math.acos(1 / (2.5 * percent + 1)) / (algo.halfPi)) + baseIntensity;

      // Blind out the outside edges
      percent = Math.max(0, 1 - (distance / (algo.presetRadius)));
      // apply a scale factor for the percent / input in the asec function
      factor *= algo.halfPi * Math.acos(1 / (2.5 * percent + 1)) / (algo.halfPi);
      
      factor = Math.max(factorC, factor * 1.5);

      return getColor(r * factor, g * factor, b * factor, algo.map[ry][rx]);
    }

	// Algorithm registration and methods ------------------------
    var shapes = new Object;
    shapes.collection = new Array(
      ["Ball", ballAlgo],
      ["Bell", bellAlgo],
      ["Diamond", diamondAlgo],
      ["Eye", eyeAlgo],
      ["Flower", flowerAlgo],
      ["Heart", heartAlgo],
      ["Hexagon", hexagonAlgo],
      ["Mask", maskAlgo],
      ["Pentagon", pentagonAlgo],
      ["Ring", ringAlgo],
      ["Snowflake", snowflakeAlgo],
      ["Snowman", snowmanAlgo],
      ["Square", squareAlgo],
      ["Star", starAlgo],
      ["Steeringwheel", steeringwheelAlgo],
      ["Tornado", tornadoAlgo],
      ["Tree", treeAlgo],
      ["Triangle", triangleAlgo],
      ["Ufo", ufoAlgo],
      ["Ventilator", ventilatorAlgo],
    );
    shapes.makeSubArray = function(_index) {
      var _array = new Array();
      for (var i = 0; i < shapes.collection.length; i++) {
        _array.push(shapes.collection[parseInt(i)][parseInt(_index)]);
      }
      return _array;
    };
    shapes.getAlgoIndex = function(_name) {
      var idx = shapes.names.indexOf(_name);
      if (idx === -1) {
        idx = (shapes.collection.length - 1);
      }
      return idx;
    };
    shapes.getAlgoEntry = function(_index) {
      if (_index < 0) {
          _index = 0;
      }
      if (_index >= shapes.collection.length) {
        _index = (shapes.collection.length - 1);
      }
      return shapes.collection[parseInt(_index)];
    };
    shapes.getAlgoName = function(_index) {
      return shapes.getAlgoEntry(_index)[0];
    };
    shapes.getAlgoObject = function(_index) {
      return shapes.getAlgoEntry(_index)[1];
    };
    shapes.names = shapes.makeSubArray(0);

    // Setters and Getters ------------------------

    algo.presetSize = 5;
    algo.properties.push("name:presetSize|type:range|display:Size|values:5,40|write:setSize|read:getSize");
    algo.presetNumber = 3;
    algo.properties.push("name:presetNumber|type:range|display:Number|values:1,50|write:setNumber|read:getNumber");
    algo.presetRandom = 1;
    algo.properties.push("name:presetRandom|type:list|display:Random Colour|values:No,Yes|write:setRandom|read:getRandom");
    algo.presetCollision = 1;
    algo.properties.push("name:presetCollision|type:list|display:Self Collision|values:No,Yes|write:setCollision|read:getCollision");

    algo.selectedAlgo = shapes.getAlgoIndex("Trees");
    algo.properties.push("name:selectedAlgo|type:list|display:Shape|"
        + "values:" + shapes.names.toString() + "|"
        + "write:setSelectedAlgo|read:getSelectedAlgo");

    algo.presetRadius = algo.presetRadius;
    algo.initialized = false;
    algo.boxRadius = 1;
    algo.realsize = 1;
    algo.progstep = 0;
    algo.totalSteps = 1024;
    // Optimize multiple calculations
    algo.twoPi = 2 * Math.PI;
    algo.halfPi = Math.PI / 2;

    var util = new Object;

    algo.setSize = function(_size)
    {
      algo.presetSize = _size;
      algo.presetRadius = _size / 2
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

	algo.setSelectedAlgo = function(_name) {
		algo.selectedAlgo = shapes.getAlgoIndex(_name);
	};
	algo.getSelectedAlgo = function() {
      return shapes.getAlgoName(algo.selectedAlgo);
	};

    // General Purpose Functions ------------------

	let geometryCalc = new Object;
	geometryCalc.cache = {
      presetRadius: 0,
    };
	geometryCalc.updateCache = function(tips)
    {
	  if (tips === 6) {
        geometryCalc.cache.innerRadius =
          0.866 * algo.presetRadius;
	  } else if (tips === 5) {
	    geometryCalc.cache.innerRadius =
          0.688 * algo.presetRadius / 0.851; 
	  } else if (tips === 4) {
        geometryCalc.cache.innerRadius =
          algo.presetRadius * 0.707;
	  } else { // tips === 3
        geometryCalc.cache.innerRadius =
          algo.presetRadius / 2;
	  }
	  geometryCalc.cache.r =
        algo.twoPi / tips / 2;
	  geometryCalc.cache.presetRadius = algo.presetRadius;
	  geometryCalc.cache.tips = tips;
    }
	geometryCalc.getTargetDistance = function(angle, tips)
    {
      if (geometryCalc.cache.presetRadius != algo.presetRadius ||
        geometryCalc.cache.tips != tips) {
        geometryCalc.updateCache(tips);
      }
      
      let anglePart = (angle + geometryCalc.cache.r) % (algo.twoPi / tips)
        - geometryCalc.cache.r;
      let targetDistance = geometryCalc.cache.innerRadius /
        Math.cos(anglePart);

      return targetDistance;
    }

    // calculate the angle from 0 to 2 pi startin north and counting clockwise
    function getAngle(i, rx, ry)
    {
        let offx = rx - algo.obj[i].x;
        let offy = ry - algo.obj[i].y;
        let angle = 0;
        // catch offx == 0
        if (offx == 0) {
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
    
    // triangle funktion: c=1; o=0; 0=0, 0.5=1, 1=0
    // try at https://www.geogebra.org/graphing
    // 1/2 (sqrt(sin^(2)((x-o) (π)/(c)))-sqrt(sin^(2)((x-o) (π)/(c)+(π)/(2)))+1)
    // compress factor is usualy 1 for the mentioned result, can be 2 for expansion
    // 2: 0 = 0; 1 = 1: 2 = 0;
//    function triangleFunction(percent, compression = 1, offset = 0)
//    {
//      let base = (percent - offset) * Math.PI / compression;
//      let sin1 = Math.sin(base);
//      let sin2 = Math.sin(base + algo.halfPi); 
//      return 0.5 * (Math.abs(sin1) - Math.abs(sin2) + 1);
//    }
    
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
    
    // Blind out towards 0 percent
    function blindoutPercent(percent, sharpness = 1)
    {
      if (percent < 0) {
        return 0;
      }
      // Normalize input
      percent = Math.min(1, percent);
      // asec consumes values > 1. asec(x) = acos(1/x)
      let factor = Math.min(1, Math.acos(1 /
        (Math.sqrt(sharpness * percent * percent) + 1)
      ) * algo.halfPi);
      return factor;
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
          // General-purpose random number per object
          random: Math.random(),
        };
        // For development align objects centered.
        algo.obj[i].x = (width /2);
        algo.obj[i].y = (height / 2);
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
      algo.boxRadius = Math.round(algo.presetRadius);
      algo.realsize = Math.floor(algo.presetRadius) * 2 + 1;

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

      let shape = shapes.getAlgoObject(algo.selectedAlgo);
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
              //algo.map[ry][rx] = getColor(0, 0, 80, algo.map[ry][rx]);

              // add the object color to the mapped location
              algo.map[ry][rx] = shape.getMapPixelColor(i, rx, ry, r, g, b);
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
              if (dish < (1.414) * (algo.presetRadius)) {
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
