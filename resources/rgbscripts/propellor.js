/*
  Q Light Controller Plus
  propellor.js

  Copyright (c) Christoph Klenge

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
(function () {
  const algo = {};
  algo.apiVersion = 2;
  algo.name = "Propellor";
  algo.author = "Christoph Klenge";
  algo.acceptColors = 2;

  algo.properties = [];
  algo.barWidth = 2;
  algo.properties.push("name:barWidth|type:range|display:Bar Width|values:1,99|write:setBarWidth|read:getBarWidth");
  algo.direction = 1;
  algo.properties.push("name:direction|type:list|display:Direction|values:Clockwise,Counterclockwise|write:setDirection|read:getDirection");
  algo.centerGap = 0;
  algo.properties.push("name:centerGap|type:range|display:Center Gap|values:0,99|write:setCenterGap|read:getCenterGap");

  algo.setBarWidth = function (barWidth) {
      algo.barWidth = barWidth;
  };
  algo.getBarWidth = function () {
      return algo.barWidth;
  };

  algo.setDirection = function (direction) {
      if (direction === "Clockwise") { algo.direction = 1; }
      else if (direction === "Counterclockwise") { algo.direction = -1; }
      else { algo.direction = 1; }
  };
  algo.getDirection = function () {
      if (algo.direction === 1) { return "Clockwise"; }
      else if (algo.direction === -1) { return "Counterclockwise"; }
      return "none";
  };

  algo.setCenterGap = function (centerGap) {
      algo.centerGap = centerGap;
  };
  algo.getCenterGap = function () {
      return algo.centerGap;
  };

  algo.rgbMap = function (width, height, rgb, step) {
      const map = new Array(height);
      const halfHeight = (height - 1) / 2;
      const halfWidth = (width - 1) / 2;
      const halfBarWidth = algo.barWidth / 2;
      const halfCenterGap = algo.centerGap / 2;
      const stepCount = algo.rgbMapStepCount(width, height);
      const preCalculatedAngle = algo.direction * step * (2 * 3.14) / stepCount;
      
      for (let y = 0; y < height; y++) {
          map[y] = new Array(width);
          for (let x = 0; x < width; x++) {
              const verticalOffset = halfHeight - y;
              const horizontalOffset = halfWidth - x;
              const distance = Math.sqrt(horizontalOffset * horizontalOffset + verticalOffset * verticalOffset);
              const normalDistance = Math.abs(horizontalOffset * Math.cos(preCalculatedAngle) + verticalOffset * Math.sin(preCalculatedAngle));
              
              if (normalDistance <= halfBarWidth && distance >= halfCenterGap)
                  map[y][x] = rgb;
              else
                  map[y][x] = 0;
          }
      }
      return map;
  };

  algo.rgbMapStepCount = function (width, height) {
      return Math.ceil(Math.max(width, height) / 4) * 8;
  };

  // Development tool access
  testAlgo = algo;

  return algo;
})();