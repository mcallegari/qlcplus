/*
  Q Light Controller Plus
  shapes.js

  Geometric shape renderer (Circle + up to 10-sided polygons) with:
  - Fill modes: Empty (outline only), Solid, Radial Gradient (inside→outside), Vertical Gradient (top→bottom)
  - Outline with configurable thickness
  - Multiple shapes on the same matrix (grid layout)
  - Size based on percentage of the matrix
  - Optional rotation (None, Left, Right) with speed

  Colors (acceptColors = 3) — recommended roles:
  1) Outline color
  2) Fill/Gradient start
  3) Fill/Gradient end

  Author: Branson Matheson <branson@sandsite.org>
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
  algo.apiVersion = 3;
  algo.name = "Shapes";
  algo.author = "Branson Matheson <branson@sandsite.org>";
  algo.acceptColors = 3;

  // --- Properties ---
  algo.properties = [];

  // Shape selection
  var SHAPE_LABELS = [
    "Circle","Triangle (3)","Square (4)","Pentagon (5)","Hexagon (6)",
    "Heptagon (7)","Octagon (8)","Nonagon (9)","Decagon (10)"
  ];
  var SHAPE_SIDES  = [0,3,4,5,6,7,8,9,10]; // 0 means circle
  algo.shapeIndex = 0; // default: Circle
  algo.properties.push("name:shape|type:list|display:Shape|values:" + SHAPE_LABELS.join(',') + "|write:setShape|read:getShape");
  algo.setShape = function(label){
    for (var i=0;i<SHAPE_LABELS.length;i++){
      if (label === SHAPE_LABELS[i]){ algo.shapeIndex = i; resetLayout(); return; }
    }
  };
  algo.getShape = function(){ return SHAPE_LABELS[algo.shapeIndex]; };

  // Fill mode
  var FILL_LABELS = ["Empty","Solid Fill","Radial Gradient","Vertical Gradient"];
  algo.fillIndex = 1; // default: Solid Fill
  algo.properties.push("name:fill|type:list|display:Fill|values:" + FILL_LABELS.join(',') + "|write:setFill|read:getFill");
  algo.setFill = function(label){
    for (var i=0;i<FILL_LABELS.length;i++){
      if (label === FILL_LABELS[i]){ algo.fillIndex = i; return; }
    }
  };
  algo.getFill = function(){ return FILL_LABELS[algo.fillIndex]; };

  // Outline size (pixels)
  algo.outline = 1;
  algo.properties.push("name:outline|type:range|display:Outline Size (px)|values:0,10|write:setOutline|read:getOutline");
  algo.setOutline = function(v){ var n = parseInt(v,10); if (isNaN(n)) n = 0; if (n<0)n=0; if (n>10)n=10; algo.outline = n; };
  algo.getOutline = function(){ return algo.outline; };

  // Number of shapes
  algo.count = 1;
  algo.properties.push("name:count|type:range|display:Shape Count|values:1,10|write:setCount|read:getCount");
  algo.setCount = function(v){ var n = parseInt(v,10); if (isNaN(n)) n = 1; if (n<1)n=1; if (n>10)n=10; algo.count = n; resetLayout(); };
  algo.getCount = function(){ return algo.count; };

  // Size percentage (relative to min dimension)
  algo.sizePct = 50;
  algo.properties.push("name:sizePct|type:range|display:Size (%)|values:5,100|write:setSizePct|read:getSizePct");
  algo.setSizePct = function(v){ var n = parseInt(v,10); if (isNaN(n)) n = 50; if (n<5)n=5; if (n>100)n=100; algo.sizePct = n; resetLayout(); };
  algo.getSizePct = function(){ return algo.sizePct; };

  // Rotation
  var ROT_LABELS = ["None","Left","Right"];
  algo.rotateIndex = 0; // None
  algo.properties.push("name:rotate|type:list|display:Rotate|values:" + ROT_LABELS.join(',') + "|write:setRotate|read:getRotate");
  algo.setRotate = function(label){ for (var i=0;i<ROT_LABELS.length;i++){ if (label === ROT_LABELS[i]){ algo.rotateIndex = i; return; } } };
  algo.getRotate = function(){ return ROT_LABELS[algo.rotateIndex]; };

  // Rotation speed (steps per frame scalar)
  algo.rotSpeed = 3;
  algo.properties.push("name:rotSpeed|type:range|display:Rotation Speed|values:0,10|write:setRotSpeed|read:getRotSpeed");
  algo.setRotSpeed = function(v){ var n = parseInt(v,10); if(isNaN(n)) n=3; if(n<0)n=0; if(n>10)n=10; algo.rotSpeed=n; };
  algo.getRotSpeed = function(){ return algo.rotSpeed; };

  // --- Colors ---
  var util = {};
  util.colors = [0xFFFFFF, 0xFF8C00, 0xFFFF00]; // outline, fill start, fill end
  algo.rgbMapSetColors = function(c){ if (c && c.length){ util.colors = c; } };
  algo.rgbMapGetColors = function(){ return util.colors; };

  // --- Layout/state ---
  util.shapes = null; // array of {cx,cy,r,angle}
  util.lastW = 0; util.lastH = 0; util.lastCount = -1; util.lastSize = -1; util.lastShape = -1;
  function resetLayout(){ util.shapes = null; }

  function ensureLayout(width, height){
    if (!util.shapes || util.lastW !== width || util.lastH !== height || util.lastCount !== algo.count || util.lastSize !== algo.sizePct || util.lastShape !== algo.shapeIndex){
      util.lastW = width; util.lastH = height; util.lastCount = algo.count; util.lastSize = algo.sizePct; util.lastShape = algo.shapeIndex;
      util.shapes = new Array(algo.count);

      // grid arrangement
      var count = algo.count;
      var aspect = (height > 0) ? (width / height) : 1.0;
      var cols = Math.ceil(Math.sqrt(count * aspect)); if (cols < 1) cols = 1;
      var rows = Math.ceil(count / cols); if (rows < 1) rows = 1;
      var cellW = width / cols;
      var cellH = height / rows;

      // base radius from size percentage
      var baseR = (algo.sizePct / 100.0) * (Math.min(width, height) * 0.5);
      var idx = 0;
      for (var r=0; r<rows; r++){
        var remaining = count - r*cols;
        var inRow = remaining < cols ? remaining : cols;
        if (inRow <= 0) break;
        var offsetX = (cols - inRow) * cellW * 0.5;
        for (var c=0; c<inRow; c++){
          var cx = offsetX + (c + 0.5) * cellW;
          var cy = (r + 0.5) * cellH;
          var maxCellR = Math.min(cellW, cellH) * 0.45;
          var rr = baseR; if (rr > maxCellR) rr = maxCellR; if (rr < 1) rr = 1;
          util.shapes[idx] = { cx: cx, cy: cy, r: rr, angle: 0.0 };
          idx++;
        }
      }
    }
  }

  // --- Helpers ---
  function clamp01(x){ return (x < 0 ? 0 : (x > 1 ? 1 : x)); }
  function lerp(a,b,t){ return a + (b - a) * t; }
  function lerpColor(c1, c2, t){
    var r1=(c1>>16)&255, g1=(c1>>8)&255, b1=c1&255;
    var r2=(c2>>16)&255, g2=(c2>>8)&255, b2=c2&255;
    var r=Math.floor(lerp(r1,r2,t)); var g=Math.floor(lerp(g1,g2,t)); var b=Math.floor(lerp(b1,b2,t));
    return (r<<16) | (g<<8) | b;
  }

  function buildRegularPolygon(cx, cy, r, sides, angle){
    var verts = new Array(sides);
    var a0 = angle - Math.PI/2; // point one vertex upward by default
    for (var i=0;i<sides;i++){
      var a = a0 + 2*Math.PI * (i / sides);
      verts[i] = [ cx + r * Math.cos(a), cy + r * Math.sin(a) ];
    }
    return verts;
  }

  function polygonBounds(verts){
    var minX=verts[0][0], maxX=verts[0][0];
    var minY=verts[0][1], maxY=verts[0][1];
    for (var i=1;i<verts.length;i++){
      var vx=verts[i][0], vy=verts[i][1];
      if (vx<minX) minX=vx; if (vx>maxX) maxX=vx; if (vy<minY) minY=vy; if (vy>maxY) maxY=vy;
    }
    return {minX:minX, maxX:maxX, minY:minY, maxY:maxY};
  }

  function pointInPolygon(px, py, verts){
    var inside = false;
    var n = verts.length;
    for (var i=0, j=n-1; i<n; j=i++){
      var xi=verts[i][0], yi=verts[i][1];
      var xj=verts[j][0], yj=verts[j][1];
      var intersect = ((yi > py) !== (yj > py)) && (px < (xj - xi) * (py - yi) / ((yj - yi) || 1e-9) + xi);
      if (intersect) inside = !inside;
    }
    return inside;
  }

  function dist2PointToSeg(px, py, ax, ay, bx, by){
    var vx = bx - ax, vy = by - ay;
    var wx = px - ax, wy = py - ay;
    var c1 = vx*wx + vy*wy;
    if (c1 <= 0) { var dx=px-ax, dy=py-ay; return dx*dx + dy*dy; }
    var c2 = vx*vx + vy*vy;
    if (c2 <= 0) { var dx2=px-ax, dy2=py-ay; return dx2*dx2 + dy2*dy2; }
    var t = c1 / c2; if (t >= 1) { var dx3=px-bx, dy3=py-by; return dx3*dx3 + dy3*dy3; }
    var projx = ax + t*vx, projy = ay + t*vy;
    var dxp = px - projx, dyp = py - projy;
    return dxp*dxp + dyp*dyp;
  }

  function minEdgeDist2(px, py, verts){
    var n = verts.length; var minD2 = 1e9;
    for (var i=0;i<n;i++){
      var j=(i+1)%n; var ax=verts[i][0], ay=verts[i][1]; var bx=verts[j][0], by=verts[j][1];
      var d2 = dist2PointToSeg(px,py,ax,ay,bx,by);
      if (d2 < minD2) minD2 = d2;
    }
    return minD2;
  }

  function getColorSafe(arr, idx, fallback){ return (arr && arr.length>idx) ? arr[idx] : fallback; }

  // --- Core render ---
  algo.rgbMap = function(width, height, _rgb, _step){
    ensureLayout(width, height);

    // Rotation update per frame
    var dir = (algo.rotateIndex === 1 ? -1 : (algo.rotateIndex === 2 ? 1 : 0));
    var dAng = (algo.rotSpeed / 10.0) * dir * 0.25 * Math.PI; // up to ~45°/frame at max speed
    if (dir !== 0 && dAng !== 0){
      for (var si=0; si<util.shapes.length; si++){
        util.shapes[si].angle += dAng;
        if (util.shapes[si].angle > 1000000) util.shapes[si].angle = 0;
        if (util.shapes[si].angle < -1000000) util.shapes[si].angle = 0;
      }
    }

    // Colors
    var outlineCol = getColorSafe(util.colors, 0, 0xFFFFFF);
    var fillA = getColorSafe(util.colors, 1, 0xFF0000);
    var fillB = getColorSafe(util.colors, 2, 0x0000FF);

    var map = new Array(height);
    for (var y=0;y<height;y++){
      var row = new Array(width);
      for (var x=0;x<width;x++) row[x] = 0x000000;
      map[y] = row;
    }

    var outlineT = algo.outline; // thickness in px
    var outlineHalf2 = (outlineT > 0) ? ( (outlineT*0.5) * (outlineT*0.5) ) : 0;

    for (var si2=0; si2<util.shapes.length; si2++){
      var S = util.shapes[si2];
      var cx = S.cx, cy = S.cy, r = S.r, ang = S.angle;
      var isCircle = (SHAPE_SIDES[algo.shapeIndex] === 0);

      var verts = null, b = null;
      if (!isCircle){
        verts = buildRegularPolygon(cx, cy, r, SHAPE_SIDES[algo.shapeIndex], ang);
        b = polygonBounds(verts);
      } else {
        b = {minX: cx - r, maxX: cx + r, minY: cy - r, maxY: cy + r};
      }

      // Clamp bounds to pixel grid
      var x0 = Math.max(0, Math.floor(b.minX - outlineT - 1));
      var x1 = Math.min(width-1, Math.ceil(b.maxX + outlineT + 1));
      var y0 = Math.max(0, Math.floor(b.minY - outlineT - 1));
      var y1 = Math.min(height-1, Math.ceil(b.maxY + outlineT + 1));

      // For vertical gradient bounds
      var vMinY = b.minY, vMaxY = b.maxY; if (vMaxY <= vMinY) vMaxY = vMinY + 1;

      // Render region
      var rr2 = r*r; // for circle tests
      var rIn2 = (r - outlineT*0.5); if (rIn2 < 0) rIn2 = 0; rIn2 = rIn2*rIn2;
      var rOut2 = (r + outlineT*0.5); rOut2 = rOut2*rOut2;

      for (var py = y0; py <= y1; py++){
        var rowRef = map[py];
        for (var px = x0; px <= x1; px++){
          var fx = px + 0.5; var fy = py + 0.5;
          var inside = false;
          var onOutline = false;

          if (isCircle){
            var dx = fx - cx, dy = fy - cy; var d2 = dx*dx + dy*dy;
            inside = (d2 <= rr2);
            if (outlineT > 0){ onOutline = (d2 >= rIn2 && d2 <= rOut2); }
          } else {
            inside = pointInPolygon(fx, fy, verts);
            if (outlineT > 0){
              var md2 = minEdgeDist2(fx, fy, verts);
              onOutline = (md2 <= outlineHalf2);
            }
          }

          // Draw fill
          if (inside && algo.fillIndex !== 0){ // not Empty
            var col = fillA;
            if (algo.fillIndex === 1){
              col = fillA; // Solid
            } else if (algo.fillIndex === 2){
              // Radial gradient (center->edge)
              var t;
              if (isCircle){
                var dx2 = fx - cx, dy2 = fy - cy; t = Math.sqrt(dx2*dx2 + dy2*dy2) / r;
              } else {
                // Approx radial by center distance
                var dx3 = fx - cx, dy3 = fy - cy; var dist = Math.sqrt(dx3*dx3 + dy3*dy3);
                t = dist / r;
              }
              if (t < 0) t = 0; if (t > 1) t = 1;
              col = lerpColor(fillA, fillB, t);
            } else if (algo.fillIndex === 3){
              // Vertical gradient (shape top->bottom)
              var t2 = (fy - vMinY) / (vMaxY - vMinY); t2 = clamp01(t2);
              col = lerpColor(fillA, fillB, t2);
            }
            rowRef[px] = col;
          }

          // Draw outline on top
          if (onOutline && outlineT > 0){
            rowRef[px] = outlineCol;
          }
        }
      }
    }

    return map;
  };

  algo.rgbMapStepCount = function(_width,_height){ return 512; };

  return algo;
})();

